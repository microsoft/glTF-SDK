// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/Extension.h>

using namespace Microsoft::glTF;

bool Extension::operator==(const Extension& rhs) const
{
    return IsEqual(rhs);
}

bool Extension::operator!=(const Extension& rhs) const
{
    return !operator==(rhs);
}
