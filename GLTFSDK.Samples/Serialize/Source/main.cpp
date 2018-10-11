// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Serialize.h>

// Replace this with <filesystem> (and use std::filesystem rather than
// std::experimental::filesystem) if your toolchain fully supports C++17
#include <experimental/filesystem>

#include <fstream>
#include <iostream>
#include <ostream>

#include <cassert>
#include <cstdlib>

using namespace Microsoft::glTF;

namespace
{
    // The glTF SDK is decoupled from all file I/O by the IStreamReader (and IStreamWriter)
    // interface(s) and the C++ stream-based I/O library. This allows the glTF SDK to be used in
    // sandboxed environments, such as WebAssembly modules and UWP apps, where any file I/O code
    // must be platform or use-case specific.
    class StreamWriter : public IStreamWriter
    {
    public:
        StreamWriter(std::experimental::filesystem::path pathBase) : m_pathBase(std::move(pathBase))
        {
            assert(m_pathBase.has_root_path());
        }

        // Resolves the relative URIs of any external resources declared in the glTF manifest
        std::shared_ptr<std::ostream> GetOutputStream(const std::string& filename) const override
        {
            // In order to construct a valid stream:
            //1. The filename argument will be encoded as UTF-8 so use filesystem::u8path to
            // correctly construct a path instance.
            //2. Generate an absolute path by concatenating m_pathBase with the specified filename
            // path. The filesystem::operator/ uses the platform's preferred directory separator if
            // appropriate.
            //3. Always open the file stream in binary mode. The glTF SDK will handle any text
            // encoding issues for us.
            auto streamPath = m_pathBase / std::experimental::filesystem::u8path(filename);
            auto stream = std::make_shared<std::ofstream>(streamPath, std::ios_base::binary);

            // Check if the stream has no errors and is ready for I/O operations
            if (!stream || !(*stream))
            {
                throw std::runtime_error("Unable to create a valid output stream for uri: " + filename);
            }

            return stream;
        }

    private:
        std::experimental::filesystem::path m_pathBase;
    };

    BufferBuilder GetBufferBuilder(std::experimental::filesystem::path path)
    {
        auto readerWriter = std::make_shared<const StreamWriter>(path);
        return BufferBuilder(std::make_unique<GLTFResourceWriter>(readerWriter));
    }

    void SerializeTriangle(std::experimental::filesystem::path path)
    {
        if (path.is_relative())
        {
            auto pathCurrent = std::experimental::filesystem::current_path();

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

        // Use the glTF SDK's BufferBuilder as this dramatically simplifies the process of constructing the binary buffer
        // Pass the absolute path, without the filename, to the stream reader
        auto bufferBuilder = GetBufferBuilder(path.parent_path());

        // Now create a very simple glTF file with the following hierarchy:
        //   Scene
        //     Node
        //       Mesh (Triangle)
        //         MeshPrimitive
        //           Material (Blue)
        // 
        // A Document can be constructed top-down or bottom up, however if constructed top-down then IDs of child nodes must
        // be known in advance, which prevents using the glTF SDK's automatic ID generation.


        // First, construct the data that will be written to the binary buffer
        // Add a Buffer
        bufferBuilder.AddBuffer();

        // Add a BufferView with target ELEMENT_ARRAY_BUFFER, meaning it will store indices
        bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

        // Add an Accessor for the indices and store the ID
        std::vector<uint16_t> indices = { 0U, 1U, 2U };
        auto indicesAccessorId = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT }).id;

        // Add a BufferView with target ARRAY_BUFFER, meaning it will store vertex data
        bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

        // Add an Accessor for the positions
        std::vector<float> positions = {
            0.0f, 0.0f, 0.0f, // Vertex 0
            0.0f, 1.0f, 0.0f, // Vertex 1
            1.0f, 0.0f, 0.0f  // Vertex 2
        };
        auto positionsAccessorId = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT }).id;



        // Second, construct a document, which represents the glTF JSON part of the output
        Document document;

        // Construct a Material
        Material material;
        material.metallicRoughness.baseColorFactor = Color4(0.0f, 0.0f, 1.0f, 1.0f);
        // Add it to the Document, and store the generated ID
        auto materialId = document.materials.Append(std::move(material), Microsoft::glTF::AppendIdPolicy::GenerateOnEmpty).id;

        // Construct a MeshPrimitive, unlike most types in glTF, MeshPrimitives are direct children of their parent Mesh
        // rather than being children of the Document. This also means that they don't have any ID.
        MeshPrimitive meshPrimitive;
        meshPrimitive.materialId = materialId;
        meshPrimitive.indicesAccessorId = indicesAccessorId;
        meshPrimitive.attributes[ACCESSOR_POSITION] = positionsAccessorId;

        // Construct a Mesh using the MeshPrimitive
        Mesh mesh;
        mesh.primitives.push_back(std::move(meshPrimitive));
        // Add it to the document, and store the generated ID
        auto meshId = document.meshes.Append(std::move(mesh), Microsoft::glTF::AppendIdPolicy::GenerateOnEmpty).id;

        // Construct a Node
        Node node;
        node.meshId = meshId;
        // Add it to the document, and store the generated ID
        auto nodeId = document.nodes.Append(std::move(node), Microsoft::glTF::AppendIdPolicy::GenerateOnEmpty).id;

        // Construct a Scene
        Scene scene;
        scene.nodes.push_back(nodeId);
        // Add it to the Document, using a utility method that also sets this Scene as the Document's default
        document.SetDefaultScene(std::move(scene), Microsoft::glTF::AppendIdPolicy::GenerateOnEmpty);

        // Add all of the Buffers, BufferViews and Accessors that were created using BufferBuilder to the Document
        // Note that after this point, no further calls should be made to BufferBuilder
        bufferBuilder.Output(document);

        // Serialize the Document into a JSON string
        const auto gltfJson = Serialize(document, Microsoft::glTF::SerializeFlags::Pretty);

        bufferBuilder.GetResourceWriter().WriteExternal(path.filename().string(), gltfJson);
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

        SerializeTriangle(argv[1U]);
    }
    catch (const std::runtime_error& ex)
    {
        std::cerr << "Error! - ";
        std::cerr << ex.what();

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
