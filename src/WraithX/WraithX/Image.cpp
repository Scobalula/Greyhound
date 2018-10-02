#include "stdafx.h"

// The class we are implementing
#include "Image.h"

// We need the strings and math classes
#include "Strings.h"
#include "VectorMath.h"

// We need the DDS library from DirectXTex
#include "DDS.h"
// We need the private library for utils
#include "DirectXTexP.h"


bool Image::ConvertImageMemory(int8_t* ImageBuffer, uint64_t ImageSize, ImageFormat InFormat, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch)
{
	// Allocate a new image for loading
	auto Image = std::make_unique<DirectX::ScratchImage>();
	// Allocate a buffer for metadata
	DirectX::TexMetadata ImageMetadata;

	// The result of loading the file, defaults to failed
	HRESULT Result = -1;

	// Load based on InFormat
	switch (InFormat)
	{
	case ImageFormat::Standard_BMP:
	case ImageFormat::Standard_GIF:
	case ImageFormat::Standard_JPEG:
	case ImageFormat::Standard_PNG:
	case ImageFormat::Standard_TIFF:
		// Load via WIC
		Result = DirectX::LoadFromWICMemory(ImageBuffer, ImageSize, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &ImageMetadata, *Image);
		// Just exit the switch
		break;
	case ImageFormat::Standard_TGA:
		// Load via function
		Result = DirectX::LoadFromTGAMemory(ImageBuffer, ImageSize, &ImageMetadata, *Image);
		// Just exit the switch
		break;
	case ImageFormat::DDS_WithHeader:
		// Load via function
		Result = DirectX::LoadFromDDSMemory(ImageBuffer, ImageSize, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &ImageMetadata, *Image);
		// Just exit the switch
		break;
	default:
		// Failed to load the image, format not supported
		return false;
	}

	// Check if we are successful
	if (FAILED(Result)) { return false; }

	// Calculate output format
	auto OutputFormat = (OutFormat == ImageFormat::NoConversion) ? InFormat : OutFormat;

	// Pass off to converter
	return ConvertToFormat(Image, ImageMetadata, OutputFile, OutputFormat, Patch);
}

bool Image::ConvertImageMemory(const std::shared_ptr<int8_t[]>& ImageBuffer, uint64_t ImageSize, ImageFormat InFormat, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch)
{
	// Ship off to buffer convert, just get pointer to data
	return ConvertImageMemory(*ImageBuffer.get(), ImageSize, InFormat, OutputFile, OutFormat, Patch);
}

bool Image::ConvertImageFile(const std::string& InputFile, ImageFormat InFormat, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch)
{
	// Allocate a new image for loading
	auto Image = std::make_unique<DirectX::ScratchImage>();
	// Allocate a buffer for metadata
	DirectX::TexMetadata ImageMetadata;

	// The result of loading the file, defaults to failed
	HRESULT Result = -1;

	// Load based on InFormat
	switch (InFormat)
	{
	case ImageFormat::Standard_BMP:
	case ImageFormat::Standard_GIF:
	case ImageFormat::Standard_JPEG:
	case ImageFormat::Standard_PNG:
	case ImageFormat::Standard_TIFF:
		// Load via WIC
		Result = DirectX::LoadFromWICFile(Strings::ToUnicodeString(InputFile).c_str(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &ImageMetadata, *Image);
		// Just exit the switch
		break;
	case ImageFormat::Standard_TGA:
		// Load via function
		Result = DirectX::LoadFromTGAFile(Strings::ToUnicodeString(InputFile).c_str(), &ImageMetadata, *Image);
		// Just exit the switch
		break;
	case ImageFormat::DDS_WithHeader:
		// Load via function
		Result = DirectX::LoadFromDDSFile(Strings::ToUnicodeString(InputFile).c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &ImageMetadata, *Image);
		// Just exit the switch
		break;
	default:
		// Failed to load the image, format not supported
		return false;
	}

	// Check if we are successful
	if (FAILED(Result)) { return false; }

	// Calculate output format
	auto OutputFormat = (OutFormat == ImageFormat::NoConversion) ? InFormat : OutFormat;

	// Pass off to converter
	return ConvertToFormat(Image, ImageMetadata, OutputFile, OutputFormat, Patch);
}

bool Image::ConvertToFormat(std::unique_ptr<DirectX::ScratchImage>& Image, DirectX::TexMetadata ImageMetadata, const std::string& OutputFile, ImageFormat OutFormat, ImagePatch Patch)
{
	// Stage 1: Check if the image is planar, if so, convert to a single plane
	if (DirectX::IsPlanar(ImageMetadata.format))
	{
		// Fetch first image
		auto FirstImage = Image->GetImage(0, 0, 0);
		// Fetch image count
		auto ImageCount = Image->GetImageCount();
		// Allocate a temporary buffer
		auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

		// Convert to normal plane
		auto Result = DirectX::ConvertToSinglePlane(FirstImage, ImageCount, ImageMetadata, *TemporaryImage);

		// Ensure success
		if (!FAILED(Result))
		{
			// Get the information
			auto& TextureInfo = TemporaryImage->GetMetadata();
			// Swap out our format
			ImageMetadata.format = TextureInfo.format;
			// Swap the temp image to normal one
			Image.reset(TemporaryImage.release());
		}
		else
		{
			// Failed to process
			return false;
		}
	}

	// Stage 2: Decompress the texture if necessary, or ensure it's in the proper format
	if (DirectX::IsCompressed(ImageMetadata.format))
	{
		// Fetch first image
		auto FirstImage = Image->GetImage(0, 0, 0);
		// Fetch image count
		auto ImageCount = Image->GetImageCount();
		// Allocate a temporary buffer
		auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

		// We must use this format for global support, it's a standard for images
		DXGI_FORMAT ResultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// Decompress the image
		auto Result = DirectX::Decompress(FirstImage, ImageCount, ImageMetadata, ResultFormat, *TemporaryImage);

		// Ensure success
		if (!FAILED(Result))
		{
			// Get the information
			auto& TextureInfo = TemporaryImage->GetMetadata();
			// Swap out our format
			ImageMetadata.format = TextureInfo.format;
			// Swap the temp image to normal one
			Image.reset(TemporaryImage.release());
		}
		else
		{
			// Failed to process
			return false;
		}
	}
	else if (ImageMetadata.format != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
	{
		// Allocate a temporary buffer
		auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

		// We must use this format for global support, it's a standard for images
		DXGI_FORMAT ResultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		// Convert to our format
		auto Result = DirectX::Convert(Image->GetImages(), Image->GetImageCount(), Image->GetMetadata(), ResultFormat, DirectX::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, *TemporaryImage);

		// Ensure success
		if (!FAILED(Result))
		{
			// Get the information
			auto& TextureInfo = TemporaryImage->GetMetadata();
			// Swap out our format
			ImageMetadata.format = TextureInfo.format;
			// Swap the temp image to normal one
			Image.reset(TemporaryImage.release());
		}
		else
		{
			// Failed to process
			return false;
		}
	}

	// Stage 3: Patching, apply an image patch if need be
	if (Patch != ImagePatch::NoPatch)
	{
		// Apply patching to the image
		switch (Patch)
		{
		case ImagePatch::Normal_Bumpmap: PatchNormalFromBumpmap(Image); break;
		case ImagePatch::Normal_Expand: PatchNormalFromCompressed(Image); break;
		case ImagePatch::Normal_COD_NOG: PatchNormalCODFromNOG(Image); break;
		case ImagePatch::Color_StripAlpha: PatchAlphaChannel(Image); break;
		}
	}

	// This format only applies for DDS saving
	DXGI_FORMAT ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;

	// Stage 4: Recompression, if the output format requires it, DDS image formats only
	if (OutFormat > ImageFormat::DDS_WithHeader)
	{
		// We must select the format for the specified output format
		switch (OutFormat)
		{
		case ImageFormat::DDS_BC1_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC1_TYPELESS; break;
		case ImageFormat::DDS_BC1_UNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM; break;
		case ImageFormat::DDS_BC1_SRGB: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB; break;

		case ImageFormat::DDS_BC2_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC2_TYPELESS; break;
		case ImageFormat::DDS_BC2_UNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM; break;
		case ImageFormat::DDS_BC2_SRGB: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM_SRGB; break;

		case ImageFormat::DDS_BC3_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC3_TYPELESS; break;
		case ImageFormat::DDS_BC3_UNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM; break;
		case ImageFormat::DDS_BC3_SRGB: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB; break;

		case ImageFormat::DDS_BC4_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC4_TYPELESS; break;
		case ImageFormat::DDS_BC4_UNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM; break;
		case ImageFormat::DDS_BC4_SNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC4_SNORM; break;

		case ImageFormat::DDS_BC5_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_TYPELESS; break;
		case ImageFormat::DDS_BC5_UNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM; break;
		case ImageFormat::DDS_BC5_SNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_SNORM; break;

		case ImageFormat::DDS_BC6_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC6H_TYPELESS; break;
		case ImageFormat::DDS_BC6_UF16: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16; break;
		case ImageFormat::DDS_BC6_SF16: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC6H_SF16; break;

		case ImageFormat::DDS_BC7_TYPELESS: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC7_TYPELESS; break;
		case ImageFormat::DDS_BC7_UNORM: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM; break;
		case ImageFormat::DDS_BC7_SRGB: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB; break;

		case ImageFormat::DDS_Standard_R8G8B8A8: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS; break;
		case ImageFormat::DDS_Standard_R8G8B8A8_SRGB: ResultFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
		}

		// Only proceed if the result is compressed
		if (DirectX::IsCompressed(ResultFormat))
		{
			// We have a DDS format to save to
			auto FirstImage = Image->GetImage(0, 0, 0);
			// Fetch image count
			auto ImageCount = Image->GetImageCount();
			// Allocate a temporary image
			auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

			// Compress the image TODO: Replace 0 with proper flags
			auto Result = DirectX::Compress(FirstImage, ImageCount, ImageMetadata, ResultFormat, 0, DirectX::TEX_THRESHOLD_DEFAULT, *TemporaryImage);

			// Ensure success
			if (!FAILED(Result))
			{
				// Get the information
				auto& TextureInfo = TemporaryImage->GetMetadata();
				// Swap out our format
				ImageMetadata.format = TextureInfo.format;
				// Swap the temp image to normal one
				Image.reset(TemporaryImage.release());
			}
			else
			{
				// Failed to process
				return false;
			}
		}
	}

	// Stage 5: Saving to a file
	{
		// We need the image to save
		auto FirstImage = Image->GetImage(0, 0, 0);
		// Fetch image count, we only want one layer anyways
		auto ImageCount = 1;

		// Handle DDS format first, otherwise handle other types
		if (OutFormat > ImageFormat::DDS_WithHeader)
		{
			// Save it to a file
			DirectX::SaveToDDSFile(FirstImage, ImageCount, ImageMetadata, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, Strings::ToUnicodeString(OutputFile).c_str());
		}
		else
		{
			// Handle WIC and alternate formats
			switch (OutFormat)
			{
			case ImageFormat::Standard_TGA:
				// Write to a TGA file
				DirectX::SaveToTGAFile(FirstImage[0], Strings::ToUnicodeString(OutputFile).c_str());
				// We can exit out
				break;
			case ImageFormat::Standard_BMP:
				// Write to a BMP file
				DirectX::SaveToWICFile(FirstImage, ImageCount, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_BMP), Strings::ToUnicodeString(OutputFile).c_str(), nullptr, nullptr);
				// We can exit out
				break;
			case ImageFormat::Standard_GIF:
				// Write to a GIF file
				DirectX::SaveToWICFile(FirstImage, ImageCount, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_GIF), Strings::ToUnicodeString(OutputFile).c_str(), nullptr, nullptr);
				// We can exit out
				break;
			case ImageFormat::Standard_PNG:
				// Write to a PNG file
				DirectX::SaveToWICFile(FirstImage, ImageCount, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_PNG), Strings::ToUnicodeString(OutputFile).c_str(), nullptr, nullptr);
				// We can exit out
				break;
			case ImageFormat::Standard_JPEG:
				// Write to a JPEG file
				DirectX::SaveToWICFile(FirstImage, ImageCount, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_JPEG), Strings::ToUnicodeString(OutputFile).c_str(), nullptr, [&](IPropertyBag2* props)
				{
					// Setup JPEG compression quality
					PROPBAG2 options = {};
					VARIANT varValues = {};
					options.pstrName = L"ImageQuality";
					varValues.vt = VT_R4;
					varValues.fltVal = 1.f;
					// Write it
					(void)props->Write(1, &options, &varValues);
				});
				// We can exit out
				break;
			case ImageFormat::Standard_TIFF:
				// Write to a TIFF file
				DirectX::SaveToWICFile(FirstImage, ImageCount, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_TIFF), Strings::ToUnicodeString(OutputFile).c_str(), nullptr, [&](IPropertyBag2* props)
				{
					// Setup TIFF compression quality
					PROPBAG2 options = {};
					VARIANT varValues = {};
					options.pstrName = L"TiffCompressionMethod";
					varValues.vt = VT_UI1;
					varValues.bVal = WICTiffCompressionNone;
					// Write it
					(void)props->Write(1, &options, &varValues);
				});
				// We can exit out
				break;
			default:
				// We failed to find a format
				return false;
			}
		}
	}

	// If we got here, we were successful
	return true;
}

void Image::PatchNormalFromBumpmap(std::unique_ptr<DirectX::ScratchImage>& Image)
{
	// Patch a normalmap in bumpmap format, to a traditional normal
	auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
	// Transform it
	auto Result = DirectX::TransformImage(Image->GetImages(), Image->GetImageCount(), Image->GetMetadata(), [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
	{
		// Prepare to transform the pixels
		UNREFERENCED_PARAMETER(y);
		// Loop through scanlines
		for (size_t j = 0; j < width; j++)
		{
			// Get the scanline
			DirectX::XMVECTOR scanline = inPixels[j];

			// Basically, we can take G and A channels and assign them as XY for a normal, then set Z to 1.0, it's closer than most calcs
			float GreenVal = DirectX::XMVectorGetY(scanline);
			float AlphaVal = DirectX::XMVectorGetW(scanline);

			// Swap out the values
			float ResultRedValue = AlphaVal;
			float ResultGreenValue = GreenVal;

			// Set it
			scanline = DirectX::XMVectorSetX(scanline, ResultRedValue);
			scanline = DirectX::XMVectorSetY(scanline, ResultGreenValue);
			scanline = DirectX::XMVectorSetZ(scanline, 1.0);
			// Ensure alpha is 1.0
			scanline = DirectX::XMVectorSetW(scanline, 1.0);
			// Set the out pixel value to new line
			outPixels[j] = scanline;
		}
		// Done with image
	}, *TemporaryImage);
	// If we succeeded, swap out the image
	if (!FAILED(Result))
	{
		// Swap out the image
		Image.reset(TemporaryImage.release());
	}
}

void Image::PatchNormalFromCompressed(std::unique_ptr<DirectX::ScratchImage>& Image)
{
	// Patch a normalmap in compressed format (XY), to a traditional normal (XYZ)
	auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
	// Transform it
	auto Result = DirectX::TransformImage(Image->GetImages(), Image->GetImageCount(), Image->GetMetadata(), [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
	{
		// Prepare to transform the pixels
		UNREFERENCED_PARAMETER(y);
		// Loop through scanlines
		for (size_t j = 0; j < width; j++)
		{
			// Get the scanline
			DirectX::XMVECTOR scanline = inPixels[j];
			// Get the values
			float RedVal = DirectX::XMVectorGetX(scanline);
			float GreenVal = DirectX::XMVectorGetY(scanline);

			// Calculate the blue channel
			float nx = 2 * RedVal - 1;
			float ny = 2 * GreenVal - 1;
			float nz = 0.0f;

			// Check if we can average it
			if (1 - nx * nx - ny * ny > 0) nz = std::sqrtf(1 - nx * nx - ny * ny);

			// Calculate the final blue value and clamp it between 0 and 1
			float ResultBlueVal = VectorMath::Clamp<float>(((nz + 1) / 2.0f), 0, 1.0);

			// Set the new blue channel
			scanline = DirectX::XMVectorSetZ(scanline, ResultBlueVal);
			// Set the out pixel value to new line
			outPixels[j] = scanline;
		}
		// Done with image
	}, *TemporaryImage);
	// If we succeeded, swap out the image
	if (!FAILED(Result))
	{
		// Swap out the image
		Image.reset(TemporaryImage.release());
	}
}

void Image::PatchNormalCODFromNOG(std::unique_ptr<DirectX::ScratchImage>& Image)
{
	// Patch a normalmap in COD NOG (G: X, A: Y) format, to a traditional normal (XYZ)
	auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
	// Transform it
	auto Result = DirectX::TransformImage(Image->GetImages(), Image->GetImageCount(), Image->GetMetadata(), [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
	{
		// Prepare to transform the pixels
		UNREFERENCED_PARAMETER(y);
		// Loop through the scanlines
		for (size_t j = 0; j < width; j++)
		{
			// Get the scanline
			DirectX::XMVECTOR scanline = inPixels[j];
			// Get the values (For conversion)
			float RedVal = DirectX::XMVectorGetY(scanline);  // X
			float GreenVal = DirectX::XMVectorGetW(scanline); // Y
			// For NOG, we replace the red channel (X) with the alpha channel (Y), then set alpha to 1.0. Blue is the normal calc.

			// Calculate the blue channel
			float nx = 2 * RedVal - 1;
			float ny = 2 * GreenVal - 1;
			float nz = 0.0f;

			// Check if we can average it
			if (1 - nx * nx - ny * ny > 0) nz = std::sqrtf(1 - nx * nx - ny * ny);

			// Calculate the final blue value
			float ResultBlueVal = VectorMath::Clamp<float>(((nz + 1) / 2.0f), 0, 1.0);

			// Set red channel
			scanline = DirectX::XMVectorSetX(scanline, RedVal); // R
			// Set green channel
			scanline = DirectX::XMVectorSetY(scanline, GreenVal); // G
			// Set blue channel
			scanline = DirectX::XMVectorSetZ(scanline, ResultBlueVal); // B
			// Set alpha channel
			scanline = DirectX::XMVectorSetW(scanline, 1.0); // A

			// Set the out pixel value to new line
			outPixels[j] = scanline;
		}
		// Done with image
	}, *TemporaryImage);
	// If we succeeded, swap out the image
	if (!FAILED(Result))
	{
		// Swap out the image
		Image.reset(TemporaryImage.release());
	}
}

void Image::PatchAlphaChannel(std::unique_ptr<DirectX::ScratchImage>& Image)
{
	// Remove alpha channel by setting it to 1.0
	auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
	// Transform it
	auto Result = DirectX::TransformImage(Image->GetImages(), Image->GetImageCount(), Image->GetMetadata(), [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
	{
		// Prepare to transform the pixels
		UNREFERENCED_PARAMETER(y);
		// Loop through the scanlines
		for (size_t j = 0; j < width; j++)
		{
			// Get the scanline
			DirectX::XMVECTOR scanline = inPixels[j];
			
			// Set alpha channel
			scanline = DirectX::XMVectorSetW(scanline, 1.0);

			// Set the out pixel value to new line
			outPixels[j] = scanline;
		}
		// Done with image
	}, *TemporaryImage);
	// If we succeeded, swap out the image
	if (!FAILED(Result))
	{
		// Swap out the image
		Image.reset(TemporaryImage.release());
	}
}

void Image::WriteDDSHeaderToFile(BinaryWriter& Writer, uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, bool isCubemap)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildDDSHeader(Width, Height, MipLevels, Format, ResultSize, isCubemap);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Write it
		Writer.Write(HeaderBuffer.get(), ResultSize);
	}
}

void Image::WriteDDSHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, bool isCubemap)
{
	// Pass off
	WriteDDSHeaderToFile(*Writer.get(), Width, Height, MipLevels, Format, isCubemap);
}

void Image::WriteDDSHeaderToStream(int8_t* Buffer, uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, uint32_t& ResultSize, bool isCubemap)
{
	// The resulting header size
	ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildDDSHeader(Width, Height, MipLevels, Format, ResultSize, isCubemap);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Copy the data
		std::memcpy(Buffer, HeaderBuffer.get(), ResultSize);
	}
}

std::unique_ptr<int8_t[]> Image::BuildDDSHeader(uint32_t Width, uint32_t Height, uint32_t MipLevels, ImageFormat Format, uint32_t& ResultSize, bool isCubemap)
{
	// Allocate a buffer for storing the DDS header (Maximum size)
	uint32_t MaximumSize = sizeof(uint32_t) + sizeof(DirectX::DDS_HEADER) + sizeof(DirectX::DDS_HEADER_DXT10);
	// Allocate it
	auto HeaderBuffer = std::make_unique<int8_t[]>(MaximumSize);

	// Build texture metadata
	DirectX::TexMetadata Metadata;
	// Set
	Metadata.width = Width;
	Metadata.height = Height;
	Metadata.depth = 1;
	Metadata.arraySize = (isCubemap) ? 6 : 1;
	Metadata.mipLevels = MipLevels;
	Metadata.miscFlags = (isCubemap) ? DirectX::TEX_MISC_FLAG::TEX_MISC_TEXTURECUBE: 0;
	Metadata.miscFlags2 = 0;
	Metadata.dimension = DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D;

	// Whether or not to exclude alpha
	bool UseAlphaValue = true;

	// Check image format and output
	switch (Format)
	{
	case ImageFormat::DDS_BC1_UNORM:
	case ImageFormat::DDS_BC1_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
		// Formats are the same for these
		break;
	case ImageFormat::DDS_BC1_SRGB: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM_SRGB; break;

	case ImageFormat::DDS_BC2_UNORM:
	case ImageFormat::DDS_BC2_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM;
		// Formats are the same
		break;
	case ImageFormat::DDS_BC2_SRGB: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC2_UNORM_SRGB; break;

	case ImageFormat::DDS_BC3_UNORM:
	case ImageFormat::DDS_BC3_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM;
		// Formats are the same
		break;
	case ImageFormat::DDS_BC3_SRGB: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC3_UNORM_SRGB; break;

	case ImageFormat::DDS_BC4_UNORM:
	case ImageFormat::DDS_BC4_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC4_UNORM;
		// Formats are the same
		break;
	case ImageFormat::DDS_BC4_SNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC4_SNORM; break;

	case ImageFormat::DDS_BC5_UNORM:
	case ImageFormat::DDS_BC5_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
		// Formats are the same
		break;
	case ImageFormat::DDS_BC5_SNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC5_SNORM; break;

	case ImageFormat::DDS_BC6_UF16:
	case ImageFormat::DDS_BC6_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16;
		// Formats are the same
		break;
	case ImageFormat::DDS_BC6_SF16: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC6H_SF16; break;

	case ImageFormat::DDS_BC7_UNORM:
	case ImageFormat::DDS_BC7_TYPELESS:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM;
		// Formats are the same
		break;
	case ImageFormat::DDS_BC7_SRGB: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB; break;

	case ImageFormat::DDS_Standard_R8G8B8A8:
	case ImageFormat::DDS_Standard_A8R8G8B8:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		// Formats are the same
		break;

	case ImageFormat::DDS_Standard_R8G8B8:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		// Don't use alpha!
		UseAlphaValue = false;
		// Done
		break;

	case ImageFormat::DDS_Standard_A8R8G8B8_SRGB:
	case ImageFormat::DDS_Standard_R8G8B8A8_SRGB:
		// Output
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		// Format are the same
		break;

	case ImageFormat::DDS_Standard_R9G9B9E5: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R9G9B9E5_SHAREDEXP; break;
	case ImageFormat::DDS_Standard_R16G16B16A16_UNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_UNORM; break;
	case ImageFormat::DDS_Standard_R16G16B16A16_FLOAT: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT; break;
	case ImageFormat::DDS_Standard_R32G32B32A32_UNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_UINT; break;
	case ImageFormat::DDS_Standard_R32G32B32A32_FLOAT: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT; break;
	case ImageFormat::DDS_Standard_R10G10B10A2_UNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R10G10B10A2_UNORM; break;
	case ImageFormat::DDS_Standard_D16_UNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_D16_UNORM; break;
	case ImageFormat::DDS_Standard_A8_UNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_A8_UNORM; break;
	case ImageFormat::DDS_Standard_R8_UNORM: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R8_UNORM; break;
	case ImageFormat::DDS_Standard_R8_UINT: Metadata.format = DXGI_FORMAT::DXGI_FORMAT_R8_UINT; break;

	default:
		// Default to BC1
		Metadata.format = DXGI_FORMAT::DXGI_FORMAT_BC1_UNORM;
		// Done
		break;
	}

	// The resulting header size
	size_t ResultBuffer = 0;

	// Encode it
	DirectX::_EncodeDDSHeader(Metadata, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, HeaderBuffer.get(), MaximumSize, ResultBuffer, UseAlphaValue);

	// If we have a result size, write it to the stream
	if (ResultBuffer > 0)
	{
		// Set the size
		ResultSize = (uint32_t)ResultBuffer;
		// Return it
		return HeaderBuffer;
	}
	// Failed
	return nullptr;
}

const uint32_t Image::GetMaximumDDSHeaderSize()
{
	// Build size
	return sizeof(uint32_t) + sizeof(DirectX::DDS_HEADER) + sizeof(DirectX::DDS_HEADER_DXT10);;
}

void Image::SetupConversionThread()
{
	// Enable COM
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

void Image::DisableConversionThread()
{
	// Disable COM
	CoUninitialize();
}

void Image::InitializeImageAPI()
{
	// Just make sure we are initialized on the main thread, after COM
	bool iswic2 = true;
	(VOID)DirectX::GetWICFactory(iswic2);
}

void Image::ShutdownImageAPI()
{
	// Cleanup resources
	DirectX::CleanupWICFactory();
}