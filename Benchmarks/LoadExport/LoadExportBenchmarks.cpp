// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Constants.h>
#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/Document.h>
#include <GLTFSDK/ExtensionsKHR.h>
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
#include <unordered_set>
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
        std::vector<std::string> caseIds;
        bool warmup = false;
    };

    struct AssetCase
    {
        std::string id;
        std::string model;
        std::string format;
        std::string relativePath;
        std::vector<std::string> typedExtensions;
        std::vector<std::string> rawExtensions;
        std::vector<std::string> requiredExtensions;
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

    struct ExtensionCoverage
    {
        std::set<std::string> typed;
        std::set<std::string> raw;
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

    const ExtensionSerializer& GetExtensionSerializer()
    {
        static const ExtensionSerializer serializer = KHR::GetKHRExtensionSerializer();
        return serializer;
    }

    const ExtensionDeserializer& GetExtensionDeserializer()
    {
        static const ExtensionDeserializer deserializer = KHR::GetKHRExtensionDeserializer();
        return deserializer;
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
            loaded.document = Deserialize(manifest, GetExtensionDeserializer());
            LoadReferencedResources(resourceReader, loaded, countedExternalUris);
        }
        else if (asset.format == "glb")
        {
            loaded.sourceBytes = GetStreamLength(*sourceStream);
            GLBResourceReader resourceReader(streamReader, sourceStream);
            loaded.document = Deserialize(
                resourceReader.GetJson(),
                GetExtensionDeserializer());
            LoadReferencedResources(resourceReader, loaded, countedExternalUris);
        }
        else
        {
            throw std::runtime_error("Unsupported format " + asset.format);
        }

        return loaded;
    }

    void RecordPropertyExtensions(
        const glTFProperty& property,
        ExtensionCoverage& coverage);

    void RecordExtension(
        const Extension& extension,
        ExtensionCoverage& coverage)
    {
        using namespace KHR::Materials;
        using namespace KHR::MeshPrimitives;
        using namespace KHR::Nodes;
        using namespace KHR::TextureInfos;

        if (const auto* pbr = dynamic_cast<const PBRSpecularGlossiness*>(&extension))
        {
            coverage.typed.insert(PBRSPECULARGLOSSINESS_NAME);
            RecordPropertyExtensions(pbr->diffuseTexture, coverage);
            RecordPropertyExtensions(pbr->specularGlossinessTexture, coverage);
        }
        else if (dynamic_cast<const Unlit*>(&extension))
        {
            coverage.typed.insert(UNLIT_NAME);
        }
        else if (const auto* clearcoat = dynamic_cast<const Clearcoat*>(&extension))
        {
            coverage.typed.insert(CLEARCOAT_NAME);
            RecordPropertyExtensions(clearcoat->texture, coverage);
            RecordPropertyExtensions(clearcoat->roughnessTexture, coverage);
            RecordPropertyExtensions(clearcoat->normalTexture, coverage);
        }
        else if (const auto* volume = dynamic_cast<const Volume*>(&extension))
        {
            coverage.typed.insert(VOLUME_NAME);
            RecordPropertyExtensions(volume->thicknessTexture, coverage);
        }
        else if (const auto* iridescence = dynamic_cast<const Iridescence*>(&extension))
        {
            coverage.typed.insert(IRIDESCENCE_NAME);
            RecordPropertyExtensions(iridescence->texture, coverage);
            RecordPropertyExtensions(iridescence->thicknessTexture, coverage);
        }
        else if (const auto* transmission = dynamic_cast<const Transmission*>(&extension))
        {
            coverage.typed.insert(TRANSMISSION_NAME);
            RecordPropertyExtensions(transmission->texture, coverage);
        }
        else if (const auto* sheen = dynamic_cast<const Sheen*>(&extension))
        {
            coverage.typed.insert(SHEEN_NAME);
            RecordPropertyExtensions(sheen->colorTexture, coverage);
            RecordPropertyExtensions(sheen->roughnessTexture, coverage);
        }
        else if (const auto* specular = dynamic_cast<const Specular*>(&extension))
        {
            coverage.typed.insert(SPECULAR_NAME);
            RecordPropertyExtensions(specular->texture, coverage);
            RecordPropertyExtensions(specular->colorTexture, coverage);
        }
        else if (dynamic_cast<const DracoMeshCompression*>(&extension))
        {
            coverage.typed.insert(DRACOMESHCOMPRESSION_NAME);
        }
        else if (dynamic_cast<const MeshGPUInstancing*>(&extension))
        {
            coverage.typed.insert(MESHGPUINSTANCING_NAME);
        }
        else if (dynamic_cast<const TextureTransform*>(&extension))
        {
            coverage.typed.insert(TEXTURETRANSFORM_NAME);
        }
        else
        {
            throw std::runtime_error(
                "Unexpected registered extension type in benchmark corpus");
        }

        const auto* extensionProperty =
            dynamic_cast<const glTFProperty*>(&extension);
        if (extensionProperty)
        {
            RecordPropertyExtensions(*extensionProperty, coverage);
        }
    }

    void RecordPropertyExtensions(
        const glTFProperty& property,
        ExtensionCoverage& coverage)
    {
        for (const auto& extension : property.extensions)
        {
            coverage.raw.insert(extension.first);
        }
        for (const auto& extension : property.GetExtensions())
        {
            RecordExtension(extension.get(), coverage);
        }
    }

    template<typename T>
    void RecordContainerExtensions(
        const IndexedContainer<const T>& container,
        ExtensionCoverage& coverage)
    {
        for (const auto& value : container.Elements())
        {
            RecordPropertyExtensions(value, coverage);
        }
    }

    ExtensionCoverage GetExtensionCoverage(const Document& document)
    {
        ExtensionCoverage coverage;
        RecordPropertyExtensions(document, coverage);
        RecordPropertyExtensions(document.asset, coverage);
        RecordContainerExtensions(document.accessors, coverage);
        RecordContainerExtensions(document.buffers, coverage);
        RecordContainerExtensions(document.bufferViews, coverage);
        RecordContainerExtensions(document.images, coverage);
        RecordContainerExtensions(document.nodes, coverage);
        RecordContainerExtensions(document.samplers, coverage);
        RecordContainerExtensions(document.scenes, coverage);
        RecordContainerExtensions(document.skins, coverage);
        RecordContainerExtensions(document.textures, coverage);

        for (const auto& camera : document.cameras.Elements())
        {
            RecordPropertyExtensions(camera, coverage);
            RecordPropertyExtensions(*camera.projection, coverage);
        }
        for (const auto& material : document.materials.Elements())
        {
            RecordPropertyExtensions(material, coverage);
            RecordPropertyExtensions(material.metallicRoughness, coverage);
            RecordPropertyExtensions(
                material.metallicRoughness.baseColorTexture,
                coverage);
            RecordPropertyExtensions(
                material.metallicRoughness.metallicRoughnessTexture,
                coverage);
            RecordPropertyExtensions(material.normalTexture, coverage);
            RecordPropertyExtensions(material.occlusionTexture, coverage);
            RecordPropertyExtensions(material.emissiveTexture, coverage);
        }
        for (const auto& mesh : document.meshes.Elements())
        {
            RecordPropertyExtensions(mesh, coverage);
            for (const auto& primitive : mesh.primitives)
            {
                RecordPropertyExtensions(primitive, coverage);
            }
        }
        for (const auto& animation : document.animations.Elements())
        {
            RecordPropertyExtensions(animation, coverage);
            for (const auto& channel : animation.channels.Elements())
            {
                RecordPropertyExtensions(channel, coverage);
                RecordPropertyExtensions(channel.target, coverage);
            }
            for (const auto& sampler : animation.samplers.Elements())
            {
                RecordPropertyExtensions(sampler, coverage);
            }
        }

        return coverage;
    }

    std::set<std::string> ToSet(const std::vector<std::string>& values)
    {
        return std::set<std::string>(values.begin(), values.end());
    }

    bool SetsEqual(
        const std::unordered_set<std::string>& actual,
        const std::set<std::string>& expected)
    {
        if (actual.size() != expected.size())
        {
            return false;
        }
        for (const auto& value : expected)
        {
            if (actual.find(value) == actual.end())
            {
                return false;
            }
        }
        return true;
    }

    void ValidateExtensionCoverage(
        const Document& document,
        const AssetCase& asset)
    {
        const auto expectedTyped = ToSet(asset.typedExtensions);
        const auto expectedRaw = ToSet(asset.rawExtensions);
        auto expectedUsed = expectedTyped;
        expectedUsed.insert(expectedRaw.begin(), expectedRaw.end());
        const auto expectedRequired =
            ToSet(asset.requiredExtensions);

        if (!SetsEqual(document.extensionsUsed, expectedUsed))
        {
            throw std::runtime_error(
                "extensionsUsed differs from the corpus manifest for " +
                asset.id);
        }
        if (!SetsEqual(document.extensionsRequired, expectedRequired))
        {
            throw std::runtime_error(
                "extensionsRequired differs from the corpus manifest for " +
                asset.id);
        }

        const auto actual = GetExtensionCoverage(document);
        if (actual.typed != expectedTyped)
        {
            throw std::runtime_error(
                "SDK-typed extension coverage differs for " + asset.id);
        }
        if (actual.raw != expectedRaw)
        {
            throw std::runtime_error(
                "Raw-preserved extension coverage differs for " + asset.id);
        }
    }

    void ValidateLoadedAsset(
        const LoadedAsset& loaded,
        const AssetCase& asset)
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

        ValidateExtensionCoverage(loaded.document, asset);
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

        const auto manifest = Serialize(
            loaded.document,
            GetExtensionSerializer());
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

        const auto manifest = Serialize(
            loaded.document,
            GetExtensionSerializer());
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
        ValidateLoadedAsset(actual, asset);

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
        if (expected.document.extensionsRequired !=
            actual.document.extensionsRequired)
        {
            throw std::runtime_error(
                "Round-tripped required extensions differ from source");
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
            ValidateLoadedAsset(loaded, asset);
            result.inputBytes = loaded.sourceBytes;
        }
        else if (operation == "export")
        {
            const auto loaded = LoadAsset(options.assetRoot, asset);
            ValidateLoadedAsset(loaded, asset);
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
            ValidateLoadedAsset(loaded, asset);
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
            { "Box-gltf", "Box", "gltf", "Models/Box/glTF/Box.gltf", {}, {}, {} },
            { "Box-glb", "Box", "glb", "Models/Box/glTF-Binary/Box.glb", {}, {}, {} },
            { "Avocado-gltf", "Avocado", "gltf", "Models/Avocado/glTF/Avocado.gltf", {}, {}, {} },
            { "Avocado-glb", "Avocado", "glb", "Models/Avocado/glTF-Binary/Avocado.glb", {}, {}, {} },
            { "MorphStressTest-glb", "MorphStressTest", "glb", "Models/MorphStressTest/glTF-Binary/MorphStressTest.glb", {}, {}, {} },
            { "SpecularTest-glb", "SpecularTest", "glb", "Models/SpecularTest/glTF-Binary/SpecularTest.glb", { "KHR_materials_specular" }, {}, {} },
            { "EmissiveStrengthTest-glb", "EmissiveStrengthTest", "glb", "Models/EmissiveStrengthTest/glTF-Binary/EmissiveStrengthTest.glb", {}, { "KHR_materials_emissive_strength" }, {} },
            { "SimpleInstancing-glb", "SimpleInstancing", "glb", "Models/SimpleInstancing/glTF-Binary/SimpleInstancing.glb", { "EXT_mesh_gpu_instancing" }, {}, {} },
            { "XmpMetadataRoundedCube-glb", "XmpMetadataRoundedCube", "glb", "Models/XmpMetadataRoundedCube/glTF-Binary/XmpMetadataRoundedCube.glb", {}, { "KHR_xmp_json_ld" }, {} },
            { "TextureTransformMultiTest-glb", "TextureTransformMultiTest", "glb", "Models/TextureTransformMultiTest/glTF-Binary/TextureTransformMultiTest.glb", { "KHR_materials_clearcoat", "KHR_materials_unlit", "KHR_texture_transform" }, {}, { "KHR_texture_transform" } },
            { "IridescenceSuzanne-glb", "IridescenceSuzanne", "glb", "Models/IridescenceSuzanne/glTF-Binary/IridescenceSuzanne.glb", { "KHR_materials_iridescence", "KHR_materials_transmission", "KHR_materials_volume" }, { "KHR_lights_punctual", "KHR_materials_ior" }, { "KHR_materials_iridescence" } },
            { "SheenTestGrid-glb", "SheenTestGrid", "glb", "Models/SheenTestGrid/glTF-Binary/SheenTestGrid.glb", { "KHR_materials_sheen" }, {}, { "KHR_materials_sheen" } },
            { "MaterialsVariantsShoe-glb", "MaterialsVariantsShoe", "glb", "Models/MaterialsVariantsShoe/glTF-Binary/MaterialsVariantsShoe.glb", {}, { "KHR_materials_variants" }, {} },
            { "ABeautifulGame-glb", "ABeautifulGame", "glb", "Models/ABeautifulGame/glTF-Binary/ABeautifulGame.glb", { "KHR_materials_transmission", "KHR_materials_volume" }, {}, {} },
            { "ABeautifulGame-draco-glb", "ABeautifulGame", "glb", "Models/ABeautifulGame/glTF-Binary-KTX-ETC1S-Draco/ABeautifulGame.glb", { "KHR_draco_mesh_compression", "KHR_materials_transmission", "KHR_materials_volume" }, { "KHR_texture_basisu" }, { "KHR_draco_mesh_compression", "KHR_texture_basisu" } },
            { "NodePerformanceTest-glb", "NodePerformanceTest", "glb", "Models/NodePerformanceTest/glTF-Binary/NodePerformanceTest.glb", {}, { "KHR_lights_punctual" }, { "KHR_lights_punctual" } }
        };
    }

    std::vector<AssetCase> SelectAssetCases(
        const std::vector<AssetCase>& allAssets,
        const std::vector<std::string>& requestedIds)
    {
        if (requestedIds.empty())
        {
            return allAssets;
        }

        const std::set<std::string> requested(
            requestedIds.begin(),
            requestedIds.end());
        if (requested.size() != requestedIds.size())
        {
            throw std::runtime_error("Duplicate --case value");
        }

        std::vector<AssetCase> selected;
        for (const auto& asset : allAssets)
        {
            if (requested.find(asset.id) != requested.end())
            {
                selected.push_back(asset);
            }
        }
        if (selected.size() != requested.size())
        {
            throw std::runtime_error("Unknown --case value");
        }
        return selected;
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
            else if (argument == "--case")
            {
                options.caseIds.push_back(requireValue("--case"));
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
        const auto assets = SelectAssetCases(
            GetAssetCases(),
            options.caseIds);
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
