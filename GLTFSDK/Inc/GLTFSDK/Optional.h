// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "Exceptions.h"

namespace Microsoft
{
    namespace glTF
    {
        namespace Detail
        {
            template<size_t N, size_t A>
            class OptionalStorage
            {
            public:
                void* GetStorage()
                {
                    return data;
                }

                const void* GetStorage() const
                {
                    return data;
                }

            protected:
                alignas(A) unsigned char data[N];
            };

            template<typename T>
            class Optional final : private OptionalStorage<sizeof(T), alignof(T)>
            {
            public:
                Optional() : isConstructed(false)
                {
                }

                Optional(T&& t) : isConstructed(true)
                {
                    new (this->GetStorage()) T(std::move(t)); // In-place new;
                }

                Optional(const T& t) : isConstructed(true)
                {
                    new (this->GetStorage()) T(t); // In-place new;
                }

                Optional(Optional<T>&& other) : isConstructed(false)
                {
                    if (other.isConstructed)
                    {
                        Swap(other);
                    }
                }

                Optional(const Optional& other) : isConstructed(other.isConstructed)
                {
                    if (other.isConstructed)
                    {
                        new (this->GetStorage()) T(other.Get()); // In-place new;
                    }
                }

                ~Optional()
                {
                    Reset();
                }

                T& Get()
                {
                    if (isConstructed)
                    {
                        return *static_cast<T*>(this->GetStorage());
                    }
                    else
                    {
                        throw GLTFException("Optional has no value");
                    }
                }

                const T& Get() const
                {
                    if (isConstructed)
                    {
                        return *static_cast<const T*>(this->GetStorage());
                    }
                    else
                    {
                        throw GLTFException("Optional has no value");
                    }
                }

                bool HasValue() const noexcept
                {
                    return isConstructed;
                }

                void Reset()
                {
                    if (isConstructed)
                    {
                        Get().~T(); // In-place delete;
                    }

                    isConstructed = false;
                }

                void Swap(Optional& other)
                {
                    Swap(*this, other);
                }

                static void Swap(Optional& lhs, Optional& rhs)
                {
                    if (lhs && rhs)
                    {
                        std::swap(lhs.Get(), rhs.Get());
                    }
                    else if (lhs)
                    {
                        new (rhs.GetStorage()) T(std::move(lhs.Get()));
                        rhs.isConstructed = true;
                        lhs.Get().~T(); // In-place delete;
                        lhs.isConstructed = false;
                    }
                    else if (rhs)
                    {
                        new (lhs.GetStorage()) T(std::move(rhs.Get()));
                        lhs.isConstructed = true;
                        rhs.Get().~T(); // In-place delete;
                        rhs.isConstructed = false;
                    }
                }

                Optional& operator=(Optional&& other)
                {
                    if (this != &other)
                    {
                        Optional(std::move(other)).Swap(*this);
                    }

                    return *this;
                }

                Optional& operator=(const Optional& other)
                {
                    if (this != &other)
                    {
                        Optional(other).Swap(*this);
                    }

                    return *this;
                }

                Optional& operator=(T&& t)
                {
                    if (isConstructed)
                    {
                        Get() = std::move(t);
                    }
                    else
                    {
                        new (this->GetStorage()) T(std::move(t)); // In-place new;
                        isConstructed = true;
                    }

                    return *this;
                }

                Optional& operator=(const T& t)
                {
                    if (isConstructed)
                    {
                        Get() = t;
                    }
                    else
                    {
                        new (this->GetStorage()) T(t); // In-place new;
                        isConstructed = true;
                    }

                    return *this;
                }

                explicit operator bool() const noexcept
                {
                    return HasValue();
                }

            private:
                bool isConstructed;
            };

            template<typename T>
            bool operator==(const Optional<T>& lhs, const Optional<T>& rhs)
            {
                return (!lhs && !rhs) || ((lhs && rhs) && (lhs.Get() == rhs.Get()));
            }

            template<typename T>
            bool operator!=(const Optional<T>& lhs, const Optional<T>& rhs)
            {
                return !(lhs == rhs);
            }
        }
    }
}
