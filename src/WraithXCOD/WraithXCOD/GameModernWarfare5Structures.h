#pragma once
#include <stdint.h>

#pragma pack(push, 1)
struct MW5XModelLod
{
    uint64_t MeshPtr;
    uint64_t SurfsPtr;

    float LodDistance[3];

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[40];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XModel
{
    uint64_t Hash;
    uint64_t NamePtr;
    uint16_t NumSurfaces;
    uint8_t NumLods;
    uint8_t MaxLods;

    uint8_t Padding[12];

    uint8_t NumBones;
    uint8_t NumRootBones;
    uint16_t UnkBoneCount;

    uint8_t Padding3[0x8C];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t UnknownPtr2;
    uint64_t MaterialHandlesPtr;
    uint64_t ModelLods;

    uint8_t Padding4[112];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XModelMesh
{
    uint64_t Hash;
    uint64_t SurfsPtr;
    uint64_t LODStreamKey;
    uint8_t Padding[0x8];

    uint64_t MeshBufferPointer;
    uint16_t NumSurfs;
    uint8_t Padding2[38];

};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XModelMeshBufferInfo
{
    uint64_t BufferPtr; // 0 if the model is not loaded, otherwise a pointer, even if streamed
    uint32_t BufferSize;
    uint32_t Streamed;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XModelSurface
{
    uint16_t StatusFlag;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t Padding6;
    uint16_t Padding7;
    uint16_t Padding8;
    uint8_t VertListCount;
    uint8_t Padding0[19];
    uint16_t WeightCounts[8];
    uint8_t Padding1[4];
    float NewScale;
    uint32_t Offsets[12];
    uint64_t MeshBufferPointer;
    uint64_t RigidWeightsPtr;
    uint8_t Padding4[40];
    float XOffset;
    float YOffset;
    float ZOffset;
    float Scale;
    float Min;
    float Max;
    uint8_t Padding9[24];
};
#pragma pack(pop)

struct MW5GfxImage
{
    uint64_t Hash;
    uint8_t Unk00[16];
    uint32_t BufferSize;
    uint32_t Unk02;
    uint16_t Width;
    uint16_t Height;
    uint16_t Depth;
    uint16_t Levels;
    uint8_t UnkByte0;
    uint8_t UnkByte1;
    uint8_t ImageFormat;
    uint8_t UnkByte3;
    uint8_t UnkByte4;
    uint8_t UnkByte5;
    uint8_t MipCount;
    uint8_t UnkByte7;
    uint64_t MipMaps;
    uint64_t PrimedMipPtr;
    uint64_t LoadedImagePtr;
};

#pragma pack(push, 1)
struct MW5GfxMip
{
    uint64_t HashID;
    uint64_t Padding;
    uint64_t PackedInfo;
};
#pragma pack(pop)

template <size_t Size>
struct MW5GfxMipArray
{
    MW5GfxMip MipMaps[Size];

    size_t GetImageSize(size_t i)
    {
        if (i == 0)
            return ((MipMaps[i].PackedInfo >> 30) & 0x1FFFFFFF);
        else
            return ((MipMaps[i].PackedInfo >> 30) & 0x1FFFFFFF) - ((MipMaps[i - 1].PackedInfo >> 30) & 0x1FFFFFFF);
    }

    size_t GetMipCount()
    {
        return Size;
    }
};


#pragma pack(push, 1)
struct MW5XAnimStreamInfo
{
    uint64_t StreamKey;
    uint32_t Size;
    uint32_t Padding;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XAnimNotetrack
{
    uint32_t Name;
    float Time;
    uint8_t Padding[24];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XAnim
{
    uint64_t Hash;
    uint64_t NamePtr;
    uint64_t BoneIDsPtr;
    uint64_t IndicesPtr;
    uint64_t NotificationsPtr;
    uint8_t Padding[24];
    uint32_t RandomDataShortCount;
    uint32_t RandomDataByteCount;
    uint32_t IndexCount;

    float Framerate;
    float Frequency;

    uint32_t DataByteCount;
    uint16_t DataShortCount;
    uint16_t DataIntCount;
    uint16_t RandomDataIntCount;
    uint16_t FrameCount;

    uint8_t Padding2[6];

    uint16_t NoneRotatedBoneCount;
    uint16_t TwoDRotatedBoneCount;
    uint16_t NormalRotatedBoneCount;
    uint16_t TwoDStaticRotatedBoneCount;
    uint16_t NormalStaticRotatedBoneCount;
    uint16_t NormalTranslatedBoneCount;
    uint16_t PreciseTranslatedBoneCount;
    uint16_t StaticTranslatedBoneCount;
    uint16_t NoneTranslatedBoneCount;
    uint16_t TotalBoneCount;

    uint8_t NotetrackCount;
    uint8_t AssetType;

    uint8_t Padding3[20];

    uint32_t DataByteOffset;
    uint32_t DataShortOffset;
    uint32_t DataIntOffset;
    uint32_t Padding4;
    uint64_t OffsetPtr;
    uint64_t OffsetPtr2;
    uint32_t StreamIndex;
    uint32_t OffsetCount;
    uint8_t Padding5[24];
    uint64_t StreamInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XMaterial
{
    uint64_t Hash;
    uint64_t NamePtr;
    uint8_t Padding[16];
    uint8_t ImageCount;
    uint8_t Padding2[15];
    uint64_t TechsetPtr;
    uint64_t ImageTablePtr;
    uint8_t Padding3[32];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XMaterialImage
{
    uint32_t Type;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5SoundBank
{
    uint64_t Hash;
    uint64_t NamePtr;
    uint8_t Padding0[336];
    uint64_t StreamKey;
    uint8_t Padding1[12];
    uint32_t SoundBankSize;
    uint8_t Padding2[24];

};
#pragma pack(pop)

extern uint32_t MW5DXGIFormats[52];