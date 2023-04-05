#include "stdafx.h"
#include "Texture.h"
#include "DDS.h"
#include "MathHelper.h"
#include <wincodec.h>

#include "..\cppkore_incl\DirectXTex\DirectXTex.h"
#include "Console.h"

#if _WIN64
#if _DEBUG
#pragma comment(lib, "..\\cppkore_libs\\DirectXTex\\DirectXTex_x64d.lib")
#else
#pragma comment(lib, "..\\cppkore_libs\\DirectXTex\\DirectXTex_x64r.lib")
#endif
#else
#error DirectXTex doesn't support non x64 builds yet
#endif

#define InternalScratchImage ((DirectX::ScratchImage*)this->DirectXImage)

namespace Assets
{
	Texture::Texture(void* ScratchImage)
		: DirectXImage(ScratchImage)
	{
	}

	Texture::Texture(uint32_t Width, uint32_t Height)
		: Texture(Width, Height, DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
	{
	}

	Texture::Texture(uint32_t Width, uint32_t Height, DXGI_FORMAT Format)
		: DirectXImage(nullptr)
	{
		this->DirectXImage = new DirectX::ScratchImage();

		DirectX::TexMetadata Data{};
		Data.width = Width;
		Data.height = Height;
		Data.depth = 1;
		Data.arraySize = 1;
		Data.mipLevels = 1;
		Data.dimension = DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D;
		Data.format = Format;
		InternalScratchImage->Initialize(Data);
	}

	Texture::~Texture()
	{
		if (DirectXImage)
			delete InternalScratchImage;
		DirectXImage = nullptr;
	}

	Texture::Texture(Texture&& Rhs) noexcept
	{
		this->DirectXImage = Rhs.DirectXImage;
		Rhs.DirectXImage = nullptr;
	}

	const uint32_t Texture::Width() const
	{
		return (uint32_t)InternalScratchImage->GetMetadata().width;
	}

	const uint32_t Texture::Height() const
	{
		return (uint32_t)InternalScratchImage->GetMetadata().height;
	}

	const uint32_t Texture::Pitch() const
	{
		return (uint32_t)InternalScratchImage->GetImages()->rowPitch;
	}

	const uint32_t Texture::MipCount() const
	{
		return (uint32_t)InternalScratchImage->GetMetadata().mipLevels;
	}

	const uint32_t Texture::BlockSize() const
	{
		return (uint32_t)InternalScratchImage->GetPixelsSize();
	}

	uint8_t* Texture::GetPixels()
	{
		return InternalScratchImage->GetImages()->pixels;
	}

	const uint8_t* Texture::GetPixels() const
	{
		return InternalScratchImage->GetImages()->pixels;
	}

	void Texture::Invert()
	{
		if (InternalScratchImage == nullptr)
			return;

		if(this->Format() != DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

		uint8_t* pixel = GetPixels();
		for (int i = 0; i < InternalScratchImage->GetImages()->height; ++i) {
			for (int j = 0; j < InternalScratchImage->GetImages()->width; ++j) {
				pixel[0] = (uint8_t)0xFF - pixel[0];
				pixel[1] = (uint8_t)0xFF - pixel[1];
				pixel[2] = (uint8_t)0xFF - pixel[2];
				pixel += 4;
			}
		}
	}

	void Texture::OnebyOneWhite()
	{
		this->ConvertToFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
		uint8_t* pixel = GetPixels();
		for (int i = 0; i < InternalScratchImage->GetImages()->height; ++i) {
			for (int j = 0; j < InternalScratchImage->GetImages()->width; ++j) {
				pixel[0] = (uint8_t)0;
				pixel[1] = (uint8_t)0;
				pixel[2] = (uint8_t)0;
				pixel[4] = (uint8_t)0;
				pixel += 4;
			}
		}
	}

	void Texture::MakeTransparent()
	{
		this->ConvertToFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
		uint8_t* pixel = GetPixels();
		for (int i = 0; i < InternalScratchImage->GetImages()->height; ++i) {
			for (int j = 0; j < InternalScratchImage->GetImages()->width; ++j) {
				pixel[4] = (uint8_t)0x00;
				pixel += 4;
			}
		}
	}

	const DXGI_FORMAT Texture::Format() const
	{
		return InternalScratchImage->GetMetadata().format;
	}

	const uint8_t Texture::GetBpp() const
	{
		DXGI_FORMAT format = this->Format();
		return DirectX::BitsPerPixel(format);
	}

	const uint8_t Texture::Pixbl() const
	{
		uint32_t format = this->Format();

		if ((format >= 70 && format <= 84) || format >= 94 && format <= 99)
			return 4;
		return 1;
	}

	// this function assumes that both of the textures are in the same format
	void Texture::CopyTextureSlice(std::unique_ptr<Texture>& SourceTexture, DirectX::Rect srcRect, uint32_t x, uint32_t y)
	{
		DirectX::CopyRectangle(*((DirectX::ScratchImage*)SourceTexture->DirectXImage)->GetImage(0, 0, 0), srcRect, *InternalScratchImage->GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, x, y);
	}

	void Texture::ConvertToFormat(DXGI_FORMAT Format)
	{
		auto fmt = InternalScratchImage->GetMetadata().format;
		if (InternalScratchImage->GetMetadata().format == Format)
			return;

		// If we need to decompress, and our target is not compressed, we can skip a loop here by directly going to the target
		DXGI_FORMAT DecompressFmt = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		if (!DirectX::IsCompressed(Format))
			DecompressFmt = Format;

		// First, check if we are currently compressed...
		if (DirectX::IsCompressed(InternalScratchImage->GetMetadata().format))
		{
			auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
			auto Result = DirectX::Decompress(InternalScratchImage->GetImages(), InternalScratchImage->GetImageCount(), InternalScratchImage->GetMetadata(), DecompressFmt, *TemporaryImage);

			if (SUCCEEDED(Result))
			{
				delete InternalScratchImage;
				DirectXImage = TemporaryImage.release();
			}
			else
			{
				throw std::exception("Failed to decompress the image format");
			}
		}

		// If the target is compressed, compress the new data
		if (DirectX::IsCompressed(Format))
		{
			auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
			auto Result = DirectX::Compress(InternalScratchImage->GetImages(), InternalScratchImage->GetImageCount(), InternalScratchImage->GetMetadata(), Format, DirectX::TEX_COMPRESS_FLAGS::TEX_COMPRESS_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, *TemporaryImage);

			if (SUCCEEDED(Result))
			{
				delete InternalScratchImage;
				DirectXImage = TemporaryImage.release();
			}
			else
			{
				throw std::exception("Failed to compress the image format");
			}
		}
		else if (InternalScratchImage->GetMetadata().format != Format)
		{
			auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
			auto Result = DirectX::Convert(InternalScratchImage->GetImages(), InternalScratchImage->GetImageCount(), InternalScratchImage->GetMetadata(), Format, DirectX::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, *TemporaryImage);

			if (SUCCEEDED(Result))
			{
				delete InternalScratchImage;
				DirectXImage = TemporaryImage.release();
			}
			else
			{
				throw std::exception("Failed to convert the image format");
			}
		}
	}

	bool Texture::ConvertToFormat(ImageFormat OutFormat, ImagePatch Patch)
	{
		if (InternalScratchImage == nullptr)
			return false;

		auto ImageMetadata = InternalScratchImage->GetMetadata();

		// Stage 1: Check if the image is planar, if so, convert to a single plane
		if (DirectX::IsPlanar(ImageMetadata.format))
		{
			// Fetch first image
			auto FirstImage = InternalScratchImage->GetImage(0, 0, 0);
			// Fetch image count
			auto ImageCount = InternalScratchImage->GetImageCount();
			// Allocate a temporary buffer
			auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

			// Convert to normal plane
			auto Result = DirectX::ConvertToSinglePlane(FirstImage, ImageCount, ImageMetadata, *TemporaryImage);

			// Ensure success
			if (SUCCEEDED(Result))
			{
				// Get the information
				auto& TextureInfo = TemporaryImage->GetMetadata();
				// Swap out our format
				ImageMetadata.format = TextureInfo.format;
				// Swap the temp image to normal one
				delete InternalScratchImage;
				DirectXImage = TemporaryImage.release();
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
			auto FirstImage = ((DirectX::ScratchImage*)this->DirectXImage)->GetImage(0, 0, 0);
			// Fetch image count
			auto ImageCount = InternalScratchImage->GetImageCount();
			// Allocate a temporary buffer
			auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

			// We must use this format for global support, it's a standard for images
			DXGI_FORMAT ResultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			// Decompress the image
			auto Result = DirectX::Decompress(FirstImage, ImageCount, ImageMetadata, ResultFormat, *TemporaryImage);

			// Ensure success
			if (SUCCEEDED(Result))
			{
				// Get the information
				auto& TextureInfo = TemporaryImage->GetMetadata();
				// Swap out our format
				ImageMetadata.format = TextureInfo.format;
				// Swap the temp image to normal one
				delete InternalScratchImage;
				DirectXImage = TemporaryImage.release();
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
			auto Result = DirectX::Convert(InternalScratchImage->GetImages(), InternalScratchImage->GetImageCount(), InternalScratchImage->GetMetadata(), ResultFormat, DirectX::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, *TemporaryImage);

			// Ensure success
			if (!FAILED(Result))
			{
				// Get the information
				auto& TextureInfo = TemporaryImage->GetMetadata();
				// Swap out our format
				ImageMetadata.format = TextureInfo.format;
				// Swap the temp image to normal one
				delete InternalScratchImage;
				DirectXImage = TemporaryImage.release();
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
			case ImagePatch::Normal_Bumpmap: PatchNormalFromBumpmap(); break;
			case ImagePatch::Normal_Expand: PatchNormalFromCompressed(); break;
			case ImagePatch::Normal_COD_NOG: PatchNormalCODFromNOG(); break;
			case ImagePatch::Color_StripAlpha: PatchAlphaChannel(); break;
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
				auto FirstImage = InternalScratchImage->GetImage(0, 0, 0);
				// Fetch image count
				auto ImageCount = InternalScratchImage->GetImageCount();
				// Allocate a temporary image
				auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();

				// Compress the image TODO: Replace 0 with proper flags
				auto Result = DirectX::Compress(
					FirstImage,
					ImageCount,
					ImageMetadata,
					ResultFormat,
					DirectX::TEX_COMPRESS_DEFAULT,
					DirectX::TEX_THRESHOLD_DEFAULT,
					*TemporaryImage);

				// Ensure success
				if (SUCCEEDED(Result))
				{
					// Get the information
					auto& TextureInfo = TemporaryImage->GetMetadata();
					// Swap out our format
					ImageMetadata.format = TextureInfo.format;
					// Swap the temp image to normal one
					delete InternalScratchImage;
					DirectXImage = TemporaryImage.release();
				}
				else
				{
					// Failed to process
					return false;
				}
			}
		}
		// If we got here, we were successful
		return true;
	}

	void Texture::PatchNormalFromBumpmap()
	{
		// Ensure R8G8B8A8 value and ordering...
		if (this->Format() != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		// Patch a normalmap in bumpmap format, to a traditional normal
		auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
		auto CurrentResult = (DirectX::ScratchImage*)this->DirectXImage;
		// Transform it
		auto Result = DirectX::TransformImage(CurrentResult->GetImages(), CurrentResult->GetImageCount(), CurrentResult->GetMetadata(), [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
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
		if (SUCCEEDED(Result))
		{
			delete (DirectX::ScratchImage*)this->DirectXImage;
			this->DirectXImage = TemporaryImage.release();
		}
	}

	void Texture::PatchNormalFromCompressed()
	{
		// Ensure R8G8B8A8 value and ordering...
		if (this->Format() != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		// Patch a normalmap in compressed format (XY), to a traditional normal (XYZ)
		auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
		auto CurrentResult = (DirectX::ScratchImage*)this->DirectXImage;
		// Transform it
		auto Result = DirectX::TransformImage(CurrentResult->GetImages(), CurrentResult->GetImageCount(), CurrentResult->GetMetadata(), [&](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
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
					float ResultBlueVal = Math::MathHelper::Clamp<float>(((nz + 1) / 2.0f), 0, 1.0);

					// Set the new blue channel
					scanline = DirectX::XMVectorSetZ(scanline, ResultBlueVal);
					// Set the out pixel value to new line
					outPixels[j] = scanline;
				}
				// Done with image
			}, *TemporaryImage);
		// If we succeeded, swap out the image
		if (SUCCEEDED(Result))
		{
			delete (DirectX::ScratchImage*)this->DirectXImage;
			this->DirectXImage = TemporaryImage.release();
		}
	}

	void Texture::PatchNormalCODFromNOG()
	{
		// Ensure R8G8B8A8 value and ordering...
		if (this->Format() != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);
		// Patch a normalmap in COD NOG (G: X, A: Y) format, to a traditional normal (XYZ)
		auto TemporaryImage = std::make_unique<DirectX::ScratchImage>();
		auto CurrentResult = (DirectX::ScratchImage*)this->DirectXImage;
		// Transform it
		auto Result = DirectX::TransformImage(CurrentResult->GetImages(), CurrentResult->GetImageCount(), CurrentResult->GetMetadata(), 
			[](DirectX::XMVECTOR* outPixels, const DirectX::XMVECTOR* inPixels, size_t width, size_t y)
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
					float ResultBlueVal = Math::MathHelper::Clamp<float>(((nz + 1) / 2.0f), 0, 1.0);

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
		if (SUCCEEDED(Result))
		{
			delete (DirectX::ScratchImage*)this->DirectXImage;
			this->DirectXImage = TemporaryImage.release();
		}
	}

	void Texture::PatchAlphaChannel()
	{
		// Ensure R8G8B8A8 value and ordering...
		if (this->Format() != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		// Create a temporary image
		auto TemporaryResult = std::make_unique<DirectX::ScratchImage>();
		auto CurrentResult = (DirectX::ScratchImage*)this->DirectXImage;

		// Transform it
		auto TranscodeResult = DirectX::TransformImage(CurrentResult->GetImages(), CurrentResult->GetImageCount(), CurrentResult->GetMetadata(),
			[](DirectX::XMVECTOR* OutPixels, const DirectX::XMVECTOR* InPixels, size_t Width, size_t Slice)
			{
				UNREFERENCED_PARAMETER(Slice);
				// Loop through the scanlines
				for (size_t j = 0; j < Width; j++)
				{
					// Get the scanline
					DirectX::XMVECTOR scanline = InPixels[j];

					// Set alpha channel
					scanline = DirectX::XMVectorSetW(scanline, 1.0);

					// Set the out pixel value to new line
					OutPixels[j] = scanline;
				}
				// Done with image
			}, * TemporaryResult);
		// If it succeeds, we must then
		if (SUCCEEDED(TranscodeResult))
		{
			delete (DirectX::ScratchImage*)this->DirectXImage;
			this->DirectXImage = TemporaryResult.release();
		}
	}

	void Texture::Transcode(TranscodeType Type)
	{
		// Depending on the transcode type, we need to ensure a format...
		switch (Type)
		{
		case TranscodeType::NormalMapBC5:
			this->Transcoder_NormalMapBC5();
			break;
		case TranscodeType::NormalMapBC5OpenGl:
			this->Transcoder_NormalMapBC5OpenGl();
			break;
		case TranscodeType::ColorMap:
			this->PatchAlphaChannel();
			break;
		}
	}

	void Texture::Save(const string& File)
	{
		if (File.EndsWith(".dds"))
			this->Save(File, SaveFileType::Dds);
		else if (File.EndsWith(".tga"))
			this->Save(File, SaveFileType::Tga);
		else if (File.EndsWith(".tiff"))
			this->Save(File, SaveFileType::Tiff);
		else if (File.EndsWith(".hdr"))
			this->Save(File, SaveFileType::Hdr);
		else if (File.EndsWith(".png"))
			this->Save(File, SaveFileType::Png);
		else if (File.EndsWith(".bmp"))
			this->Save(File, SaveFileType::Bmp);
		else if (File.EndsWith(".jpg"))
			this->Save(File, SaveFileType::Jpeg);
		else if (File.EndsWith(".jxr"))
			this->Save(File, SaveFileType::Jxr);
		else if (File.EndsWith(".gif"))
			this->Save(File, SaveFileType::Gif);
		else
			throw std::exception("Unknown image file extension");
	}

	void Texture::Save(const string& File, SaveFileType Type)
	{
		this->EnsureFormatForType(Type);
		HRESULT SaveResult = 0;

		auto OutputWide = File.ToWString();

		switch (Type)
		{
		case SaveFileType::Dds:
			SaveResult = DirectX::SaveToDDSFile(InternalScratchImage->GetImages(), InternalScratchImage->GetImageCount(), InternalScratchImage->GetMetadata(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, (const wchar_t*)OutputWide);
			break;
		case SaveFileType::Tga:
			SaveResult = DirectX::SaveToTGAFile(*InternalScratchImage->GetImages(), (const wchar_t*)OutputWide);
			break;
		case SaveFileType::Hdr:
			SaveResult = DirectX::SaveToHDRFile(*InternalScratchImage->GetImages(), (const wchar_t*)OutputWide);
			break;
		default:
		{
			// This handles all WIC-Codec based formats
			auto Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_BMP);
			std::function<void __cdecl(IPropertyBag2*)> PropertyWriter = nullptr;

			switch (Type)
			{
			case SaveFileType::Gif:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_GIF);
				break;
			case SaveFileType::Jpeg:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_JPEG);
				PropertyWriter = [](IPropertyBag2* props)
				{
					PROPBAG2 options{};
					VARIANT varValues{};
					options.pstrName = (LPOLESTR)L"ImageQuality";
					varValues.vt = VT_R4;
					varValues.fltVal = 1.f;

					(void)props->Write(1, &options, &varValues);
				};
				break;
			case SaveFileType::Jxr:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_WMP);
				break;
			case SaveFileType::Png:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_PNG);
				PropertyWriter = [](IPropertyBag2* props)
				{
					PROPBAG2 options{};
					VARIANT varValues{};
					options.pstrName = (LPOLESTR)L"FilterOption";
					varValues.vt = VT_UI1;
					varValues.bVal = WICPngFilterOption::WICPngFilterUp;

					(void)props->Write(1, &options, &varValues);
				};
				break;
			case SaveFileType::Tiff:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_TIFF);
				PropertyWriter = [](IPropertyBag2* props)
				{
					PROPBAG2 options{};
					VARIANT varValues{};
					options.pstrName = (LPOLESTR)L"TiffCompressionMethod";
					varValues.vt = VT_UI1;
					varValues.bVal = WICTiffCompressionOption::WICTiffCompressionDontCare;

					(void)props->Write(1, &options, &varValues);
				};
				break;
			}

			SaveResult = DirectX::SaveToWICFile(*InternalScratchImage->GetImages(), DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB, Wc, (const wchar_t*)OutputWide, nullptr, PropertyWriter);
		}
		break;
		}

		if (FAILED(SaveResult))
			throw std::exception("An error occured while saving the image");
	}

	void Texture::Save(IO::Stream& Stream, SaveFileType Type)
	{
		this->EnsureFormatForType(Type);
		DirectX::Blob TemporaryBuffer;

		SaveToMemoryBlob(&TemporaryBuffer, Type);

		Stream.Write((uint8_t*)TemporaryBuffer.GetBufferPointer(), 0, (uint64_t)TemporaryBuffer.GetBufferSize());
	}

	void Texture::Save(uint8_t* Buffer, uint64_t BufferLength, SaveFileType Type)
	{
		this->EnsureFormatForType(Type);
		DirectX::Blob TemporaryBuffer;

		SaveToMemoryBlob(&TemporaryBuffer, Type);

		std::memcpy(Buffer, TemporaryBuffer.GetBufferPointer(), min(TemporaryBuffer.GetBufferSize(), BufferLength));
	}

	Texture Texture::FromFile(const string& File)
	{
		Texture Result;
		DirectX::TexMetadata MetaData;
		HRESULT LoadResult = 0;

		Result.DirectXImage = new DirectX::ScratchImage();

		auto WidePath = File.ToWString();
		if (File.EndsWith(".dds"))
			LoadResult = DirectX::LoadFromDDSFile((const wchar_t*)WidePath, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
		else if (File.EndsWith(".tga"))
			LoadResult = DirectX::LoadFromTGAFile((const wchar_t*)WidePath, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
		else if (File.EndsWith(".hdr"))
			LoadResult = DirectX::LoadFromHDRFile((const wchar_t*)WidePath, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
		else
			LoadResult = DirectX::LoadFromWICFile((const wchar_t*)WidePath, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);

		if (FAILED(LoadResult))
		{
			delete (DirectX::ScratchImage*)Result.DirectXImage;
			Result.DirectXImage = nullptr;

			throw std::exception("An error occured loading the image");
		}

		return Result;
	}

	Texture Texture::FromFile(const string& File, TextureType Type)
	{
		Texture Result;
		DirectX::TexMetadata MetaData;
		HRESULT LoadResult = 0;

		Result.DirectXImage = new DirectX::ScratchImage();

		auto WidePath = File.ToWString();
		switch (Type)
		{
		case TextureType::DDS:
			LoadResult = DirectX::LoadFromDDSFile((const wchar_t*)WidePath, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::TGA:
			LoadResult = DirectX::LoadFromTGAFile((const wchar_t*)WidePath, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::HDR:
			LoadResult = DirectX::LoadFromHDRFile((const wchar_t*)WidePath, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::WIC:
			LoadResult = DirectX::LoadFromWICFile((const wchar_t*)WidePath, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		}

		if (FAILED(LoadResult))
		{
			delete (DirectX::ScratchImage*)Result.DirectXImage;
			Result.DirectXImage = nullptr;

			throw std::exception("An error occured loading the image");
		}

		return Result;
	}

	Texture Texture::FromStream(IO::Stream& Stream, TextureType Type)
	{
		Texture Result;
		DirectX::TexMetadata MetaData;
		HRESULT LoadResult = 0;

		Result.DirectXImage = new DirectX::ScratchImage();

		auto ScratchBuffer = std::make_unique<uint8_t[]>(Stream.GetLength());
		Stream.Read(ScratchBuffer.get(), 0, Stream.GetLength());

		switch (Type)
		{
		case TextureType::DDS:
			LoadResult = DirectX::LoadFromDDSMemory(ScratchBuffer.get(), Stream.GetLength(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::TGA:
			LoadResult = DirectX::LoadFromTGAMemory(ScratchBuffer.get(), Stream.GetLength(), &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::HDR:
			LoadResult = DirectX::LoadFromHDRMemory(ScratchBuffer.get(), Stream.GetLength(), &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::WIC:
			LoadResult = DirectX::LoadFromWICMemory(ScratchBuffer.get(), Stream.GetLength(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		}
		
		if (FAILED(LoadResult))
		{
			delete (DirectX::ScratchImage*)Result.DirectXImage;
			Result.DirectXImage = nullptr;

			throw std::exception("An error occured loading the image");
		}

		return Result;
	}

	Texture Texture::FromBuffer(uint8_t* Buffer, uint64_t BufferLength, TextureType Type)
	{
		Texture Result;
		DirectX::TexMetadata MetaData;
		HRESULT LoadResult = 0;

		Result.DirectXImage = new DirectX::ScratchImage();

		switch (Type)
		{
		case TextureType::DDS:
			LoadResult = DirectX::LoadFromDDSMemory(Buffer, BufferLength, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::TGA:
			LoadResult = DirectX::LoadFromTGAMemory(Buffer, BufferLength, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::HDR:
			LoadResult = DirectX::LoadFromHDRMemory(Buffer, BufferLength, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		case TextureType::WIC:
			LoadResult = DirectX::LoadFromWICMemory(Buffer, BufferLength, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &MetaData, *(DirectX::ScratchImage*)Result.DirectXImage);
			break;
		}

		if (FAILED(LoadResult))
		{
			delete (DirectX::ScratchImage*)Result.DirectXImage;
			Result.DirectXImage = nullptr;

			//throw std::exception("An error occured loading the image");
		}

		return Result;
	}

	Texture Texture::FromRawBlock(uint8_t* Buffer, uint64_t BufferLength, uint32_t Width, uint32_t Height, DXGI_FORMAT Format)
	{
		DDSFormat BlockFormat{};
		BlockFormat.Format = Format;

		auto ExpectedBlockSize = Assets::DDS::CalculateBlockSize(Width, Height, BlockFormat);

		if (BufferLength < ExpectedBlockSize)
			throw std::exception("Invalid buffer length specified");

		auto Result = Texture(Width, Height, Format);

		std::memcpy(((DirectX::ScratchImage*)Result.DirectXImage)->GetImages()->pixels, Buffer, ExpectedBlockSize);

		return Result;
	}

	int Texture::Morton(uint32_t i, uint32_t sx, uint32_t sy)
	{
		int v0 = 1;
		int v1 = 1;
		int v2 = i;
		int v3 = sx;
		int v4 = sy;
		int v5 = 0;
		int v6 = 0;
		while (v3 > 1 || v4 > 1)
		{
			if (v3 > 1)
			{
				v5 += v1 * (v2 & 1);
				v2 >>= 1;
				v1 *= 2;
				v3 >>= 1;
			}
			if (v4 > 1)
			{
				v6 += v0 * (v2 & 1);
				v2 >>= 1;
				v0 *= 2;
				v4 >>= 1;
			}
		}
		return v6 * sx + v5;
	}

	Texture::Texture()
		: DirectXImage(nullptr)
	{
	}

	void Texture::EnsureFormatForType(SaveFileType Type)
	{
		switch (Type)
		{
			// These formats require a 32bpp, so B8G8R8A8_UNORM - like format to encode (Only transcode if not 32bpp)
		case SaveFileType::Png:
		case SaveFileType::Bmp:
		case SaveFileType::Jpeg:
		case SaveFileType::Tiff:
		case SaveFileType::Gif:
		case SaveFileType::Jxr:
		case SaveFileType::Tga:
			if (!IsValid32bppFormat(InternalScratchImage->GetMetadata().format))
			{
				if (DirectX::IsSRGB(InternalScratchImage->GetMetadata().format))
					this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
				else
					this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM);
			}
			break;
			// This format requires either R32G32B32A32_FLOAT or R32G32B32_FLOAT
		case SaveFileType::Hdr:
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT);
			break;
		}
	}

	void Texture::SaveToMemoryBlob(void* Blob, SaveFileType Type)
	{
		HRESULT SaveResult = 0;

		switch (Type)
		{
		case SaveFileType::Dds:
			SaveResult = DirectX::SaveToDDSMemory(InternalScratchImage->GetImages(), InternalScratchImage->GetImageCount(), InternalScratchImage->GetMetadata(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, *(DirectX::Blob*)Blob);
			break;
		case SaveFileType::Tga:
			SaveResult = DirectX::SaveToTGAMemory(*InternalScratchImage->GetImages(), *(DirectX::Blob*)Blob);
			break;
		case SaveFileType::Hdr:
			SaveResult = DirectX::SaveToHDRMemory(*InternalScratchImage->GetImages(), *(DirectX::Blob*)Blob);
			break;
		default:
		{
			// This handles all WIC-Codec based formats
			auto Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_BMP);
			std::function<void __cdecl(IPropertyBag2*)> PropertyWriter = nullptr;

			switch (Type)
			{
			case SaveFileType::Gif:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_GIF);
				break;
			case SaveFileType::Jpeg:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_JPEG);
				PropertyWriter = [](IPropertyBag2 * props)
				{
					PROPBAG2 options{};
					VARIANT varValues{};
					options.pstrName = (LPOLESTR)L"ImageQuality";
					varValues.vt = VT_R4;
					varValues.fltVal = 1.f;

					(void)props->Write(1, &options, &varValues);
				};
				break;
			case SaveFileType::Jxr:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_WMP);
				break;
			case SaveFileType::Png:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_PNG);
				break;
			case SaveFileType::Tiff:
				Wc = DirectX::GetWICCodec(DirectX::WICCodecs::WIC_CODEC_TIFF);
				PropertyWriter = [](IPropertyBag2 * props)
				{
					PROPBAG2 options{};
					VARIANT varValues{};
					options.pstrName = (LPOLESTR)L"TiffCompressionMethod";
					varValues.vt = VT_UI1;
					varValues.bVal = WICTiffCompressionOption::WICTiffCompressionDontCare;

					(void)props->Write(1, &options, &varValues);
				};
				break;
			}

			SaveResult = DirectX::SaveToWICMemory(*InternalScratchImage->GetImages(), DirectX::WIC_FLAGS::WIC_FLAGS_NONE, Wc, *(DirectX::Blob*)Blob, nullptr, PropertyWriter);
		}
		break;
		}

		if (FAILED(SaveResult))
			throw std::exception("An error occured while saving the image");
	}

	void Texture::Transcoder_NormalMapBC5()
	{
		// Ensure R8G8B8A8 value and ordering...
		if (this->Format() != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		// Create a temporary image
		auto TemporaryResult = std::make_unique<DirectX::ScratchImage>();
		auto CurrentResult = (DirectX::ScratchImage*)this->DirectXImage;

		auto TranscodeResult = DirectX::TransformImage(CurrentResult->GetImages(), CurrentResult->GetImageCount(), CurrentResult->GetMetadata(),
			[](DirectX::XMVECTOR* OutPixels, const DirectX::XMVECTOR* InPixels, size_t Width, size_t Slice)
			{
				for (size_t i = 0; i < Width; i++)
				{
					auto Scanline = InPixels[i];

					auto Red = DirectX::XMVectorGetX(Scanline);
					auto Green = DirectX::XMVectorGetY(Scanline);

					// Calculate the blue channel
					float NormalX = 2 * Red - 1;
					float NormalY = 2 * Green - 1;
					float NormalZ = 0.0f;

					// Check if we can average it
					if (1 - NormalX * NormalX - NormalY * NormalY > 0)
					{
						NormalZ = std::sqrtf(1 - NormalX * NormalX - NormalY * NormalY);
					}

					// Calculate the final blue value and clamp it between 0 and 1
					float ResultBlueVal = Math::MathHelper::Clamp<float>(((NormalZ + 1) / 2.0f), 0, 1.0);

					OutPixels[i] = DirectX::XMVectorSetZ(Scanline, ResultBlueVal);
				}
			}, *TemporaryResult);

		// If it succeeds, we must then
		if (SUCCEEDED(TranscodeResult))
		{
			delete (DirectX::ScratchImage*)this->DirectXImage;
			this->DirectXImage = TemporaryResult.release();
		}
	}

	void Texture::Transcoder_NormalMapBC5OpenGl()
	{
		// Ensure R8G8B8A8 value and ordering...
		if (this->Format() != DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM)
			this->ConvertToFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM);

		// Create a temporary image
		auto TemporaryResult = std::make_unique<DirectX::ScratchImage>();
		auto CurrentResult = (DirectX::ScratchImage*)this->DirectXImage;

		auto TranscodeResult = DirectX::TransformImage(CurrentResult->GetImages(), CurrentResult->GetImageCount(), CurrentResult->GetMetadata(),
			[](DirectX::XMVECTOR* OutPixels, const DirectX::XMVECTOR* InPixels, size_t Width, size_t Slice)
			{
				for (size_t i = 0; i < Width; i++)
				{
					auto Scanline = InPixels[i];

					auto Red = DirectX::XMVectorGetX(Scanline);
					auto Green = DirectX::XMVectorGetY(Scanline);

					// Invert G (Y) to match DirectX
					Green = 1.0f - Green;
					Scanline = DirectX::XMVectorSetY(Scanline, Green);

					// Calculate the blue channel
					float NormalX = 2 * Red - 1;
					float NormalY = 2 * Green - 1;
					float NormalZ = 0.0f;

					// Check if we can average it
					if (1 - NormalX * NormalX - NormalY * NormalY > 0)
					{
						NormalZ = std::sqrtf(1 - NormalX * NormalX - NormalY * NormalY);
					}

					// Calculate the final blue value and clamp it between 0 and 1
					float ResultBlueVal = Math::MathHelper::Clamp<float>(((NormalZ + 1) / 2.0f), 0, 1.0);

					OutPixels[i] = DirectX::XMVectorSetZ(Scanline, ResultBlueVal);
				}
			}, *TemporaryResult);

		// If it succeeds, we must then
		if (SUCCEEDED(TranscodeResult))
		{
			delete (DirectX::ScratchImage*)this->DirectXImage;
			this->DirectXImage = TemporaryResult.release();
		}
	}

	bool Texture::IsValid32bppFormat(DXGI_FORMAT Format)
	{
		switch (Format)
		{
		case DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT::DXGI_FORMAT_B8G8R8X8_UNORM:
			return true;
		default:
			return false;
		}
	}

	bool Texture::ImageValid()
	{
		if (InternalScratchImage != nullptr)
			return true;

		return false;
	}
}