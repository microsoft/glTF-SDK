// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Serialize.h>

// Replace this with <filesystem> (and use std::filesystem rather than
// std::experimental::filesystem) if your toolchain fully supports C++17
#include <experimental/filesystem>

#include <fstream>
#include <sstream>
#include <iostream>

#include <cassert>
#include <cstdlib>

using namespace Microsoft::glTF;

namespace
{
    // The glTF SDK is decoupled from all file I/O by the IStreamWriter (and IStreamReader)
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

    void CreateTriangleResources(Document& document, BufferBuilder& bufferBuilder, std::string& accessorIdIndices, std::string& accessorIdPositions)
    {
        // Create all the resource data (e.g. triangle indices and
        // vertex positions) that will be written to the binary buffer
        const char* bufferId = nullptr;

        // Specify the 'special' GLB buffer ID. This informs the GLBResourceWriter that it should use
        // the GLB container's binary chunk (usually the desired buffer location when creating GLBs)
        if (dynamic_cast<const GLBResourceWriter*>(&bufferBuilder.GetResourceWriter()))
        {
            bufferId = GLB_BUFFER_ID;
        }

        // Create a Buffer - it will be the 'current' Buffer that all the BufferViews
        // created by this BufferBuilder will automatically reference
        bufferBuilder.AddBuffer(bufferId);

        // Create a BufferView with a target of ELEMENT_ARRAY_BUFFER (as it will reference index
        // data) - it will be the 'current' BufferView that all the Accessors created by this
        // BufferBuilder will automatically reference
        bufferBuilder.AddBufferView(BufferViewTarget::ELEMENT_ARRAY_BUFFER);

        // Add an Accessor for the indices
        std::vector<uint16_t> indices = {
            0U, 1U, 2U
        };

        // Copy the Accessor's id - subsequent calls to AddAccessor may invalidate the returned reference
        accessorIdIndices = bufferBuilder.AddAccessor(indices, { TYPE_SCALAR, COMPONENT_UNSIGNED_SHORT }).id;

        // Create a BufferView with target ARRAY_BUFFER (as it will reference vertex attribute data)
        bufferBuilder.AddBufferView(BufferViewTarget::ARRAY_BUFFER);

        // Add an Accessor for the positions
        std::vector<float> positions = {
            0.0f, 0.0f, 0.0f, // Vertex 0
            1.0f, 0.0f, 0.0f, // Vertex 1
            0.0f, 1.0f, 0.0f  // Vertex 2
        };

        std::vector<float> minValues(3U, std::numeric_limits<float>::max());
        std::vector<float> maxValues(3U, std::numeric_limits<float>::lowest());

        const size_t positionCount = positions.size();

        // Accessor min/max properties must be set for vertex position data so calculate them here
        for (size_t i = 0U, j = 0U; i < positionCount; ++i, j = (i % 3U))
        {
            minValues[j] = std::min(positions[i], minValues[j]);
            maxValues[j] = std::max(positions[i], maxValues[j]);
        }

        accessorIdPositions = bufferBuilder.AddAccessor(positions, { TYPE_VEC3, COMPONENT_FLOAT, false, std::move(minValues), std::move(maxValues) }).id;

        // Add all of the Buffers, BufferViews and Accessors that were created using BufferBuilder to
        // the Document. Note that after this point, no further calls should be made to BufferBuilder
        bufferBuilder.Output(document);
    }

    void CreateTriangleEntities(Document& document, const std::string& accessorIdIndices, const std::string& accessorIdPositions)
    {
        // Create a very simple glTF Document with the following hierarchy:
        //  Scene
        //     Node
        //       Mesh (Triangle)
        //         MeshPrimitive
        //           Material (Blue)
        // 
        // A Document can be constructed top-down or bottom up. However, if constructed top-down
        // then the IDs of child entities must be known in advance, which prevents using the glTF
        // SDK's automatic ID generation functionality.

        // Construct a Material
        Material material;
        material.metallicRoughness.baseColorFactor = Color4(0.0f, 0.0f, 1.0f, 1.0f);
        material.metallicRoughness.metallicFactor = 0.2f;
        material.metallicRoughness.roughnessFactor = 0.4f;
        material.doubleSided = true;

        // Add it to the Document and store the generated ID
        auto materialId = document.materials.Append(std::move(material), AppendIdPolicy::GenerateOnEmpty).id;

        // Construct a MeshPrimitive. Unlike most types in glTF, MeshPrimitives are direct children
        // of their parent Mesh entity rather than being children of the Document. This is why they
        // don't have an ID member.
        MeshPrimitive meshPrimitive;
        meshPrimitive.materialId = materialId;
        meshPrimitive.indicesAccessorId = accessorIdIndices;
        meshPrimitive.attributes[ACCESSOR_POSITION] = accessorIdPositions;

        // Construct a Mesh and add the MeshPrimitive as a child
        Mesh mesh;
        mesh.primitives.push_back(std::move(meshPrimitive));
        // Add it to the Document and store the generated ID
        auto meshId = document.meshes.Append(std::move(mesh), AppendIdPolicy::GenerateOnEmpty).id;

        // Construct a Node adding a reference to the Mesh
        Node node;
        node.meshId = meshId;
        // Add it to the Document and store the generated ID
        auto nodeId = document.nodes.Append(std::move(node), AppendIdPolicy::GenerateOnEmpty).id;

        // Construct a Scene
        Scene scene;
        scene.nodes.push_back(nodeId);
        // Add it to the Document, using a utility method that also sets the Scene as the Document's default
        document.SetDefaultScene(std::move(scene), AppendIdPolicy::GenerateOnEmpty);
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

        // Pass the absolute path, without the filename, to the stream writer
        auto streamWriter = std::make_unique<StreamWriter>(path.parent_path());

        std::experimental::filesystem::path pathFile = path.filename();
        std::experimental::filesystem::path pathFileExt = pathFile.extension();

        auto MakePathExt = [](const std::string& ext)
        {
            return "." + ext;
        };

        std::unique_ptr<ResourceWriter> resourceWriter;

        // If the file has a '.gltf' extension then create a GLTFResourceWriter
        if (pathFileExt == MakePathExt(GLTF_EXTENSION))
        {
            resourceWriter = std::make_unique<GLTFResourceWriter>(std::move(streamWriter));
        }

        // If the file has a '.glb' extension then create a GLBResourceWriter. This class derives
        // from GLTFResourceWriter and adds support for writing manifests to a GLB container's
        // JSON chunk and resource data to the binary chunk.
        if (pathFileExt == MakePathExt(GLB_EXTENSION))
        {
            resourceWriter = std::make_unique<GLBResourceWriter>(std::move(streamWriter));
        }

        if (!resourceWriter)
        {
            throw std::runtime_error("Command line argument path filename extension must be .gltf or .glb");
        }

        // The Document instance represents the glTF JSON manifest
        Document document;

        std::string accessorIdIndices;
        std::string accessorIdPositions;

        // Use the BufferBuilder helper class to simplify the process of
        // constructing valid glTF Buffer, BufferView and Accessor entities
        BufferBuilder bufferBuilder(std::move(resourceWriter));

        CreateTriangleResources(document, bufferBuilder, accessorIdIndices, accessorIdPositions);
        CreateTriangleEntities(document, accessorIdIndices, accessorIdPositions);

        std::string manifest;

        try
        {
            // Serialize the glTF Document into a JSON manifest
            manifest = Serialize(document, SerializeFlags::Pretty);
        }
        catch (const GLTFException& ex)
        {
            std::stringstream ss;

            ss << "Microsoft::glTF::Serialize failed: ";
            ss << ex.what();

            throw std::runtime_error(ss.str());
        }

        auto& gltfResourceWriter = bufferBuilder.GetResourceWriter();

        if (auto glbResourceWriter = dynamic_cast<GLBResourceWriter*>(&gltfResourceWriter))
        {
            glbResourceWriter->Flush(manifest, pathFile.u8string()); // A GLB container isn't created until the GLBResourceWriter::Flush member function is called
        }
        else
        {
            gltfResourceWriter.WriteExternal(pathFile.u8string(), manifest); // Binary resources have already been written, just need to write the manifest
        }
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
