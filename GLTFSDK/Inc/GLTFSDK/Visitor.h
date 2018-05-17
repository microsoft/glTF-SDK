// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Traverse.h>

#include <set>
#include <typeindex>

namespace Microsoft
{
    namespace glTF
    {
        // Enumeration used to track if a particular entity has been visited or not
        enum class VisitState
        {
            New,
            Duplicate
        };

        // Namespace contains implementation details of the Visit function - not part of the public API
        namespace Detail
        {
            // When the callable object can be invokable there will be two equally valid function
            // overloads to choose from. These tag structs are used to ensure that the 'do nothing'
            // overload is never the 'best' candidate function during overload resolution.
            struct Priority2 {};
            struct Priority1 : Priority2 {};

            // The function overload called when 'callable' object fn can be invoked with the argument types TArgs...
            template<typename Fn, typename ...TArgs>
            auto TryInvokeImpl(Priority1, Fn&& fn, TArgs&& ...args) -> decltype(std::declval<Fn>()(std::declval<TArgs>()...))
            {
                return fn(std::forward<TArgs>(args)...);
            }

            // The 'do nothing' overload called if the 'callable' object invoked with the argument types is not well formed
            template<typename Fn, typename ...TArgs>
            auto TryInvokeImpl(Priority2, Fn&&, TArgs&& ...) -> void
            {
            }

            // TryInvoke will invoke the 'callable' object fn only if the type Fn, when invoked with the argument
            // types TArgs..., is a well formed expression. Otherwise TryInvoke evaluates to an empty function call.
            template<typename Fn, typename ...TArgs>
            void TryInvoke(Fn&& fn, TArgs&& ...args)
            {
                TryInvokeImpl(Priority1(), std::forward<Fn>(fn), std::forward<TArgs>(args)...);
            }

            // Helper class for keeping track of which entities have been previously visited
            class VisitStateSet
            {
            public:
                template<typename T>
                VisitState GetVisitState(const T& t) const
                {
                    return visitStateSet.find({ typeid(T), std::stoul(t.id) }) == visitStateSet.end() ?
                        VisitState::New :
                        VisitState::Duplicate;
                }

                template<typename T>
                VisitState SetVisitState(const T& t)
                {
                    // Attempt to insert a unique identifier for the entity instance into the container by
                    // combining the type information (resolved at compile time) and glTF index property.
                    const auto result = visitStateSet.emplace(typeid(T), std::stoul(t.id));

                    // If the insertion succeeded then the entity hasn't yet been visited. Otherwise
                    // it is a 'duplicate' reference to an object that has already been visited.
                    return result.second ?
                        VisitState::New :
                        VisitState::Duplicate;
                }

            private:
                std::set<std::pair<std::type_index, size_t>> visitStateSet;
            };

            // Gets the 'raw' type of a template parameter so it can be inherited from
            template<typename Fn>
            using CombineInvokableBase = std::remove_cv_t<std::remove_reference_t<Fn>>;

            // Wrapper type that is used to combine multiple 'callable' objects (e.g. lambda or
            // function objects) into a single callable type with multiple function call operators.
            // This is done by defining a new type deriving from all of the input callable types.
            //
            // Inspired by the 'overloaded' type used in the example code at:
            //
            // http://en.cppreference.com/w/cpp/utility/variant/visit
            //
            // but modified to work with C++14 as using statements are not valid pack expansion
            // loci until C++17.
            template<typename Fn, typename ...FArgs>
            struct CombineInvokable : CombineInvokable<Fn>, CombineInvokable<FArgs...>
            {
                CombineInvokable(Fn fn, FArgs ...fargs) :
                    CombineInvokable<Fn>(std::forward<Fn>(fn)),
                    CombineInvokable<FArgs...>(std::forward<FArgs>(fargs)...)
                {
                }

                using CombineInvokable<Fn>::operator();
                using CombineInvokable<FArgs...>::operator();
            };

            // Specialization for the single type case - inherits from, and also copies, the passed 'callable' object
            template<typename Fn>
            struct CombineInvokable<Fn> : CombineInvokableBase<Fn>
            {
                CombineInvokable(Fn fn) : CombineInvokableBase<Fn>(std::forward<Fn>(fn))
                {
                }

                using CombineInvokableBase<Fn>::operator();
            };
        }

        class VisitDefaultAction
        {
        public:
            VisitDefaultAction(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet) : gltfDocument(gltfDocument), visitStateSet(visitStateSet)
            {
            }

            // There is no Visit() overload for MeshPrimitive as it lacks an 'id'
            // member variable and therefore cannot have its visit state tracked

            virtual void Visit(const Mesh& mesh) const = 0;
            virtual void Visit(const Material& material) const = 0;
            virtual void Visit(const Texture& texture, TextureType textureType) const = 0;
            virtual void Visit(const Image& image) const = 0;
            virtual void Visit(const Sampler& sampler) const = 0;
            virtual void Visit(const Skin& skin) const = 0;
            virtual void Visit(const Camera& camera) const = 0;

            const Document& gltfDocument;

        protected:
            Detail::VisitStateSet& visitStateSet;
        };

        template<typename Fn>
        class VisitDefaultActionImpl : public VisitDefaultAction
        {
        public:
            VisitDefaultActionImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn) : VisitDefaultAction(gltfDocument, visitStateSet), fn(fn)
            {
            }

            void Visit(const Mesh& mesh) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, mesh);
            }

            void Visit(const Material& material) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, material);
            }

            void Visit(const Texture& texture, TextureType textureType) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, texture, textureType);
            }

            void Visit(const Image& image) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, image);
            }

            void Visit(const Sampler& sampler) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, sampler);
            }

            void Visit(const Skin& skin) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, skin);
            }

            void Visit(const Camera& camera) const override
            {
                VisitImpl(gltfDocument, visitStateSet, fn, camera);
            }

        private:
            Fn& fn;
        };

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Mesh& mesh)
        {
            const VisitState visitStateMesh = visitStateSet.GetVisitState(mesh);

            Detail::TryInvoke(fn, mesh, visitStateMesh);
            Detail::TryInvoke(fn, mesh, visitStateMesh, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(mesh);

            for (const auto& meshPrimitive : mesh.primitives)
            {
                if (!meshPrimitive.materialId.empty())
                {
                    VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.materials.Get(meshPrimitive.materialId));
                }

                // The 'visit state' used here is that of the parent mesh, not the mesh
                // primitive, as a mesh primitive is defined inline and has no id property
                Detail::TryInvoke(fn, meshPrimitive, visitStateMesh);
                Detail::TryInvoke(fn, meshPrimitive, visitStateMesh, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));
            }
        }

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Material& material)
        {
            const VisitState visitStateMaterial = visitStateSet.GetVisitState(material);

            Detail::TryInvoke(fn, material, visitStateMaterial);
            Detail::TryInvoke(fn, material, visitStateMaterial, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(material);

            for (const auto& textureInfo : material.GetTextures())
            {
                const auto& textureId = textureInfo.first;

                if (!textureId.empty())
                {
                    VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.textures.Get(textureId), textureInfo.second);
                }
            }
        }

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Texture& texture, TextureType textureType)
        {
            const VisitState visitStateTexture = visitStateSet.GetVisitState(texture);

            // Note that the texture 'callback' also includes a 'type' parameter
            Detail::TryInvoke(fn, texture, textureType, visitStateTexture);
            Detail::TryInvoke(fn, texture, textureType, visitStateTexture, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(texture);

            if (!texture.imageId.empty())
            {
                VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.images.Get(texture.imageId));
            }

            if (!texture.samplerId.empty())
            {
                VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.samplers.Get(texture.samplerId));
            }
        }

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Image& image)
        {
            const VisitState visitStateImage = visitStateSet.GetVisitState(image);

            Detail::TryInvoke(fn, image, visitStateImage);
            Detail::TryInvoke(fn, image, visitStateImage, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(image);
        }

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Sampler& sampler)
        {
            const VisitState visitStateSampler = visitStateSet.GetVisitState(sampler);

            Detail::TryInvoke(fn, sampler, visitStateSampler);
            Detail::TryInvoke(fn, sampler, visitStateSampler, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(sampler);
        }

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Skin& skin)
        {
            const VisitState visitStateSkin = visitStateSet.GetVisitState(skin);

            Detail::TryInvoke(fn, skin, visitStateSkin);
            Detail::TryInvoke(fn, skin, visitStateSkin, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(skin);
        }

        template<typename Fn>
        void VisitImpl(const Document& gltfDocument, Detail::VisitStateSet& visitStateSet, Fn& fn, const Camera& camera)
        {
            const VisitState visitStateCamera = visitStateSet.GetVisitState(camera);

            Detail::TryInvoke(fn, camera, visitStateCamera);
            Detail::TryInvoke(fn, camera, visitStateCamera, VisitDefaultActionImpl<Fn>(gltfDocument, visitStateSet, fn));

            visitStateSet.SetVisitState(camera);
        }

        // Visit - implements a variant of the visitor pattern.
        //
        // Applies the visitor object 'fn' to each of the entities in the passed glTF document. The order in which
        // entities are visited is controlled by the Algorithm template parameter. The traversal algorithm can be
        // either depth-first or breadth-first search.
        //
        // The visitor object can be any callable type, such as a function pointer, functor or lambda. The visitor
        // does not need to explicitly implement any interface but will only be called if it exactly matches the
        // expected function signature for a given glTF entity. The signatures for each entity are as follows:
        //
        // Node          -> visitor.operator()(const Node&, const Node*)
        // Mesh          -> visitor.operator()(const Mesh&, VisitState, const VisitDefaultAction&)
        // MeshPrimitive -> visitor.operator()(const MeshPrimitive&, VisitState, const VisitDefaultAction&)
        // Material      -> visitor.operator()(const Material&, VisitState, const VisitDefaultAction&)
        // Texture       -> visitor.operator()(const Texture&, TextureType, VisitState, const VisitDefaultAction&)
        // Image         -> visitor.operator()(const Image&, VisitState, const VisitDefaultAction&)
        // Sampler       -> visitor.operator()(const Sampler&, VisitState, const VisitDefaultAction&)
        // Skin          -> visitor.operator()(const Skin&, VisitState, const VisitDefaultAction&)
        // Camera        -> visitor.operator()(const Camera&, VisitState, const VisitDefaultAction&)
        //
        // All of the 'callbacks' are optional - it is even acceptable to call Visit with a type that matches none
        // of the expected function signatures.
        //
        // Example:
        //
        // struct MyVisitor
        // {
        //     void operator()(const Node& node, const Node* nodeParent)
        //     {
        //         if (!nodeParent)
        //         {
        //             std::cout << "Visited a root node!";
        //         }
        //     }
        // };
        //
        // Visit(document, DefaultSceneIndex, MyVisitor());
        //
        template<TraversalAlgorithm Algorithm = DepthFirst, typename Fn>
        void Visit(const Document& gltfDocument, size_t sceneIndex, Fn&& fn)
        {
            Detail::VisitStateSet visitStateSet;

            Traverse<Algorithm>(gltfDocument, sceneIndex,
                [&gltfDocument, &visitStateSet, fn = std::forward<Fn>(fn)](const Node& node, const Node* nodeParent) mutable
            {
                // Ensure that the scene hierarchy is a tree rather than a DAG
                if (visitStateSet.SetVisitState(node) == VisitState::New)
                {
                    Detail::TryInvoke(fn, node, nodeParent);
                }
                else
                {
                    throw InvalidGLTFException("Node " + node.id + " has already been visited. This is not allowed - nodes may only have a single parent.");
                }

                if (!node.meshId.empty())
                {
                    VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.meshes.Get(node.meshId));
                }

                if (!node.skinId.empty())
                {
                    VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.skins.Get(node.skinId));
                }

                if (!node.cameraId.empty())
                {
                    VisitImpl(gltfDocument, visitStateSet, fn, gltfDocument.cameras.Get(node.cameraId));
                }
            });
        }

        // Visit - implements a variant of the visitor pattern.
        //
        // Overload that accepts multiple 'callable' objects. Can be used with multiple lambda functions.
        //
        // Example:
        //
        // Visit(document, DefaultSceneIndex,
        //     [](const Node& node, const Node* nodeParent)
        // {
        //     std::cout << "Visited a node!";
        // },
        //     [](const Mesh& mesh, VisitState vs)
        // {
        //     std::cout << "Visited a mesh!";
        // });
        //
        template<TraversalAlgorithm Algorithm = DepthFirst, typename ...FArgs>
        void Visit(const Document& gltfDocument, size_t sceneIndex, FArgs&& ...fargs)
        {
            Visit<Algorithm>(gltfDocument, sceneIndex, Detail::CombineInvokable<FArgs...>(std::forward<FArgs>(fargs)...));
        }
    }
}
