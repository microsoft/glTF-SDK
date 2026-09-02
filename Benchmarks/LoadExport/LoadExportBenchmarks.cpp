// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Constants.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Document.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/IStreamReader.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/ResourceWriter.h>
#include <GLTFSDK/Serialize.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace Microsoft::glTF;

namespace
{
    using Clock = std::chrono::steady_clock;

    volatile std::size_t g_sink = 0U;

    struct Options
    {
        std::string assetRoot;
        std::string outputRoot;
        std::string csvPath;
        std::string implementation;
        std::string sourceCommit;
        std::size_t sample = 0U;
        std::uint32_t orderSeed = 0U;
        bool warmup = false;
    };

    struct AssetCase
    {
        std::string id;
        std::string model;
        std::string format;
        std::string relativePath;
    };

    struct LoadedAsset
    {
        Document document;
        std::vector<std::vector<std::uint8_t>> buffers;
        std::vector<std::vector<std::uint8_t>> images;
        std::uint64_t sourceBytes = 0U;
    };

    struct Result
    {
        std::string caseId;
        std::string model;
        std::string format;
        std::string operation;
        double elapsedMicroseconds = 0.0;
        std::uint64_t inputBytes = 0U;
        std::string outputDirectory;
    };

    struct WorkItem
    {
        std::size_t assetIndex;
        std::string operation;
    };

    std::string JoinPath(const std::string& directory, const std::string& child)
    {
        if (directory.empty())
        {
            return child;
        }

        const char last = directory[directory.size() - 1U];
        if (last == '\\' || last == '/')
        {
            return directory + child;
        }

#ifdef _WIN32
        return directory + "\\" + child;
#else
        return directory + "/" + child;
#endif
    }

    std::string GetDirectoryName(const std::string& path)
    {
        const auto separator = path.find_last_of("\\/");
        return separator == std::string::npos ? std::string() : path.substr(0U, separator);
    }

    std::string GetFileName(const std::string& path)
    {
        const auto separator = path.find_last_of("\\/");
        return separator == std::string::npos ? path : path.substr(separator + 1U);
    }

    bool IsDataUri(const std::string& uri)
    {
        return uri.size() >= 5U && uri.compare(0U, 5U, "data:") == 0;
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
            escaped += character == '"' ? "\"\"" : std::string(1U, character);
        }
        escaped += '"';
        return escaped;
    }

    void Consume(std::size_t value)
    {
        g_sink ^= value + 0x9e3779b9U + (g_sink << 6U) + (g_sink >> 2U);
    }

    class FileStreamReader final : public IStreamReader
    {
    public:
        explicit FileStreamReader(std::string baseDirectory)
            : m_baseDirectory(std::move(baseDirectory))
        {
        }

        std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
        {
            const auto path = JoinPath(m_baseDirectory, filename);
            auto stream = std::make_shared<std::ifstream>(path, std::ios::binary);
            if (!stream || !(*stream))
            {
                throw std::runtime_error("Unable to open input file " + path);
            }
            return stream;
        }

    private:
        std::string m_baseDirectory;
    };

    class FileStreamWriter final : public IStreamWriter
    {
    public:
        explicit FileStreamWriter(std::string baseDirectory)
            : m_baseDirectory(std::move(baseDirectory))
        {
        }

        std::shared_ptr<std::ostream> GetOutputStream(const std::string& filename) const override
        {
            const auto path = JoinPath(m_baseDirectory, filename);
            auto stream = std::make_shared<std::ofstream>(
                path,
                std::ios::binary | std::ios::trunc);
            if (!stream || !(*stream))
            {
                throw std::runtime_error("Unable to open output file " + path);
            }
            return stream;
        }

    private:
        std::string m_baseDirectory;
    };

    std::string ReadText(std::istream& stream)
    {
        std::ostringstream output;
        output << stream.rdbuf();
        if (stream.bad())
        {
            throw std::runtime_error("Unable to read glTF manifest");
        }
        return output.str();
    }

    std::uint64_t GetStreamLength(std::istream& stream)
    {
        stream.clear();
        const auto current = stream.tellg();
        stream.seekg(0, std::ios::end);
        const auto end = stream.tellg();
        if (end < 0)
        {
            throw std::runtime_error("Unable to determine input stream length");
        }
        stream.seekg(current);
        if (!stream)
        {
            throw std::runtime_error("Unable to restore input stream position");
        }
        return static_cast<std::uint64_t>(end);
    }

    template<typename TReader>
    std::vector<std::uint8_t> ReadWholeBuffer(
        const TReader& reader,
        const Document& document,
        const Buffer& buffer)
    {
        BufferView wholeBuffer;
        wholeBuffer.bufferId = buffer.id;
        wholeBuffer.byteOffset = 0U;
        wholeBuffer.byteLength = buffer.byteLength;
        return reader.template ReadBinaryData<std::uint8_t>(document, wholeBuffer);
    }

    template<typename TReader>
    void LoadReferencedResources(
        const TReader& reader,
        LoadedAsset& loaded,
        std::set<std::string>& countedExternalUris)
    {
        loaded.buffers.reserve(loaded.document.buffers.Size());
        for (const auto& buffer : loaded.document.buffers.Elements())
        {
            auto data = ReadWholeBuffer(reader, loaded.document, buffer);
            if (!buffer.uri.empty() && !IsDataUri(buffer.uri) &&
                countedExternalUris.insert(buffer.uri).second)
            {
                loaded.sourceBytes += data.size();
            }
            loaded.buffers.push_back(std::move(data));
        }

        loaded.images.reserve(loaded.document.images.Size());
        for (const auto& image : loaded.document.images.Elements())
        {
            auto data = reader.ReadBinaryData(loaded.document, image);
            if (!image.uri.empty() && !IsDataUri(image.uri) &&
                countedExternalUris.insert(image.uri).second)
            {
                loaded.sourceBytes += data.size();
            }
            loaded.images.push_back(std::move(data));
        }
    }

    LoadedAsset LoadAsset(const std::string& assetRoot, const AssetCase& asset)
    {
        const auto sourceDirectory = JoinPath(assetRoot, GetDirectoryName(asset.relativePath));
        const auto sourceFileName = GetFileName(asset.relativePath);
        auto streamReader = std::make_shared<FileStreamReader>(sourceDirectory);
        auto sourceStream = streamReader->GetInputStream(sourceFileName);

        LoadedAsset loaded;
        std::set<std::string> countedExternalUris;

        if (asset.format == "gltf")
        {
            const auto manifest = ReadText(*sourceStream);
            loaded.sourceBytes = manifest.size();
            GLTFResourceReader resourceReader(streamReader);
            loaded.document = Deserialize(manifest);
            LoadReferencedResources(resourceReader, loaded, countedExternalUris);
        }
        else if (asset.format == "glb")
        {
            loaded.sourceBytes = GetStreamLength(*sourceStream);
            GLBResourceReader resourceReader(streamReader, sourceStream);
            loaded.document = Deserialize(resourceReader.GetJson());
            LoadReferencedResources(resourceReader, loaded, countedExternalUris);
        }
        else
        {
            throw std::runtime_error("Unsupported format " + asset.format);
        }

        return loaded;
    }

    void ValidateLoadedAsset(const LoadedAsset& loaded)
    {
        if (loaded.buffers.size() != loaded.document.buffers.Size())
        {
            throw std::runtime_error("Loaded buffer count does not match Document");
        }
        if (loaded.images.size() != loaded.document.images.Size())
        {
            throw std::runtime_error("Loaded image count does not match Document");
        }

        for (std::size_t i = 0U; i < loaded.buffers.size(); ++i)
        {
            if (loaded.buffers[i].size() != loaded.document.buffers[i].byteLength)
            {
                throw std::runtime_error("Loaded buffer byte length does not match Document");
            }
        }

        Consume(
            loaded.document.accessors.Size() +
            loaded.document.meshes.Size() +
            loaded.document.nodes.Size() +
            loaded.buffers.size() +
            loaded.images.size());
    }

    void WriteExternalResource(
        ResourceWriter& writer,
        const std::string& uri,
        const std::vector<std::uint8_t>& data,
        std::map<std::string, const std::vector<std::uint8_t>*>& written)
    {
        if (uri.empty() || IsDataUri(uri))
        {
            return;
        }

        const auto existing = written.find(uri);
        if (existing != written.end())
        {
            if (*(existing->second) != data)
            {
                throw std::runtime_error("Conflicting resources use URI " + uri);
            }
            return;
        }

        writer.WriteExternal(uri, data);
        written.emplace(uri, &data);
    }

    void ExportGLTF(
        const LoadedAsset& loaded,
        const std::string& outputDirectory,
        const std::string& outputFileName)
    {
        std::shared_ptr<const IStreamWriter> streamWriter =
            std::make_shared<FileStreamWriter>(outputDirectory);
        GLTFResourceWriter writer(streamWriter);
        std::map<std::string, const std::vector<std::uint8_t>*> written;

        for (std::size_t i = 0U; i < loaded.buffers.size(); ++i)
        {
            WriteExternalResource(
                writer,
                loaded.document.buffers[i].uri,
                loaded.buffers[i],
                written);
        }
        for (std::size_t i = 0U; i < loaded.images.size(); ++i)
        {
            WriteExternalResource(
                writer,
                loaded.document.images[i].uri,
                loaded.images[i],
                written);
        }

        const auto manifest = Serialize(loaded.document);
        writer.WriteExternal(outputFileName, manifest);
    }

    void ExportGLB(
        const LoadedAsset& loaded,
        const std::string& outputDirectory,
        const std::string& outputFileName)
    {
        std::shared_ptr<const IStreamWriter> streamWriter =
            std::make_shared<FileStreamWriter>(outputDirectory);
        GLBResourceWriter writer(streamWriter);
        std::map<std::string, const std::vector<std::uint8_t>*> written;
        bool wroteBinaryChunk = false;

        for (std::size_t i = 0U; i < loaded.buffers.size(); ++i)
        {
            const auto& buffer = loaded.document.buffers[i];
            const auto& data = loaded.buffers[i];
            if (buffer.uri.empty() || buffer.uri == EMPTY_URI)
            {
                if (wroteBinaryChunk)
                {
                    throw std::runtime_error("GLB contains more than one binary-chunk buffer");
                }

                BufferView wholeBuffer;
                wholeBuffer.bufferId = GLB_BUFFER_ID;
                wholeBuffer.byteOffset = 0U;
                wholeBuffer.byteLength = data.size();
                writer.Write(wholeBuffer, data);
                wroteBinaryChunk = true;
            }
            else
            {
                WriteExternalResource(writer, buffer.uri, data, written);
            }
        }

        for (std::size_t i = 0U; i < loaded.images.size(); ++i)
        {
            WriteExternalResource(
                writer,
                loaded.document.images[i].uri,
                loaded.images[i],
                written);
        }

        const auto manifest = Serialize(loaded.document);
        writer.Flush(manifest, outputFileName);
    }

    void ExportAsset(
        const LoadedAsset& loaded,
        const AssetCase& asset,
        const std::string& outputDirectory)
    {
        const auto outputFileName = GetFileName(asset.relativePath);
        if (asset.format == "gltf")
        {
            ExportGLTF(loaded, outputDirectory, outputFileName);
        }
        else
        {
            ExportGLB(loaded, outputDirectory, outputFileName);
        }
    }

    void ValidateRoundTrip(
        const LoadedAsset& expected,
        const AssetCase& asset,
        const std::string& outputDirectory)
    {
        AssetCase outputAsset = asset;
        outputAsset.relativePath = GetFileName(asset.relativePath);
        const auto actual = LoadAsset(outputDirectory, outputAsset);
        ValidateLoadedAsset(actual);

        if (!(expected.document == actual.document))
        {
            throw std::runtime_error("Round-tripped Document differs from source");
        }
        if (expected.buffers != actual.buffers)
        {
            throw std::runtime_error("Round-tripped buffer bytes differ from source");
        }
        if (expected.images != actual.images)
        {
            throw std::runtime_error("Round-tripped image bytes differ from source");
        }
    }

    double MeasureMicroseconds(const std::function<void()>& action)
    {
        const auto start = Clock::now();
        action();
        const auto end = Clock::now();
        return std::chrono::duration<double, std::micro>(end - start).count();
    }

    Result RunWorkItem(
        const Options& options,
        const AssetCase& asset,
        const std::string& operation)
    {
        Result result;
        result.caseId = asset.id;
        result.model = asset.model;
        result.format = asset.format;
        result.operation = operation;

        if (operation == "load")
        {
            LoadedAsset loaded;
            result.elapsedMicroseconds = MeasureMicroseconds([&]()
            {
                loaded = LoadAsset(options.assetRoot, asset);
            });
            ValidateLoadedAsset(loaded);
            result.inputBytes = loaded.sourceBytes;
        }
        else if (operation == "export")
        {
            const auto loaded = LoadAsset(options.assetRoot, asset);
            ValidateLoadedAsset(loaded);
            result.inputBytes = loaded.sourceBytes;
            result.outputDirectory = JoinPath(options.outputRoot, asset.id + "-export");
            result.elapsedMicroseconds = MeasureMicroseconds([&]()
            {
                ExportAsset(loaded, asset, result.outputDirectory);
            });
            ValidateRoundTrip(loaded, asset, result.outputDirectory);
        }
        else if (operation == "roundtrip")
        {
            LoadedAsset loaded;
            result.outputDirectory = JoinPath(options.outputRoot, asset.id + "-roundtrip");
            result.elapsedMicroseconds = MeasureMicroseconds([&]()
            {
                loaded = LoadAsset(options.assetRoot, asset);
                ExportAsset(loaded, asset, result.outputDirectory);
            });
            ValidateLoadedAsset(loaded);
            ValidateRoundTrip(loaded, asset, result.outputDirectory);
            result.inputBytes = loaded.sourceBytes;
        }
        else
        {
            throw std::runtime_error("Unsupported operation " + operation);
        }

        return result;
    }

    std::vector<AssetCase> GetAssetCases()
    {
        return {
            { "Box-gltf", "Box", "gltf", "Models/Box/glTF/Box.gltf" },
            { "Box-glb", "Box", "glb", "Models/Box/glTF-Binary/Box.glb" },
            { "Avocado-gltf", "Avocado", "gltf", "Models/Avocado/glTF/Avocado.gltf" },
            { "Avocado-glb", "Avocado", "glb", "Models/Avocado/glTF-Binary/Avocado.glb" }
        };
    }

    Options ParseOptions(int argc, char** argv)
    {
        Options options;
        for (int i = 1; i < argc; ++i)
        {
            const std::string argument = argv[i];
            const auto requireValue = [&](const char* name) -> std::string
            {
                if (++i >= argc)
                {
                    throw std::runtime_error(std::string("Missing value for ") + name);
                }
                return argv[i];
            };

            if (argument == "--assets")
            {
                options.assetRoot = requireValue("--assets");
            }
            else if (argument == "--output-root")
            {
                options.outputRoot = requireValue("--output-root");
            }
            else if (argument == "--csv")
            {
                options.csvPath = requireValue("--csv");
            }
            else if (argument == "--implementation")
            {
                options.implementation = requireValue("--implementation");
            }
            else if (argument == "--source-commit")
            {
                options.sourceCommit = requireValue("--source-commit");
            }
            else if (argument == "--sample")
            {
                options.sample = static_cast<std::size_t>(
                    std::stoull(requireValue("--sample")));
            }
            else if (argument == "--order-seed")
            {
                options.orderSeed = static_cast<std::uint32_t>(
                    std::stoul(requireValue("--order-seed")));
            }
            else if (argument == "--warmup")
            {
                options.warmup = true;
            }
            else
            {
                throw std::runtime_error("Unknown argument " + argument);
            }
        }

        if (options.assetRoot.empty() || options.outputRoot.empty() ||
            options.implementation.empty() || options.sourceCommit.empty())
        {
            throw std::runtime_error(
                "--assets, --output-root, --implementation, and --source-commit are required");
        }
        if (!options.warmup && options.csvPath.empty())
        {
            throw std::runtime_error("--csv is required for a measured run");
        }

        return options;
    }

    void WriteResults(
        const Options& options,
        const std::vector<Result>& results)
    {
        std::ofstream output(options.csvPath, std::ios::binary | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("Unable to create result CSV " + options.csvPath);
        }

        output
            << "implementation,source_commit,case_id,asset,format,operation,"
            << "sample,order_seed,elapsed_us,input_bytes,output_directory,semantic_status\n";
        output.precision(17);
        for (const auto& result : results)
        {
            output
                << EscapeCsv(options.implementation) << ','
                << EscapeCsv(options.sourceCommit) << ','
                << EscapeCsv(result.caseId) << ','
                << EscapeCsv(result.model) << ','
                << EscapeCsv(result.format) << ','
                << EscapeCsv(result.operation) << ','
                << options.sample << ','
                << options.orderSeed << ','
                << result.elapsedMicroseconds << ','
                << result.inputBytes << ','
                << EscapeCsv(result.outputDirectory) << ','
                << "ok\n";
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        const auto options = ParseOptions(argc, argv);
        const auto assets = GetAssetCases();
        const std::vector<std::string> operations = { "load", "export", "roundtrip" };

        std::vector<WorkItem> workItems;
        for (std::size_t assetIndex = 0U; assetIndex < assets.size(); ++assetIndex)
        {
            for (const auto& operation : operations)
            {
                workItems.push_back({ assetIndex, operation });
            }
        }

        std::mt19937 random(options.orderSeed);
        std::shuffle(workItems.begin(), workItems.end(), random);

        std::vector<Result> results;
        results.reserve(workItems.size());
        for (const auto& workItem : workItems)
        {
            results.push_back(
                RunWorkItem(options, assets[workItem.assetIndex], workItem.operation));
        }

        if (!options.warmup)
        {
            WriteResults(options, results);
        }

        std::cout
            << (options.warmup ? "Warm-up" : "Measured")
            << " cycle completed for " << options.implementation
            << "; sink=" << g_sink << std::endl;
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Load/export benchmark failure: " << exception.what() << std::endl;
        return 1;
    }
}
