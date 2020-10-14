// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>

#include <fstream>
#include <sstream>
#include <iostream>

#include <cassert>
#include <cstdlib>

//If c++17 is supported
#if ( __cplusplus >= 201703L )
    #warning "--- 1 ---"
    #include <filesystem>
    namespace FILE_SYSTEM = std::filesystem;
    using namespace FILE_SYSTEM;
#else
    #warning "--- 2 ---"
    #include <experimental/filesystem>
    namespace FILE_SYSTEM = std::experimental::filesystem;
    using namespace FILE_SYSTEM;
#endif

using namespace Microsoft::glTF;

namespace
{
    // The glTF SDK is decoupled from all file I/O by the IStreamReader (and IStreamWriter)
    // interface(s) and the C++ stream-based I/O library. This allows the glTF SDK to be used in
    // sandboxed environments, such as WebAssembly modules and UWP apps, where any file I/O code
    // must be platform or use-case specific.
    class StreamReader : public IStreamReader
    {
    public:
        StreamReader(FILE_SYSTEM::path pathBase) : m_pathBase(std::move(pathBase))
        {
            assert(m_pathBase.has_root_path());
        }

        // Resolves the relative URIs of any external resources declared in the glTF manifest
        std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
        {
            // In order to construct a valid stream:
            // 1. The filename argument will be encoded as UTF-8 so use filesystem::u8path to
            //    correctly construct a path instance.
            // 2. Generate an absolute path by concatenating m_pathBase with the specified filename
            //    path. The filesystem::operator/ uses the platform's preferred directory separator
            //    if appropriate.
            // 3. Always open the file stream in binary mode. The glTF SDK will handle any text
            //    encoding issues for us.
            auto streamPath = m_pathBase / FILE_SYSTEM::u8path(filename);
            auto stream = std::make_shared<std::ifstream>(streamPath, std::ios_base::binary);

            // Check if the stream has no errors and is ready for I/O operations
            if (!stream || !(*stream))
            {
                throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
            }

            return stream;
        }

    private:
        FILE_SYSTEM::path m_pathBase;
    };

    // Uses the Document class to print some basic information about various top-level glTF entities
    void PrintDocumentInfo(const Document& document)
    {
        // Asset Info
        std::cout << "Asset Version:    " << document.asset.version << "\n";
        std::cout << "Asset MinVersion: " << document.asset.minVersion << "\n";
        std::cout << "Asset Generator:  " << document.asset.generator << "\n";
        std::cout << "Asset Copyright:  " << document.asset.copyright << "\n\n";

        // Scene Info
        std::cout << "Scene Count: " << document.scenes.Size() << "\n";

        if (document.scenes.Size() > 0U)
        {
            std::cout << "Default Scene Index: " << document.GetDefaultScene().id << "\n\n";
        }
        else
        {
            std::cout << "\n";
        }

        // Entity Info
        std::cout << "Node Count:     " << document.nodes.Size() << "\n";
        std::cout << "Camera Count:   " << document.cameras.Size() << "\n";
        std::cout << "Material Count: " << document.materials.Size() << "\n\n";

        // Mesh Info
        std::cout << "Mesh Count: " << document.meshes.Size() << "\n";
        std::cout << "Skin Count: " << document.skins.Size() << "\n\n";

        // Texture Info
        std::cout << "Image Count:   " << document.images.Size() << "\n";
        std::cout << "Texture Count: " << document.textures.Size() << "\n";
        std::cout << "Sampler Count: " << document.samplers.Size() << "\n\n";

        // Buffer Info
        std::cout << "Buffer Count:     " << document.buffers.Size() << "\n";
        std::cout << "BufferView Count: " << document.bufferViews.Size() << "\n";
        std::cout << "Accessor Count:   " << document.accessors.Size() << "\n\n";

        // Animation Info
        std::cout << "Animation Count: " << document.animations.Size() << "\n\n";

        for (const auto& extension : document.extensionsUsed)
        {
            std::cout << "Extension Used: " << extension << "\n";
        }

        if (!document.extensionsUsed.empty())
        {
            std::cout << "\n";
        }

        for (const auto& extension : document.extensionsRequired)
        {
            std::cout << "Extension Required: " << extension << "\n";
        }

        if (!document.extensionsRequired.empty())
        {
            std::cout << "\n";
        }
    }

    // Uses the Document and GLTFResourceReader classes to print information about various glTF binary resources
    void PrintResourceInfo(const Document& document, const GLTFResourceReader& resourceReader)
    {
        // Use the resource reader to get each mesh primitive's position data
        for (const auto& mesh : document.meshes.Elements())
        {
            std::cout << "Mesh: " << mesh.id << "\n";

            for (const auto& meshPrimitive : mesh.primitives)
            {
                std::string accessorId;

                if (meshPrimitive.TryGetAttributeAccessorId(ACCESSOR_POSITION, accessorId))
                {
                    const Accessor& accessor = document.accessors.Get(accessorId);

                    const auto data = resourceReader.ReadBinaryData<float>(document, accessor);
                    const auto dataByteLength = data.size() * sizeof(float);

                    std::cout << "MeshPrimitive: " << dataByteLength << " bytes of position data\n";
                }
            }

            std::cout << "\n";
        }

        // Use the resource reader to get each image's data
        for (const auto& image : document.images.Elements())
        {
            std::string filename;

            if (image.uri.empty())
            {
                assert(!image.bufferViewId.empty());

                auto& bufferView = document.bufferViews.Get(image.bufferViewId);
                auto& buffer = document.buffers.Get(bufferView.bufferId);

                filename += buffer.uri; //NOTE: buffer uri is empty if image is stored in GLB binary chunk
            }
            else if (IsUriBase64(image.uri))
            {
                filename = "Data URI";
            }
            else
            {
                filename = image.uri;
            }

            auto data = resourceReader.ReadBinaryData(document, image);

            std::cout << "Image: " << image.id << "\n";
            std::cout << "Image: " << data.size() << " bytes of image data\n";

            if (!filename.empty())
            {
                std::cout << "Image filename: " << filename << "\n\n";
            }
        }
    }

    void PrintInfo(const FILE_SYSTEM::path& path)
    {
        // Pass the absolute path, without the filename, to the stream reader
        auto streamReader = std::make_unique<StreamReader>(path.parent_path());

        FILE_SYSTEM::path pathFile = path.filename();
        FILE_SYSTEM::path pathFileExt = pathFile.extension();

        std::string manifest;

        auto MakePathExt = [](const std::string& ext)
        {
            return "." + ext;
        };

        std::unique_ptr<GLTFResourceReader> resourceReader;

        // If the file has a '.gltf' extension then create a GLTFResourceReader
        if (pathFileExt == MakePathExt(GLTF_EXTENSION))
        {
            auto gltfStream = streamReader->GetInputStream(pathFile.u8string()); // Pass a UTF-8 encoded filename to GetInputString
            auto gltfResourceReader = std::make_unique<GLTFResourceReader>(std::move(streamReader));

            std::stringstream manifestStream;

            // Read the contents of the glTF file into a string using a std::stringstream
            manifestStream << gltfStream->rdbuf();
            manifest = manifestStream.str();

            resourceReader = std::move(gltfResourceReader);
        }

        // If the file has a '.glb' extension then create a GLBResourceReader. This class derives
        // from GLTFResourceReader and adds support for reading manifests from a GLB container's
        // JSON chunk and resource data from the binary chunk.
        if (pathFileExt == MakePathExt(GLB_EXTENSION))
        {
            auto glbStream = streamReader->GetInputStream(pathFile.u8string()); // Pass a UTF-8 encoded filename to GetInputString
            auto glbResourceReader = std::make_unique<GLBResourceReader>(std::move(streamReader), std::move(glbStream));

            manifest = glbResourceReader->GetJson(); // Get the manifest from the JSON chunk

            resourceReader = std::move(glbResourceReader);
        }

        if (!resourceReader)
        {
            throw std::runtime_error("Command line argument path filename extension must be .gltf or .glb");
        }

        Document document;

        try
        {
            document = Deserialize(manifest);
        }
        catch (const GLTFException& ex)
        {
            std::stringstream ss;

            ss << "Microsoft::glTF::Deserialize failed: ";
            ss << ex.what();

            throw std::runtime_error(ss.str());
        }

        std::cout << "### glTF Info - " << pathFile << " ###\n\n";

        PrintDocumentInfo(document);
        PrintResourceInfo(document, *resourceReader);
    }
}

#if defined _WIN32 && defined _UNICODE
int wmain(int argc, wchar_t* argv[])
#else // _WIN32 & _UNICODE
int main(int argc, char* argv[])
#endif
{
    try
    {
        if (argc != 2U)
        {
            throw std::runtime_error("Unexpected number of command line arguments");
        }

        FILE_SYSTEM::path path = argv[1U];

        if (path.is_relative())
        {
            auto pathCurrent = FILE_SYSTEM::current_path();

            // Convert the relative path into an absolute path by appending the command line argument to the current path
            pathCurrent /= path;
            pathCurrent.swap(path);
        }

        if (!path.has_filename())
        {
            throw std::runtime_error("Command line argument path has no filename");
        }

        if (!path.has_extension())
        {
            throw std::runtime_error("Command line argument path has no filename extension");
        }

        PrintInfo(path);
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error! - ";
        std::cerr << ex.what() << "\n";

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
