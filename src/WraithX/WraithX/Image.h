#pragma once

#include <cstdint>
#include <memory>

// We need to include the external libraries for Images
#include "DirectXTex.h"

// We need the binary writer class
#include "BinaryWriter.h"

// A list of supported image formats, input and output
enum class ImageFormat
{
	// Standard imaging formats

	Standard_BMP,
	Standard_GIF,
	Standard_JPEG,
	Standard_PNG,
	Standard_TGA,
	Standard_TIFF,
	Standard_EXR,

	// Use the InFormat ImageFormat for export, if InFormat is NoConversion conversion will fail
	NoConversion,

	// This is used only for InFormat, which will read the DDS format from the header in the stream
	DDS_WithHeader,

	// DDS block image formats

	DDS_BC1_TYPELESS,
	DDS_BC1_UNORM,
	DDS_BC1_SRGB,
	DDS_BC2_TYPELESS,
	DDS_BC2_UNORM,
	DDS_BC2_SRGB,
	DDS_BC3_TYPELESS,
	DDS_BC3_UNORM,
	DDS_BC3_SRGB,
	DDS_BC4_TYPELESS,
	DDS_BC4_UNORM,
	DDS_BC4_SNORM,
	DDS_BC5_TYPELESS,
	DDS_BC5_UNORM,
	DDS_BC5_SNORM,
	DDS_BC6_TYPELESS,
	DDS_BC6_UF16,
	DDS_BC6_SF16,
	DDS_BC7_TYPELESS,
	DDS_BC7_UNORM,
	DDS_BC7_SRGB,

	// DDS standard image formats

	DDS_Standard_A8R8G8B8,
	DDS_Standard_A8R8G8B8_SRGB,
	DDS_Standard_R8G8B8A8,
	DDS_Standard_R8G8B8A8_SRGB,
	DDS_Standard_R8G8B8,
	DDS_Standard_R9G9B9E5,
	DDS_Standard_R16G16B16A16_UNORM,
	DDS_Standard_R16G16B16A16_FLOAT,
	DDS_Standard_R32G32B32A32_UNORM,
	DDS_Standard_R32G32B32A32_FLOAT,
	DDS_Standard_R10G10B10A2_UNORM,
	DDS_Standard_D16_UNORM,
	DDS_Standard_A8_UNORM,
	DDS_Standard_R8_UNORM,
	DDS_Standard_R8_UINT
};

// A list of supported image patch functions, applied on export
enum class ImagePatch
{
	// -- Default

	// Do nothing to the export image
	NoPatch,

	// -- Normal map patches

	// Convert a gray-scale bumpmap to a regular normalmap
	Normal_Bumpmap,
	// Convert a yellow-scale, compressed normalmap to a regular normalmap
	Normal_Expand,
	// Convert a normal, gloss, and occlusion map to a regular normalmap (Call of Duty: Infinite Warfare)
	Normal_COD_NOG,

	// -- Color map patches

	// Removes the alpha channel from the colormap
	Color_StripAlpha
};

class Image
{
public:
	// -- Conversion functions

	// Converts an image stream from memory to a file with the specified format
	static bool ConvertImageMemory(int8_t* ImageBuffer, uint64_t ImageSize, ImageFormat InFormat, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch = ImagePatch::NoPatch);
	// Converts a safe image stream from memory to a file
	static bool ConvertImageMemory(const std::shared_ptr<int8_t[]>& ImageBuffer, uint64_t ImageSize, ImageFormat InFormat, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch = ImagePatch::NoPatch);
	// Converts an image file to another file with the specified format
	static bool ConvertImageFile(const std::string& InputFile, ImageFormat InFormat, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch = ImagePatch::NoPatch);

	// Converts a loaded image to the specified output format
	static bool ConvertToFormat(std::unique_ptr<DirectX::ScratchImage>& Image, DirectX::TexMetadata ImageMetadata, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch = ImagePatch::NoPatch);

	// -- Patching functions

	// Convert a gray-scale, bumpmap, to a normalmap
	static void PatchNormalFromBumpmap(std::unique_ptr<DirectX::ScratchImage>& Image);
	// Convert a compressed normalmap to a regular one
	static void PatchNormalFromCompressed(std::unique_ptr<DirectX::ScratchImage>& Image);
	// Convert a (COD) NOG (Normal, ambientocc, gloss) map to a normalmap)
	static void PatchNormalCODFromNOG(std::unique_ptr<DirectX::ScratchImage>& Image);

	// Strip the alpha channel from this image
	static void PatchAlphaChannel(std::unique_ptr<DirectX::ScratchImage>& Image);

	// -- DDS functions

	// Builds and writes a DDS header with the specified format
	static void WriteDDSHeaderToFile(BinaryWriter& Writer, uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, bool isCubemap = false);
	// Builds and writes a DDS header with the specified format to the safe writer
	static void WriteDDSHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, bool isCubemap = false);

	// Builds and writes a DDS header to a stream with the specified format
	static void WriteDDSHeaderToStream(int8_t* Buffer, uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, uint32_t& ResultSize, bool isCubemap = false);

	// Gets the maximum size of a DDS header
	static const uint32_t GetMaximumDDSHeaderSize();

	// -- Utility functions

	// Enables this thread to load the conversion objects
	static void SetupConversionThread();
	// Disables this thread for conversion, cleaning up
	static void DisableConversionThread();

	// Sets up the WIC library for COM
	static void InitializeImageAPI();
	// Shuts down the WIC library for COM
	static void ShutdownImageAPI();

private:

	// Gets a DDS header buffer with the specified format
	static std::unique_ptr<int8_t[]> BuildDDSHeader(uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, uint32_t& ResultSize, bool isCubemap = false);
};