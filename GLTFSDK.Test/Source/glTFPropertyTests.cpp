// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/GLTF.h>

namespace
{
    template<typename TDerived>
    class TestExtensionBase : public Microsoft::glTF::Extension
    {
    public:
        std::unique_ptr<Microsoft::glTF::Extension> Clone() const override
        {
            return std::make_unique<TDerived>(static_cast<const TDerived&>(*this));
        }

        bool IsEqual(const Microsoft::glTF::Extension& rhs) const override
        {
            return dynamic_cast<const TDerived*>(&rhs) != nullptr;
        }

    protected:
        TestExtensionBase() = default;
    };

    template<int N>
    class TestExtension : public TestExtensionBase<TestExtension<N>> {};
}

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(glTFPropertyTests)
            {
                GLTFSDK_TEST_METHOD(glTFPropertyTests, RegisteredExtensionEqualsTrue)
                {
                    Node node1;
                    node1.SetExtension<TestExtension<0>>();
                    node1.SetExtension<TestExtension<1>>();
                    node1.SetExtension<TestExtension<2>>();

                    // Adding same extensions in a different order - nodes should be considered equal
                    Node node2;
                    node2.SetExtension<TestExtension<2>>();
                    node2.SetExtension<TestExtension<1>>();
                    node2.SetExtension<TestExtension<0>>();

                    Assert::IsTrue(node1 == node2);
                }

                GLTFSDK_TEST_METHOD(glTFPropertyTests, RegisteredExtensionEqualsFalse)
                {
                    Node node1;
                    node1.SetExtension<TestExtension<0>>();
                    node1.SetExtension<TestExtension<1>>();
                    node1.SetExtension<TestExtension<2>>();

                    // Adding different types of extensions - nodes should not be considered equal
                    Node node2;
                    node2.SetExtension<TestExtension<3>>();
                    node2.SetExtension<TestExtension<4>>();
                    node2.SetExtension<TestExtension<5>>();

                    Assert::IsFalse(node1 == node2);

                    // Adding different numbers of extensions - nodes should not be considered equal
                    Node node3;
                    node3.SetExtension<TestExtension<0>>();
                    node3.SetExtension<TestExtension<1>>();

                    Assert::IsFalse(node1 == node3);
                }
            };
        }
    }
}
