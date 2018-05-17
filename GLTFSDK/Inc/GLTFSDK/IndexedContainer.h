// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <GLTFSDK/Exceptions.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace Microsoft
{
    namespace glTF
    {
        // Empty strings aren't valid ids. AppendIdPolicy enum values control what
        // happens when the Append function encounters an element with an empty id
        //
        // ThrowOnEmpty    - A GLTFException is thrown when trying to Append an
        //                   element with an empty id.
        // GenerateOnEmpty - A unique id is generated and assigned to the element
        //                   stored in the IndexedContainer. Use the reference
        //                   returned by Append to determine the id generated.
        enum class AppendIdPolicy
        {
            ThrowOnEmpty,
            GenerateOnEmpty
        };

        template<typename T, bool = std::is_const<T>::value>
        class IndexedContainer;

        // Const template parameter T partial specialization
        template<typename T>
        class IndexedContainer<const T, true>
        {
        public:
            const T& Front() const
            {
                return m_elements.front();
            }

            const T& Back() const
            {
                return m_elements.back();
            }

            const T& operator[](size_t index) const
            {
                if (index < m_elements.size())
                {
                    return m_elements[index]; // operator[] used rather than at() to avoid unnecessary bounds checking
                }

                throw GLTFException("index " + std::to_string(index) + " not in container");
            }

            const T& operator[](const std::string& key) const
            {
                return operator[](GetIndex(key));
            }

            bool operator==(const IndexedContainer& rhs) const
            {
                return (m_elements == rhs.m_elements);
            }

            bool operator!=(const IndexedContainer& rhs) const
            {
                return !(operator==(rhs));
            }

            const T& Append(const T& element, AppendIdPolicy policy = AppendIdPolicy::ThrowOnEmpty)
            {
                return Append(T(element), policy);
            }

            const T& Append(T&& element, AppendIdPolicy policy = AppendIdPolicy::ThrowOnEmpty)
            {
                const bool isEmptyId = element.id.empty();

                if (isEmptyId)
                {
                    if (policy != AppendIdPolicy::GenerateOnEmpty)
                    {
                        throw GLTFException("key is an empty string");
                    }

                    element.id = std::to_string(m_elements.size());
                }

                while (!m_elementIndices.emplace(element.id, m_elements.size()).second)
                {
                    if (isEmptyId) // Can only be true if policy is GenerateOnEmpty
                    {
                        // If id policy is GenerateOnEmpty then postfix the auto-generated key with '+'
                        // chars until a unique value is generated (i.e. the call to emplace succeeds)
                        element.id += "+";
                    }
                    else
                    {
                        throw GLTFException("key " + element.id + " already exists in IndexedContainer");
                    }
                }

                m_elements.push_back(std::move(element));
                return m_elements.back();
            }

            void Clear()
            {
                m_elementIndices.clear();
                m_elements.clear();
            }

            const std::vector<T>& Elements() const
            {
                return m_elements;
            }

            const T& Get(size_t index) const
            {
                return operator[](index);
            }

            const T& Get(const std::string& key) const
            {
                return operator[](key);
            }

            size_t GetIndex(const std::string& key) const
            {
                if (key.empty())
                {
                    throw GLTFException("Invalid key - cannot be empty");
                }

                auto it = m_elementIndices.find(key);

                if (it == m_elementIndices.end())
                {
                    throw GLTFException("key " + key + " not in container");
                }

                return it->second;
            }

            bool Has(const std::string& key) const
            {
                return m_elementIndices.find(key) != m_elementIndices.end();
            }

            void Remove(const std::string& key)
            {
                const auto index = GetIndex(key);

                m_elementIndices.erase(key);
                m_elements.erase(m_elements.begin() + index);

                for (auto& elementIndex : m_elementIndices)
                {
                    if (elementIndex.second > index)
                    {
                        elementIndex.second--;
                    }
                }
            }

            void Replace(const T& element)
            {
                Replace(T(element));
            }

            void Replace(T&& element)
            {
                const auto index = GetIndex(element.id);
                m_elements[index] = std::move(element);
            }

            void Reserve(size_t capacity)
            {
                m_elements.reserve(capacity);
                m_elementIndices.reserve(capacity);
            }

            size_t Size() const
            {
                return m_elements.size();
            }

        private:
            std::vector<T> m_elements;
            std::unordered_map<std::string, size_t> m_elementIndices;
        };

        // Mutable template parameter T partial specialization - Uses private inheritance to gain the const template parameter functionality without an is-a relationship
        template<typename T>
        class IndexedContainer<T, false> : private IndexedContainer<const T>
        {
            // IndexedContainer<const T> is a 'dependent base class' (because it is dependent on template parameter
            // T). This means all inherited members must be qualified with 'this->' or 'IndexedContainer<const T>::'

        public:
            T& Front()
            {
                return const_cast<T&>(IndexedContainer<const T>::Front());
            }

            T& Back()
            {
                return const_cast<T&>(IndexedContainer<const T>::Back());
            }

            T& operator[](size_t index)
            {
                return const_cast<T&>(IndexedContainer<const T>::operator[](index));
            }

            T& operator[](const std::string& key)
            {
                return operator[](GetIndex(key));
            }

            bool operator==(const IndexedContainer& rhs) const
            {
                return IndexedContainer<const T>::operator==(rhs);
            }

            bool operator!=(const IndexedContainer& rhs) const
            {
                return !(operator==(rhs));
            }

            T& Append(const T& element, AppendIdPolicy policy = AppendIdPolicy::ThrowOnEmpty)
            {
                return Append(T(element), policy);
            }

            T& Append(T&& element, AppendIdPolicy policy = AppendIdPolicy::ThrowOnEmpty)
            {
                return const_cast<T&>(IndexedContainer<const T>::Append(std::move(element), policy));
            }

            std::vector<T>& Elements()
            {
                return const_cast<std::vector<T>&>(IndexedContainer<const T>::Elements());
            }

            T& Get(size_t index)
            {
                return operator[](index);
            }

            T& Get(const std::string& key)
            {
                return operator[](key);
            }

            // No using declaration for Append, operator== or operator!= as we don't
            // want to make the base class versions of these functions publically
            // accessible (the mutable versions replace rather than complement them)
            using IndexedContainer<const T>::Front;
            using IndexedContainer<const T>::Back;
            using IndexedContainer<const T>::Clear;
            using IndexedContainer<const T>::Elements;
            using IndexedContainer<const T>::Get;
            using IndexedContainer<const T>::GetIndex;
            using IndexedContainer<const T>::Has;
            using IndexedContainer<const T>::Remove;
            using IndexedContainer<const T>::Replace;
            using IndexedContainer<const T>::Reserve;
            using IndexedContainer<const T>::Size;
            using IndexedContainer<const T>::operator[];
        };
    }
}
