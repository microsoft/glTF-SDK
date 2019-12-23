// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <GLTFSDK/GLTFResourceReader.h>

using namespace Microsoft::glTF;

namespace
{
	// Conversions of normalized component types to/from floats are explicitly defined in the 2.0 spec
	float ComponentToFloat(const int8_t w)  { return std::max(static_cast<float>(w) / 127.0f, -1.0f); }
	float ComponentToFloat(const uint8_t w) { return static_cast<float>(w) / 255.0f; }
	float ComponentToFloat(const int16_t w) { return std::max(static_cast<float>(w) / 32767.0f, -1.0f); }
	float ComponentToFloat(const uint16_t w){ return static_cast<float>(w) / 65535.0f; }

	template<typename T>
	std::vector<float> DecodeToFloats(const Document& doc, const GLTFResourceReader& reader, const Accessor& accessor)
	{
	    std::vector<T> rawData = reader.ReadBinaryData<T>(doc, accessor);

	    std::vector<float> floatData;
	    floatData.reserve(rawData.size());

		if (accessor.normalized)
		{
			std::transform(rawData.begin(), rawData.end(), std::back_inserter(floatData),
				[](T value) -> float { return ComponentToFloat(value); });
		}
		else
		{
			std::copy(rawData.begin(), rawData.end(), std::back_inserter(floatData));
		}

	    return floatData;
	}
}

std::vector<float> GLTFResourceReader::ReadFloatData(const Document& gltfDocument, const Accessor& accessor) const
{
	switch (accessor.componentType)
	{
	case COMPONENT_BYTE:
		return DecodeToFloats<int8_t>(gltfDocument, *this, accessor);

	case COMPONENT_UNSIGNED_BYTE:
		return DecodeToFloats<uint8_t>(gltfDocument, *this, accessor);

	case COMPONENT_SHORT:
		return DecodeToFloats<int16_t>(gltfDocument, *this, accessor);

	case COMPONENT_UNSIGNED_SHORT:
		return DecodeToFloats<uint16_t>(gltfDocument, *this, accessor);

	case COMPONENT_FLOAT:
		return ReadBinaryData<float>(gltfDocument, accessor);

	default:
		throw GLTFException("Unsupported accessor ComponentType");
	}
}
