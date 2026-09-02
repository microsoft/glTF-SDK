import argparse
import csv
import glob
import json
import math
import os
import statistics


METRICS = [
    "total_load_us",
    "file_open_us",
    "json_chunk_us",
    "strict_parse_us",
    "validation_dom_us",
    "schema_compile_us",
    "schema_locate_us",
    "schema_parse_us",
    "schema_populate_us",
    "validate_us",
    "typed_us",
    "buffer_read_us",
    "image_read_us",
    "accessors_us",
    "animations_us",
    "buffers_us",
    "buffer_views_us",
    "cameras_us",
    "images_us",
    "materials_us",
    "meshes_us",
    "nodes_us",
    "samplers_us",
    "scenes_us",
    "skins_us",
    "textures_us",
    "typed_other_us",
    "serializer_dom_us",
    "serializer_text_us",
    "resource_write_us",
]


def percentile95(values):
    ordered = sorted(values)
    return ordered[math.ceil(0.95 * len(ordered)) - 1]


def read_profile_rows(root):
    rows = []
    patterns = [
        "variants--*.csv",
        "controls--*.csv",
        "scaling--*.csv",
        "counts--*.csv",
        "serialization--*.csv",
    ]
    for pattern in patterns:
        for path in glob.glob(os.path.join(root, pattern)):
            group = os.path.basename(path).split("--", 1)[0]
            with open(path, newline="", encoding="utf-8") as source:
                for row in csv.DictReader(source):
                    row["group"] = group
                    rows.append(row)
    return rows


def write_raw(path, rows):
    fields = ["group"] + [
        field for field in rows[0].keys() if field != "group"
    ]
    with open(path, "w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def aggregate(rows):
    groups = {}
    for row in rows:
        key = (row["group"], row["case_id"], row["variant"])
        groups.setdefault(key, []).append(row)

    output = []
    for key, values in sorted(groups.items()):
        for metric in METRICS:
            samples = [float(row[metric]) for row in values]
            output.append(
                {
                    "group": key[0],
                    "case_id": key[1],
                    "variant": key[2],
                    "metric": metric,
                    "samples": len(samples),
                    "mean_us": statistics.mean(samples),
                    "median_us": statistics.median(samples),
                    "p95_us": percentile95(samples),
                    "minimum_us": min(samples),
                    "maximum_us": max(samples),
                }
            )
    return output


def write_aggregates(path, rows):
    fields = [
        "group",
        "case_id",
        "variant",
        "metric",
        "samples",
        "mean_us",
        "median_us",
        "p95_us",
        "minimum_us",
        "maximum_us",
    ]
    with open(path, "w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--result-root", required=True)
    parser.add_argument("--output-root", required=True)
    args = parser.parse_args()

    os.makedirs(args.output_root, exist_ok=True)
    rows = read_profile_rows(args.result_root)
    write_raw(os.path.join(args.output_root, "profile-raw.csv"), rows)
    aggregates = aggregate(rows)
    write_aggregates(
        os.path.join(args.output_root, "profile-aggregates.csv"),
        aggregates,
    )

    summary = {
        "profileRows": len(rows),
        "aggregateRows": len(aggregates),
        "groups": sorted({row["group"] for row in rows}),
        "cases": sorted({row["case_id"] for row in rows}),
        "variants": sorted({row["variant"] for row in rows}),
    }
    with open(
        os.path.join(args.output_root, "profile-summary.json"),
        "w",
        encoding="utf-8",
    ) as output:
        json.dump(summary, output, indent=2)
        output.write("\n")


if __name__ == "__main__":
    main()
