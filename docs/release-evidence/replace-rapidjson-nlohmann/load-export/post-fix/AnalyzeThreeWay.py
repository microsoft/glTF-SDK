from __future__ import print_function

import csv
import json
import math
from collections import defaultdict
from datetime import datetime
from pathlib import Path


HERE = Path(__file__).resolve().parent
RETAINED = HERE.parent

DATASETS = {
    "retainedRapidJson": RETAINED / "load-export-rapidjson-1.9.5-raw.csv",
    "retainedPreFix": RETAINED / "load-export-nlohmann-2.0.0-raw.csv",
    "freshRapidJson": HERE / "load-export-rapidjson-1.9.5-raw.csv",
    "postFix": HERE / "load-export-nlohmann-2.0.0-raw.csv",
}

PROCESS_DATASETS = {
    "retained": RETAINED / "load-export-processes.csv",
    "fresh": HERE / "load-export-processes.csv",
}


def read_csv(path):
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream))


def percentile(values, fraction):
    ordered = sorted(values)
    return ordered[max(0, min(len(ordered) - 1, int(math.ceil(fraction * len(ordered))) - 1))]


def summarize(values):
    return {
        "samples": len(values),
        "meanUs": sum(values) / len(values),
        "medianUs": percentile(values, 0.50),
        "p95Us": percentile(values, 0.95),
    }


def delta_percent(baseline, candidate):
    return None if baseline == 0.0 else ((candidate - baseline) / baseline) * 100.0


def reduction_percent(baseline, candidate):
    return None if baseline == 0.0 else ((baseline - candidate) / baseline) * 100.0


def parse_utc(value):
    return datetime.fromisoformat(value.replace("Z", "+00:00"))


rows_by_dataset = {}
stats_by_dataset = {}
case_metadata = {}

for label, path in DATASETS.items():
    rows = read_csv(path)
    if len(rows) != 4170:
        raise RuntimeError("{} contains {} rows instead of 4170".format(path, len(rows)))
    if any(row["semantic_status"] != "ok" for row in rows):
        raise RuntimeError("{} contains a semantic failure".format(path))

    grouped = defaultdict(list)
    for row in rows:
        key = (row["case_id"], row["operation"])
        grouped[key].append(float(row["elapsed_us"]))
        case_metadata[row["case_id"]] = {
            "asset": row["asset"],
            "format": row["format"],
            "tier": row["tier"],
            "complexityClass": row["complexity_class"],
            "sampleClass": row["sample_class"],
            "extensionCoverage": row["extension_coverage"],
            "extensionsUsed": [value for value in row["extensions_used"].split(";") if value],
        }

    rows_by_dataset[label] = rows
    stats_by_dataset[label] = {
        key: summarize(values) for key, values in grouped.items()
    }


case_rows = []
for case_id in sorted(case_metadata):
    for operation in ("load", "export", "roundtrip"):
        key = (case_id, operation)
        retained_rapid = stats_by_dataset["retainedRapidJson"][key]
        retained_pre = stats_by_dataset["retainedPreFix"][key]
        fresh_rapid = stats_by_dataset["freshRapidJson"][key]
        post_fix = stats_by_dataset["postFix"][key]

        record = {
            "caseId": case_id,
            "operation": operation,
            **case_metadata[case_id],
            "retainedRapidJson": retained_rapid,
            "retainedPreFix": retained_pre,
            "freshRapidJson": fresh_rapid,
            "postFix": post_fix,
            "postVsPreFixMeanPercent": delta_percent(
                retained_pre["meanUs"], post_fix["meanUs"]
            ),
            "postVsFreshRapidMeanPercent": delta_percent(
                fresh_rapid["meanUs"], post_fix["meanUs"]
            ),
            "preFixReductionMeanPercent": reduction_percent(
                retained_pre["meanUs"], post_fix["meanUs"]
            ),
        }

        rapid_gap = retained_pre["meanUs"] - retained_rapid["meanUs"]
        record["retainedRegressionGapRecoveryMeanPercent"] = (
            None
            if rapid_gap == 0.0
            else ((retained_pre["meanUs"] - post_fix["meanUs"]) / rapid_gap) * 100.0
        )
        case_rows.append(record)


def group_members(kind):
    groups = defaultdict(set)
    for case_id, metadata in case_metadata.items():
        if kind == "overall":
            groups["all-cases"].add(case_id)
        elif kind == "complexityClass":
            groups[metadata["complexityClass"]].add(case_id)
        elif kind == "tier":
            groups[metadata["tier"]].add(case_id)
        elif kind == "extensionCoverage":
            groups[metadata["extensionCoverage"]].add(case_id)
        elif kind == "extension":
            for extension in metadata["extensionsUsed"]:
                groups[extension].add(case_id)
    return groups


aggregate_rows = []
for group_kind in (
    "overall",
    "complexityClass",
    "tier",
    "extensionCoverage",
    "extension",
):
    for group_name, case_ids in sorted(group_members(group_kind).items()):
        for operation in ("load", "export", "roundtrip"):
            aggregate = {
                "groupKind": group_kind,
                "group": group_name,
                "operation": operation,
                "caseCount": len(case_ids),
            }
            for dataset in DATASETS:
                members = [
                    stats_by_dataset[dataset][(case_id, operation)]
                    for case_id in sorted(case_ids)
                ]
                aggregate[dataset] = {
                    "meanTotalUs": sum(item["meanUs"] for item in members),
                    "medianTotalUs": sum(item["medianUs"] for item in members),
                    "p95TotalUs": sum(item["p95Us"] for item in members),
                }
            aggregate["postVsPreFixMeanPercent"] = delta_percent(
                aggregate["retainedPreFix"]["meanTotalUs"],
                aggregate["postFix"]["meanTotalUs"],
            )
            aggregate["postVsFreshRapidMeanPercent"] = delta_percent(
                aggregate["freshRapidJson"]["meanTotalUs"],
                aggregate["postFix"]["meanTotalUs"],
            )
            aggregate_rows.append(aggregate)


def process_summary(rows, implementation, cohort):
    selected = [
        row
        for row in rows
        if row["phase"] == "measured"
        and row["implementation"] == implementation
        and row["cohort"] == cohort
    ]
    peaks = [float(row["peak_working_set_bytes"]) for row in selected]
    durations = [float(row["duration_ms"]) for row in selected]
    return {
        "samples": len(selected),
        "peakWorkingSetBytes": {
            "mean": sum(peaks) / len(peaks),
            "median": percentile(peaks, 0.50),
            "maximum": max(peaks),
        },
        "processDurationMs": {
            "mean": sum(durations) / len(durations),
            "median": percentile(durations, 0.50),
            "p95": percentile(durations, 0.95),
        },
    }


retained_processes = read_csv(PROCESS_DATASETS["retained"])
fresh_processes = read_csv(PROCESS_DATASETS["fresh"])
memory = {
    "retainedRapidJson": {
        cohort: process_summary(retained_processes, "rapidjson-1.9.5", cohort)
        for cohort in ("large-enabled", "standard-only")
    },
    "retainedPreFix": {
        cohort: process_summary(retained_processes, "nlohmann-2.0.0", cohort)
        for cohort in ("large-enabled", "standard-only")
    },
    "freshRapidJson": {
        cohort: process_summary(fresh_processes, "rapidjson-1.9.5", cohort)
        for cohort in ("large-enabled", "standard-only")
    },
    "postFix": {
        cohort: process_summary(fresh_processes, "nlohmann-2.0.0", cohort)
        for cohort in ("large-enabled", "standard-only")
    },
}


def wall_time(rows):
    starts = [parse_utc(row["started_utc"]) for row in rows]
    ends = [parse_utc(row["completed_utc"]) for row in rows]
    return (max(ends) - min(starts)).total_seconds()


wall_times = {
    "retainedSeconds": wall_time(retained_processes),
    "freshSeconds": wall_time(fresh_processes),
}


def output_map(path):
    return {
        (row["implementation"], row["case_id"]): {
            "outputBytes": int(row["output_bytes"]),
            "outputFileCount": int(row["output_file_count"]),
            "outputSetSha256": row["output_set_sha256"],
        }
        for row in read_csv(path)
    }


retained_outputs = output_map(RETAINED / "load-export-output-hashes.csv")
fresh_outputs = output_map(HERE / "load-export-output-hashes.csv")
current_equivalence = json.loads(
    (HERE / "pre-post-output-equivalence.json").read_text(encoding="utf-8")
)
output_checks = {
    "retainedPreFixVsPostFixHashMatches": sum(
        retained_outputs[("nlohmann-2.0.0", case_id)]
        == fresh_outputs[("nlohmann-2.0.0", case_id)]
        for case_id in case_metadata
    ),
    "retainedRapidVsFreshRapidHashMatches": sum(
        retained_outputs[("rapidjson-1.9.5", case_id)]
        == fresh_outputs[("rapidjson-1.9.5", case_id)]
        for case_id in case_metadata
    ),
    "historicalOutputSizesMatch": all(
        retained_outputs[(implementation, case_id)]["outputBytes"]
        == fresh_outputs[(implementation, case_id)]["outputBytes"]
        for implementation in ("nlohmann-2.0.0", "rapidjson-1.9.5")
        for case_id in case_metadata
    ),
    "currentPrePostOutputComparisons": len(current_equivalence["comparisons"]),
    "currentPrePostOutputsByteIdentical": current_equivalence[
        "allOutputsByteIdentical"
    ],
}
output_checks["postFixTotalBytes"] = sum(
    fresh_outputs[("nlohmann-2.0.0", case_id)]["outputBytes"]
    for case_id in case_metadata
)
output_checks["postFixTotalFiles"] = sum(
    fresh_outputs[("nlohmann-2.0.0", case_id)]["outputFileCount"]
    for case_id in case_metadata
)


node = next(
    row
    for row in case_rows
    if row["caseId"] == "NodePerformanceTest-glb" and row["operation"] == "load"
)

summary = {
    "schemaVersion": 1,
    "method": {
        "sampleAssetCommit": "9429648735279342b4c32b8745f7904196607379",
        "warmups": 5,
        "standardSamples": 100,
        "largeSamples": 30,
        "configuration": "MSVC x64 Release",
        "candidateCommit": "dc9c9f78dc33b9df5201e28dea2eeddcb5aa7b2e",
        "rapidJsonCommit": "743b7ecd749a53e8d848db56839752a65205e915",
    },
    "nodePerformanceLoad": node,
    "cases": case_rows,
    "aggregates": aggregate_rows,
    "memory": memory,
    "wallTime": wall_times,
    "outputs": output_checks,
}


with (HERE / "load-export-three-way-summary.json").open(
    "w", encoding="utf-8", newline="\n"
) as stream:
    json.dump(summary, stream, indent=2, sort_keys=False)
    stream.write("\n")


with (HERE / "load-export-three-way-aggregates.csv").open(
    "w", encoding="utf-8", newline=""
) as stream:
    fieldnames = [
        "case_id",
        "operation",
        "tier",
        "complexity_class",
        "samples",
        "retained_rapid_mean_us",
        "retained_rapid_median_us",
        "retained_rapid_p95_us",
        "retained_pre_fix_mean_us",
        "retained_pre_fix_median_us",
        "retained_pre_fix_p95_us",
        "fresh_rapid_mean_us",
        "fresh_rapid_median_us",
        "fresh_rapid_p95_us",
        "post_fix_mean_us",
        "post_fix_median_us",
        "post_fix_p95_us",
        "post_vs_pre_fix_mean_percent",
        "post_vs_fresh_rapid_mean_percent",
        "pre_fix_reduction_mean_percent",
        "retained_regression_gap_recovery_mean_percent",
    ]
    writer = csv.DictWriter(stream, fieldnames=fieldnames, lineterminator="\n")
    writer.writeheader()
    for row in case_rows:
        writer.writerow(
            {
                "case_id": row["caseId"],
                "operation": row["operation"],
                "tier": row["tier"],
                "complexity_class": row["complexityClass"],
                "samples": row["postFix"]["samples"],
                "retained_rapid_mean_us": row["retainedRapidJson"]["meanUs"],
                "retained_rapid_median_us": row["retainedRapidJson"]["medianUs"],
                "retained_rapid_p95_us": row["retainedRapidJson"]["p95Us"],
                "retained_pre_fix_mean_us": row["retainedPreFix"]["meanUs"],
                "retained_pre_fix_median_us": row["retainedPreFix"]["medianUs"],
                "retained_pre_fix_p95_us": row["retainedPreFix"]["p95Us"],
                "fresh_rapid_mean_us": row["freshRapidJson"]["meanUs"],
                "fresh_rapid_median_us": row["freshRapidJson"]["medianUs"],
                "fresh_rapid_p95_us": row["freshRapidJson"]["p95Us"],
                "post_fix_mean_us": row["postFix"]["meanUs"],
                "post_fix_median_us": row["postFix"]["medianUs"],
                "post_fix_p95_us": row["postFix"]["p95Us"],
                "post_vs_pre_fix_mean_percent": row["postVsPreFixMeanPercent"],
                "post_vs_fresh_rapid_mean_percent": row[
                    "postVsFreshRapidMeanPercent"
                ],
                "pre_fix_reduction_mean_percent": row[
                    "preFixReductionMeanPercent"
                ],
                "retained_regression_gap_recovery_mean_percent": row[
                    "retainedRegressionGapRecoveryMeanPercent"
                ],
            }
        )


def ms_triplet(value):
    return "{:.2f} / {:.2f} / {:.2f}".format(
        value["meanUs"] / 1000.0,
        value["medianUs"] / 1000.0,
        value["p95Us"] / 1000.0,
    )


def format_percent(value):
    return "n/a" if value is None else "{:+.2f}%".format(value)


lines = [
    "# Production JSON performance optimization",
    "",
    "Generated from the retained T-38 corpus and the clean T-40 matched run.",
    "",
    "## Acceptance result",
    "",
    "| Implementation | N | LOAD mean / median / p95 |",
    "| --- | ---: | ---: |",
    "| Retained RapidJSON evidence | {} | {} ms |".format(
        node["retainedRapidJson"]["samples"], ms_triplet(node["retainedRapidJson"])
    ),
    "| Retained pre-fix Release/2.0.0 | {} | {} ms |".format(
        node["retainedPreFix"]["samples"], ms_triplet(node["retainedPreFix"])
    ),
    "| Fresh RapidJSON control | {} | {} ms |".format(
        node["freshRapidJson"]["samples"], ms_triplet(node["freshRapidJson"])
    ),
    "| Production post-fix Release/2.0.0 | {} | **{} ms** |".format(
        node["postFix"]["samples"], ms_triplet(node["postFix"])
    ),
    "",
    "Post-fix mean LOAD is {:.2f}% lower than the retained pre-fix result and "
    "recovers {:.2f}% of the retained RapidJSON regression gap. It is {} versus "
    "the fresh matched RapidJSON control.".format(
        node["preFixReductionMeanPercent"],
        node["retainedRegressionGapRecoveryMeanPercent"],
        format_percent(node["postVsFreshRapidMeanPercent"]),
    ),
    "",
    "## All 16 LOAD cases",
    "",
    "Times are mean / median / p95 in milliseconds.",
    "",
    "| Case | N | Retained pre-fix | Fresh RapidJSON | Post-fix | Post vs pre mean | Post vs fresh Rapid mean |",
    "| --- | ---: | ---: | ---: | ---: | ---: | ---: |",
]

for row in case_rows:
    if row["operation"] != "load":
        continue
    lines.append(
        "| {} | {} | {} | {} | {} | {} | {} |".format(
            row["caseId"],
            row["postFix"]["samples"],
            ms_triplet(row["retainedPreFix"]),
            ms_triplet(row["freshRapidJson"]),
            ms_triplet(row["postFix"]),
            format_percent(row["postVsPreFixMeanPercent"]),
            format_percent(row["postVsFreshRapidMeanPercent"]),
        )
    )

lines.extend(
    [
        "",
        "## Aggregate timing",
        "",
        "Summed per-case means retain equal case weighting within each group.",
        "",
        "| Group | Operation | Cases | Retained pre-fix mean total | Fresh RapidJSON mean total | Post-fix mean total | Post vs pre |",
        "| --- | --- | ---: | ---: | ---: | ---: | ---: |",
    ]
)
for row in aggregate_rows:
    if row["groupKind"] not in ("overall", "complexityClass"):
        continue
    lines.append(
        "| {}:{} | {} | {} | {:.2f} ms | {:.2f} ms | {:.2f} ms | {} |".format(
            row["groupKind"],
            row["group"],
            row["operation"],
            row["caseCount"],
            row["retainedPreFix"]["meanTotalUs"] / 1000.0,
            row["freshRapidJson"]["meanTotalUs"] / 1000.0,
            row["postFix"]["meanTotalUs"] / 1000.0,
            format_percent(row["postVsPreFixMeanPercent"]),
        )
    )

lines.extend(
    [
        "",
        "Per-tier, extension-coverage, and exact-extension mean/median/p95 "
        "aggregates are retained in `load-export-three-way-summary.json`; every "
        "case/operation triplet is in `load-export-three-way-aggregates.csv`.",
        "",
        "## Process memory, outputs, and wall time",
        "",
        "| Implementation | Cohort | N | Peak working set median / maximum | Process duration median / p95 |",
        "| --- | --- | ---: | ---: | ---: |",
    ]
)
for dataset in (
    "retainedRapidJson",
    "retainedPreFix",
    "freshRapidJson",
    "postFix",
):
    for cohort in ("large-enabled", "standard-only"):
        value = memory[dataset][cohort]
        peak = value["peakWorkingSetBytes"]
        duration = value["processDurationMs"]
        lines.append(
            "| {} | {} | {} | {:.2f} / {:.2f} MiB | {:.3f} / {:.3f} s |".format(
                dataset,
                cohort,
                value["samples"],
                peak["median"] / (1024.0 * 1024.0),
                peak["maximum"] / (1024.0 * 1024.0),
                duration["median"] / 1000.0,
                duration["p95"] / 1000.0,
            )
        )

lines.extend(
    [
        "",
        "- A same-toolchain current A/B check against exact pre-fix `20dbbc9` "
        "matched all {} export/round-trip output sets byte-for-byte: `{}`.".format(
            output_checks["currentPrePostOutputComparisons"],
            str(output_checks["currentPrePostOutputsByteIdentical"]).lower(),
        ),
        "- Historical retained-versus-fresh canonical hashes match {}/16 "
        "nlohmann cases and {}/16 RapidJSON cases; every output size matches. "
        "Only Avocado glTF changed across independent rebuilds on both branches, "
        "so it is not a parser/validator delta.".format(
            output_checks["retainedPreFixVsPostFixHashMatches"],
            output_checks["retainedRapidVsFreshRapidHashMatches"],
        ),
        "- One deterministic post-fix export set across 16 cases contains {} files and {} bytes.".format(
            output_checks["postFixTotalFiles"], output_checks["postFixTotalBytes"]
        ),
        "- Retained matched-run wall time: {:.3f} s; clean post-fix matched-run wall time: {:.3f} s ({:+.2f}%).".format(
            wall_times["retainedSeconds"],
            wall_times["freshSeconds"],
            delta_percent(wall_times["retainedSeconds"], wall_times["freshSeconds"]),
        ),
        "",
        "## Integrity",
        "",
        "- All four raw datasets contain 4,170 successful timing rows.",
        "- Five alternating warm-ups, 100 standard samples, and 30 large samples were used.",
        "- Semantic/resource/extension verification and output hashing remained outside timers.",
        "- The post-fix run uses Sample Assets `9429648735279342b4c32b8745f7904196607379`, RapidJSON `743b7ecd749a53e8d848db56839752a65205e915`, and production candidate `dc9c9f78dc33b9df5201e28dea2eeddcb5aa7b2e`.",
        "- The first pilot was stopped and deleted because unrelated local validation work overlapped it; only the subsequent idle-machine clean run is retained.",
        "",
    ]
)

with (HERE / "production-optimization.md").open(
    "w", encoding="utf-8", newline="\n"
) as stream:
    stream.write("\n".join(lines))
