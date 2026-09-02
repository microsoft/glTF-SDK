// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/ExtensionsKHR.h>
#include <GLTFSDK/Schema.h>
#include <GLTFSDK/Serialize.h>

#include "Internal/Json.h"

#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace Microsoft::glTF;

namespace
{
    using Clock = std::chrono::steady_clock;

    volatile std::size_t g_sink = 0U;

    struct Options
    {
        std::string assetDirectory = GLTFSDK_BENCHMARK_ASSET_DIR;
        std::string outputPath;
        std::size_t samples = 30U;
        std::size_t warmup = 5U;
    };

    struct Sample
    {
        std::string operation;
        std::string workload;
        std::size_t sample;
        std::size_t iterations;
        double totalMicroseconds;
        double microsecondsPerOperation;
        std::string status;
    };

    using JsonValue = Internal::JsonValue;
    using SchemaAdapter =
        valijson::adapters::NlohmannJsonAdapter;

    class SchemaDocumentProvider
    {
    public:
        const JsonValue* GetDocument(const std::string& uri)
        {
            const auto existing = m_documents.find(uri);
            if (existing != m_documents.end())
            {
                return existing->second.get();
            }

            const auto& schemas = GetDefaultSchemaUriMap();
            const auto schema = schemas.find(uri);
            if (schema == schemas.end())
            {
                throw std::runtime_error("Unable to locate schema " + uri);
            }

            auto document = std::unique_ptr<JsonValue>(
                new JsonValue(Internal::ParseJson(schema->second)));
            const JsonValue* result = document.get();
            m_documents.emplace(uri, std::move(document));
            return result;
        }

    private:
        std::unordered_map<
            std::string,
            std::unique_ptr<JsonValue>> m_documents;
    };

    valijson::Schema CompileDefaultSchema()
    {
        SchemaDocumentProvider provider;
        JsonValue root = *provider.GetDocument(SCHEMA_URI_GLTF);
        if (Internal::FindJsonMember(root, "id") == nullptr)
        {
            Internal::SetJsonMember(
                root,
                "id",
                Internal::CreateJsonString(SCHEMA_URI_GLTF));
        }

        valijson::Schema schema;
        valijson::SchemaParser parser(
            valijson::SchemaParser::kDraft4);
        parser.populateSchema(
            SchemaAdapter(root),
            schema,
            [&provider](const std::string& uri)
            {
                return provider.GetDocument(uri);
            },
            [](const JsonValue*)
            {
            });
        return schema;
    }

    Options ParseOptions(int argc, char** argv)
    {
        Options options;

        for (int i = 1; i < argc; ++i)
        {
            const std::string argument = argv[i];
            const auto requireValue = [&](const char* option) -> std::string
            {
                if (++i >= argc)
                {
                    throw std::runtime_error(std::string("Missing value for ") + option);
                }
                return argv[i];
            };

            if (argument == "--assets")
            {
                options.assetDirectory = requireValue("--assets");
            }
            else if (argument == "--output")
            {
                options.outputPath = requireValue("--output");
            }
            else if (argument == "--samples")
            {
                options.samples = static_cast<std::size_t>(
                    std::stoul(requireValue("--samples")));
            }
            else if (argument == "--warmup")
            {
                options.warmup = static_cast<std::size_t>(
                    std::stoul(requireValue("--warmup")));
            }
            else
            {
                throw std::runtime_error("Unknown argument " + argument);
            }
        }

        if (options.samples < 30U)
        {
            throw std::runtime_error("--samples must be at least 30");
        }
        if (options.outputPath.empty())
        {
            throw std::runtime_error("--output is required");
        }

        return options;
    }

    std::string JoinPath(const std::string& directory, const std::string& fileName)
    {
        if (directory.empty())
        {
            return fileName;
        }

        const char finalCharacter = directory[directory.size() - 1U];
        if (finalCharacter == '\\' || finalCharacter == '/')
        {
            return directory + fileName;
        }

#ifdef _WIN32
        return directory + "\\" + fileName;
#else
        return directory + "/" + fileName;
#endif
    }

    std::string ReadTextFile(const std::string& path)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input)
        {
            throw std::runtime_error("Unable to open " + path);
        }

        return std::string(
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());
    }

    std::string EscapeCsv(const std::string& value)
    {
        if (value.find_first_of(",\"\r\n") == std::string::npos)
        {
            return value;
        }

        std::string escaped = "\"";
        for (const char character : value)
        {
            if (character == '"')
            {
                escaped += "\"\"";
            }
            else
            {
                escaped += character;
            }
        }
        escaped += '"';
        return escaped;
    }

    void Consume(std::size_t value)
    {
        g_sink ^= value + 0x9e3779b9U + (g_sink << 6U) + (g_sink >> 2U);
    }

    void Measure(
        const std::string& operation,
        const std::string& workload,
        std::size_t iterations,
        const Options& options,
        const std::function<void()>& action,
        std::vector<Sample>& samples)
    {
        for (std::size_t warmup = 0U; warmup < options.warmup; ++warmup)
        {
            for (std::size_t iteration = 0U; iteration < iterations; ++iteration)
            {
                action();
            }
        }

        for (std::size_t sample = 0U; sample < options.samples; ++sample)
        {
            const auto start = Clock::now();
            std::string status = "ok";

            try
            {
                for (std::size_t iteration = 0U; iteration < iterations; ++iteration)
                {
                    action();
                }
            }
            catch (const std::exception& exception)
            {
                status = std::string("error: ") + exception.what();
            }

            const auto end = Clock::now();
            const double elapsed = std::chrono::duration<double, std::micro>(end - start).count();

            samples.push_back({
                operation,
                workload,
                sample,
                iterations,
                elapsed,
                elapsed / static_cast<double>(iterations),
                status
            });

            if (status != "ok")
            {
                throw std::runtime_error(
                    operation + "/" + workload + " failed: " + status);
            }
        }
    }

    std::size_t IterationsFor(const std::string& workload)
    {
        if (workload == "small")
        {
            return 200U;
        }
        if (workload == "large")
        {
            return 2U;
        }
        if (workload == "extension-heavy")
        {
            return 40U;
        }
        if (workload == "float-heavy")
        {
            return 10U;
        }
        if (workload == "malformed")
        {
            return 200U;
        }
        if (workload == "deep")
        {
            return 50U;
        }
        return 1U;
    }

    void BenchmarkRawJson(
        const std::string& workload,
        const std::string& json,
        const Options& options,
        std::vector<Sample>& samples)
    {
        const auto iterations = IterationsFor(workload);

        Measure("parse", workload, iterations, options, [&json]()
        {
            const auto document = Internal::ParseJson(json);
            Consume(
                static_cast<std::size_t>(
                    Internal::GetJsonCategory(document)) +
                json.size());
        }, samples);

        const auto document = Internal::ParseJson(json);
        Measure("write", workload, iterations, options, [&document]()
        {
            const auto output = Internal::WriteJson(document);
            Consume(output.size());
        }, samples);
    }

    void BenchmarkMalformed(
        const std::string& json,
        const Options& options,
        std::vector<Sample>& samples)
    {
        const auto iterations = IterationsFor("malformed");

        Measure("parse_reject", "malformed", iterations, options, [&json]()
        {
            try
            {
                const auto document = Internal::ParseJson(json);
                Consume(static_cast<std::size_t>(
                    Internal::GetJsonCategory(document)));
            }
            catch (const GLTFException&)
            {
                Consume(1U);
                return;
            }
            throw std::runtime_error("Malformed JSON was accepted");
        }, samples);

        Measure("deserialize_reject", "malformed", iterations, options, [&json]()
        {
            try
            {
                const auto document = Deserialize(
                    json,
                    DeserializeFlags::None,
                    SchemaFlags::DisableSchemaRoot);
                Consume(document.nodes.Size());
            }
            catch (const GLTFException&)
            {
                Consume(1U);
                return;
            }
            throw std::runtime_error("Malformed glTF JSON was accepted");
        }, samples);
    }

    void BenchmarkSchemaCompile(
        const Options& options,
        std::vector<Sample>& samples)
    {
        Measure("schema_compile", "gltf-root", 1U, options, []()
        {
            const auto schema = CompileDefaultSchema();
            Consume(reinterpret_cast<std::uintptr_t>(&schema));
        }, samples);
    }

    void BenchmarkSchemaValidation(
        const std::string& workload,
        const std::string& json,
        const Options& options,
        std::vector<Sample>& samples)
    {
        const auto schema = CompileDefaultSchema();
        const auto document = Internal::ParseJson(json);
        const auto iterations = std::max<std::size_t>(1U, IterationsFor(workload) / 4U);

        Measure("schema_validate", workload, iterations, options, [&document, &schema]()
        {
            valijson::Validator validator(
                valijson::Validator::kStrongTypes,
                valijson::Validator::kStrictDateTime);
            if (!validator.validate(
                    schema,
                    SchemaAdapter(document),
                    nullptr))
            {
                throw std::runtime_error("Schema rejected document");
            }
            Consume(1U);
        }, samples);
    }

    void BenchmarkCoreApi(
        const std::string& workload,
        const std::string& json,
        const Options& options,
        std::vector<Sample>& samples)
    {
        const auto iterations = std::max<std::size_t>(1U, IterationsFor(workload) / 4U);

        Measure("deserialize_schema_on", workload, iterations, options, [&json]()
        {
            const auto document = Deserialize(json);
            Consume(document.nodes.Size() + document.meshes.Size());
        }, samples);

        Measure("deserialize_schema_off", workload, iterations, options, [&json]()
        {
            const auto document = Deserialize(
                json,
                DeserializeFlags::None,
                SchemaFlags::DisableSchemaRoot);
            Consume(document.nodes.Size() + document.meshes.Size());
        }, samples);

        const auto document = Deserialize(
            json,
            DeserializeFlags::None,
            SchemaFlags::DisableSchemaRoot);

        Measure("public_serialize", workload, iterations, options, [&document]()
        {
            const auto output = Serialize(document);
            Consume(output.size());
        }, samples);

        Measure("end_to_end", workload, iterations, options, [&json]()
        {
            const auto documentValue = Deserialize(json);
            const auto output = Serialize(documentValue);
            Consume(output.size());
        }, samples);
    }

    void BenchmarkExtensionApi(
        const std::string& json,
        const Options& options,
        std::vector<Sample>& samples)
    {
        const auto iterations = std::max<std::size_t>(
            1U,
            IterationsFor("extension-heavy") / 4U);
        const auto deserializer = KHR::GetKHRExtensionDeserializer();
        const auto serializer = KHR::GetKHRExtensionSerializer();

        Measure("deserialize_schema_on", "extension-heavy", iterations, options,
            [&json, &deserializer]()
        {
            const auto document = Deserialize(json, deserializer);
            Consume(document.materials.Size());
        }, samples);

        Measure("deserialize_schema_off", "extension-heavy", iterations, options,
            [&json, &deserializer]()
        {
            const auto document = Deserialize(
                json,
                deserializer,
                DeserializeFlags::None,
                SchemaFlags::DisableSchemaRoot);
            Consume(document.materials.Size());
        }, samples);

        const auto document = Deserialize(
            json,
            deserializer,
            DeserializeFlags::None,
            SchemaFlags::DisableSchemaRoot);

        Measure("public_serialize", "extension-heavy", iterations, options,
            [&document, &serializer]()
        {
            const auto output = Serialize(document, serializer);
            Consume(output.size());
        }, samples);

        Measure("end_to_end", "extension-heavy", iterations, options,
            [&json, &deserializer, &serializer]()
        {
            const auto documentValue = Deserialize(json, deserializer);
            const auto output = Serialize(documentValue, serializer);
            Consume(output.size());
        }, samples);
    }

    void WriteSamples(const std::string& path, const std::vector<Sample>& samples)
    {
        std::ofstream output(path, std::ios::binary);
        if (!output)
        {
            throw std::runtime_error("Unable to create " + path);
        }

        output << "operation,workload,sample,iterations,total_us,us_per_op,status\n";
        output.precision(17);
        for (const auto& sample : samples)
        {
            output
                << EscapeCsv(sample.operation) << ','
                << EscapeCsv(sample.workload) << ','
                << sample.sample << ','
                << sample.iterations << ','
                << sample.totalMicroseconds << ','
                << sample.microsecondsPerOperation << ','
                << EscapeCsv(sample.status) << '\n';
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        const auto options = ParseOptions(argc, argv);
        const auto small = ReadTextFile(JoinPath(options.assetDirectory, "small.gltf"));
        const auto large = ReadTextFile(JoinPath(options.assetDirectory, "large.gltf"));
        const auto extensionHeavy = ReadTextFile(
            JoinPath(options.assetDirectory, "extension-heavy.gltf"));
        const auto floatHeavy = ReadTextFile(
            JoinPath(options.assetDirectory, "float-heavy.gltf"));
        const auto malformed = ReadTextFile(
            JoinPath(options.assetDirectory, "malformed.json"));
        const auto deep = ReadTextFile(JoinPath(options.assetDirectory, "deep.json"));

        std::vector<Sample> samples;
        samples.reserve(options.samples * 32U);

        BenchmarkRawJson("small", small, options, samples);
        BenchmarkRawJson("large", large, options, samples);
        BenchmarkRawJson("extension-heavy", extensionHeavy, options, samples);
        BenchmarkRawJson("float-heavy", floatHeavy, options, samples);
        BenchmarkRawJson("deep", deep, options, samples);
        BenchmarkMalformed(malformed, options, samples);

        BenchmarkSchemaCompile(options, samples);
        BenchmarkSchemaValidation("small", small, options, samples);
        BenchmarkSchemaValidation("large", large, options, samples);
        BenchmarkSchemaValidation("extension-heavy", extensionHeavy, options, samples);
        BenchmarkSchemaValidation("float-heavy", floatHeavy, options, samples);

        BenchmarkCoreApi("small", small, options, samples);
        BenchmarkCoreApi("large", large, options, samples);
        BenchmarkCoreApi("float-heavy", floatHeavy, options, samples);
        BenchmarkExtensionApi(extensionHeavy, options, samples);

        WriteSamples(options.outputPath, samples);
        std::cout
            << "Wrote " << samples.size() << " timed samples to "
            << options.outputPath << "; sink=" << g_sink << std::endl;
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Benchmark failure: " << exception.what() << std::endl;
        return 1;
    }
}
