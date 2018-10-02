#include "stdafx.h"

// The class we are implementing
#include "CoDRawImageTranslator.h"

// We need the following Wraith classes
#include "Image.h"
#include "MemoryReader.h"

std::unique_ptr<XImageDDS> CoDRawImageTranslator::TranslateBC(const std::unique_ptr<uint8_t[]>& BCBuffer, uint32_t BCBufferSize, uint32_t Width, uint32_t Height, uint8_t ImageFormat, uint8_t MipLevels, bool isCubemap)
{
	// Prepare to translate the image
	auto Result = std::make_unique<XImageDDS>();

	// Allocate buffer
	auto ImageBuffer = new int8_t[Image::GetMaximumDDSHeaderSize() + BCBufferSize];

	// Get format
	auto ImageDataFormat = ImageFormat::DDS_BC1_SRGB;
	// Calculate format from input
	switch (ImageFormat)
	{
	case 24: ImageDataFormat = ImageFormat::DDS_Standard_R10G10B10A2_UNORM; break;
	case 28: ImageDataFormat = ImageFormat::DDS_Standard_A8R8G8B8; break;
	case 29: ImageDataFormat = ImageFormat::DDS_Standard_A8R8G8B8_SRGB; break;
	case 61: ImageDataFormat = ImageFormat::DDS_Standard_R8_UNORM; break;
	case 62: ImageDataFormat = ImageFormat::DDS_Standard_R8_UINT; break;
	case 71: ImageDataFormat = ImageFormat::DDS_BC1_UNORM; break;
	case 72: ImageDataFormat = ImageFormat::DDS_BC1_SRGB; break;
	case 74: ImageDataFormat = ImageFormat::DDS_BC2_UNORM; break;
	case 75: ImageDataFormat = ImageFormat::DDS_BC2_SRGB; break;
	case 77: ImageDataFormat = ImageFormat::DDS_BC3_UNORM; break;
	case 78: ImageDataFormat = ImageFormat::DDS_BC3_SRGB; break;
	case 80: ImageDataFormat = ImageFormat::DDS_BC4_UNORM; break;
	case 81: ImageDataFormat = ImageFormat::DDS_BC4_SNORM; break;
	case 83: ImageDataFormat = ImageFormat::DDS_BC5_UNORM; break;
	case 84: ImageDataFormat = ImageFormat::DDS_BC5_SNORM; break;
	case 95: ImageDataFormat = ImageFormat::DDS_BC6_UF16; break;
	case 96: ImageDataFormat = ImageFormat::DDS_BC6_SF16; break;
	case 98: ImageDataFormat = ImageFormat::DDS_BC7_UNORM; break;
	case 99: ImageDataFormat = ImageFormat::DDS_BC7_SRGB; break;

		// Handle unknown BC formats in debug
#ifdef _DEBUG
	default: printf("Unknown image format: %d (0x%X)\n", ImageFormat, ImageFormat); break;
#endif
	}

	// Result size
	uint32_t ResultSize = 0;
	// Write the header
	Image::WriteDDSHeaderToStream(ImageBuffer, Width, Height, MipLevels, ImageDataFormat, ResultSize, isCubemap);

	// Copy image data
	std::memcpy(ImageBuffer + ResultSize, BCBuffer.get(), BCBufferSize);

	// Assign data
	Result->DataBuffer = ImageBuffer;
	Result->DataSize = (uint32_t)(ResultSize + BCBufferSize);

	// Return it
	return Result;
}