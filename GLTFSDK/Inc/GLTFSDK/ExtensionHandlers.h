// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/GLTF.h>

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace Microsoft
{
    namespace glTF
    {
        namespace Detail
        {
            using TypeKey = std::pair<std::type_index, std::type_index>;

            template<typename T1, typename T2>
            TypeKey MakeTypeKey()
            {
                return { typeid(T1), typeid(T2) };
            }

            template<typename T>
            TypeKey MakeTypeKey(const Extension& extension)
            {
                return { typeid(extension), typeid(T) };
            }

            TypeKey MakeTypeKey(const Extension& extension, const glTFProperty& property);

            using NameKey = std::pair<std::string, std::type_index>;

            template<typename T>
            NameKey MakeNameKey(const std::string& name)
            {
                return { name, typeid(T) };
            }

            NameKey MakeNameKey(const std::string& name, const glTFProperty& property);
        }

        template<typename TReturn, typename ...TArgs>
        class ExtensionHandlers
        {
        public:
            template<typename TExt, typename Fn>
            void AddHandler(const std::string& name, Fn fn)
            {
                AddHandler<TExt, glTFPropertyAll>(name, fn);
            }

            template<typename TExt, typename TProp, typename Fn>
            void AddHandler(const std::string& name, Fn fn)
            {
                static_assert(std::is_base_of<Extension, TExt>::value, "ExtensionHandlers::AddHandler: TExt template parameter must derive from Extension");
                static_assert(std::is_base_of<glTFProperty, TProp>::value, "ExtensionHandlers::AddHandler: TProp template parameter must derive from glTFProperty");

                auto resultName = nameToType.emplace(Detail::MakeNameKey<TProp>(name), typeid(TExt));

                if (!resultName.second)
                {
                    throw GLTFException("A handler for the " + name + " extension already exists");
                }

                auto resultType = typeToName.emplace(Detail::MakeTypeKey<TExt, TProp>(), name);

                if (!resultType.second)
                {
                    throw GLTFException("A handler for the " + name + " extension already exists");
                }

                // Wrap the passed callable type Fn so that the handler is passed a reference to TExt rather than Extension
                auto fnConvert = [fn](std::add_lvalue_reference_t<const TArgs> ...args)
                {
                    return fn(Convert<TExt>(args)...);
                };

                auto resultHandler = handlers.emplace(Detail::MakeTypeKey<TExt, TProp>(), fnConvert);

                if (!resultHandler.second)
                {
                    throw GLTFException("A handler for the " + name + " extension already exists");
                }
            }

            template<typename TExt>
            bool HasHandler() const
            {
                return HasHandler<TExt, glTFPropertyAll>();
            }

            template<typename TExt, typename TProp>
            bool HasHandler() const
            {
                return typeToName.find(Detail::MakeTypeKey<TExt, TProp>()) != typeToName.end();
            }

            bool HasHandler(const std::string& name) const
            {
                return nameToType.find(Detail::MakeNameKey<glTFPropertyAll>(name)) != nameToType.end();
            }

            bool HasHandler(const std::string& name, const glTFProperty& property) const
            {
                return nameToType.find(Detail::MakeNameKey(name, property)) != nameToType.end();
            }

            typedef std::function<TReturn(std::add_lvalue_reference_t<const TArgs>...)> Func;

        protected:
            TReturn Process(const Detail::TypeKey& key, std::add_lvalue_reference_t<const TArgs> ...args) const
            {
                auto it = handlers.find(key);

                if (it == handlers.end())
                {
                    throw GLTFException("No handler is registered for the specified type_index");
                }

                return it->second(args...);
            }

            // Called when the argument type inherits from Extension - converts the argument from Extension to the derived type TExt
            template<typename TExt, typename TArg>
            static auto Convert(const TArg& arg) -> std::enable_if_t< std::is_base_of<Extension, TArg>::value, const TExt&>
            {
                return dynamic_cast<const TExt&>(arg);
            }

            // Called when the argument type doesn't inherit from Extension - passes the argument through unchanged
            template<typename TExt, typename TArg>
            static auto Convert(const TArg& arg) -> std::enable_if_t<!std::is_base_of<Extension, TArg>::value, const TArg&>
            {
                return arg;
            }

            // Non-constructible sentinel type for adding a handler that operates on all glTFProperty derived types
            struct glTFPropertyAll : glTFProperty
            {
                glTFPropertyAll() = delete;
            };

            struct Hash
            {
                template<typename T1, typename T2>
                size_t operator()(const std::pair<T1, T2>& key) const
                {
                    // This method of combining hashes is from boost::hash_combine - https://www.boost.org/doc/libs/1_67_0/boost/container_hash/hash.hpp
                    auto fnCombine = [](size_t& seed, size_t value)
                    {
                        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                    };

                    std::hash<T1> hash1;
                    std::hash<T2> hash2;

                    size_t hash = {};

                    fnCombine(hash, hash1(std::get<0>(key)));
                    fnCombine(hash, hash2(std::get<1>(key)));

                    return hash;
                }
            };

            std::unordered_map<Detail::TypeKey, Func, Hash> handlers;

            std::unordered_map<Detail::TypeKey, std::string, Hash>     typeToName;
            std::unordered_map<Detail::NameKey, std::type_index, Hash> nameToType;
        };

        struct ExtensionPair
        {
            std::string name;
            std::string value;
        };

        class Document;

        class ExtensionSerializer final : public ExtensionHandlers<std::string, Extension, Document, ExtensionSerializer>
        {
        public:
            ExtensionPair Serialize(const Extension& extension, const glTFProperty& property, const Document& document) const;
        };

        class ExtensionDeserializer final : public ExtensionHandlers<std::unique_ptr<Extension>, std::string, ExtensionDeserializer>
        {
        public:
            std::unique_ptr<Extension> Deserialize(const ExtensionPair& extensionPair, const glTFProperty& property) const;
        };
    }
}
