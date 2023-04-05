#pragma once

#include <cstdint>

// A structure that represents game offset information
struct DBGameInfo
{
    uint64_t DBAssetPools;
    uint64_t DBPoolSizes;
    uint64_t StringTable;
    uint64_t ImagePackageTable;

    DBGameInfo(uint64_t Pools, uint64_t Sizes, uint64_t Strings, uint64_t Package);
};

// -- Contains structures for various game asset / memory formats

#pragma region Quantum Solace
#pragma pack(push, 1)
struct QSXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint32_t PartBits[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct QSXModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t SurfacesPtr;
    uint32_t MaterialHandlesPtr;

    QSXModelLod ModelLods[4];

    uint8_t Padding[44];

    uint16_t NumLods;

    uint8_t Padding3[26];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct QSXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t Padding1;
    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;

    uint32_t Padding2;


    uint32_t VerticiesPtr;


    uint8_t Padding3[12];


    uint32_t VertListCount;
    uint32_t RigidWeightsPtr;
    uint32_t UnkPtr;
    uint32_t PartBits[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct QSXAnim
{
    uint32_t NamePtr;

    uint8_t Padding[14];

    uint16_t NumFrames;
    uint8_t Looped;
    uint8_t isDelta;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[10];

    float Framerate;
    float Frequency;

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)
#pragma endregion

#pragma region World At War

#pragma pack(push, 1)
struct WAWXMaterial
{
    uint32_t NamePtr;

    uint8_t Padding[0x57];

    uint8_t ImageCount;

    uint8_t Padding2[8];

    uint32_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[8];
    uint32_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWGfxImage
{
    uint8_t Padding[0x18];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding2[2];

    uint8_t MipLevels;
    uint8_t Streamed;

    uint32_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint8_t Padding[6];

    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;
    uint32_t VerticiesPtr;

    uint32_t D3DVBufferPtr;
    uint32_t VertListCount;
    uint32_t RigidWeightsPtr;
    uint32_t D3DIBufferPtr;

    uint32_t PartBits[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint32_t PartBits[5];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWXModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t SurfacesPtr;
    uint32_t MaterialHandlesPtr;

    WAWXModelLod ModelLods[4];

    uint8_t Padding[0xC];

    uint32_t BoneInfoPtr;

    uint8_t Padding2[0x1C];

    uint16_t NumLods;

    uint8_t Padding3[0x1E];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWXAnimDeltaParts
{
    uint32_t DeltaTranslationsPtr;
    uint32_t Delta2DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWXAnim
{
    uint32_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Looped;
    uint8_t isDelta;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[10];

    float Framerate;
    float Frequency;

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WAWLoadedSound
{
    uint32_t NamePtr;
    uint32_t SoundDataPtr;
    uint32_t SoundDataSize;
};
#pragma pack(pop)

#pragma endregion

#pragma region Black Ops 1

#pragma pack(push, 1)
const struct BOFxEffectDef
{
    uint32_t NamePtr;

    uint8_t Flags;
    uint8_t EFPriority;
    uint8_t Reserved[2];

    uint32_t TotalSize;
    uint32_t MSECLoopingLife;

    uint32_t ElemDefCountLooping;
    uint32_t ElemDefCountOneShot;
    uint32_t ElemDefCountEmission;

    uint32_t FxElementsPtr;

    float BoundingBoxDim[3];
    float BoundingSphere[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXMaterial
{
    uint32_t NamePtr;

    uint8_t Padding[166];

    uint8_t ImageCount;

    uint8_t Padding2[9];

    uint32_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[8];
    uint32_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOGfxImage
{
    uint8_t Padding[20];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding2[2];

    uint8_t MipLevels;
    uint8_t Streamed;

    uint8_t Padding3[16];

    uint32_t NamePtr;
    uint32_t Hash;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint32_t PartBits[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t SurfacesPtr;
    uint32_t MaterialHandlesPtr;

    BOXModelLod ModelLods[4];

    uint8_t Padding[0x10];

    uint32_t BoneInfoPtr;

    uint8_t Padding2[0x1C];

    uint16_t NumLods;

    uint8_t Padding3[0x22];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXModelSurface
{
    int8_t TileMode;
    int8_t VertListCount;

    uint16_t Flags;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t BaseTriIndex;
    uint16_t BaseVertIndex;

    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;
    uint32_t TensionPtr;

    uint32_t VerticiesPtr;
    uint32_t D3DVBufferPtr;
    uint32_t RigidWeightsPtr;
    uint32_t D3DIBufferPtr;

    uint32_t PartBits[5];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXAnimDeltaParts
{
    uint32_t DeltaTranslationsPtr;
    uint32_t Delta2DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXAnim
{
    uint32_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Looped;
    uint8_t isDelta;

    uint8_t Padding2[0x6];

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;
    uint8_t isDefault;

    uint8_t Padding3[11];

    float Framerate;
    float Frequency;

    uint8_t Padding4[0x8];

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOXRawFile
{
    uint32_t NamePtr;

    uint32_t AssetSize;
    uint32_t RawDataPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region Black Ops 2

#pragma pack(push, 1)
const struct BO2FxEffectDef
{
    uint32_t NamePtr;

    uint16_t Flags;
    uint16_t EFPriority;

    uint16_t ElemDefCountLooping;
    uint16_t ElemDefCountOneShot;
    uint16_t ElemDefCountEmission;

    uint16_t Padding;

    uint32_t TotalSize;
    uint32_t MSECLoopingLife;
    uint32_t MSECNonLoopingLife;

    uint32_t FxElementsPtr;

    float BoundingBoxDim[3];
    float BoundingCenter[3];

    float OcclusionQueryDepthBias;

    uint32_t OcclusionQueryFadeIn;
    uint32_t OcclusionQueryFadeOut;

    float OcclusionQueryScaleRange[2];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XMaterial
{
    uint32_t NamePtr;

    uint8_t Padding[80];

    uint8_t ImageCount;

    uint8_t Padding2[11];

    uint32_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[8];
    uint32_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2GfxImage
{
    uint8_t Padding[20];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding2[2];

    uint8_t MipLevels;
    uint8_t Streamed;

    uint8_t Padding3[12];

    uint32_t KeyLower;

    uint8_t Padding4[28];

    uint32_t NamePtr;
    uint32_t KeyUpper;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint32_t PartBits[5];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t SurfacesPtr;
    uint32_t MaterialHandlesPtr;

    BO2XModelLod ModelLods[4];

    uint8_t Padding[0xC];

    uint32_t BoneInfoPtr;

    uint8_t Padding2[0x1C];

    uint16_t NumLods;

    uint8_t Padding3[0x32];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XModelSurface
{
    int8_t TileMode;
    int8_t VertListCount;

    uint16_t Flags;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t BaseVertIndex;
    uint16_t BaseTriIndex;

    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;
    uint32_t TensionPtr;

    uint32_t VerticiesPtr;
    uint32_t D3DVBufferPtr;
    uint32_t RigidWeightsPtr;
    uint32_t D3DIBufferPtr;

    uint32_t PartBits[5];

    uint8_t Padding[0xC];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XAnim
{
    uint32_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Looped;
    uint8_t isDelta;

    uint8_t Padding2[0x6];

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;
    uint8_t isDefault;

    uint8_t Padding3[11];

    float Framerate;
    float Frequency;

    uint8_t Padding4[0x8];

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XAnimDeltaParts
{
    uint32_t DeltaTranslationsPtr;
    uint32_t Delta2DRotationsPtr;
    uint32_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2XRawFile
{
    uint32_t NamePtr;

    uint32_t AssetSize;
    uint32_t RawDataPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region Black Ops 3

#pragma pack(push, 1)
const struct BO3FxEffectDef
{
    uint64_t NamePtr;

    uint16_t Flags;
    uint16_t EFPriority;

    uint16_t ElemDefCountLooping;
    uint16_t ElemDefCountOneShot;
    uint16_t ElemDefCountEmission;

    uint16_t Padding;

    uint32_t TotalSize;
    uint32_t MSECLoopingLife;
    uint32_t MSECNonLoopingLife;

    uint64_t FxElementsPtr;

    float BoundingBoxDim[3];
    float BoundingCenter[3];

    float OcclusionQueryDepthBias;

    uint32_t OcclusionQueryFadeIn;
    uint32_t OcclusionQueryFadeOut;

    float OcclusionQueryScaleRange[2];

    uint8_t Unknown1[0x3C];    // Probably new BO3 values packed here...
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3GfxMip
{
    uint32_t Size;
    uint16_t Width;
    uint16_t Height;

    uint64_t HashID;

    uint8_t Padding[24];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3GfxImage
{
    BO3GfxMip MipLevels[4];

    uint8_t Padding2[31];

    uint8_t LoadedMipLevels;

    uint16_t LoadedMipWidth;
    uint16_t LoadedMipHeight;

    uint8_t Padding3[20];

    uint64_t LoadedMipPtr;
    uint64_t LoadedMipUnknown;
    uint64_t LoadedMipSize;

    uint8_t ImageFormat;

    uint8_t Padding4[7];

    uint64_t NamePtr;

    uint8_t Padding5[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XMaterialImage
{
    uint64_t ImagePtr;
    uint32_t SemanticHash;
    uint8_t Padding[0x14];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XMaterial
{
    uint64_t NamePtr;

    uint8_t Padding[616];

    uint8_t ImageCount;

    uint8_t Padding2[15];

    uint64_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XModelMeshInfo
{
    uint8_t StatusFlag;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t Flag4;

    uint32_t VertexCount;
    uint32_t WeightCount;
    uint32_t FacesCount;

    uint64_t XModelMeshBufferPtr;
    uint32_t XModelMeshBufferSize;

    uint32_t VertexOffset;
    uint32_t UVOffset;
    uint32_t FacesOffset;
    uint32_t WeightsOffset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XModelSurface
{
    uint8_t Flag1;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t Flag4;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint32_t VerticiesIndex;
    uint32_t FacesIndex;

    uint8_t Padding[80];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XModelLod
{
    uint8_t Padding[60];

    uint8_t NumSurfs;
    uint8_t SurfacesIndex;

    uint16_t UnknownFlags;

    float LodMinDistance;
    float LodDistance;

    uint64_t LODStreamKey;

    uint8_t Padding2[24];

    uint64_t XSurfacePtr;
    uint64_t XModelMeshPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XModel
{
    uint64_t NamePtr;

    uint8_t NumBones;
    uint8_t NumRootBones;
    uint16_t NumCosmeticBones;

    uint8_t Padding[4];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;

    uint8_t NumLods;

    uint8_t Padding2[71];

    uint64_t ModelLodPtrs[8];

    uint64_t MaterialHandlesPtr;

    uint8_t Padding3[0x18];

    uint64_t BoneInfoPtr;

    uint8_t Padding4[0x98];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XRawFile
{
    uint64_t NamePtr;

    uint64_t AssetSize;
    uint64_t RawDataPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XAnim
{
    uint64_t NamePtr;

    uint8_t Padding[0x18];

    uint16_t NumFrames;

    uint8_t Padding2[2];

    uint8_t LoopingFlag;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t Flag4;

    uint8_t Padding3[8];

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

    uint8_t AssetType;

    uint8_t Padding4[0xB];

    float Framerate;
    float Frequency;

    uint8_t Padding5[0x10];

    uint64_t BoneIDsPtr;
    uint64_t DataBytePtr;
    uint64_t DataShortPtr;
    uint64_t DataIntPtr;
    uint64_t RandomDataShortPtr;
    uint64_t RandomDataBytePtr;
    uint64_t RandomDataIntPtr;
    uint64_t UnknownIndiciesPtr;

    uint8_t Padding6[0x18];

    uint64_t NotificationsPtr;
    uint32_t NotificationCount;

    uint8_t Padding7[0x24];

    uint64_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region Black Ops 4

#pragma pack(push, 1)
struct BO4XMaterialImage
{
    uint64_t ImagePtr;
    uint32_t SemanticHash;
    uint8_t Padding[0x14];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4XMaterial
{
    uint64_t NamePtr;
    uint64_t Unk01;

    uint8_t Padding[0x28];

    uint64_t ImageTablePtr;

    uint8_t Padding2[0xF0];

    uint8_t ImageCount;

    uint8_t Padding3[7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4GfxMip
{
    uint64_t HashID;

    uint8_t Padding[0x1C];

    uint16_t Width;
    uint16_t Height;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4GfxImageOriginal // Leaving this here in case they switch it back
{
    uint64_t NamePtr;

    uint64_t UnknownPtr1;
    uint64_t UnknownZero;

    uint64_t LoadedMipPtr;

    uint64_t UnknownHash;
    uint64_t UnknownZero2;

    BO4GfxMip MipLevels[4];

    uint8_t Padding[0x28];

    uint32_t LoadedMipSize;
    uint32_t ImageFormat;

    uint16_t LoadedMipWidth;
    uint16_t LoadedMipHeight;
    uint8_t LoadedMipLevels;

    uint8_t Padding2[0x1B];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4GfxImage
{
    uint64_t UnknownPtr;

    uint64_t UnknownPtr1;
    uint64_t UnknownZero;

    uint64_t LoadedMipPtr;

    uint64_t NamePtr;
    uint64_t UnknownZero2;

    uint64_t GfxMipsPtr;

    uint8_t Padding[0x28];

    uint32_t LoadedMipSize;
    uint32_t ImageFormat;

    uint16_t LoadedMipWidth;
    uint16_t LoadedMipHeight;
    uint8_t LoadedMipLevels;

    uint8_t Padding2[0x10];

    uint8_t GfxMipMaps;

    uint8_t Padding3[0xA];


};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4XModelMeshInfo
{
    uint8_t StatusFlag;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t Flag4;

    uint32_t VertexCount;
    uint32_t WeightCount;
    uint32_t FacesCount;

    uint8_t Padding[0x10];

    uint64_t XModelMeshBufferPtr;
    uint32_t XModelMeshBufferSize;

    uint32_t VertexOffset;
    uint32_t UVOffset;
    uint32_t FacesOffset;
    uint32_t WeightsOffset;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct BO4XModelSurface
{
    uint8_t Flag1;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t Flag4;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint32_t VerticiesIndex;
    uint32_t FacesIndex;

    uint8_t Padding[0x20];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4XModelLod
{
    uint64_t UnknownZero;
    uint64_t NameHash;
    uint64_t XSurfacePtr;
    uint64_t XModelMeshPtr;
    uint64_t UnknownPtr;

    uint64_t LODStreamKey;

    uint8_t Padding[0x18];

    float LodDistance;

    uint8_t NumSurfs;
    uint8_t SurfacesIndex;

    uint16_t UnknownFlags;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4XModel
{
    uint64_t NamePtr;
    uint64_t UnknownZero;
    uint64_t BoneIDsPtr;
    uint64_t UnknownPtr2;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr1;

    uint8_t Padding[0x28];

    uint64_t ModelLodPtrs[8];

    uint64_t MaterialHandlesPtr;
    uint64_t UnknownPtr4;

    uint32_t NumLods;

    uint8_t Padding3[0x78];

    uint16_t NumCosmeticBones;

    uint8_t Padding4[0x5];

    uint8_t NumBones;
    uint8_t NumRootBones;
    uint16_t NumUnknown;
    uint8_t Padding5;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4XAnim
{
    uint64_t UnknownPtr;
    uint64_t BoneIDsPtr;
    uint64_t UnknownZero1;
    uint64_t DeltaPartsPtr;

    uint8_t Unknown1[0x10];

    uint64_t UnknownPtr2;
    uint64_t NotificationsPtr;

    uint8_t Unknown2[0x18];

    uint64_t DataBytePtr;
    uint64_t UnknownZero3;
    uint64_t RandomDataBytePtr;

    uint64_t NamePtr;
    uint64_t UnknownZero;

    uint64_t DataIntPtr;
    uint64_t UnknownZero2;
    uint64_t DataShortPtr;
    uint64_t RandomDataShortPtr;

    uint8_t Unknown3[0x20];

    float Framerate;
    float Frequency;

    uint8_t Unknown4[0x34];

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
    uint16_t Unknown6;

    uint16_t NumFrames;

    uint8_t Unknown[0x1A];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO4XAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region Modern Warfare

#pragma pack(push, 1)
struct MWXMaterial
{
    uint32_t NamePtr;

    uint8_t Padding[0x36];

    uint8_t ImageCount;

    uint8_t Padding2[9];

    uint32_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint32_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWGfxImage
{
    uint8_t Padding[0x18];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding2[2];

    uint8_t MipLevels;
    uint8_t Streamed;

    uint32_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint8_t Padding[6];

    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;
    uint32_t VerticiesPtr;

    uint32_t VertListCount;
    uint32_t RigidWeightsPtr;

    uint32_t PartBits[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint32_t PartBits[5];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWXModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t SurfacesPtr;
    uint32_t MaterialHandlesPtr;

    MWXModelLod ModelLods[4];

    uint8_t Padding[0xC];

    uint32_t BoneInfoPtr;

    uint8_t Padding2[0x1C];

    uint16_t NumLods;

    uint8_t Padding3[0x16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWXAnimDeltaParts
{
    uint32_t DeltaTranslationsPtr;
    uint32_t Delta2DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWXAnim
{
    uint32_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Looped;
    uint8_t isDelta;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[10];

    float Framerate;
    float Frequency;

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWLoadedSound
{
    uint32_t NamePtr;
    uint32_t UnknownInt;
    uint32_t SoundDataPtr;
    uint32_t SoundDataSize;
    uint32_t FrameRate;
    uint32_t BitsPerSample;
    uint32_t Channels;
    uint32_t FrameCount;
    uint32_t UnknownInt2;
    uint32_t UnknownPtr;
    uint32_t UnknownPtr2;
};
#pragma pack(pop)

#pragma endregion

#pragma region Modern Warfare 2

#pragma pack(push, 1)
struct MW2XMaterial
{
    uint32_t NamePtr;

    uint8_t Padding[0x44];

    uint8_t ImageCount;

    uint8_t Padding2[0xB];

    uint32_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2XMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint32_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2GfxImage
{
    uint8_t Padding[0x14];

    uint16_t Width;
    uint16_t Height;

    uint8_t MipLevels;
    uint8_t Streamed;

    uint8_t Padding2[2];

    uint32_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2XModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint8_t Padding[6];

    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;
    uint32_t VerticiesPtr;

    uint32_t VertListCount;
    uint32_t RigidWeightsPtr;

    uint32_t PartBits[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2XModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[28];

    uint32_t SurfsPtr;

    uint8_t Padding2[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2XModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint8_t Padding[0x1C];

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t MaterialHandlesPtr;

    MW2XModelLod ModelLods[4];

    uint8_t MaxLods;
    uint8_t NumLods;

    uint8_t Padding2[0x3E];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2XAnim
{
    uint32_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Looped;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;
    uint8_t isDefault;

    uint8_t Padding3[10];

    float Framerate;
    float Frequency;

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW2XAnimDeltaParts
{
    uint32_t DeltaTranslationsPtr;
    uint32_t Delta2DRotationsPtr;
    uint32_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region Modern Warfare 3

#pragma pack(push, 1)
struct MW3XMaterial
{
    uint32_t NamePtr;

    uint8_t Padding[0x4A];

    uint8_t ImageCount;

    uint8_t Padding2[9];

    uint32_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3XMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint32_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3GfxImage
{
    uint8_t Padding[0x14];

    uint16_t Width;
    uint16_t Height;

    uint8_t MipLevels;
    uint8_t Streamed;

    uint8_t Padding2[2];

    uint32_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3XModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint8_t Padding[0xA];

    uint32_t FacesPtr;

    uint16_t WeightCounts[4];
    uint32_t WeightsPtr;
    uint32_t VerticiesPtr;

    uint32_t VertListCount;
    uint32_t RigidWeightsPtr;

    uint32_t PartBits[6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3XModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[28];

    uint32_t SurfsPtr;

    uint8_t Padding2[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3XModel
{
    uint32_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint8_t Padding[0x1C];

    uint32_t BoneIDsPtr;
    uint32_t ParentListPtr;
    uint32_t RotationsPtr;
    uint32_t TranslationsPtr;
    uint32_t PartClassificationPtr;
    uint32_t BaseMatriciesPtr;
    uint32_t MaterialHandlesPtr;

    MW3XModelLod ModelLods[4];

    uint8_t MaxLods;
    uint8_t NumLods;

    uint8_t Padding2[0x42];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3XAnim
{
    uint32_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Looped;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;
    uint8_t isDefault;

    uint8_t Padding3[10];

    float Framerate;
    float Frequency;

    uint32_t BoneIDsPtr;
    uint32_t DataBytePtr;
    uint32_t DataShortPtr;
    uint32_t DataIntPtr;
    uint32_t RandomDataShortPtr;
    uint32_t RandomDataBytePtr;
    uint32_t RandomDataIntPtr;
    uint32_t LongIndiciesPtr;
    uint32_t NotificationsPtr;
    uint32_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW3XAnimDeltaParts
{
    uint32_t DeltaTranslationsPtr;
    uint32_t Delta2DRotationsPtr;
    uint32_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region Ghosts

#pragma pack(push, 1)
const struct GhostsFxEffectDef
{
    uint64_t NamePtr;

    uint32_t Flags;
    uint32_t TotalSize;
    uint32_t MSECLoopingLife;

    uint32_t ElemDefCountLooping;
    uint32_t ElemDefCountOneShot;
    uint32_t ElemDefCountEmission;

    float ElemMaxRadius;
    float OcclusionQueryDepthBias;

    uint32_t OcclusionQueryFadeIn;
    uint32_t OcclusionQueryFadeOut;

    float OcclusionQueryScaleRange[2];

    uint64_t FxElementsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXMaterial
{
    uint64_t NamePtr;

    uint8_t Padding[0x1BC];

    uint8_t ImageCount;

    uint8_t Padding2[19];

    uint64_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsPAKImageEntry
{
    uint64_t ImageOffset;
    uint64_t ImageEndOffset;
    uint64_t ImagePAKInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsGfxMip
{
    uint16_t Width;
    uint16_t Height;

    uint8_t Padding[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsGfxImage
{
    uint64_t NextHead;

    uint8_t Padding[16];

    uint8_t ImageFormat;
    uint8_t Padding5[3];
    uint8_t MapType;

    uint8_t Padding2[24];

    uint8_t Streamed;

    uint8_t Padding3[10];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding4[4];

    GhostsGfxMip MipLevels[3];

    uint64_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;
    uint8_t VertListCount;
    uint8_t PaddingCount;

    uint16_t WeightCounts[8];

    uint64_t VerticiesPtr;
    uint64_t FacesPtr;

    uint8_t Padding[24];

    uint64_t RigidWeightsPtr;
    uint64_t WeightsPtr;

    uint8_t Padding2[0x98];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[40];

    uint64_t SurfsPtr;

    uint8_t Padding2[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXModel
{
    uint64_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint8_t Padding[0x24];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t MaterialHandlesPtr;

    GhostsXModelLod ModelLods[6];

    uint8_t MaxLods;
    uint8_t NumLods;

    uint8_t Padding2[0x66];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXAnim
{
    uint64_t NamePtr;

    uint8_t Padding[10];

    uint16_t NumFrames;
    uint8_t Flags;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;
    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[0xb];

    float Framerate;

    uint8_t Padding4[0x8];

    uint64_t BoneIDsPtr;
    uint64_t DataBytePtr;
    uint64_t DataShortPtr;
    uint64_t DataIntPtr;
    uint64_t RandomDataShortPtr;
    uint64_t RandomDataBytePtr;
    uint64_t RandomDataIntPtr;
    uint64_t LongIndiciesPtr;
    uint64_t NotificationsPtr;
    uint64_t DeltaPartsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsXAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsSoundAlias
{
    uint64_t NamePtr;
    uint64_t EntriesPtr;
    uint8_t EntryCount;
    uint8_t Padding[0x7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsSoundAliasEntry
{
    uint64_t NamePtr;
    uint8_t Padding[0x20];
    uint64_t FileSpecPtr;
    uint8_t Padding2[0x98];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsStreamedSound
{
    uint16_t Localization;
    uint16_t PackageIndex;
    uint8_t Padding[4];
    uint64_t Offset;
    uint64_t Size;
    uint32_t Length;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct GhostsPrimedSound
{
    uint16_t Localization;
    uint16_t PackageIndex;
    uint8_t Padding[4];
    uint64_t Offset;
    uint64_t Size;
    uint64_t LoadedSoundPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GhostsLoadedSound
{
    uint64_t NamePtr;
    uint64_t SoundDataPtr;
    uint8_t Padding1[8];
    uint32_t FrameRate;
    uint32_t SoundDataSize;
    uint32_t FrameCount;
    uint32_t ByteRate;
    uint8_t Channels;
    uint8_t Padding2[15];
};
#pragma pack(pop)

#pragma endregion

#pragma region AdvancedWarfare

#pragma pack(push, 1)
struct AWXMaterial
{
    uint64_t NamePtr;

    uint8_t Padding[0xDC];

    uint8_t ImageCount;

    uint8_t Padding2[19];

    uint64_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWPAKImageEntry
{
    uint64_t ImageOffset;
    uint64_t ImageEndOffset;
    uint64_t ImagePAKInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWGfxMip
{
    uint16_t Width;
    uint16_t Height;

    uint8_t Padding[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWGfxImage
{
    uint64_t NextHead;

    uint8_t Padding[16];

    uint8_t ImageFormat;
    uint8_t Padding5[3];
    uint8_t MapType;

    uint8_t Padding2[24];

    uint8_t Streamed;

    uint8_t Padding3[10];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding4[4];

    AWGfxMip MipLevels[3];

    uint64_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;
    uint8_t VertListCount;
    uint8_t PaddingCount;

    uint16_t WeightCounts[8];

    uint64_t VerticiesPtr;
    uint64_t FacesPtr;

    uint8_t Padding[32];

    uint64_t RigidWeightsPtr;
    uint64_t UnknownPtr;
    uint64_t WeightsPtr;

    uint8_t Padding2[0xA8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[40];

    uint64_t SurfsPtr;

    uint8_t Padding2[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWXModel
{
    uint64_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint8_t Padding[0x2C];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t UnknownPtr2;
    uint64_t MaterialHandlesPtr;

    AWXModelLod ModelLods[6];

    uint8_t NumLods;
    uint8_t MaxLods;

    uint8_t Padding2[0xA6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWXAnim
{
    uint64_t NamePtr;

    uint8_t Padding[6];

    uint16_t NumFrames;
    uint8_t Flags;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;

    uint8_t Padding2[2];

    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[0x11];

    float Framerate;

    uint8_t Padding4[0x4];

    uint64_t BoneIDsPtr;
    uint64_t DataBytePtr;
    uint64_t DataShortPtr;
    uint64_t DataIntPtr;
    uint64_t RandomDataShortPtr;
    uint64_t RandomDataBytePtr;
    uint64_t RandomDataIntPtr;
    uint64_t LongIndiciesPtr;
    uint64_t NotificationsPtr;
    uint64_t DeltaPartsPtr;

    uint8_t Padding5[0x50];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWXAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWLoadedSound
{
    uint64_t NamePtr;
    uint8_t Padding1[2];
    int16_t PackFileIndex;
    uint8_t Padding2[4];
    uint64_t PackFileOffset;
    uint8_t Padding3[8];
    uint64_t SoundDataPtr;
    uint32_t FrameRate;
    uint32_t SoundDataSize;
    uint32_t FrameCount;
    uint8_t Channels;
    uint8_t Padding4[3];
    uint8_t Format;
    uint8_t Padding5[7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWSoundAlias
{
    uint64_t NamePtr;
    uint64_t EntriesPtr;
    uint64_t Unknown1;
    uint8_t EntryCount;
    uint8_t Padding[0x7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWSoundAliasEntry
{
    uint64_t NamePtr;
    uint8_t Padding[0x18];
    uint64_t FileSpecPtr;
    uint8_t Padding2[0xC8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWSoundAliasFileSpec
{
    uint8_t Type;
    uint8_t Padding[0x7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWStreamedSound
{
    uint8_t Localization;
    bool Exists;
    uint16_t PackageIndex;
    uint8_t Padding[4];
    uint64_t Offset;
    uint64_t Size;
    uint32_t Length;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct AWPrimedSound
{
    uint64_t LoadedSoundPtr;
    uint8_t Localization;
    bool Exists;
    uint16_t PackageIndex;
    uint8_t Padding[4];
    uint64_t Offset;
    uint64_t Size;
    uint32_t Length;
};
#pragma pack(pop)

#pragma endregion

#pragma region Modern Warfare RM

#pragma pack(push, 1)
struct MWRXMaterial
{
    uint64_t NamePtr;

    uint8_t Padding[0x118];

    uint8_t ImageCount;

    uint8_t Padding2[15];

    uint64_t ImageTablePtr;

    uint8_t Padding3[0x118];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRPAKImageEntry
{
    uint64_t ImageOffset;
    uint64_t ImageEndOffset;
    uint64_t ImagePAKInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRGfxMip
{
    uint16_t Width;
    uint16_t Height;

    uint8_t Padding[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRGfxImage
{
    uint64_t NextHead;

    uint8_t Padding[16];

    uint8_t ImageFormat;
    uint8_t Padding5[3];
    uint8_t MapType;

    uint8_t Padding2[24];

    uint8_t Streamed;

    uint8_t Padding3[10];

    uint16_t Width;
    uint16_t Height;

    uint8_t Padding4[4];

    MWRGfxMip MipLevels[3];

    uint64_t NamePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;
    uint8_t VertListCount;
    uint8_t PaddingCount;

    uint16_t WeightCounts[8];

    uint64_t VerticiesPtr;
    uint64_t FacesPtr;

    uint8_t Padding[32];

    uint64_t RigidWeightsPtr;
    uint64_t UnknownPtr;
    uint64_t WeightsPtr;

    uint8_t Padding2[0xA8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[40];

    uint64_t SurfsPtr;

    uint8_t Padding2[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRXModel
{
    uint64_t NamePtr;
    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint8_t Padding[0x2C];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t UnknownPtr2;
    uint64_t MaterialHandlesPtr;

    MWRXModelLod ModelLods[6];

    uint8_t NumLods;
    uint8_t MaxLods;

    uint8_t Padding2[0xA6];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRXAnim
{
    uint64_t NamePtr;

    uint8_t Padding[6];

    uint16_t NumFrames;
    uint8_t Flags;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;

    uint8_t Padding2[2];

    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[0x11];

    float Framerate;

    uint8_t Padding4[0x4];

    uint64_t BoneIDsPtr;
    uint64_t DataBytePtr;
    uint64_t DataShortPtr;
    uint64_t DataIntPtr;
    uint64_t RandomDataShortPtr;
    uint64_t RandomDataBytePtr;
    uint64_t RandomDataIntPtr;
    uint64_t LongIndiciesPtr;
    uint64_t NotificationsPtr;
    uint64_t DeltaPartsPtr;

    uint8_t Padding5[0x50];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRXAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRSoundAlias
{
    uint64_t NamePtr;
    uint64_t EntriesPtr;
    uint64_t Unknown1;
    uint8_t EntryCount;
    uint8_t Padding[0x7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRSoundAliasEntry
{
    uint64_t NamePtr;
    uint8_t Padding[0x18];
    uint64_t FileSpecPtr;
    uint8_t Padding2[0xD0];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRSoundAliasFileSpec
{
    uint8_t Type;
    uint8_t Padding[0x7];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MWRStreamedSound
{
    uint8_t Padding[2];
    uint16_t PackageIndex;
    uint32_t Exists;
    uint64_t Offset;
    uint64_t Size;
    uint32_t Length;
    uint32_t Padding3;
    uint64_t Padding4;
};
#pragma pack(pop)

#pragma endregion

#pragma region InfiniteWarfare

#pragma pack(push, 1)
struct IWXMaterial
{
    uint64_t NamePtr;

    uint8_t Padding[40];

    uint8_t ImageCount;

    uint8_t Padding2[15];

    uint64_t ImageTablePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWPAKImageEntry
{
    uint64_t ImageOffset;
    uint64_t ImageEndOffset;
    uint64_t ImagePAKInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWGfxMip
{
    uint16_t Width;
    uint16_t Height;

    uint8_t Padding[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWGfxImage
{
    uint64_t NextHead;
    uint8_t Padding[16];
    uint8_t ImageFormat;
    uint8_t Padding5[7];
    uint8_t MapType;
    uint8_t Padding2[7];
    uint32_t Size;
    uint32_t Size2;
    uint16_t LoadedWidth;
    uint16_t LoadedHeight;
    uint16_t LoadedDepth;
    uint16_t LoadedArrays;
    uint8_t Flags;
    uint8_t Streamed;
    uint8_t Padding3[14];
    uint16_t Width;
    uint16_t Height;
    uint8_t Padding4[4];
    IWGfxMip MipLevels[3];
    uint64_t NamePtr;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct IWXModelSurface
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;
    uint8_t VertListCount;
    uint8_t PaddingCount;

    uint16_t WeightCounts[8];

    uint8_t Padding[8];

    uint64_t VerticiesPtr;
    uint64_t FacesPtr;

    uint8_t Padding2[24];

    uint64_t RigidWeightsPtr;
    uint64_t WeightsPtr;

    uint8_t Padding3[0xA8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[40];

    uint64_t SurfsPtr;

    uint8_t Padding2[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWXModel
{
    uint64_t NamePtr;

    uint8_t Padding[3];

    uint8_t NumLods;
    uint8_t MaxLods;

    uint8_t Padding2[7];

    uint8_t NumBones;
    uint8_t NumRootBones;
    uint8_t NumSurfaces;
    uint8_t LodRampType;

    uint8_t Padding3[0x80];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t UnknownPtr2;
    uint64_t MaterialHandlesPtr;

    IWXModelLod ModelLods[6];

    uint8_t Padding4[0x80];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWXAnim
{
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
    uint64_t DeltaPartsPtr;

    uint8_t Padding[0xC];

    float Framerate;

    uint8_t Padding2[0xC];

    uint16_t NumFrames;
    uint8_t Flags;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;

    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[5];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWXAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma region World War 2

#pragma pack(push, 1)
struct WWIIXMaterial
{
    uint64_t NamePtr;

    uint8_t Padding[0x9A];

    uint8_t ImageCount;

    uint8_t Padding2[0x15];

    uint64_t TechsetPtr;
    uint64_t ImageTablePtr;
    uint64_t UnknownPtr;
    uint64_t ConstantsPtr;

    uint8_t Padding3[144];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXMaterialImage
{
    uint32_t SemanticHash;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIPAKImageEntry
{
    uint64_t ImageInfoPacked;
    uint64_t ImagePAKInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIGfxMip
{
    uint16_t Width;
    uint16_t Height;

    uint8_t Padding[4];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIGfxImage
{
    uint64_t NamePtr;

    uint8_t Padding[0x18];

    uint64_t LoadedImagePtr;

    uint16_t Width;
    uint16_t Height;
    uint32_t ImageBufferSize;

    WWIIGfxMip MipLevels[3];

    uint8_t ImageFormat;

    uint8_t Padding2[0x17];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIISoundAlias
{
    uint64_t NamePtr;
    uint64_t AliasEntrysPtr;
    uint64_t Unknown1;
    uint64_t AliasEntryCount;
};

struct WWIISoundEntry
{
    uint64_t AliasNamePtr;
    uint8_t Unknown1[0x20];
    uint64_t SoundFilePtr;
    uint8_t Unknown2[0x120];
};

struct WWIISoundFileBase
{
    uint8_t SoundFileType;
    uint8_t SoundFileExists;
    uint8_t Padding[6];
};

struct WWIILoadedSoundFile
{
    uint64_t SoundFileName;

    uint8_t Unknown1[0x28];

    uint32_t BufferSize;
    uint32_t FrameCount;
    uint16_t FrameRate;
    uint8_t ChannelCount;
    uint16_t BitsPerSample;
    uint8_t BlockAlign;
    uint8_t Format;
    uint8_t Padding;

    uint64_t SoundDataPtr;
    uint32_t SoundDataSize;
};

struct WWIIPrimedSoundFile
{
    uint64_t IsPrimed;
    uint64_t SoundFilePath;
    uint64_t SoundFileName;
    uint64_t PackFileOffset;
    uint32_t PackFileSize;

    uint8_t IsLocalized;
    uint8_t Flag1;
    uint8_t PackFileIndex;
    uint8_t Flag2;

    uint32_t BufferSize;
    uint32_t FrameCount;
    uint16_t FrameRate;
    uint8_t ChannelCount;
    uint16_t BitsPerSample;
    uint8_t BlockAlign;
    uint8_t Format;
    uint8_t Padding;
};

struct WWIIStreamedSoundFile
{
    uint64_t ExtendedInfoPtr;

    uint64_t IsLoaded;
    uint64_t SoundFilePath;
    uint64_t SoundFileName;

    uint64_t PackFileOffset;
    uint32_t PackFileSize;

    uint8_t IsLocalized;
    uint8_t Flag1;
    uint8_t PackFileIndex;
    uint8_t Flag2;
};

struct WWIIStreamedSoundInfo
{
    uint64_t SoundFileName;

    uint8_t Unknown1[0x28];

    uint32_t BufferSize;
    uint32_t FrameCount;
    uint16_t FrameRate;
    uint8_t ChannelCount;
    uint16_t BitsPerSample;
    uint8_t BlockAlign;
    uint8_t Format;
    uint8_t Padding;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXStreamSurface
{
    uint16_t Unknown1;

    char SubmeshKeyHash[0x21];

    uint8_t Padding1;

    uint32_t VerticiesCount;
    uint32_t TriIndiciesCount;

    uint32_t Unknown2;

    uint64_t UnknownPtr1;
    uint64_t UnknownPtr2;
    uint64_t LoadedSurfaceInfoPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXStreamSurfaceInfo
{
    uint64_t FacesPtr;
    uint64_t VertexPositionsPtr;
    uint64_t VertexTangentsPtr;
    uint64_t VertexColorsPtr;
    uint64_t VertexUVsPtr;
    uint64_t VertexNormalsPtr;

    uint8_t Padding[0x90];

    uint64_t VertexWeightsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXModelSurface
{
    uint16_t Flags;

    uint16_t VertexCount;
    uint16_t FacesCount;

    uint8_t VertListCount;
    uint8_t PaddingCount;

    uint64_t Unknown1;

    uint64_t XStreamSurfacePtr;
    uint64_t UnknownPtr1;
    uint64_t RigidWeightsPtr;
    uint64_t UnknownPtr3;

    uint8_t Padding1[0x38];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXModelLod
{
    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[0x40];

    uint64_t SurfsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXModel
{
    uint64_t AlternateNamePtr;

    uint16_t NumBones;
    uint16_t NumRootBones;

    uint8_t Padding1[0x4C];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;

    WWIIXModelLod ModelLods[6];

    uint8_t NumLods;

    uint8_t Padding2[0x10F];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXModelBase
{
    uint64_t NamePtr;

    uint64_t XModelPtr;
    uint64_t MaterialHandlesPtr;
    uint64_t UnknownPtr;

    uint32_t Unknown1;
    uint32_t Unknown2;

    uint8_t Padding1[2];

    uint8_t XModelPartCount;

    uint8_t Padding2[5];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXAnim
{
    uint64_t NamePtr;

    uint8_t Padding[0x6];

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

    uint8_t Padding2[0x14];

    float Framerate;
    float Frequency;

    uint8_t Padding3[0x4];

    uint64_t BoneIDsPtr;
    uint64_t DataBytePtr;
    uint64_t DataShortPtr;
    uint64_t DataIntPtr;
    uint64_t RandomDataShortPtr;
    uint64_t RandomDataBytePtr;
    uint64_t RandomDataIntPtr;
    uint64_t LongIndiciesPtr;
    uint64_t NotificationsPtr;
    uint64_t DeltaPartsPtr;

    uint8_t Padding4[0x28];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma endregion

#pragma pack(push, 1)
struct MW4XModelLod
{
    uint64_t MeshPtr;
    uint64_t SurfsPtr;

    float LodDistance;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[40];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XModel
{
    uint64_t NamePtr;
    uint16_t NumSurfaces;
    uint8_t NumLods;
    uint8_t MaxLods;

    uint8_t Padding[8];

    uint8_t NumBones;
    uint8_t NumRootBones;
    uint16_t UnkBoneCount;

    uint8_t Padding3[0x80];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t UnknownPtr2;
    uint64_t MaterialHandlesPtr;

    MW4XModelLod ModelLods[6];

    uint8_t Padding4[80];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XModelMesh
{
    uint64_t NamePtr;
    uint64_t SurfsPtr;
    uint64_t LODStreamKey;
    uint8_t Padding[0x18];

    uint64_t MeshBufferPointer;
    uint16_t NumSurfs;
    uint8_t Padding2[38];

};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XModelMeshBufferInfo
{
    uint64_t BufferPtr; // 0 if the model is not loaded, otherwise a pointer, even if streamed
    uint32_t BufferSize;
    uint32_t Streamed;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct IWXModelSurfaceT
{
    int8_t TileMode;
    int8_t Deformed;

    uint16_t VertexCount;
    uint16_t FacesCount;
    uint8_t VertListCount;
    uint8_t PaddingCount;

    uint16_t WeightCounts[8];

    uint8_t Padding[8];

    uint64_t VerticiesPtr;
    uint64_t FacesPtr;

    uint8_t Padding2[24];

    uint64_t RigidWeightsPtr;
    uint64_t WeightsPtr;

    uint8_t Padding3[0xA8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XModelSurface
{
    uint16_t StatusFlag;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t Padding6;
    uint8_t VertListCount;

    uint8_t PaddingNew[11];
    uint16_t WeightCounts[8];
    uint8_t Padding1[4];

    // Offset 1 - Vertex Data (Offsets and UV)
    // Offset 2 - Face Indices
    // Offset 3 - Unknown, 24-bytes per entry, count not found
    // Offset 4 - Vertex Colours, not always present (4 * Vertex Count)
    // Offset 5 - Unknown, Half Floats that work when parsed as UVs (4 * Vertex Count)
    // Offset 6 - Unknown, might be normal values, not always present (4 * Vertex Count)
    // Offset 7 - Not seen yet; Always -1
    // Offset 8 - Not seen yet; Always -1
    uint32_t Offsets[12];

    uint64_t MeshBufferPointer;
    uint64_t Padding3;
    uint64_t RigidWeightsPtr;
    uint64_t WeightsPtr;

    uint8_t Padding4[40];

    float XOffset;
    float YOffset;
    float ZOffset;
    float Scale;
    float Min;
    float Max;
    uint8_t Padding5[24];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4GfxMip
{
    uint64_t HashID;
    uint8_t Padding[24];
    uint32_t Size;
    uint16_t Width;
    uint16_t Height;
};
#pragma pack(pop)

#pragma pack(push, 1)
template <size_t Size>
struct MW4GfxMipArray
{
    MW4GfxMip Levels[Size];

    size_t GetImageSize(size_t i)
    {
        if (i == 0)
            return (size_t)(Levels[i].Size >> 4);
        else
            return (size_t)((Levels[i].Size >> 4) - (Levels[i - 1].Size >> 4));
    }

    size_t GetMipCount()
    {
        return Size;
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4GfxImage
{
    uint64_t NamePtr;

    uint8_t Padding[12];

    uint8_t ImageFormat;

    uint8_t Padding2[15];

    uint16_t LoadedMipWidth;
    uint16_t LoadedMipHeight;

    uint8_t Padding3[10];

    uint8_t LoadedMipLevels;

    uint8_t Padding4[5];

    MW4GfxMipArray<4> Mips;

    uint8_t Padding5[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XAnim
{
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
    uint8_t Flags;

    uint8_t NoneRotatedBoneCount;
    uint8_t TwoDRotatedBoneCount;
    uint8_t NormalRotatedBoneCount;
    uint8_t TwoDStaticRotatedBoneCount;
    uint8_t NormalStaticRotatedBoneCount;
    uint8_t NormalTranslatedBoneCount;
    uint8_t PreciseTranslatedBoneCount;
    uint8_t StaticTranslatedBoneCount;
    uint8_t NoneTranslatedBoneCount;
    uint8_t TotalBoneCount;

    uint8_t NotificationCount;

    uint8_t AssetType;

    uint8_t Padding3[27];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XAnimDeltaParts
{
    uint64_t DeltaTranslationsPtr;
    uint64_t Delta2DRotationsPtr;
    uint64_t Delta3DRotationsPtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XMaterial
{
    uint64_t NamePtr;
    uint8_t Padding[20];
    uint8_t ImageCount;
    uint8_t Padding2[35];
    uint64_t TechsetPtr;
    uint64_t ImageTablePtr;
    uint8_t Padding3[40];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4XMaterialImage
{
    uint32_t Type;
    uint8_t Padding[4];
    uint64_t ImagePtr;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MW4SoundBank
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

#pragma region Black Ops Cold War

#pragma pack(push, 1)
struct BOCWSoundAsset
{
    uint64_t NamePtr;
    uint32_t FrameCount;
    uint8_t FrameRateIndex;
    uint8_t ChannelCount;
    uint8_t Looping;
    uint8_t Format;
    uint32_t Unknown2;
    uint32_t BufferSize;
    uint8_t Padding[32];
    uint64_t StreamKey;
    uint64_t StreamInfoPtr;
    uint8_t Padding1[16];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXAnim
{
    uint64_t UnknownPtr;
    uint64_t BoneIDsPtr;
    uint64_t UnknownZero1;
    uint64_t DeltaPartsPtr;
    uint8_t Unknown1[0x10];
    uint64_t UnknownPtr2;
    uint64_t NotificationsPtr;
    uint8_t Unknown2[0x18];
    uint64_t DataBytePtr;
    uint64_t UnknownZero3;
    uint64_t RandomDataBytePtr;
    uint64_t NamePtr;
    uint64_t DataIntPtr;
    uint64_t UnknownZero2;
    uint64_t DataShortPtr;
    uint64_t RandomDataShortPtr;
    uint8_t Unknown3[0x18];
    float Framerate;
    float Frequency;
    uint8_t Unknown4[0x34];
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
    uint16_t Unknown6;
    uint16_t NumFrames;
    uint8_t Unknown[0x1A];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWGfxMip
{
    uint64_t HashID;

    uint8_t Padding[0x10];

    uint32_t Size;
    uint16_t Width;
    uint16_t Height;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWGfxImage
{
    uint64_t NamePtr;
    uint64_t UnknownPtr1;
    uint64_t UnknownZero;
    uint64_t UnknownZero2;
    uint64_t UnknownZero3;
    uint64_t LoadedMipPtr;
    uint64_t GfxMipsPtr;
    uint8_t Padding[96];
    uint32_t LoadedMipSize;
    uint32_t ImageFormat;
    uint16_t LoadedMipWidth;
    uint16_t LoadedMipHeight;
    uint8_t LoadedMipLevels;
    uint8_t Padding1[19];
    uint8_t GfxMipMaps;
    uint8_t Padding2[23];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXMaterial
{
    uint64_t NamePtr;
    uint8_t Padding[0x20];
    uint64_t TechsetPtr;
    uint64_t ImageTablePtr;
    uint8_t Padding2[0x110];
    uint8_t ImageCount;
    uint8_t Padding3[15];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXSkeleton
{
    uint64_t NamePtr;
    uint64_t BoneIDsPtr;
    uint64_t UnknownPtr2;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr1;
    uint16_t BoneCounts[4];
    uint64_t UnknownZero2;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXModel
{
    uint64_t NamePtr;
    uint64_t XSkeletonPtr;
    uint64_t XCollisionPtr;
    uint64_t UnknownPtr2;
    uint64_t ModelLodPtrs[8];
    uint64_t MaterialHandlesPtr;
    uint64_t UnknownPtr4;
    uint16_t NumLods;
    uint16_t UnkValue01;
    uint8_t Padding3[0x74];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXModelLod
{
    uint64_t NamePtr;
    uint64_t XSurfacePtr;
    uint64_t XModelMeshPtr;
    uint64_t UnknownPtr;
    uint64_t LODStreamKey;
    uint8_t Padding[0x10];
    float LodDistance;
    uint8_t NumSurfs;
    uint8_t SurfacesIndex;
    uint16_t Pad;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXModelSurface
{
    uint8_t Flag1;
    uint8_t Flag2;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t Padding;
    uint32_t VerticiesIndex;
    uint32_t FacesIndex;
    uint8_t Padding2[0x20];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BOCWXModelMeshInfo
{
    uint8_t StatusFlag;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t Flag4;

    uint32_t VertexCount;
    uint32_t WeightCount;
    uint32_t FacesCount;
    uint32_t BlendCount;
    uint32_t BlendShapeCount;
    uint8_t Padding[0x8];

    uint64_t XModelMeshBufferPtr;
    uint32_t XModelMeshBufferSize;

    uint32_t VertexOffset;
    uint32_t UVOffset;
    uint32_t FacesOffset;
    uint32_t WeightsOffset;
    uint32_t BlendshapesOffset;

    uint8_t UnknownData[360];

    uint32_t FuckKnows;
    uint32_t ShapeCount;
    uint32_t FuckKnowsAgain;
    uint32_t HowTheFuck;
    uint64_t BlendShapeNames;
    uint64_t BlendShapeIndexes;
    uint64_t BlendShapeCounts;
};
#pragma pack(pop)

#pragma endregion

struct VGXAssetPoolData
{
    // The beginning of the pool
    uint64_t PoolPtr;

    // A pointer to the closest free header
    uint64_t PoolFreeHeadPtr;

    // The maximum pool size
    uint32_t PoolSize;

    // The size of the asset header
    uint32_t AssetSize;
};

struct VGSoundBankInfo
{
    uint64_t BankNamePointer;
    uint64_t StreamKey;
    uint8_t Padding[8];
    uint64_t BankFilePointer;
    uint64_t UnkPointer;
    uint32_t BankFileSize;
    uint32_t Unk;
};

struct VGSoundAlias
{
    uint64_t NamePtr;
    uint32_t NameHash;
    uint32_t Padding;
    uint64_t EntriesPtr;
    uint64_t EntriesCount;
};

struct VGSoundAliasEntry
{
    uint64_t FilePtr;
    uint32_t FileHash;
    uint32_t Unk;
    uint64_t NamingInfoPtr;
    uint64_t UnkPtr2;
};

// In the Season 5 patch the Name/Secondary pointers are no longer stored directly in the sound alias entry and are instead their own structure pointed to from the entry.
struct VGSoundAliasNamingInfo
{
    uint64_t NamePtr;
    uint64_t SecondaryPtr;
};

#pragma pack(push, 1)
struct VGXModelLod
{
    uint64_t MeshPtr;
    uint64_t SurfsPtr;

    float LodDistance;
    float LodDistanceAgain;

    uint16_t NumSurfs;
    uint16_t SurfacesIndex;

    uint8_t Padding[36];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGXModel
{
    uint64_t NamePtr;
    uint16_t NumSurfaces;
    uint8_t NumLods;
    uint8_t MaxLods;

    uint8_t Padding[12];

    uint8_t NumBones;
    uint8_t NumRootBones;
    uint16_t UnkBoneCount;

    uint8_t Padding3[100];

    uint64_t BoneIDsPtr;
    uint64_t ParentListPtr;
    uint64_t RotationsPtr;
    uint64_t TranslationsPtr;
    uint64_t PartClassificationPtr;
    uint64_t BaseMatriciesPtr;
    uint64_t UnknownPtr;
    uint64_t UnknownPtr2;
    uint64_t MaterialHandlesPtr;

    uint64_t ModelLodsPtr;

    uint8_t Padding4[184];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGXModelSurface
{
    uint16_t StatusFlag;
    uint16_t VertexCount;
    uint16_t FacesCount;
    uint16_t Padding6;
    uint8_t VertListCount;
    uint8_t Padding0[19];
    uint16_t WeightCounts[8];
    uint8_t Padding1[4];
    float NewScale;
    uint32_t Offsets[9];
    uint64_t MeshBufferPointer;
    uint64_t Padding3;
    uint64_t RigidWeightsPtr;
    uint64_t WeightsPtr;
    uint8_t Padding4[40];
    float XOffset;
    float YOffset;
    float ZOffset;
    float Scale;
    float Min;
    float Max;
    uint8_t Padding7[24];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGXModelMesh
{
    uint64_t NamePtr;
    uint64_t SurfsPtr;
    uint64_t LODStreamKey;
    uint8_t Padding[8];
    uint64_t MeshBufferPointer;
    uint8_t Padding2[40];

};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGXMaterial
{
    uint64_t NamePtr;
    uint8_t Padding[20];
    uint8_t ImageCount;
    uint8_t Padding2[19];
    uint64_t TechsetPtr;
    uint64_t ImageTablePtr;
    uint8_t Padding3[64];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct VGSoundBank
{
    uint64_t NamePtr;
    uint64_t BankNamePtr;
    uint64_t LanguagePtr;
    uint64_t TypePtr;
    uint64_t AliasCount;
    uint64_t AliasesPtr;
    uint8_t Padding[432];
    uint64_t SoundBankPtr;
    uint64_t StreamedSoundsPtr;
    uint32_t StreamedSoundCount;
    uint32_t Unk2;
};
#pragma pack(pop)