#pragma once

#include <string>

// We need the WraithAsset type
#include "WraithAsset.h"
#include "WraithAnim.h"

// We need the base game assets
#include "DBGameAssets.h"

// We need the following classes
#include "Image.h"
#include "CoDXAnimReader.h"

// The asset sort methods.
enum class AssetCompareMethod
{
    // No comparison is made.
    None,
    // Compares the assets by name only.
    ByName,
    // Compares the assets by details if they are the same type, otherwise their name.
    ByDetails,
};

// A class to help with Asset Comparison
class AssetCompareMethodHelper
{
public:
    // Calculates the compare method from a string.
    static AssetCompareMethod CalculateCompareMethod(const std::string& compareMethodStr);
};

// A class that represents an asset
class CoDAsset_t : public WraithAsset
{
public:
    CoDAsset_t();
    virtual ~CoDAsset_t();

    // -- Asset properties

    // Whether or not this is a file entry
    bool IsFileEntry;

    // A pointer to the asset in memory
    uint64_t AssetPointer;

    // The assets offset in the loaded pool
    uint32_t AssetLoadedIndex;

    // -- Asset Methods

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    virtual bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};

// A class that represents an asset pool
class AssetPool
{
public:
    ~AssetPool();

    // A list of loaded assets
    std::vector<CoDAsset_t*> LoadedAssets;
};

// -- Classes that extend general cod assets (models, animations, sounds, images, fx, etc)

// A class that represents an animation asset
class CoDAnim_t : public CoDAsset_t
{
public:
    CoDAnim_t();
    virtual ~CoDAnim_t();

    // -- Animation properties

    // The bones
    std::vector<std::string> BoneNames;
    // The framerate
    float Framerate;
    // The number of frames
    uint32_t FrameCount;
    // The number of frames
    uint32_t BoneCount;
    // The number of shapes
    uint32_t ShapeCount;

    // Modern games
    bool isSiegeAnim;

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};

// A class that represents a model asset
class CoDModel_t : public CoDAsset_t
{
public:
    CoDModel_t();
    virtual ~CoDModel_t();

    // -- Model properties

    // The bones
    std::vector<std::string> BoneNames;
    // The bone count
    uint32_t BoneCount;
    // The cosmetic bone count
    uint32_t CosmeticBoneCount;
    // The lod count
    uint16_t LodCount;

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};

// A class that represents an image asset
class CoDImage_t : public CoDAsset_t
{
public:
    CoDImage_t();
    virtual ~CoDImage_t();

    // -- Image properties

    // The width
    uint16_t Width;
    // The height
    uint16_t Height;
    // The format
    uint16_t Format;

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};

class CoDRawFile_t : public CoDAsset_t
{
public:
    CoDRawFile_t();
    virtual ~CoDRawFile_t();

    // -- Rawfile properties

    // The file path
    std::string RawFilePath;
    // The data itself
    uint64_t RawDataPointer;

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};

// Types of sound assets
enum class SoundDataTypes
{
    WAV_WithHeader,
    WAV_NeedsHeader,
    FLAC_WithHeader,
    FLAC_NeedsHeader,
    Opus_Interleaved,
    Opus_Interleaved_Streamed,
};

// A class that represents a sound asset
class CoDSound_t : public CoDAsset_t
{
public:
    CoDSound_t();
    virtual ~CoDSound_t();

    // -- Sound properties

    // The full file path of the audio file
    std::string FullPath;
    // The framerate of this audio file
    uint32_t FrameRate;
    // The total frame count
    uint32_t FrameCount;
    // Channels count
    uint8_t ChannelsCount;
    // Channels count
    uint32_t Length;
    // The index of the sound package for this entry
    uint16_t PackageIndex;
    // Whether or not this is a localized entry
    bool IsLocalized;

    // The datatype for this entry
    SoundDataTypes DataType;

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};

// -- Generic structures for translating similar game formats

// Types of bone data
enum class BoneDataTypes
{
    DivideBySize,
    QuatPackingA,
    HalfFloat
};

struct XModelSubmesh_t
{
    // Constructor
    XModelSubmesh_t();

    // Count of rigid vertex pairs
    uint32_t VertListcount;
    // A pointer to rigid weights
    uint64_t RigidWeightsPtr;

    // Count of verticies
    uint32_t VertexCount;
    // Count of faces
    uint32_t FaceCount;

    // Pointer to faces
    uint64_t FacesPtr;
    // Pointer to verticies
    uint64_t VertexPtr;
    // Pointer to verticies
    uint64_t VertexNormalsPtr;
    // Pointer to UVs
    uint64_t VertexUVsPtr;
    // Pointer to vertex colors
    uint64_t VertexColorPtr;

    // A list of weights
    uint16_t WeightCounts[8];
    // A pointer to weight data
    uint64_t WeightsPtr;
    // A pointer to blendshapes data
    uint64_t BlendShapesPtr;

    // Submesh Scale
    float Scale;

    // Submesh Offset
    float XOffset;
    float YOffset;
    float ZOffset;

    // The index of the material
    int32_t MaterialIndex;
};

// Types of images
enum class ImageUsageType : uint8_t
{
    Unknown,
    DiffuseMap,
    NormalMap,
    SpecularMap,
    GlossMap,
    AmbientOcclusionMap
};

struct XImage_t
{
    // Constructor
    XImage_t(ImageUsageType Usage, uint32_t Hash, uint64_t Pointer, const std::string& Name);

    // The usage of this image asset
    ImageUsageType ImageUsage;
    // The pointer to the image asset
    uint64_t ImagePtr;

    uint32_t SemanticHash;

    // The name of this image asset
    std::string ImageName;
};

struct XMaterialSetting_t
{
    // Constructors
    XMaterialSetting_t(
        const char* name,
        const char* type,
        const float* data,
        const size_t numElements);

    XMaterialSetting_t(
        const char* name,
        const char* type,
        const int32_t* data,
        const size_t numElements);

    XMaterialSetting_t(
        const char* name,
        const char* type,
        const uint32_t* data,
        const size_t numElements);

    // The name
    std::string Name;
    // The type
    std::string Type;
    // The data
    float Data[4];
};

struct XMaterial_t
{
    // Constructor
    XMaterial_t(uint32_t ImageCount);

    // The material name
    std::string MaterialName;
    // The techset name
    std::string TechsetName;
    // The surface type name
    std::string SurfaceTypeName;

    // A list of images
    std::vector<XImage_t> Images;

    // A list of settings, if any
    std::vector<XMaterialSetting_t> Settings;
};

struct XModelLod_t
{
    // Constructors
    XModelLod_t(uint16_t SubmeshCount);

    // Name of the LOD
    std::string Name;
    // A list of submeshes for this specific lod
    std::vector<XModelSubmesh_t> Submeshes;
    // A list of material info per-submesh
    std::vector<XMaterial_t> Materials;

    // A key used for the lod data if it's streamed
    uint64_t LODStreamKey;
    // A pointer used for the stream mesh info
    uint64_t LODStreamInfoPtr;

    // The distance this lod displays at
    float LodDistance;
    // The max distance this lod displays at
    float LodMaxDistance;
};

struct XModel_t
{
    // Constructor
    XModel_t(uint16_t LodCount);

    // The name of the asset
    std::string ModelName;

    // The type of data used for bone rotations
    BoneDataTypes BoneRotationData;

    // Whether or not we use the stream mesh loader
    bool IsModelStreamed;

    // The model bone count
    uint32_t BoneCount;
    // The model root bone count
    uint32_t RootBoneCount;
    // The model cosmetic bone count
    uint32_t CosmeticBoneCount;
    // The model blendshape count
    uint32_t BlendShapeCount;

    // A pointer to bone name string indicies
    uint64_t BoneIDsPtr;
    // The size of the bone name index
    uint8_t BoneIndexSize;

    // A pointer to bone parent indicies
    uint64_t BoneParentsPtr;
    // The size of the bone parent index
    uint8_t BoneParentSize;

    // A pointer to local rotations
    uint64_t RotationsPtr;
    // A pointer to local positions
    uint64_t TranslationsPtr;

    // A pointer to global matricies
    uint64_t BaseMatriciesPtr;

    // A pointer to the bone collision data, hitbox offsets
    uint64_t BoneInfoPtr;

    // A pointer to the blendshape names
    uint64_t BlendShapeNamesPtr;

    // A list of lods per this model
    std::vector<XModelLod_t> ModelLods;
};

// Types of key data
enum class AnimationKeyTypes
{
    DivideBySize,
    MinSizeTable,
    QuatPackingA,
    HalfFloat
};

// A structure that represents an xanim from Call of Duty
struct XAnim_t
{
    // Constructor
    XAnim_t();

    // The name of the asset
    std::string AnimationName;

    // The framerate of this animation
    float FrameRate;
    // The framecount of this animation
    uint32_t FrameCount;

    // Whether or not this is a viewmodel animation
    bool ViewModelAnimation;
    // Whether or not this animation loops
    bool LoopingAnimation;
    // Whether or not this animation is additive
    bool AdditiveAnimation;
    // Whether or not we support inline-indicies
    bool SupportsInlineIndicies;

    // A pointer to bone name string indicies
    uint64_t BoneIDsPtr;
    // The size of the bone name index
    uint8_t BoneIndexSize;
    // The size of the inline bone indicies (0) when not in use
    uint8_t BoneTypeSize;

    // What type of rotation data we have
    AnimationKeyTypes RotationType;
    // What type of translation data we have
    AnimationKeyTypes TranslationType;

    // A pointer to animation byte size data
    uint64_t DataBytesPtr;
    // A pointer to animation short size data
    uint64_t DataShortsPtr;
    // A pointer to animation integer size data
    uint64_t DataIntsPtr;

    // A pointer to animation (rand) byte size data
    uint64_t RandomDataBytesPtr;
    // A pointer to animation (rand) short size data
    uint64_t RandomDataShortsPtr;
    // A pointer to animation (rand) integer size data
    uint64_t RandomDataIntsPtr;

    // A pointer to animation indicies (When we have more than 255)
    uint64_t LongIndiciesPtr;
    // A pointer to animation notetracks
    uint64_t NotificationsPtr;
    // A pointer to blendshape names
    uint64_t BlendShapeNamesPtr;
    // A pointer to blendshape weights
    uint64_t BlendShapeWeightsPtr;

    // A pointer to animation delta translations
    uint64_t DeltaTranslationPtr;
    // A pointer to 2D animation delta rotations
    uint64_t Delta2DRotationsPtr;
    // A pointer to 3D animation delta rotations
    uint64_t Delta3DRotationsPtr;

    // The count of non-rotated bones
    uint32_t NoneRotatedBoneCount;
    // The count of 2D rotated bones
    uint32_t TwoDRotatedBoneCount;
    // The count of 3D rotated bones
    uint32_t NormalRotatedBoneCount;
    // The count of 2D static rotated bones
    uint32_t TwoDStaticRotatedBoneCount;
    // The count of 3D static rotated bones
    uint32_t NormalStaticRotatedBoneCount;
    // The count of normal translated bones
    uint32_t NormalTranslatedBoneCount;
    // The count of precise translated bones
    uint32_t PreciseTranslatedBoneCount;
    // The count of static translated bones
    uint32_t StaticTranslatedBoneCount;
    // The count of non-translated bones
    uint32_t NoneTranslatedBoneCount;
    // The total bone count
    uint32_t TotalBoneCount;
    // The count of notetracks
    uint32_t NotificationCount;
    // The count of blendshape weights
    uint32_t BlendShapeWeightCount;

    // XAnim Reader (Streamed)
    std::unique_ptr<CoDXAnimReader> Reader;
    // XAnim Reader Function
    std::function<void(const std::unique_ptr<XAnim_t>&, std::unique_ptr<WraithAnim>&)> ReaderFunction;
    // XAnim Reader Information Pointer.
    uint64_t ReaderInformationPointer;
};

struct XImageDDS
{
    // Constructors
    XImageDDS();
    ~XImageDDS();

    // The DDS data buffer
    int8_t* DataBuffer;
    // The size of the DDS buffer
    uint32_t DataSize;

    // The requested image patch type
    ImagePatch ImagePatchType;
};

struct XSound
{
    // Constructors
    XSound();
    ~XSound();

    // The sound data buffer
    int8_t* DataBuffer;
    // The size of the sound buffer
    uint32_t DataSize;

    // The sound format, WAV/FLAC
    SoundDataTypes DataType;
};

// A class that represents a material asset
class CoDMaterial_t : public CoDAsset_t
{
public:
    CoDMaterial_t();
    virtual ~CoDMaterial_t();

    // -- Material properties

    // The number of images
    size_t ImageCount;

    // Compares the asset to this one, if compare details is enabled, they are used if the assets are the same type.
    bool Compare(const CoDAsset_t* candidate, const AssetCompareMethod compareMethod) const;
};