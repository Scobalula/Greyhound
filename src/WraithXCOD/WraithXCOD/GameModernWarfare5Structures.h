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
    uint64_t NamePtr;
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
    uint8_t UnkByte6;
    uint8_t UnkByte7;
    uint64_t MipMaps;
    uint64_t PrimedMipPtr;
    uint64_t LoadedImagePtr;
};

#pragma pack(push, 1)
struct MW5GfxMip
{
    uint64_t HashID;
    uint8_t Padding[8];
    uint16_t Width;
    uint16_t Height;
    uint32_t Size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW5XAnim
{
    uint64_t Hash;
    uint64_t NamePtr;
    uint64_t BoneIDsPtr;
    uint64_t DataBytePtr;
    uint64_t DataShortPtr;
    uint64_t DataIntPtr;
    uint64_t RandomDataShortPtr;
    uint64_t RandomDataBytePtr;
    uint64_t RandomDataIntPtr;
    uint64_t LongIndiciesPtr;
    uint64_t NotificationsPtr;
    uint8_t PaddingNew[0x8];
    uint64_t DeltaPartsPtr;
    uint8_t Padding[0xC];

    float Framerate;

    uint8_t Padding2[14];

    uint16_t NumFrames;
    uint16_t Flags;

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

    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[27];
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
    uint64_t NamePtr;
    uint64_t BankNamePtr;
    uint64_t LanguagePtr;
    uint64_t TypePtr;
    uint64_t AliasCount;
    uint64_t AliasesPtr;
    uint8_t Padding[448];
    uint64_t SoundBankPtr;
    uint64_t Unk1;
};
#pragma pack(pop)

extern uint32_t MW5DXGIFormats[52];