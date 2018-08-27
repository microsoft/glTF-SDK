// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"

#include <GLTFSDK/GLBResourceWriter.h>

#include "TestUtils.h"

using namespace glTF::UnitTest;

namespace Microsoft
{
    namespace  glTF
    {
        namespace Test
        {
            GLTFSDK_TEST_CLASS(GLBResourceWriterTests)
            {
                GLTFSDK_TEST_METHOD(GLBResourceWriterTests, WriteBufferView_Empty)
                {
                    auto streamWriter = std::make_shared<const StreamReaderWriter>();
                    GLBResourceWriter writer(streamWriter);
                    std::string uri = "glb";
                    writer.Flush("", uri);

                    std::vector<char> output(100);
                    writer.WriteExternal(uri, output);
                }
            };
        }
    }
}
