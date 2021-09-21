#include "stdafx.h"

// The class we are implementing
#include "CoDIWITranslator.h"

// We need the following Wraith classes
#include "Image.h"
#include "MemoryReader.h"

// -- Structures for reading

struct IWIHeader
{
    uint8_t Magic[3];
    uint8_t Version;
};

struct IWIInfo
{
    uint8_t ImageFormat;
    uint8_t ImageFlags;
    uint16_t ImageWidth;
    uint16_t ImageHeight;
};

struct IWIBigMips
{
    int32_t MipOffset1;
    int32_t MipOffset2;
    int32_t MipOffset3;
    int32_t MipOffset4;
    int32_t MipOffset5;
    int32_t MipOffset6;
    int32_t MipOffset7;
    int32_t MipOffset8;
};

struct IWISmallMips
{
    int32_t MipOffset1;
    int32_t MipOffset2;
    int32_t MipOffset3;
    int32_t MipOffset4;
};

enum class IWIFileFlags : uint8_t
{
    IMG_FLAG_CUBEMAP = 0x4
};

// -- Verify structures

static_assert(sizeof(IWIHeader) == 0x4, "Invalid IWIHeader Size (Expected 0x4)");
static_assert(sizeof(IWIInfo) == 0x6, "Invalid IWIInfo Size (Expected 0x6)");
static_assert(sizeof(IWISmallMips) == 0x10, "Invalid IWISmallMips Size (Expected 0x10)");
static_assert(sizeof(IWIBigMips) == 0x20, "Invalid IWIBigMips Size (Expected 0x20)");

// -- End reading structures

std::unique_ptr<XImageDDS> CoDIWITranslator::TranslateIWI(const std::unique_ptr<uint8_t[]>& IWIBuffer, uint32_t IWIBufferSize)
{
    // Prepare to parse the IWI
    auto Reader = MemoryReader((int8_t*)IWIBuffer.get(), IWIBufferSize, true);

    // Read the header
    auto Header = Reader.Read<IWIHeader>();

    // Check if we need to skip
    if (Header.Version == 0x8) { Reader.SetPosition(8); }

    // Read the info
    auto Info = Reader.Read<IWIInfo>();

    // Verify magic (IWi)
    if (Header.Magic[0] == 0x49 && Header.Magic[1] == 0x57 && Header.Magic[2] == 0x69)
    {
        // Buffers for IWI data
        int32_t OffsetToDump = -1, SizeToDump = -1;

        // Check version and read mip data, then calculate the size of DXT data
        if (Header.Version == 0x1B || Header.Version == 0x0D)
        {
            // Big mips
            if (Header.Version == 0x0D) { Reader.SetPosition(0x10); }
            if (Header.Version == 0x1B) { Reader.SetPosition(0x20); }

            auto BigMips = Reader.Read<IWIBigMips>();

            if (BigMips.MipOffset1 == BigMips.MipOffset2 || BigMips.MipOffset1 == BigMips.MipOffset8)
            {
                // Grab offet
                OffsetToDump = (int32_t)Reader.GetPosition();
            }
            else
            {
                // Offset is mip 2
                OffsetToDump = (BigMips.MipOffset2);
            }

            // Calculate size
            SizeToDump = (int32_t)(Reader.GetLength() - OffsetToDump);
        }
        else if (Header.Version == 0x6 || Header.Version == 0x8)
        {
            // Small mips
            if (Header.Version == 0x6) { Reader.SetPosition(0xC); }
            if (Header.Version == 0x8) { Reader.SetPosition(0x10); }

            auto SmallMips = Reader.Read<IWISmallMips>();

            if (SmallMips.MipOffset1 == SmallMips.MipOffset2 || SmallMips.MipOffset1 == SmallMips.MipOffset4)
            {
                // Grab ofset
                OffsetToDump = (int32_t)Reader.GetPosition();
            }
            else
            {
                // Offset is mip 2
                OffsetToDump = (SmallMips.MipOffset2);
            }

            // Calculate size
            SizeToDump = (int32_t)(Reader.GetLength() - OffsetToDump);
        }

        // Make sure we have offsets
        if (OffsetToDump > -1 && SizeToDump > -1)
        {
            // Set position
            Reader.SetPosition(OffsetToDump);

            // Calculate data type
            auto ImageDataFormat = ImageFormat::DDS_BC1_SRGB;
            // Check types
            switch (Info.ImageFormat)
            {
            case 0x1:
                ImageDataFormat = ImageFormat::DDS_Standard_A8R8G8B8;
                break;
            case 0x2:
                ImageDataFormat = ImageFormat::DDS_Standard_R8G8B8;
                break;
            case 0x3:
                ImageDataFormat = ImageFormat::DDS_Standard_D16_UNORM;
                break;
            case 0x4:
            case 0x5:
                ImageDataFormat = ImageFormat::DDS_Standard_A8_UNORM;
                break;
            case 0x0E:
                ImageDataFormat = ImageFormat::DDS_BC5_UNORM;
                break;
            case 0x0D:
                ImageDataFormat = ImageFormat::DDS_BC3_UNORM;
                break;
            case 0x0B:
                ImageDataFormat = ImageFormat::DDS_BC1_UNORM;
                break;
            case 0x0C:
                ImageDataFormat = ImageFormat::DDS_BC2_UNORM;
                break;
            default:
#ifdef _DEBUG
                // Log the format
                printf("CoDIWITranslator(): Unknown format: %d\n", Info.ImageFormat);
#endif
                break;
            }

            // Whether or not we are a cubemap
            bool isCubemap = false;

            // Validate file flags
            if ((Info.ImageFlags & (uint8_t)IWIFileFlags::IMG_FLAG_CUBEMAP) == (uint8_t)IWIFileFlags::IMG_FLAG_CUBEMAP)
                isCubemap = true;

            // Allocate a new result
            auto Result = std::make_unique<XImageDDS>();

            // Allocate buffer
            auto ImageBuffer = new int8_t[Image::GetMaximumDDSHeaderSize() + SizeToDump];

            // Result size
            uint32_t ResultSize = 0;
            // Write the header
            Image::WriteDDSHeaderToStream(ImageBuffer, Info.ImageWidth, Info.ImageHeight, 1, ImageDataFormat, ResultSize, isCubemap);

            // Read buffer to our stream
            Reader.Read(SizeToDump, ImageBuffer + ResultSize);

            // Correct the output
            // TODO: Bypass ImageFormat and use DXGI_FORMAT directly since there
            // is actually a DXGI_FORMAT that supports this, but for now it's easier
            // to quickly switch channels here due to it's limited use case
            if (Header.Version == 0x8 && Info.ImageFormat == 0x1)
            {
                // Convert from BGRA to RGBA
                auto Pixels = ImageBuffer + ResultSize;
                auto PixelCount = Info.ImageWidth * Info.ImageHeight;

                for (size_t Pixel = 0; Pixel < PixelCount; Pixel++)
                {
                    // 32Bbp, verify we're not outside the bounds
                    auto PixelOffset = Pixel * 4;

                    if (PixelOffset >= SizeToDump)
                    {
                        break;
                    }

                    auto NewR = Pixels[PixelOffset + 2];
                    auto NewG = Pixels[PixelOffset + 1];
                    auto NewB = Pixels[PixelOffset + 0];
                    auto NewA = Pixels[PixelOffset + 3];

                    Pixels[PixelOffset + 0] = NewR;
                    Pixels[PixelOffset + 1] = NewG;
                    Pixels[PixelOffset + 2] = NewB;
                    Pixels[PixelOffset + 3] = NewA;
                }
            }

            // Assign data
            Result->DataBuffer = ImageBuffer;
            Result->DataSize = (uint32_t)(ResultSize + SizeToDump);

            // Return it
            return Result;
        }
    }

    // Failed to convert
    return nullptr;
}