#include "stdafx.h"

// The class we are implementing
#include "CoDEffectTranslator.h"

// We need the following WraithX classes
#include "ProcessReader.h"
#include "HalfFloats.h"
#include "Strings.h"
#include "TextWriter.h"
#include "FileSystems.h"
#include "VectorMath.h"
#include "SettingsManager.h"
#include "DBGameAssets.h"

// -- Begin Effect Structures

struct FxIntRange
{
	uint32_t Base;
	uint32_t Amplitude;
};

struct FxFloatRange
{
	float Base;
	float Amplitude;
};

struct FxElemAtlas
{
	uint8_t Behavior;
	uint8_t Index;
	uint8_t Fps;
	uint8_t LoopCount;
	uint8_t ColIndexBits;
	uint8_t RowIndexBits;
	uint16_t EntryCountAndIndexRange;
};

struct FxElemVec3Range
{
	float Base[3];
	float Amplitude[3];
};

struct FxTrailVertex
{
	float Position[2];
	float Normal[2];
	float TexCoord;
};

struct FxElemVelStateInFrame
{
	FxElemVec3Range Velocity;
	FxElemVec3Range TotalDelta;
};

struct FxElemVelStateSample
{
	FxElemVelStateInFrame Local;
	FxElemVelStateInFrame World;

	FxElemVelStateSample()
	{
		// Clear
		Local.Velocity.Base[0] = 0;
		Local.Velocity.Base[1] = 0;
		Local.Velocity.Base[2] = 0;
		Local.Velocity.Amplitude[0] = 0;
		Local.Velocity.Amplitude[1] = 0;
		Local.Velocity.Amplitude[2] = 0;
	}
};

struct BOFxTrailDef // Compatible with Black Ops 1-2
{
	int32_t ScrollTimeMsec;
	int32_t RepeatDist;
	int32_t SplitDist;
	uint32_t VertCount;
	uint32_t VertPtr;
	uint32_t IndCount;
	uint32_t IndexPtr;
};

struct BO3FxTrailDef // Compatible with Black Ops 3
{
	int32_t ScrollTimeMsec;
	int32_t RepeatDist;
	int32_t SplitDist;
	float FadeInDist;
	float FadeOutDist;
	uint32_t VertCount;
	uint64_t VertPtr;
	uint32_t IndCount;
	uint32_t Padding;
	uint64_t IndexPtr;
};

struct BOFxElemVisualState // Compatible with Black Ops 1-2
{
	uint8_t Color[4];
	float RotationDelta;
	float RotationTotal;
	float Size[2];
	float Scale;
};

struct BOFxElemVisStateSample // Compatible with Black Ops 1-2
{
	BOFxElemVisualState Base;
	BOFxElemVisualState Amplitude;
};

struct BO3FxElemVisualState // Compatible with Black Ops 3
{
	uint8_t Color[4];
	float RotationDelta;
	float RotationTotal;
	float Size[2];
	float Scale;
	float LightIntensity;
	float LightRadius;
	float LightFOV;
	float ChildScale;
};

struct BO3FxElemVisStateSample // Compatible with Black Ops 3
{
	BO3FxElemVisualState Base;
	BO3FxElemVisualState Amplitude;
};

struct FxLensFlare
{
	char LensFlareHash[0x10];
	float LFSourceDir[3];
	float LFSourceSize;
};

struct FxSpotLight
{
	float SpotlightFOV;
	float SpotlightStartRadius;
	float SpotlightEndRadius;
};

struct FxDynamicLight
{
	uint64_t NamePtr;

	uint8_t Unmapped[0x40];

	uint32_t PrimaryType;   // 0x4 = OMNI 0x2 = SPOT
	uint32_t GUID;

	float Origin[3];
	float FallOffDistance;
	float EncodedColors[3];
	float SpecularComp;

	float CutOn;
	float Radius;
	float NearEdge;
	float FarEdge;

	uint8_t CullingUsePureRadius;
	uint8_t Padding2[0x3];
	float CullingFallOff;
	float CullingCutOff;
	uint32_t PrimaryNoShadowMap;    // 0x4 = 0 0x1 = 1
	uint8_t ProbeOnly;
	uint8_t ExcludeDedicated;
	uint8_t Unmapped9[0xA];

	float PenumbraRadius;
	uint32_t ShadowUpdate;
	float Angles[3];

	uint8_t Unmapped2[0x64];

	float FOVOuter;
	float UnknownZero;

	float Roundness;
	float SuperEllipse[4];
	float OrthoEffect;

	uint8_t Unmapped7[0x18];

	float BulbLength;

	uint8_t Unmapped4[0xC];
	uint32_t ShadowMapScale;
	uint8_t Unmapped11[0xC];

	uint8_t Volumetric;
	uint8_t VolumetricCookies;
	uint16_t Padding;
	uint32_t VolumetricSampleCount;
	float VolumetricIntensityBoost;

	uint8_t Unmapped5[0x60];

	float DefAngle;
	float DefRotation;
	float DefTile[2];
	float DefScroll[2];
	float DefOffset[2];

	float ColMins[3];
	float ColMaxs[3];

	uint64_t UnknownPtr1;
	uint64_t UnknownPtr2;

	float Unknown1;
	uint32_t Unknown2;

	uint64_t UnknownPtr3;
	uint64_t NegativeOne;
	uint32_t Unknown3;
	float Unknown4;
};

union BO3FxVisuals
{
	uint64_t VisualPtr;
	char VisualString[0x40];
};

enum class FXAtlasFlags
{
	FX_ATLAS_START_FIXED = 0x0,
	FX_ATLAS_START_RANDOM = 0x1,
	FX_ATLAS_START_INDEXED = 0x2,
	FX_ATLAS_START_FIXED_RANGE = 0x3,
	FX_ATLAS_START_MASK = 0x3,
	FX_ATLAS_PLAY_OVER_LIFE = 0x4,
	FX_ATLAS_LOOP_ONLY_N_TIMES = 0x8,
	FX_ATLAS_LERP_FRAMES = 0x10,
	FX_ATLAS_REVERSE = 0x20,
};

enum class BOFXElementFlags : uint32_t	// Compatible with Black Ops 1-2
{
	FX_ELEM_SPAWN_RELATIVE_TO_EFFECT = 0x2,
	FX_ELEM_SPAWN_FRUSTUM_CULL = 0x4,
	FX_ELEM_RUNNER_USES_RAND_ROT = 0x8,
	FX_ELEM_SPAWN_OFFSET_NONE = 0x0,
	FX_ELEM_SPAWN_OFFSET_SPHERE = 0x10,
	FX_ELEM_SPAWN_OFFSET_CYLINDER = 0x20,
	FX_ELEM_SPAWN_OFFSET_MASK = 0x30,
	FX_ELEM_RUN_RELATIVE_TO_WORLD = 0x0,
	FX_ELEM_RUN_RELATIVE_TO_SPAWN = 0x40,
	FX_ELEM_RUN_RELATIVE_TO_EFFECT = 0x80,
	FX_ELEM_RUN_RELATIVE_TO_OFFSET = 0xC0,
	FX_ELEM_RUN_MASK = 0xC0,
	FX_ELEM_USE_COLLISION = 0x100,
	FX_ELEM_DIE_ON_TOUCH = 0x200,
	FX_ELEM_DRAW_PAST_FOG = 0x400,
	FX_ELEM_DRAW_WITH_VIEWMODEL = 0x800,
	FX_ELEM_BLOCK_SIGHT = 0x1000,
	FX_ELEM_USE_ITEM_CLIP = 0x2000,
	FX_ELEM_ORIENT_TYPE0 = 0x0,
	FX_ELEM_ORIENT_TYPE1 = 0x4000,
	FX_ELEM_ORIENT_TYPE2 = 0x8000,
	FX_ELEM_ORIENT_TYPE3 = 0xC000,
	FX_ELEM_ORIENT_TYPE4 = 0x10000,
	FX_ELEM_ORIENT_TYPE5 = 0x14000,
	FX_ELEM_ORIENT_MASK = 0x1C000,
	FX_ELEM_USE_WORLD_UP = 0x80000,
	FX_ELEM_ALIGN_VIEWPOINT = 0x100000,
	FX_ELEM_USE_BILLBOARD_PIVOT = 0x200000,
	FX_ELEM_USE_GAUSSIAN_CLOUD = 0x400000,
	FX_ELEM_USE_ROTATIONAXIS = 0x800000,
	FX_ELEM_HAS_VELOCITY_GRAPH_LOCAL = 0x1000000,
	FX_ELEM_HAS_VELOCITY_GRAPH_WORLD = 0x2000000,
	FX_ELEM_HAS_GRAVITY = 0x4000000,
	FX_ELEM_USE_MODEL_PHYSICS = 0x8000000,
	FX_ELEM_NONUNIFORM_SCALE = 0x10000000,
	FX_ELEM_FLAME_CHUNK = 0x20000000,
	FX_ELEM_HAS_REFLECTION = 0x40000000,
	FX_ELEM_IS_MATURE_CONTENT = 0x80000000,
};

enum class BO3FXElementFlags : uint32_t	// Compatible with Black Ops 3
{
	FX_ELEM_SPAWN_RELATIVE_TO_EFFECT = 0x2,
	FX_ELEM_SPAWN_FRUSTUM_CULL = 0x4,
	FX_ELEM_RUNNER_USES_RAND_ROT = 0x8,
	FX_ELEM_SPAWN_OFFSET_NONE = 0x0,
	FX_ELEM_SPAWN_OFFSET_SPHERE = 0x10,
	FX_ELEM_SPAWN_OFFSET_CYLINDER = 0x20,
	FX_ELEM_SPAWN_OFFSET_MASK = 0x30,
	FX_ELEM_RUN_RELATIVE_TO_WORLD = 0x0,
	FX_ELEM_RUN_RELATIVE_TO_SPAWN = 0x40,
	FX_ELEM_RUN_RELATIVE_TO_EFFECT = 0x80,
	FX_ELEM_RUN_RELATIVE_TO_OFFSET = 0xC0,
	FX_ELEM_RUN_RELATIVE_TO_EFFECT_NOW = 0x100,
	FX_ELEM_RUN_MASK = 0x1C0,
	FX_ELEM_USE_COLLISION = 0x200,
	FX_ELEM_DIE_ON_TOUCH = 0x400,
	FX_ELEM_DRAW_PAST_FOG = 0x800,
	FX_ELEM_DRAW_WITH_VIEWMODEL = 0x1000,
	FX_ELEM_BLOCK_SIGHT = 0x2000,
	FX_ELEM_USE_ITEM_CLIP = 0x4000,
	FX_ELEM_USE_OCCLUSION_QUERY = 0x8000,
	FX_ELEM_INHERIT_PARENT_MOVEMENT = 0x10000,
	FX_ELEM_USE_THERMAL = 0x20000,
	FX_ELEM_ATTRACTOR_GRAPH_ENABLE = 0x40000,
	FX_ELEM_USE_WORLD_UP = 0x80000,
	FX_ELEM_ALIGN_VIEWPOINT = 0x100000,
	FX_ELEM_USE_BILLBOARD_PIVOT = 0x200000,
	FX_ELEM_USE_GAUSSIAN_CLOUD = 0x400000,
	FX_ELEM_USE_ROTATIONAXIS = 0x800000,
	FX_ELEM_USE_MODEL_PHYSICS = 0x8000000,
	FX_ELEM_NONUNIFORM_SCALE = 0x10000000,
	FX_ELEM_NO_COMPUTE_SPRITES = 0x20000000,
	FX_ELEM_SPAWN_DYN_ENT = 0x40000000,
	FX_ELEM_IS_MATURE_CONTENT = 0x80000000,
};

enum class FXElementExtraFlags : uint32_t
{
	FX_ELEM_DISTRIBUTE_X = 0x1,
	FX_ELEM_DISTRIBUTE_Y = 0x2,
	FX_ELEM_DISTRIBUTE_Z = 0x4,
	FX_ELEM_TEAM_FRIENDLY = 0x8,
	FX_ELEM_TEAM_FOE = 0x10,
	FX_ELEM_CAST_SHADOW = 0x20,
	FX_ELEM_UNDERWATER_ONLY = 0x40,
	FX_ELEM_OVERWATER_ONLY = 0x80,
	FX_ELEM_WATER_RELAXED_CULL = 0x100,
	FX_ELEM_GAMEPLAY_INTENSIVE = 0x400,
	FX_ELEM_USE_SIZE_BASED_DENSITY_EMISSION = 0x800,
	FX_ELEM_USE_SIZE_BASED_DENSITY_ATTACHMENT = 0x1000,
	FX_ELEM_LF_OCCLUDE_GEOMETRY = 0x2000,
	FX_ELEM_LF_OCCLUDE_PARTICLES = 0x4000,
	FX_ELEM_RESPECT_EXCLUSION_VOLUMES = 0x8000,
	FX_ELEM_NO_GPU_FX = 0x10000,
	FX_ELEM_INDOOR = 0x20000,
	FX_ELEM_USE_GLOBAL_TINT = 0x80000,
	FX_ELEM_USE_TRAIL_TRAPEZOID_FIX = 0x100000,
};

enum class BOFXElementType : uint8_t // Compatible with Black Ops 1-2
{
	FX_ELEM_TYPE_SPRITE_BILLBOARD = 0x0,
	FX_ELEM_TYPE_SPRITE_ORIENTED = 0x1,
	FX_ELEM_TYPE_SPRITE_ROTATED = 0x2,
	FX_ELEM_TYPE_TAIL = 0x3,
	FX_ELEM_TYPE_LINE = 0x4,
	FX_ELEM_TYPE_TRAIL = 0x5,
	FX_ELEM_TYPE_CLOUD = 0x6,
	FX_ELEM_TYPE_MODEL = 0x7,
	FX_ELEM_TYPE_OMNI_LIGHT = 0x8,
	FX_ELEM_TYPE_SPOT_LIGHT = 0x9,
	FX_ELEM_TYPE_SOUND = 0xA,
	FX_ELEM_TYPE_DECAL = 0xB,
	FX_ELEM_TYPE_RUNNER = 0xC,
	FX_ELEM_TYPE_COUNT = 0xD,
	FX_ELEM_TYPE_LAST_SPRITE = 0x5,
	FX_ELEM_TYPE_LAST_DRAWN = 0x9,
};

enum class BO3FXElementType : uint8_t // Compatible with Black Ops 3
{
	FX_ELEM_TYPE_SPRITE_BILLBOARD = 0x0,
	FX_ELEM_TYPE_SPRITE_ORIENTED = 0x1,
	FX_ELEM_TYPE_SPRITE_ROTATED = 0x2,
	FX_ELEM_TYPE_TAIL = 0x3,
	FX_ELEM_TYPE_LINE = 0x4,
	FX_ELEM_TYPE_TRAIL = 0x5,
	FX_ELEM_TYPE_CLOUD = 0x6,
	FX_ELEM_TYPE_MODEL = 0x7,
	FX_ELEM_TYPE_DYNAMIC_LIGHT = 0x8,
	FX_ELEM_TYPE_OMNI_LIGHT = 0x9,
	FX_ELEM_TYPE_SPOT_LIGHT = 0xA,
	FX_ELEM_TYPE_DYNAMIC_SOUND = 0xb,
	FX_ELEM_TYPE_LENS_FLARE = 0xC,
	FX_ELEM_TYPE_DECAL = 0xD,
	FX_ELEM_TYPE_RUNNER = 0xE,
	FX_ELEM_TYPE_BEAM_SOURCE = 0xF,
	FX_ELEM_TYPE_BEAM_TARGET = 0x10,
	FX_ELEM_TYPE_COUNT = 0x10,
	FX_ELEM_TYPE_LAST_SPRITE = 0x5,
	FX_ELEM_TYPE_LAST_DRAWN = 0x9,
};

#pragma pack(1)
struct BOFxElemDef	// Compatible with Black Ops 1-2
{
	uint32_t Flags;

	FxIntRange Spawn;
	FxFloatRange SpawnRange;
	FxFloatRange FadeInRange;
	FxFloatRange FadeOutRange;

	float SpawnFrustumCullRadius;

	FxIntRange SpawnDelayMsec;
	FxIntRange LifeSpanMsec;
	FxFloatRange SpawnOrigin[3];
	FxFloatRange SpawnOffsetRadius;
	FxFloatRange SpawnOffsetHeight;
	FxFloatRange SpawnAngles[3];
	FxFloatRange AngularVelocity[3];
	FxFloatRange InitialRotation;

	uint32_t RotationAxis;

	FxFloatRange Gravity;
	FxFloatRange ReflectionFactor;
	FxElemAtlas Atlas;

	float WindInfluence;
	uint8_t ElemType;
	uint8_t VisualCount;
	uint8_t VelIntervalCount;
	uint8_t VisStateIntervalCount;

	uint32_t VelSamplesPtr;
	uint32_t VisSamples;

	uint32_t VisualsPtr;

	float CollMins[3];
	float CollMaxs[3];

	uint32_t EffectOnImpact;
	uint32_t EffectOnDeath;
	uint32_t EffectEmitted;

	FxFloatRange EmitDist;
	FxFloatRange EmitDistVariance;

	uint32_t EffectAttached;
	uint32_t FxTrailDefsPtr;

	uint8_t SortOrder;
	uint8_t LightingFrac;
	uint8_t Unused[2];

	uint16_t AlphaFadeTimeMsec;
	uint16_t MaxWindStrength;
	uint16_t SpawnIntervalAtMaxWind;
	uint16_t LifespanAtMaxWind;

	union
	{
		FxFloatRange BillboardTrim;
		FxIntRange BillboardCloudRange;
	} BillboardInfo;

	uint32_t SpawnSound;

	float BillboardPivot[2];
};

struct BO3FxElemDef // Compatible with Black Ops 3
{
	uint32_t Flags;
	uint32_t ExtraFlags;

	FxIntRange Spawn;
	FxIntRange SpawnLoopingSpawnCount;
	FxFloatRange SpawnRange;
	FxFloatRange FadeInRange;
	FxFloatRange FadeOutRange;

	float SpawnFrustumCullRadius;

	FxIntRange SpawnDelayMsec;
	FxIntRange LifeSpanMsec;
	FxFloatRange SpawnOrigin[3];
	FxFloatRange SpawnOffsetRadius;
	FxFloatRange SpawnOffsetHeight;

	float SpawnOffsetCylindricalAxis;

	FxFloatRange SpawnAngles[3];
	FxFloatRange AngularVelocity[3];
	FxFloatRange InitialRotation;

	uint32_t RotationAxis;

	FxFloatRange Gravity;
	FxFloatRange ReflectionFactor;
	FxElemAtlas Atlas;

	float WindInfluence;
	uint8_t ElemType;
	uint8_t VisualCount;
	uint8_t VelIntervalCount;
	uint8_t ParentMovementStateIntervalCount;
	uint8_t AttractorStateIntervalCount;
	uint8_t VisStateIntervalCount;
	uint16_t Padding;

	uint64_t VelSamplesPtr;
	uint64_t ParentMovementSamples;
	uint64_t VisSamples;
	uint64_t AttractorSamples;

	union
	{
		uint64_t VisualsPtr;
		FxLensFlare LensFlare;
		char BeamSource[0x40];
	} VisualsInfo;

	uint8_t Unused1[0x40];

	float CollMins[3];
	float CollMaxs[3];

	float AttractorLocalPosition[3];

	uint8_t Unused2[0x4];

	uint64_t EffectOnImpact;
	uint64_t EffectOnDeath;
	uint64_t EffectEmitted;

	FxFloatRange EmitDist;
	FxFloatRange EmitDistVariance;
	FxFloatRange EmitDensity;
	float EmitSizeForDensity;

	uint32_t UnknownPad;
	uint64_t EffectAttached;
	FxFloatRange AttachmentDensity;
	float AttachmentSizeForDensity;

	uint32_t UnknownPad2;

	union
	{
		uint64_t LensFlarePtr;
		uint64_t SpotLightPtr;
		uint64_t FxTrailDefsPtr;
	} LightingInfo;

	uint8_t Displacement;
	uint8_t LightingFrac;
	uint8_t Unused[2];

	uint16_t AlphaFadeTimeMsec;
	uint16_t MaxWindStrength;
	uint16_t SpawnIntervalAtMaxWind;
	uint16_t LifespanAtMaxWind;

	union
	{
		FxFloatRange BillboardTrim;
		FxIntRange BillboardCloudRange;
	} BillboardInfo;

	uint32_t Padding3;

	uint64_t SpawnSound;
	uint64_t SpawnSoundHash;

	uint64_t FollowSound;
	uint64_t FollowSoundHash;

	float BillboardPivot[2];

	float AlphaDissolve;
	float ZFeather;
	uint32_t FallOffBeginAngle;
	uint32_t FallOffEndAngle;

	uint8_t UnmappedData[0x20];
};

// Verify the structures
static_assert(sizeof(BOFxElemDef) == 0x124, "Invalid BO 1-2 ElemDef Size, Expected 0x124");
static_assert(sizeof(BO3FxElemDef) == 0x260, "Invalid BO 3 ElemDef Size, Expected 0x260");

// -- Utility functions

// Inverts the given integer range
void FxIntRangeInverse(FxIntRange& Range);
// Inverts the given float range
void FxFloatRangeInverse(FxFloatRange& Range);
// Converts angular velocity to degrees
float AngularVelToDegrees(float Value);
// Unpacks a packed vector 4 into a quaternion
Quaternion UnpackVec4Quat(uint32_t Value);
// Converts color data
Vector3 ConvertLinearToSRGB(const float Values[3]);

// -- Writer Functions

// Translates a vel sample to a curve, Compatible with Black Ops 1-3
void BOTranslateVelSample(TextWriter& Writer, const std::string& CurveName, uint32_t CurveIndex, const std::vector<FxElemVelStateSample>& VelSamples);
// Translate a vis sample to a curve, Compatible with Black Ops 1-2
void BOTranslateVisSample(TextWriter& Writer, const std::vector<BOFxElemVisStateSample>& VisSamples);
// Translate a vis sample to a curve, Compatible with Black Ops 3
void BO3TranslateVisSample(TextWriter& Writer, const std::vector<BO3FxElemVisStateSample>& VisSamples);
// Translates a generic sample to a curve, Compatible with Black Ops 1-3 (Generic Base+Amp)
void BOTranslateGenericSample(TextWriter& Writer, const std::string& CurveName, const std::vector<FxFloatRange>& GenericSamples);
// Translate a dynamic light, Compatible with Black Ops 3
void BO3TranslateDynamicLight(TextWriter& Writer, const FxDynamicLight& Light);
// Produces a element name, Compatible with Black Ops 1-3
void BOProduceElementName(TextWriter& Writer, uint32_t ElementType, uint32_t ElementIndex);

void CoDEffectTranslator::TranslateEffect(const CoDEffect_t* EffectAsset, const std::string& EffectPath)
{
	// Build export path
	std::string ExportFolder = EffectPath;

	// Check to preserve the paths
	if (SettingsManager::GetSetting("keeprawpath", "true") == "true")
	{
		// Apply the base path
		ExportFolder = FileSystems::CombinePath(ExportFolder, EffectAsset->FXFilePath);
	}

	// Make the directory
	FileSystems::CreateDirectory(ExportFolder);

	// Ship it off to the game handler
	switch (CoDAssets::GameID)
	{
	case SupportedGames::BlackOps: BOTranslateEffect(EffectAsset->AssetPointer, EffectAsset->AssetName, ExportFolder); break;
	case SupportedGames::BlackOps2: BO2TranslateEffect(EffectAsset->AssetPointer, EffectAsset->AssetName, ExportFolder); break;
	case SupportedGames::BlackOps3: BO3TranslateEffect(EffectAsset->AssetPointer, EffectAsset->AssetName, ExportFolder); break;
	}
}

void CoDEffectTranslator::BOTranslateEffect(uint64_t EffectPtr, const std::string& EffectName, const std::string& EffectPath)
{
	// Re-read the effect structure
	auto EffectHeader = CoDAssets::GameInstance->Read<BOFxEffectDef>(EffectPtr);

	// Prepare the output file
	auto Writer = TextWriter();
	// Create new one...
	Writer.Create(FileSystems::CombinePath(EffectPath, EffectName + ".efx"));

	// Header
	Writer.WriteLine("iwfx 2\n");

	// Loop through looping
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountLooping; i++)
	{
		// Translate
		BOTranslateElement(Writer, EffectHeader.FxElementsPtr, i, 0);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BOFxElemDef);
	}

	// Loop through oneshot
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountOneShot; i++)
	{
		// Translate
		BOTranslateElement(Writer, EffectHeader.FxElementsPtr, i, 1);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BOFxElemDef);
	}

	// Loop through emission
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountEmission; i++)
	{
		// Translate
		BOTranslateElement(Writer, EffectHeader.FxElementsPtr, i, 2);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BOFxElemDef);
	}
}

void CoDEffectTranslator::BO2TranslateEffect(uint64_t EffectPtr, const std::string& EffectName, const std::string& EffectPath)
{
	// Re-read the effect structure
	auto EffectHeader = CoDAssets::GameInstance->Read<BO2FxEffectDef>(EffectPtr);

	// Prepare the output file
	auto Writer = TextWriter();
	// Create new one...
	Writer.Create(FileSystems::CombinePath(EffectPath, EffectName + ".efx"));

	// Header
	Writer.WriteLine("iwfx 2\n");

	// Loop through looping
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountLooping; i++)
	{
		// Translate
		BOTranslateElement(Writer, EffectHeader.FxElementsPtr, i, 0);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BOFxElemDef);
	}

	// Loop through oneshot
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountOneShot; i++)
	{
		// Translate
		BOTranslateElement(Writer, EffectHeader.FxElementsPtr, i, 1);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BOFxElemDef);
	}

	// Loop through emission
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountEmission; i++)
	{
		// Translate
		BOTranslateElement(Writer, EffectHeader.FxElementsPtr, i, 2);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BOFxElemDef);
	}
}

void CoDEffectTranslator::BO3TranslateEffect(uint64_t EffectPtr, const std::string& EffectName, const std::string& EffectPath)
{
	// Re-read the effect structure
	auto EffectHeader = CoDAssets::GameInstance->Read<BO3FxEffectDef>(EffectPtr);

	// Prepare the output file
	auto Writer = TextWriter();
	// Create new one...
	Writer.Create(FileSystems::CombinePath(EffectPath, EffectName + ".efx"));

	// Header
	Writer.WriteLine("iwfx 2\n");

	// Loop through looping
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountLooping; i++)
	{
		// Translate
		BO3TranslateElement(Writer, EffectHeader.FxElementsPtr, i, 0);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BO3FxElemDef);
	}

	// Loop through oneshot
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountOneShot; i++)
	{
		// Translate
		BO3TranslateElement(Writer, EffectHeader.FxElementsPtr, i, 1);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BO3FxElemDef);
	}

	// Loop through emission
	for (uint32_t i = 0; i < EffectHeader.ElemDefCountEmission; i++)
	{
		// Translate
		BO3TranslateElement(Writer, EffectHeader.FxElementsPtr, i, 2);
		// Advance
		EffectHeader.FxElementsPtr += sizeof(BO3FxElemDef);
	}
}

void CoDEffectTranslator::BO3TranslateElement(TextWriter& Writer, uint64_t ElementPtr, uint32_t ElementIndex, uint32_t ElementType)
{
	// Read the element
	auto FXElement = CoDAssets::GameInstance->Read<BO3FxElemDef>(ElementPtr);

	// Begin the element
	BOProduceElementName(Writer, ElementType, ElementIndex);

	// Prepare editor flags
	std::string EditorFlags = "";

	// Check various flags
	if (FXElement.EffectOnDeath > 0)
		EditorFlags += " playOnDeath";
	if (FXElement.EffectOnImpact > 0)
		EditorFlags += " playOnTouch";
	if (FXElement.EffectEmitted > 0)
		EditorFlags += " playOnRun";
	if (FXElement.EffectAttached > 0)
		EditorFlags += " playAttached";

	// Check looping
	if (ElementType == 0)
		EditorFlags = " looping" + EditorFlags;

	// Output flags
	Writer.WriteLineFmt("\teditorFlags%s;", EditorFlags.c_str());

	// Prepare element flags
	std::string ElementFlags = "";
	// Check various flags
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_RELATIVE_TO_EFFECT) { ElementFlags += " spawnRelative"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_FRUSTUM_CULL) { ElementFlags += " spawnFrustumCull"; }

	// Element spawn effects
	switch (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_OFFSET_MASK)
	{
	case (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_OFFSET_NONE: ElementFlags += " spawnOffsetNone"; break;
	case (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_OFFSET_SPHERE: ElementFlags += " spawnOffsetSphere"; break;
	case (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_OFFSET_CYLINDER: ElementFlags += " spawnOffsetCylinder"; break;
	}

	// Element run effects
	switch (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_RUN_MASK)
	{
	case (uint32_t)BO3FXElementFlags::FX_ELEM_RUN_RELATIVE_TO_WORLD: ElementFlags += " runRelToWorld"; break;
	case (uint32_t)BO3FXElementFlags::FX_ELEM_RUN_RELATIVE_TO_SPAWN: ElementFlags += " runRelToSpawn"; break;
	case (uint32_t)BO3FXElementFlags::FX_ELEM_RUN_RELATIVE_TO_EFFECT: ElementFlags += " runRelToEffect"; break;
	case (uint32_t)BO3FXElementFlags::FX_ELEM_RUN_RELATIVE_TO_OFFSET: ElementFlags += " runRelToOffset"; break;
	case (uint32_t)BO3FXElementFlags::FX_ELEM_RUN_RELATIVE_TO_EFFECT_NOW: ElementFlags += " runRelToOffsetEffectNow"; break;
	}

	// Other element modifiers
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_DRAW_WITH_VIEWMODEL) { ElementFlags += " drawWithViewModel"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_DRAW_PAST_FOG) { ElementFlags += " drawPastFog"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_DIE_ON_TOUCH) { ElementFlags += " dieOnTouch"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_ALIGN_VIEWPOINT) { ElementFlags += " alignViewpoint"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_NONUNIFORM_SCALE) { ElementFlags += " nonUniformScale"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_COLLISION) { ElementFlags += " useCollision"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_WORLD_UP) { ElementFlags += " useWorldUp"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_ROTATIONAXIS) { ElementFlags += " useRotationAxis"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_BILLBOARD_PIVOT) { ElementFlags += " useBillboardPivot"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_GAUSSIAN_CLOUD) { ElementFlags += " useGaussianCloud"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_ITEM_CLIP) { ElementFlags += " useItemClip"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_BLOCK_SIGHT) { ElementFlags += " blocksSight"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_MODEL_PHYSICS) { ElementFlags += " modelUsesPhysics"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_RUNNER_USES_RAND_ROT) { ElementFlags += " runnerUsesRandRot"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_OCCLUSION_QUERY) { ElementFlags += " useOcclusionQuery"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_INHERIT_PARENT_MOVEMENT) { ElementFlags += " inheritParentMovement"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_USE_THERMAL) { ElementFlags += " useThermal"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_ATTRACTOR_GRAPH_ENABLE) { ElementFlags += " attractorGraphEnable"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_NO_COMPUTE_SPRITES) { ElementFlags += " noComputeSprites"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_SPAWN_DYN_ENT) { ElementFlags += " spawnDynEnt"; }
	if (FXElement.Flags & (uint32_t)BO3FXElementFlags::FX_ELEM_IS_MATURE_CONTENT) { ElementFlags += " isMatureContent"; }

	// Output flags
	Writer.WriteLineFmt("\tflags%s;", ElementFlags.c_str());

	// Prepare extra element flags
	ElementFlags = "";

	// Other extra modifiers
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_DISTRIBUTE_X) { ElementFlags += " distribX"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_DISTRIBUTE_Y) { ElementFlags += " distribY"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_DISTRIBUTE_Z) { ElementFlags += " distribZ"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_TEAM_FRIENDLY) { ElementFlags += " teamFriendly"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_TEAM_FOE) { ElementFlags += " teamFoe"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_CAST_SHADOW) { ElementFlags += " castShadow"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_UNDERWATER_ONLY) { ElementFlags += " underwaterOnly"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_OVERWATER_ONLY) { ElementFlags += " overwaterOnly"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_WATER_RELAXED_CULL) { ElementFlags += " waterRelaxedCull"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_GAMEPLAY_INTENSIVE) { ElementFlags += " gameplayIntensity"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_LF_OCCLUDE_GEOMETRY) { ElementFlags += " lfOccludeGeometry"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_LF_OCCLUDE_PARTICLES) { ElementFlags += " lfOccludeParticles"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_USE_SIZE_BASED_DENSITY_EMISSION) { ElementFlags += " useSizeBasedDensityEmission"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_USE_SIZE_BASED_DENSITY_ATTACHMENT) { ElementFlags += " useSizeBasedDensityAttachment"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_RESPECT_EXCLUSION_VOLUMES) { ElementFlags += " respectExclusionVolumes"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_NO_GPU_FX) { ElementFlags += " noGpuFx"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_INDOOR) { ElementFlags += " indoor"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_USE_GLOBAL_TINT) { ElementFlags += " useGlobalTint"; }
	if (FXElement.ExtraFlags & (uint32_t)FXElementExtraFlags::FX_ELEM_USE_TRAIL_TRAPEZOID_FIX) { ElementFlags += " useTrailTrapezoidFix"; }

	// Output extra flags
	Writer.WriteLineFmt("\textraFlags%s;", ElementFlags.c_str());

	//
	// -- Convert ranges before export
	//

	FxFloatRangeInverse(FXElement.SpawnRange);
	FxFloatRangeInverse(FXElement.FadeInRange);
	FxFloatRangeInverse(FXElement.FadeOutRange);

	FxIntRangeInverse(FXElement.SpawnDelayMsec);
	FxIntRangeInverse(FXElement.LifeSpanMsec);

	FxFloatRangeInverse(FXElement.SpawnOrigin[0]);
	FxFloatRangeInverse(FXElement.SpawnOrigin[1]);
	FxFloatRangeInverse(FXElement.SpawnOrigin[2]);
	FxFloatRangeInverse(FXElement.SpawnOffsetRadius);
	FxFloatRangeInverse(FXElement.SpawnOffsetHeight);

	// Spawn ranges
	Writer.WriteLineFmt("\tspawnRange %f %f;", FXElement.SpawnRange.Base, FXElement.SpawnRange.Amplitude);
	Writer.WriteLineFmt("\tfadeInRange %f %f;", FXElement.FadeInRange.Base, FXElement.FadeInRange.Amplitude);
	Writer.WriteLineFmt("\tfadeOutRange %f %f;", FXElement.FadeOutRange.Base, FXElement.FadeOutRange.Amplitude);
	Writer.WriteLineFmt("\tspawnFrustumCullRadius %f;", FXElement.SpawnFrustumCullRadius);

	// Type specific
	Writer.WriteLineFmt("\tspawnLooping %d %d;", FXElement.Spawn.Base, ((FXElement.Spawn.Amplitude != INT_MAX) ? FXElement.Spawn.Amplitude : 0));
	Writer.WriteLineFmt("\tspawnLoopingSpawnCount %d %d;", FXElement.SpawnLoopingSpawnCount.Base, ((FXElement.SpawnLoopingSpawnCount.Amplitude != INT_MAX) ? FXElement.SpawnLoopingSpawnCount.Amplitude : 0));
	Writer.WriteLineFmt("\tspawnOneShot %d %d;", FXElement.Spawn.Base, ((FXElement.Spawn.Amplitude != INT_MAX) ? FXElement.Spawn.Amplitude : 0));

	// Times
	Writer.WriteLineFmt("\tspawnDelayMsec %d %d;", FXElement.SpawnDelayMsec.Base, FXElement.SpawnDelayMsec.Amplitude);
	Writer.WriteLineFmt("\tlifeSpanMsec %d %d;", FXElement.LifeSpanMsec.Base, FXElement.LifeSpanMsec.Amplitude);

	// Spawn data
	Writer.WriteLineFmt("\tspawnOrgX %f %f;", FXElement.SpawnOrigin[0].Base, FXElement.SpawnOrigin[0].Amplitude);
	Writer.WriteLineFmt("\tspawnOrgY %f %f;", FXElement.SpawnOrigin[1].Base, FXElement.SpawnOrigin[1].Amplitude);
	Writer.WriteLineFmt("\tspawnOrgZ %f %f;", FXElement.SpawnOrigin[2].Base, FXElement.SpawnOrigin[2].Amplitude);

	// Spawn offset
	Writer.WriteLineFmt("\tspawnOffsetRadius %f %f;", FXElement.SpawnOffsetRadius.Base, FXElement.SpawnOffsetRadius.Amplitude);
	Writer.WriteLineFmt("\tspawnOffsetHeight %f %f;", FXElement.SpawnOffsetHeight.Base, FXElement.SpawnOffsetHeight.Amplitude);
	Writer.WriteLineFmt("\tspawnOffsetCylindricalAxis %f;", FXElement.SpawnOffsetCylindricalAxis);

	// Spawn angles
	Writer.WriteLineFmt("\tspawnAnglePitch %f %f;", VectorMath::RadiansToDegrees(FXElement.SpawnAngles[0].Base), VectorMath::RadiansToDegrees(FXElement.SpawnAngles[0].Amplitude));
	Writer.WriteLineFmt("\tspawnAngleYaw %f %f;", VectorMath::RadiansToDegrees(FXElement.SpawnAngles[1].Base), VectorMath::RadiansToDegrees(FXElement.SpawnAngles[1].Amplitude));
	Writer.WriteLineFmt("\tspawnAngleRoll %f %f;", VectorMath::RadiansToDegrees(FXElement.SpawnAngles[2].Base), VectorMath::RadiansToDegrees(FXElement.SpawnAngles[2].Amplitude));

	// Angle velocity
	Writer.WriteLineFmt("\tangleVelPitch %f %f;", AngularVelToDegrees(FXElement.AngularVelocity[0].Base), AngularVelToDegrees(FXElement.AngularVelocity[0].Amplitude));
	Writer.WriteLineFmt("\tangleVelYaw %f %f;", AngularVelToDegrees(FXElement.AngularVelocity[1].Base), AngularVelToDegrees(FXElement.AngularVelocity[1].Amplitude));
	Writer.WriteLineFmt("\tangleVelRoll %f %f;", AngularVelToDegrees(FXElement.AngularVelocity[2].Base), AngularVelToDegrees(FXElement.AngularVelocity[2].Amplitude));

	// Initial rotation
	Writer.WriteLineFmt("\tinitialRot %f %f;", VectorMath::RadiansToDegrees(FXElement.InitialRotation.Base), VectorMath::RadiansToDegrees(FXElement.InitialRotation.Amplitude));

	// Output rotation axis
	auto RotAxis = UnpackVec4Quat(FXElement.RotationAxis);
	Writer.WriteLineFmt("\trotationAxis %f %f %f %f;", RotAxis.X, RotAxis.Y, RotAxis.Z, RotAxis.W);

	// Gravity and elasticity
	Writer.WriteLineFmt("\tgravity %f %f;", FXElement.Gravity.Base * 100, FXElement.Gravity.Amplitude * 100);
	Writer.WriteLineFmt("\telasticity %f %f;", FXElement.ReflectionFactor.Base, FXElement.ReflectionFactor.Amplitude);
	Writer.WriteLineFmt("\twindinfluence %f;", FXElement.WindInfluence);

	// Prepare atlas flags
	std::string AtlasFlags = "";
	// Atlas start flags
	switch (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_START_MASK)
	{
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_FIXED: AtlasFlags += " startFixed"; break;
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_RANDOM: AtlasFlags += " startRandom"; break;
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_INDEXED: AtlasFlags += " startIndexed"; break;
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_FIXED_RANGE: AtlasFlags += " startFixedRange"; break;
	}

	// Other atlas flags
	if (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_PLAY_OVER_LIFE) { AtlasFlags += " playOverLife"; }
	if (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_LOOP_ONLY_N_TIMES) { AtlasFlags += " loopOnlyNTimes"; }
	if (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_LERP_FRAMES) { AtlasFlags += " lerpFrames"; }
	if (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_REVERSE) { AtlasFlags += " reverse"; }

	// Output flags
	Writer.WriteLineFmt("\tatlasBehavior%s;", AtlasFlags.c_str());

	// Atlas info
	Writer.WriteLineFmt("\tatlasIndex %d;", FXElement.Atlas.Index);
	Writer.WriteLineFmt("\tatlasFps %d;", FXElement.Atlas.Fps);
	Writer.WriteLineFmt("\tatlasLoopCount %d;", FXElement.Atlas.LoopCount);
	Writer.WriteLineFmt("\tatlasColIndexBits %d;", FXElement.Atlas.ColIndexBits);
	Writer.WriteLineFmt("\tatlasRowIndexBits %d;", FXElement.Atlas.RowIndexBits);

	// Packed atlas info
	auto IndexRangeVal = (FXElement.Atlas.EntryCountAndIndexRange >> 9) & 0x7f;
	// Format the output
	Writer.WriteLineFmt("\tatlasEntryCount %d;", ((1 << (FXElement.Atlas.ColIndexBits + FXElement.Atlas.RowIndexBits))));
	Writer.WriteLineFmt("\tatlasIndexRange %d;", IndexRangeVal);

	// A list of the samples
	std::vector<FxElemVelStateSample> VelSamples;

	// Prepare to loop and read vel samples
	for (uint32_t i = 0; i < (uint32_t)(FXElement.VelIntervalCount + 1); i++)
	{
		// Add it
		VelSamples.emplace_back(CoDAssets::GameInstance->Read<FxElemVelStateSample>(FXElement.VelSamplesPtr));

		// Advance
		FXElement.VelSamplesPtr += sizeof(FxElemVelStateSample);
	}

	// Output curves
	BOTranslateVelSample(Writer, "velGraph0X", 0, VelSamples);
	BOTranslateVelSample(Writer, "velGraph0Y", 1, VelSamples);
	BOTranslateVelSample(Writer, "velGraph0Z", 2, VelSamples);
	// Output world curves?
	BOTranslateVelSample(Writer, "velGraph1X", 0, { FxElemVelStateSample(), FxElemVelStateSample() });
	BOTranslateVelSample(Writer, "velGraph1Y", 1, { FxElemVelStateSample(), FxElemVelStateSample() });
	BOTranslateVelSample(Writer, "velGraph1Z", 2, { FxElemVelStateSample(), FxElemVelStateSample() });

	// A list of the samples
	std::vector<BO3FxElemVisStateSample> VisSamples;

	// Prepare to loop and read vis samples
	for (uint32_t i = 0; i < (uint32_t)(FXElement.VisStateIntervalCount + 1); i++)
	{
		// Add it
		VisSamples.emplace_back(CoDAssets::GameInstance->Read<BO3FxElemVisStateSample>(FXElement.VisSamples));

		// Advance
		FXElement.VisSamples += sizeof(BO3FxElemVisStateSample);
	}

	// Inject a visual sample if we only have one, required for some fx files
	if (VisSamples.size() == 1)
	{
		auto DefaultSample = BO3FxElemVisStateSample();
		// Default everything to 0
		std::memset(&DefaultSample, 0, sizeof(DefaultSample));

		// Reset properties to 1.0 if they have it
		DefaultSample.Base.RotationTotal = 1.0f;
		DefaultSample.Base.Size[0] = 0.5f;
		DefaultSample.Base.Size[1] = 0.5f;
		DefaultSample.Base.Scale = 1.0f;
		DefaultSample.Base.ChildScale = 1.0f;
		DefaultSample.Base.LightFOV = 1.0f;
		DefaultSample.Base.LightIntensity = 1.0f;
		DefaultSample.Base.LightRadius = 1.0f;
		DefaultSample.Base.Color[0] = 255;
		DefaultSample.Base.Color[1] = 255;
		DefaultSample.Base.Color[2] = 255;
		DefaultSample.Base.Color[3] = 255;
		DefaultSample.Amplitude.Color[0] = 255;
		DefaultSample.Amplitude.Color[1] = 255;
		DefaultSample.Amplitude.Color[2] = 255;
		DefaultSample.Amplitude.Color[3] = 255;

		// Add the sample
		VisSamples.insert(VisSamples.begin(), DefaultSample);
	}

	// Output it
	BO3TranslateVisSample(Writer, VisSamples);

	// A list of parent movement + attractor samples
	std::vector<FxFloatRange> ParentMovementSamples;
	std::vector<FxFloatRange> AttractorSamples;

	// Prepare to loop and read parent movement samples
	for (uint32_t i = 0; i < (uint32_t)(FXElement.ParentMovementStateIntervalCount + 1); i++)
	{
		// Add it
		ParentMovementSamples.emplace_back(CoDAssets::GameInstance->Read<FxFloatRange>(FXElement.ParentMovementSamples));

		// Advance
		FXElement.ParentMovementSamples += sizeof(FxFloatRange);
	}
	// Prepare to loop and read attractor samples
	for (uint32_t i = 0; i < (uint32_t)(FXElement.AttractorStateIntervalCount + 1); i++)
	{
		// Add it
		AttractorSamples.emplace_back(CoDAssets::GameInstance->Read<FxFloatRange>(FXElement.AttractorSamples));

		// Advance
		FXElement.AttractorSamples += sizeof(FxFloatRange);
	}

	// Output generic graphs
	BOTranslateGenericSample(Writer, "inheritParentMovementGraph", ParentMovementSamples);
	BOTranslateGenericSample(Writer, "attractorGraph", AttractorSamples);

	// Lighting and attractors
	Writer.WriteLineFmt("\tattractorLocalPosition %f %f %f;", FXElement.AttractorLocalPosition[0], FXElement.AttractorLocalPosition[1], FXElement.AttractorLocalPosition[2]);
	Writer.WriteLineFmt("\tlightingFrac %f;", ((float)FXElement.LightingFrac / 255.0f));

	// Calculate radius first
	auto ColRadix = ((std::abs(FXElement.CollMins[0]) + std::abs(FXElement.CollMaxs[1])) / 2.0f);

	// Generate offsets and output
	Writer.WriteLineFmt("\tcollOffset %f %f %f;", (FXElement.CollMaxs[0] - ColRadix), (FXElement.CollMaxs[1] - ColRadix), (FXElement.CollMaxs[2] - ColRadix));
	Writer.WriteLineFmt("\tcollRadius %f;", ColRadix);

	// Effect Impact / Death
	std::string OnImpact = "", OnDeath = "", OnEmission = "", OnAttachment = "";
	// Load the names
	if (FXElement.EffectOnImpact > 0) { OnImpact = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.EffectOnImpact)); }
	if (FXElement.EffectOnDeath > 0) { OnDeath = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.EffectOnDeath)); }
	if (FXElement.EffectEmitted > 0) { OnEmission = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.EffectEmitted)); }
	if (FXElement.EffectAttached > 0) { OnAttachment = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.EffectAttached)); }

	// Output
	Writer.WriteLineFmt("\tfxOnImpact \"%s\";\n\tfxOnDeath \"%s\";", OnImpact.c_str(), OnDeath.c_str());

	// Sort order and emission
	Writer.WriteLineFmt("\tdisplacement %d;", FXElement.Displacement);
	Writer.WriteLineFmt("\temission \"%s\";", OnEmission.c_str());
	Writer.WriteLineFmt("\temitDist %f %f;", FXElement.EmitDist.Base, FXElement.EmitDist.Amplitude);
	Writer.WriteLineFmt("\temitDistVariance %f %f;", FXElement.EmitDistVariance.Base, FXElement.EmitDistVariance.Amplitude);
	Writer.WriteLineFmt("\temitDensity %f %f;", FXElement.EmitDensity.Base, FXElement.EmitDensity.Amplitude);
	Writer.WriteLineFmt("\temitSizeForDensity %f;", FXElement.EmitSizeForDensity);

	// Attachments
	Writer.WriteLineFmt("\tattachment \"%s\";", OnAttachment.c_str());
	Writer.WriteLineFmt("\tattachmentDensity %f %f;", FXElement.AttachmentDensity.Base, FXElement.AttachmentDensity.Amplitude);
	Writer.WriteLineFmt("\tattachmentSizeForDensity %f;", FXElement.AttachmentSizeForDensity);

	// Trails
	BO3FxTrailDef TrailDef = { 0 };
	// Read if valid
	if (FXElement.ElemType == (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TRAIL) { TrailDef = CoDAssets::GameInstance->Read<BO3FxTrailDef>(FXElement.LightingInfo.FxTrailDefsPtr); }

	// Basic trails
	Writer.WriteLineFmt("\ttrailSplitDist %d;", TrailDef.SplitDist);
	Writer.WriteLineFmt("\ttrailScrollTime %f;", ((float)TrailDef.ScrollTimeMsec / 1000.0f));
	Writer.WriteLineFmt("\ttrailRepeatDist %d;", TrailDef.RepeatDist);
	Writer.WriteLineFmt("\ttrailFadeInDist %f;", TrailDef.FadeInDist);
	Writer.WriteLineFmt("\ttrailFadeOutDist %f;", TrailDef.FadeOutDist);

	// Wind and alpha info
	Writer.WriteLineFmt("\talphafadetimemsec %d;", FXElement.AlphaFadeTimeMsec);
	Writer.WriteLineFmt("\tmaxwind_mag %d;", FXElement.MaxWindStrength);
	Writer.WriteLineFmt("\tmaxwind_life %d;", FXElement.LifespanAtMaxWind);
	Writer.WriteLineFmt("\tmaxwind_interval %d;", FXElement.SpawnIntervalAtMaxWind);

	// Billboards
	if (FXElement.ElemType != (uint8_t)BO3FXElementType::FX_ELEM_TYPE_CLOUD)
	{
		Writer.WriteLineFmt("\tbillboardTopWidth %f;", FXElement.BillboardInfo.BillboardTrim.Base);
		Writer.WriteLineFmt("\tbillboardBottomWidth %f;", FXElement.BillboardInfo.BillboardTrim.Amplitude);
	}
	else
	{
		Writer.WriteLine("\tbillboardTopWidth 1;");
		Writer.WriteLine("\tbillboardBottomWidth 1;");
	}

	// Sound spawn
	Writer.WriteLine("\telemSpawnSound\n\t{");
	// Output if we have one
	if (FXElement.SpawnSound > 0)
	{
		// Read name
		auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(FXElement.SpawnSound);
		// Output
		Writer.WriteLineFmt("\t\t\"%s\"", SoundName.c_str());
	}
	// End sounds
	Writer.WriteLine("\t};");
	// Sound follow
	Writer.WriteLine("\telemFollowSound\n\t{");
	// Output if we have one
	if (FXElement.FollowSound > 0)
	{
		// Read name
		auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(FXElement.FollowSound);
		// Output
		Writer.WriteLineFmt("\t\t\"%s\"", SoundName.c_str());
	}
	// End sounds
	Writer.WriteLine("\t};");

	// Clouds
	if (FXElement.ElemType == (uint8_t)BO3FXElementType::FX_ELEM_TYPE_CLOUD)
	{
		Writer.WriteLineFmt("\tcloudDensity %d %d;", FXElement.BillboardInfo.BillboardCloudRange.Base, FXElement.BillboardInfo.BillboardCloudRange.Amplitude);
	}
	else
	{
		Writer.WriteLine("\tcloudDensity 1024 0;");
	}

	// Trail def
	if (FXElement.ElemType == (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TRAIL)
	{
		// Begin
		Writer.WriteLine("\ttrailDef\n\t{");

		// Loop
		for (uint32_t v = 0; v < TrailDef.VertCount; v++)
		{
			// Read it
			auto Vertex = CoDAssets::GameInstance->Read<FxTrailVertex>(TrailDef.VertPtr);

			// Format
			Writer.WriteLineFmt("\t\t %f %f %f", Vertex.Position[0], Vertex.Position[1], Vertex.TexCoord);

			// Advance
			TrailDef.VertPtr += sizeof(FxTrailVertex);
		}

		// End and begin ind
		Writer.WriteLine("\t} {");

		// Loop
		for (uint32_t v = 0; v < TrailDef.IndCount; v++)
		{
			// Read it
			auto Index = CoDAssets::GameInstance->Read<uint16_t>(TrailDef.IndexPtr);

			// Format
			Writer.WriteLineFmt("\t\t %d", Index);

			// Advance
			TrailDef.IndexPtr += sizeof(uint16_t);
		}

		// End
		Writer.WriteLine("\t};");
	}

	// Spotlight data, only valid if we are a spotlight
	if (FXElement.ElemType == (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPOT_LIGHT)
	{
		// Read the info
		auto SpotLightDef = CoDAssets::GameInstance->Read<FxSpotLight>(FXElement.LightingInfo.SpotLightPtr);

		Writer.WriteLineFmt("\tspotLightFovInnerFraction %f;", SpotLightDef.SpotlightFOV);
		Writer.WriteLineFmt("\tspotLightStartRadius %f;", SpotLightDef.SpotlightStartRadius);
		Writer.WriteLineFmt("\tspotLightEndRadius %f;", SpotLightDef.SpotlightEndRadius);
	}
	else
	{
		Writer.WriteLine("\tspotLightFovInnerFraction 0;");
		Writer.WriteLine("\tspotLightStartRadius 0;");
		Writer.WriteLine("\tspotLightEndRadius 0;");
	}

	// Alpha and feathering
	Writer.WriteLineFmt("\talphaDissolve %f;", FXElement.AlphaDissolve);
	Writer.WriteLineFmt("\tzFeather %f;", FXElement.ZFeather);
	Writer.WriteLineFmt("\tfalloffBeginAngle %d;", FXElement.FallOffBeginAngle);
	Writer.WriteLineFmt("\tfalloffEndAngle %d;", FXElement.FallOffEndAngle);
	
	// LF data, only valid if we are a lens flare type
	if (FXElement.ElemType == (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LENS_FLARE)
	{
		Writer.WriteLineFmt("\tlfSourceDir %f %f %f;", FXElement.VisualsInfo.LensFlare.LFSourceDir[0], FXElement.VisualsInfo.LensFlare.LFSourceDir[1], FXElement.VisualsInfo.LensFlare.LFSourceDir[2]);
		Writer.WriteLineFmt("\tlfSourceSize %f;", FXElement.VisualsInfo.LensFlare.LFSourceSize);
	}
	else
	{
		Writer.WriteLine("\tlfSourceDir 0 0 0;");
		Writer.WriteLine("\tlfSourceSize 0;");
	}

	// Pivot and LOD
	Writer.WriteLineFmt("\tbillboardPivot %f %f;", (FXElement.BillboardPivot[0] / 2.0f), (FXElement.BillboardPivot[1] / -2.0f));
	Writer.WriteLineFmt("\tlevelOfDetail 0;");

	// Prepare visuals
	switch (FXElement.ElemType)
	{
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD: Writer.WriteLine("\tbillboardSprite\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED: Writer.WriteLine("\torientedSprite\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ROTATED: Writer.WriteLine("\trotatedSprite\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TAIL: Writer.WriteLine("\ttail\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LINE: Writer.WriteLine("\tline\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TRAIL: Writer.WriteLine("\ttrail\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_CLOUD: Writer.WriteLine("\tcloud\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_MODEL: Writer.WriteLine("\tmodel\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_LIGHT:
		// If we have data, it's a dynamic definition
		if (FXElement.VisualsInfo.VisualsPtr != 0x0)
			Writer.WriteLine("\tdynamicLight2\n\t{");
		else
			Writer.WriteLine("\tdynamicLight2;");
		break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_OMNI_LIGHT: Writer.WriteLine("\tlight;"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPOT_LIGHT: Writer.WriteLine("\tspotLight;"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_SOUND: Writer.WriteLine("\tdynamicSound\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DECAL: Writer.WriteLine("\tdecal\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_RUNNER: Writer.WriteLine("\trunner\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LENS_FLARE: Writer.WriteLine("\tlensFlare\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_BEAM_SOURCE: Writer.WriteLine("\tbeamSource\n\t{"); break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_BEAM_TARGET: Writer.WriteLine("\tbeamTarget;"); break;

#if _DEBUG
	default:
		// Only log on debug
		printf("Unknown ElemType: 0x%X\n", FXElement.ElemType);
		break;
#endif
	}

	// Check count
	if (FXElement.VisualCount == 1)
	{
		switch (FXElement.ElemType)
		{
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ROTATED:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TRAIL:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LINE:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_CLOUD:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TAIL:
			// Pointer to pointer (Requires removed path)
			Writer.WriteLineFmt("\t\t\"%s\"", FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.VisualsInfo.VisualsPtr))).c_str());
			break;
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_MODEL:
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_RUNNER:
			// Pointer to pointer
			Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.VisualsInfo.VisualsPtr)).c_str());
			break;
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DECAL:
			// Pointer to material
			Writer.WriteLineFmt("\t\t\"%s\"", FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(CoDAssets::GameInstance->Read<uint64_t>(FXElement.VisualsInfo.VisualsPtr)))).c_str());
			break;
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_SOUND:
			// Raw name
			Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(FXElement.VisualsInfo.VisualsPtr).c_str());
			break;
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_BEAM_SOURCE:
			// Saved over at our visuals ptr
			Writer.WriteLineFmt("\t\t\"%s\"", FXElement.VisualsInfo.BeamSource);
			break;
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LENS_FLARE:
			// Lens flare hash
			Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.LightingInfo.LensFlarePtr)).c_str());
			break;
		case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_LIGHT:
			// Pointer to dynamic lighting (Only valid when we have data to read)
			if (FXElement.VisualsInfo.VisualsPtr != 0x0)
				BO3TranslateDynamicLight(Writer, CoDAssets::GameInstance->Read<FxDynamicLight>(FXElement.VisualsInfo.VisualsPtr));
			break;
		}
	}
	else
	{
		// We have an array
		for (uint32_t v = 0; v < FXElement.VisualCount; v++)
		{
			// Read
			auto VisualData = CoDAssets::GameInstance->Read<BO3FxVisuals>(FXElement.VisualsInfo.VisualsPtr);

			switch (FXElement.ElemType)
			{
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ROTATED:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DECAL:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TRAIL:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LINE:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_CLOUD:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TAIL:
				// Pointer to pointer (Requires removed path)
			{
				auto PathBuffer = FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(VisualData.VisualPtr)));
				// Check for and remove '|dup' at the end of the path
				if (Strings::EndsWith(PathBuffer, "|dup"))
					PathBuffer = PathBuffer.substr(0, PathBuffer.size() - 4);

				// Output the cleaned path
				Writer.WriteLineFmt("\t\t\"%s\"", PathBuffer.c_str());
			}
				break;
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_MODEL:
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_RUNNER:
				// Pointer to pointer
				Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(VisualData.VisualPtr)).c_str());
				break;
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_SOUND:
				// Pointer to sound alias name (With extra data)
				Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(VisualData.VisualPtr).c_str());
				break;
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_BEAM_SOURCE:
				// Raw name
				Writer.WriteLineFmt("\t\t\"%s\"", VisualData.VisualString);
				break;
			case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LENS_FLARE:
				// Lens flare hash
				Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint64_t>(FXElement.LightingInfo.LensFlarePtr)).c_str());
				break;
			}

			// Advance
			FXElement.VisualsInfo.VisualsPtr += sizeof(BO3FxVisuals);
		}
	}

	// Close it up, lights and targets are not included, dynamicLight2 isn't when we have no data...
	switch (FXElement.ElemType)
	{
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_SPRITE_ROTATED:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TAIL:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LINE:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_TRAIL:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_CLOUD:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_MODEL:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_SOUND:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DECAL:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_RUNNER:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_LENS_FLARE:
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_BEAM_SOURCE:
		Writer.WriteLine("\t};");
		break;
	case (uint8_t)BO3FXElementType::FX_ELEM_TYPE_DYNAMIC_LIGHT:
		// Only if we had a definition
		if (FXElement.VisualsInfo.VisualsPtr != 0x0)
			Writer.WriteLine("\t};");
		break;
	}

	// End the effect
	Writer.WriteLine("}\n");
}

void CoDEffectTranslator::BOTranslateElement(TextWriter& Writer, uint64_t ElementPtr, uint32_t ElementIndex, uint32_t ElementType)
{
	// Read the element
	auto FXElement = CoDAssets::GameInstance->Read<BOFxElemDef>(ElementPtr);

	// Begin the element
	BOProduceElementName(Writer, ElementType, ElementIndex);

	// Prepare editor flags
	std::string EditorFlags = "";

	// Check various flags
	if (FXElement.EffectOnDeath > 0)
		EditorFlags += " playOnDeath";
	if (FXElement.EffectOnImpact > 0)
		EditorFlags += " playOnTouch";
	if (FXElement.EffectEmitted > 0)
		EditorFlags += " playOnRun";
	if (FXElement.EffectAttached > 0)
		EditorFlags += " playAttached";

	// Check looping
	if (ElementType == 0)
		EditorFlags = " looping" + EditorFlags;

	// Output flags
	Writer.WriteLineFmt("\teditorFlags%s;", EditorFlags.c_str());

	// Prepare element flags
	std::string ElementFlags = "";
	// Check various flags
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_SPAWN_RELATIVE_TO_EFFECT) { ElementFlags += " spawnRelative"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_SPAWN_FRUSTUM_CULL) { ElementFlags += " spawnFrustumCull"; }

	// Element spawn effects
	switch (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_SPAWN_OFFSET_MASK)
	{
	case (uint32_t)BOFXElementFlags::FX_ELEM_SPAWN_OFFSET_NONE: ElementFlags += " spawnOffsetNone"; break;
	case (uint32_t)BOFXElementFlags::FX_ELEM_SPAWN_OFFSET_SPHERE: ElementFlags += " spawnOffsetSphere"; break;
	case (uint32_t)BOFXElementFlags::FX_ELEM_SPAWN_OFFSET_CYLINDER: ElementFlags += " spawnOffsetCylinder"; break;
	}

	// Element run effects
	switch (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_RUN_MASK)
	{
	case (uint32_t)BOFXElementFlags::FX_ELEM_RUN_RELATIVE_TO_WORLD: ElementFlags += " runRelToWorld"; break;
	case (uint32_t)BOFXElementFlags::FX_ELEM_RUN_RELATIVE_TO_SPAWN: ElementFlags += " runRelToSpawn"; break;
	case (uint32_t)BOFXElementFlags::FX_ELEM_RUN_RELATIVE_TO_EFFECT: ElementFlags += " runRelToEffect"; break;
	case (uint32_t)BOFXElementFlags::FX_ELEM_RUN_RELATIVE_TO_OFFSET: ElementFlags += " runRelToOffset"; break;
	}

	// Other element modifiers
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_DRAW_WITH_VIEWMODEL) { ElementFlags += " drawWithViewModel"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_DRAW_PAST_FOG) { ElementFlags += " drawPastFog"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_DIE_ON_TOUCH) { ElementFlags += " dieOnTouch"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_ALIGN_VIEWPOINT) { ElementFlags += " alignViewpoint"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_NONUNIFORM_SCALE) { ElementFlags += " nonUniformScale"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_COLLISION) { ElementFlags += " useCollision"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_WORLD_UP) { ElementFlags += " useWorldUp"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_ROTATIONAXIS) { ElementFlags += " useRotationAxis"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_BILLBOARD_PIVOT) { ElementFlags += " useBillboardPivot"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_GAUSSIAN_CLOUD) { ElementFlags += " useGaussianCloud"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_ITEM_CLIP) { ElementFlags += " useItemClip"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_HAS_REFLECTION) { ElementFlags += " hasReflection"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_USE_MODEL_PHYSICS) { ElementFlags += " modelUsesPhysics"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_RUNNER_USES_RAND_ROT) { ElementFlags += " runnerUsesRandRot"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_BLOCK_SIGHT) { ElementFlags += " blocksSight"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_FLAME_CHUNK) { ElementFlags += " flamechunk"; }
	if (FXElement.Flags & (uint32_t)BOFXElementFlags::FX_ELEM_IS_MATURE_CONTENT) { ElementFlags += " isMatureContent"; }

	// Output flags
	Writer.WriteLineFmt("\tflags%s;", ElementFlags.c_str());

	//
	// -- Convert ranges before export
	//

	FxFloatRangeInverse(FXElement.SpawnRange);
	FxFloatRangeInverse(FXElement.FadeInRange);
	FxFloatRangeInverse(FXElement.FadeOutRange);

	FxIntRangeInverse(FXElement.SpawnDelayMsec);
	FxIntRangeInverse(FXElement.LifeSpanMsec);

	FxFloatRangeInverse(FXElement.SpawnOrigin[0]);
	FxFloatRangeInverse(FXElement.SpawnOrigin[1]);
	FxFloatRangeInverse(FXElement.SpawnOrigin[2]);
	FxFloatRangeInverse(FXElement.SpawnOffsetRadius);
	FxFloatRangeInverse(FXElement.SpawnOffsetHeight);

	// Spawn ranges
	Writer.WriteLineFmt("\tspawnRange %f %f;", FXElement.SpawnRange.Base, FXElement.SpawnRange.Amplitude);
	Writer.WriteLineFmt("\tfadeInRange %f %f;", FXElement.FadeInRange.Base, FXElement.FadeInRange.Amplitude);
	Writer.WriteLineFmt("\tfadeOutRange %f %f;", FXElement.FadeOutRange.Base, FXElement.FadeOutRange.Amplitude);
	Writer.WriteLineFmt("\tspawnFrustumCullRadius %f;", FXElement.SpawnFrustumCullRadius);

	// Type specific
	Writer.WriteLineFmt("\tspawnLooping %d %d;", FXElement.Spawn.Base, ((FXElement.Spawn.Amplitude != INT_MAX) ? FXElement.Spawn.Amplitude : 0));
	Writer.WriteLineFmt("\tspawnOneShot %d %d;", FXElement.Spawn.Base, ((FXElement.Spawn.Amplitude != INT_MAX) ? FXElement.Spawn.Amplitude : 0));

	// Times
	Writer.WriteLineFmt("\tspawnDelayMsec %d %d;", FXElement.SpawnDelayMsec.Base, FXElement.SpawnDelayMsec.Amplitude);
	Writer.WriteLineFmt("\tlifeSpanMsec %d %d;", FXElement.LifeSpanMsec.Base, FXElement.LifeSpanMsec.Amplitude);

	// Spawn data
	Writer.WriteLineFmt("\tspawnOrgX %f %f;", FXElement.SpawnOrigin[0].Base, FXElement.SpawnOrigin[0].Amplitude);
	Writer.WriteLineFmt("\tspawnOrgY %f %f;", FXElement.SpawnOrigin[1].Base, FXElement.SpawnOrigin[1].Amplitude);
	Writer.WriteLineFmt("\tspawnOrgZ %f %f;", FXElement.SpawnOrigin[2].Base, FXElement.SpawnOrigin[2].Amplitude);

	// Spawn offset
	Writer.WriteLineFmt("\tspawnOffsetRadius %f %f;", FXElement.SpawnOffsetRadius.Base, FXElement.SpawnOffsetRadius.Amplitude);
	Writer.WriteLineFmt("\tspawnOffsetHeight %f %f;", FXElement.SpawnOffsetHeight.Base, FXElement.SpawnOffsetHeight.Amplitude);

	// Spawn angles
	Writer.WriteLineFmt("\tspawnAnglePitch %f %f;", VectorMath::RadiansToDegrees(FXElement.SpawnAngles[0].Base), VectorMath::RadiansToDegrees(FXElement.SpawnAngles[0].Amplitude));
	Writer.WriteLineFmt("\tspawnAngleYaw %f %f;", VectorMath::RadiansToDegrees(FXElement.SpawnAngles[1].Base), VectorMath::RadiansToDegrees(FXElement.SpawnAngles[1].Amplitude));
	Writer.WriteLineFmt("\tspawnAngleRoll %f %f;", VectorMath::RadiansToDegrees(FXElement.SpawnAngles[2].Base), VectorMath::RadiansToDegrees(FXElement.SpawnAngles[2].Amplitude));

	// Angle velocity
	Writer.WriteLineFmt("\tangleVelPitch %f %f;", AngularVelToDegrees(FXElement.AngularVelocity[0].Base), AngularVelToDegrees(FXElement.AngularVelocity[0].Amplitude));
	Writer.WriteLineFmt("\tangleVelYaw %f %f;", AngularVelToDegrees(FXElement.AngularVelocity[1].Base), AngularVelToDegrees(FXElement.AngularVelocity[1].Amplitude));
	Writer.WriteLineFmt("\tangleVelRoll %f %f;", AngularVelToDegrees(FXElement.AngularVelocity[2].Base), AngularVelToDegrees(FXElement.AngularVelocity[2].Amplitude));

	// Initial rotation
	Writer.WriteLineFmt("\tinitialRot %f %f;", VectorMath::RadiansToDegrees(FXElement.InitialRotation.Base), VectorMath::RadiansToDegrees(FXElement.InitialRotation.Amplitude));

	// Output rotation axis
	auto RotAxis = UnpackVec4Quat(FXElement.RotationAxis);
	Writer.WriteLineFmt("\trotationAxis %f %f %f %f;", RotAxis.X, RotAxis.Y, RotAxis.Z, RotAxis.W);

	// Gravity and elasticity
	Writer.WriteLineFmt("\tgravity %f %f;", FXElement.Gravity.Base * 100, FXElement.Gravity.Amplitude * 100);
	Writer.WriteLineFmt("\telasticity %f %f;", FXElement.ReflectionFactor.Base, FXElement.ReflectionFactor.Amplitude);
	Writer.WriteLineFmt("\twindinfluence %f;", FXElement.WindInfluence);

	// Prepare atlas flags
	std::string AtlasFlags = "";
	// Atlas start flags
	switch (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_START_MASK)
	{
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_FIXED: AtlasFlags += " startFixed"; break;
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_RANDOM: AtlasFlags += " startRandom"; break;
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_INDEXED: AtlasFlags += " startIndexed"; break;
	case (uint8_t)FXAtlasFlags::FX_ATLAS_START_FIXED_RANGE: AtlasFlags += " startFixedRange"; break;
	}

	// Other atlas flags
	if (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_PLAY_OVER_LIFE) { AtlasFlags += " playOverLife"; }
	if (FXElement.Atlas.Behavior & (uint8_t)FXAtlasFlags::FX_ATLAS_LOOP_ONLY_N_TIMES) { AtlasFlags += " loopOnlyNTimes"; }

	// Output flags
	Writer.WriteLineFmt("\tatlasBehavior%s;", AtlasFlags.c_str());

	// Atlas info
	Writer.WriteLineFmt("\tatlasIndex %d;", FXElement.Atlas.Index);
	Writer.WriteLineFmt("\tatlasFps %d;", FXElement.Atlas.Fps);
	Writer.WriteLineFmt("\tatlasLoopCount %d;", FXElement.Atlas.LoopCount);
	Writer.WriteLineFmt("\tatlasColIndexBits %d;", FXElement.Atlas.ColIndexBits);
	Writer.WriteLineFmt("\tatlasRowIndexBits %d;", FXElement.Atlas.RowIndexBits);

	// Packed atlas info
	auto IndexRangeVal = (FXElement.Atlas.EntryCountAndIndexRange >> 9) & 0x7f;
	// Format the output
	Writer.WriteLineFmt("\tatlasEntryCount %d;", ((1 << (FXElement.Atlas.ColIndexBits + FXElement.Atlas.RowIndexBits))));
	Writer.WriteLineFmt("\tatlasIndexRange %d;", IndexRangeVal);

	// A list of the samples
	std::vector<FxElemVelStateSample> VelSamples;

	// Prepare to loop and read vel samples
	for (uint32_t i = 0; i < (uint32_t)(FXElement.VelIntervalCount + 1); i++)
	{
		// Add it
		VelSamples.emplace_back(CoDAssets::GameInstance->Read<FxElemVelStateSample>(FXElement.VelSamplesPtr));

		// Advance
		FXElement.VelSamplesPtr += sizeof(FxElemVelStateSample);
	}

	// Output curves
	BOTranslateVelSample(Writer, "velGraph0X", 0, VelSamples);
	BOTranslateVelSample(Writer, "velGraph0Y", 1, VelSamples);
	BOTranslateVelSample(Writer, "velGraph0Z", 2, VelSamples);
	// Output world curves?
	BOTranslateVelSample(Writer, "velGraph1X", 0, { FxElemVelStateSample(), FxElemVelStateSample() });
	BOTranslateVelSample(Writer, "velGraph1Y", 1, { FxElemVelStateSample(), FxElemVelStateSample() });
	BOTranslateVelSample(Writer, "velGraph1Z", 2, { FxElemVelStateSample(), FxElemVelStateSample() });

	// A list of the samples
	std::vector<BOFxElemVisStateSample> VisSamples;

	// Prepare to loop and read vis samples
	for (uint32_t i = 0; i < (uint32_t)(FXElement.VisStateIntervalCount + 1); i++)
	{
		// Add it
		VisSamples.emplace_back(CoDAssets::GameInstance->Read<BOFxElemVisStateSample>(FXElement.VisSamples));

		// Advance
		FXElement.VisSamples += sizeof(BOFxElemVisStateSample);
	}

	// Inject a visual sample if we only have one, required for some fx files
	if (VisSamples.size() == 1)
	{
		auto DefaultSample = BOFxElemVisStateSample();
		// Default everything to 0
		std::memset(&DefaultSample, 0, sizeof(DefaultSample));

		// Reset properties to 1.0 if they have it
		DefaultSample.Base.RotationTotal = 1.0f;
		DefaultSample.Base.Size[0] = 0.5f;
		DefaultSample.Base.Size[1] = 0.5f;
		DefaultSample.Base.Scale = 1.0f;
		DefaultSample.Base.Color[0] = 255;
		DefaultSample.Base.Color[1] = 255;
		DefaultSample.Base.Color[2] = 255;
		DefaultSample.Base.Color[3] = 255;
		DefaultSample.Amplitude.Color[0] = 255;
		DefaultSample.Amplitude.Color[1] = 255;
		DefaultSample.Amplitude.Color[2] = 255;
		DefaultSample.Amplitude.Color[3] = 255;

		// Add the sample
		VisSamples.insert(VisSamples.begin(), DefaultSample);
	}

	// Output it
	BOTranslateVisSample(Writer, VisSamples);

	// Lighting
	Writer.WriteLineFmt("\tlightingFrac %f;", ((float)FXElement.LightingFrac / 255.0f));

	// Calculate radius first
	auto ColRadix = ((std::abs(FXElement.CollMins[0]) + std::abs(FXElement.CollMaxs[1])) / 2.0f);

	// Generate offsets and output
	Writer.WriteLineFmt("\tcollOffset %f %f %f;", (FXElement.CollMaxs[0] - ColRadix), (FXElement.CollMaxs[1] - ColRadix), (FXElement.CollMaxs[2] - ColRadix));
	Writer.WriteLineFmt("\tcollRadius %f;", ColRadix);

	// Effect Impact / Death
	std::string OnImpact = "", OnDeath = "", OnEmission = "", OnAttachment = "";
	// Load the names
	if (FXElement.EffectOnImpact > 0) { OnImpact = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(FXElement.EffectOnImpact)); }
	if (FXElement.EffectOnDeath > 0) { OnDeath = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(FXElement.EffectOnDeath)); }
	if (FXElement.EffectEmitted > 0) { OnEmission = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(FXElement.EffectEmitted)); }
	if (FXElement.EffectAttached > 0) { OnAttachment = CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(FXElement.EffectAttached)); }

	// Output
	Writer.WriteLineFmt("\tfxOnImpact \"%s\";\n\tfxOnDeath \"%s\";", OnImpact.c_str(), OnDeath.c_str());

	// Sort order
	Writer.WriteLineFmt("\tsortOrder %d;", FXElement.SortOrder);
	Writer.WriteLineFmt("\temission \"%s\";", OnEmission.c_str());
	Writer.WriteLineFmt("\temitDist %f %f;", FXElement.EmitDist.Base, FXElement.EmitDist.Amplitude);
	Writer.WriteLineFmt("\temitDistVariance %f %f;", FXElement.EmitDistVariance.Base, FXElement.EmitDistVariance.Amplitude);
	Writer.WriteLineFmt("\tattachment \"%s\";", OnAttachment.c_str());

	// Trails
	BOFxTrailDef TrailDef = { 0 };
	// Read if valid
	if (FXElement.FxTrailDefsPtr > 0) { TrailDef = CoDAssets::GameInstance->Read<BOFxTrailDef>(FXElement.FxTrailDefsPtr); }

	// Basic trails
	Writer.WriteLineFmt("\ttrailSplitDist %d;", TrailDef.SplitDist);
	Writer.WriteLineFmt("\ttrailScrollTime %f;", ((float)TrailDef.ScrollTimeMsec / 1000.0f));
	Writer.WriteLineFmt("\ttrailRepeatDist %d;", TrailDef.RepeatDist);
	Writer.WriteLineFmt("\talphafadetimemsec %d;", FXElement.AlphaFadeTimeMsec);
	Writer.WriteLineFmt("\tmaxwind_mag %d;", FXElement.MaxWindStrength);
	Writer.WriteLineFmt("\tmaxwind_life %d;", FXElement.LifespanAtMaxWind);
	Writer.WriteLineFmt("\tmaxwind_interval %d;", FXElement.SpawnIntervalAtMaxWind);

	// Billboards
	if (FXElement.ElemType != (uint8_t)BOFXElementType::FX_ELEM_TYPE_CLOUD)
	{
		Writer.WriteLineFmt("\tbillboardTopWidth %f;", FXElement.BillboardInfo.BillboardTrim.Base);
		Writer.WriteLineFmt("\tbillboardBottomWidth %f;", FXElement.BillboardInfo.BillboardTrim.Amplitude);
	}
	else
	{
		Writer.WriteLine("\tbillboardTopWidth 1;");
		Writer.WriteLine("\tbillboardBottomWidth 1;");
	}

	// Sounds
	Writer.WriteLine("\telemSpawnSound\n\t{");
	// Output if we have one
	if (FXElement.SpawnSound > 0)
	{
		// Read name
		auto SoundName = CoDAssets::GameInstance->ReadNullTerminatedString(FXElement.SpawnSound);
		// Output
		Writer.WriteLineFmt("\t\t\"%s\"", SoundName.c_str());
	}
	// End sounds
	Writer.WriteLine("\t};");

	// Clouds
	if (FXElement.ElemType == (uint8_t)BOFXElementType::FX_ELEM_TYPE_CLOUD)
	{
		Writer.WriteLineFmt("\tcloudDensity %d %d;", FXElement.BillboardInfo.BillboardCloudRange.Base, FXElement.BillboardInfo.BillboardCloudRange.Amplitude);
	}
	else
	{
		Writer.WriteLine("\tcloudDensity 1024 0;");
	}

	// Trail def
	if (FXElement.FxTrailDefsPtr > 0)
	{
		// Begin
		Writer.WriteLine("\ttrailDef\n\t{");

		// Loop
		for (uint32_t v = 0; v < TrailDef.VertCount; v++)
		{
			// Read it
			auto Vertex = CoDAssets::GameInstance->Read<FxTrailVertex>(TrailDef.VertPtr);

			// Format
			Writer.WriteLineFmt("\t\t %f %f %f", Vertex.Position[0], Vertex.Position[1], Vertex.TexCoord);

			// Advance
			TrailDef.VertPtr += sizeof(FxTrailVertex);
		}

		// End and begin ind
		Writer.WriteLine("\t} {");

		// Loop
		for (uint32_t v = 0; v < TrailDef.IndCount; v++)
		{
			// Read it
			auto Index = CoDAssets::GameInstance->Read<uint16_t>(TrailDef.IndexPtr);

			// Format
			Writer.WriteLineFmt("\t\t %d", Index);

			// Advance
			TrailDef.IndexPtr += sizeof(uint16_t);
		}

		// End
		Writer.WriteLine("\t};");
	}

	// Pivot
	Writer.WriteLineFmt("\tbillboardPivot %f %f;", (FXElement.BillboardPivot[0] / 2.0f), (FXElement.BillboardPivot[1] / -2.0f));

	// Prepare visuals
	switch (FXElement.ElemType)
	{
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD: Writer.WriteLine("\tbillboardSprite\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED: Writer.WriteLine("\torientedSprite\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_ROTATED: Writer.WriteLine("\trotatedSprite\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_TAIL: Writer.WriteLine("\ttail\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_LINE: Writer.WriteLine("\tline\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_TRAIL: Writer.WriteLine("\ttrail\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_CLOUD: Writer.WriteLine("\tcloud\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_MODEL: Writer.WriteLine("\tmodel\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_OMNI_LIGHT: Writer.WriteLine("\tlight;"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPOT_LIGHT: Writer.WriteLine("\tspotLight;"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SOUND: Writer.WriteLine("\tsound\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_DECAL: Writer.WriteLine("\tdecal\n\t{"); break;
	case (uint8_t)BOFXElementType::FX_ELEM_TYPE_RUNNER: Writer.WriteLine("\trunner\n\t{"); break;

#if _DEBUG
	default:
		// Only log on debug
		printf("Unknown ElemType: 0x%X\n", FXElement.ElemType);
		break;
#endif
	}

	// Check count
	if (FXElement.VisualCount == 1)
	{
		switch (FXElement.ElemType)
		{
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_ROTATED:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_TRAIL:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_LINE:
			// Pointer to pointer (Requires removed path)
			Writer.WriteLineFmt("\t\t\"%s\"", FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(FXElement.VisualsPtr))).c_str());
			break;
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_TAIL:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_CLOUD:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_MODEL:
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_RUNNER:
			// Pointer to pointer
			Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(FXElement.VisualsPtr)).c_str());
			break;
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_DECAL:
			// Pointer to material
			Writer.WriteLineFmt("\t\t\"%s\"", FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(CoDAssets::GameInstance->Read<uint32_t>(FXElement.VisualsPtr)))).c_str());
			break;
		case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SOUND:
			// Raw name
			Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(FXElement.VisualsPtr).c_str());
			break;
		}
	}
	else
	{
		// We have an array
		for (uint32_t v = 0; v < FXElement.VisualCount; v++)
		{
			// Read
			auto VisualPtr = CoDAssets::GameInstance->Read<uint32_t>(FXElement.VisualsPtr);

			switch (FXElement.ElemType)
			{
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_BILLBOARD:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_ORIENTED:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPRITE_ROTATED:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_DECAL:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_TRAIL:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_LINE:
				// Pointer to pointer (Requires removed path)
				Writer.WriteLineFmt("\t\t\"%s\"", FileSystems::GetFileName(CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(VisualPtr))).c_str());
				break;
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_TAIL:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_CLOUD:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_MODEL:
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_RUNNER:
				// Pointer to pointer
				Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(CoDAssets::GameInstance->Read<uint32_t>(VisualPtr)).c_str());
				break;
			case (uint8_t)BOFXElementType::FX_ELEM_TYPE_SOUND:
				// Raw name
				Writer.WriteLineFmt("\t\t\"%s\"", CoDAssets::GameInstance->ReadNullTerminatedString(VisualPtr).c_str());
				break;
			}

			// Advance
			FXElement.VisualsPtr += sizeof(uint32_t);
		}
	}

	// Close it up, lights are not included...
	if (FXElement.ElemType != (uint8_t)BOFXElementType::FX_ELEM_TYPE_OMNI_LIGHT
		&& FXElement.ElemType != (uint8_t)BOFXElementType::FX_ELEM_TYPE_SPOT_LIGHT)
	{
		Writer.WriteLine("\t};");
	}

	// End the effect
	Writer.WriteLine("}\n");
}

void BOTranslateVelSample(TextWriter& Writer, const std::string& CurveName, uint32_t CurveIndex, const std::vector<FxElemVelStateSample>& VelSamples)
{
	// Calculate max value for this entry
	float IndexMaxValue = 0;

	// Loop and check
	for (auto& Samples : VelSamples)
	{
		// Calculate base
		auto Shifted = std::abs(Samples.Local.Velocity.Base[CurveIndex] * ((VelSamples.size() - 1) * 1000));
		// Check initial base
		if (IndexMaxValue < Shifted)
		{
			// Set it
			IndexMaxValue = Shifted;
		}
		// Calculate with amp
		Shifted = std::abs((Samples.Local.Velocity.Base[CurveIndex] + Samples.Local.Velocity.Amplitude[CurveIndex]) * ((VelSamples.size() - 1) * 1000));
		// Check base and amp
		if (IndexMaxValue < (Shifted))
		{
			// Set it
			IndexMaxValue = Shifted;
		}
	}

	// Output start
	Writer.WriteLineFmt("\t%s %f\n\t{", CurveName.c_str(), (IndexMaxValue * 2));

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VelSamples.size(); i++)
	{
		// Get it
		auto& Samples = VelSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VelSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (IndexMaxValue > 0) ? ((Samples.Local.Velocity.Base[CurveIndex] * ((VelSamples.size() - 1) * 1000) / IndexMaxValue) / 2.0f) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base
	Writer.WriteLine("\t\t}");

	// Begin amp
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VelSamples.size(); i++)
	{
		// Get it
		auto& Samples = VelSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VelSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (IndexMaxValue > 0) ? (((Samples.Local.Velocity.Amplitude[CurveIndex] + Samples.Local.Velocity.Base[CurveIndex]) * ((VelSamples.size() - 1) * 1000) / IndexMaxValue) / 2.0f) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}");

	// Close
	Writer.WriteLine("\t};");
}

void BOTranslateVisSample(TextWriter& Writer, const std::vector<BOFxElemVisStateSample>& VisSamples)
{
	// Calculate color indicies
	uint8_t RIndex = 2, GIndex = 1, BIndex = 0, AIndex = 3;

	// Black Ops 2 swaps B->R channels in color vis samples, we swap them back...
	if (CoDAssets::GameID == SupportedGames::BlackOps2)
	{
		RIndex = 0;
		BIndex = 2;
	}

	// Calculate max value for this entry
	float RotationMaxValue = 0, Size1Value = 0, Size2Value = 0, ScaleMaxValue = 0;

	// Loop and check
	for (auto& Samples : VisSamples)
	{
		// Calculate base+amp
		float Shifted = std::abs(((Samples.Base.RotationDelta + Samples.Amplitude.RotationDelta) * (VisSamples.size())));
		// Check base+amp
		if (RotationMaxValue < Shifted)
		{
			// Set it
			RotationMaxValue = Shifted;
		}

		// Check base+amp
		if (Size1Value < Samples.Base.Size[0])
		{
			// Set it
			Size1Value = Samples.Base.Size[0];
		}
		if (Size1Value < (Samples.Base.Size[0] + Samples.Amplitude.Size[0]))
		{
			// Set it
			Size1Value = (Samples.Base.Size[0] + Samples.Amplitude.Size[0]);
		}

		// Check base+amp
		if (Size2Value < Samples.Base.Size[1])
		{
			// Set it
			Size2Value = Samples.Base.Size[1];
		}
		if (Size2Value < (Samples.Base.Size[1] + Samples.Amplitude.Size[1]))
		{
			// Set it
			Size2Value = (Samples.Base.Size[1] + Samples.Amplitude.Size[1]);
		}

		// Check base+amp
		if (ScaleMaxValue < Samples.Base.Scale)
		{
			// Set it
			ScaleMaxValue = Samples.Base.Scale;
		}
		if (ScaleMaxValue < (Samples.Base.Scale + Samples.Amplitude.Scale))
		{
			// Set it
			ScaleMaxValue = (Samples.Base.Scale + Samples.Amplitude.Scale);
		}
	}

	// Output start
	Writer.WriteLineFmt("\trotGraph %f\n\t{", RotationMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (RotationMaxValue > 0) ? ((Samples.Base.RotationDelta * (VisSamples.size()) / RotationMaxValue) / 2.0f) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (RotationMaxValue > 0) ? (((Samples.Base.RotationDelta + Samples.Amplitude.RotationDelta) * (VisSamples.size()) / RotationMaxValue) / 2.0f) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tsizeGraph0 %f\n\t{", (Size1Value * 2));

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size1Value > 0) ? (Samples.Base.Size[0] / Size1Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size1Value > 0) ? ((Samples.Base.Size[0] + Samples.Amplitude.Size[0]) / Size1Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tsizeGraph1 %f\n\t{", (Size2Value * 2));

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size2Value > 0) ? (Samples.Base.Size[1] / Size2Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size2Value > 0) ? ((Samples.Base.Size[1] + Samples.Amplitude.Size[1]) / Size2Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tscaleGraph %f\n\t{", ScaleMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (ScaleMaxValue > 0) ? (Samples.Base.Scale / ScaleMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (ScaleMaxValue > 0) ? ((Samples.Base.Scale + Samples.Amplitude.Scale) / ScaleMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLine("\tcolorGraph 1\n\t{");

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f %f %f", TimeFrame, Samples.Base.Color[RIndex] / 255.0f, Samples.Base.Color[GIndex] / 255.0f, Samples.Base.Color[BIndex] / 255.0f);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f %f %f", TimeFrame, Samples.Amplitude.Color[RIndex] / 255.0f, Samples.Amplitude.Color[GIndex] / 255.0f, Samples.Amplitude.Color[BIndex] / 255.0f);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLine("\talphaGraph 1\n\t{");

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, Samples.Base.Color[AIndex] / 255.0f);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, Samples.Amplitude.Color[AIndex] / 255.0f);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");
}

void BO3TranslateVisSample(TextWriter& Writer, const std::vector<BO3FxElemVisStateSample>& VisSamples)
{
	// Calculate color indicies (Correct for BO3)
	uint8_t RIndex = 0, GIndex = 1, BIndex = 2, AIndex = 3;

	// Calculate max value for this entry
	float RotationMaxValue = 0, Size1Value = 0, Size2Value = 0, ScaleMaxValue = 0, ChildScaleMaxValue = 0, LightIntensityMaxValue = 0, LightRadiusMaxValue = 0, LightFOVMaxValue = 0;

	// Loop and check
	for (auto& Samples : VisSamples)
	{
		// Calculate base+amp
		float Shifted = std::abs(((Samples.Base.RotationDelta + Samples.Amplitude.RotationDelta) * (VisSamples.size())));
		// Check base+amp
		if (RotationMaxValue < Shifted)
		{
			// Set it
			RotationMaxValue = Shifted;
		}

		// Check base+amp
		if (Size1Value < Samples.Base.Size[0])
		{
			// Set it
			Size1Value = Samples.Base.Size[0];
		}
		if (Size1Value < (Samples.Base.Size[0] + Samples.Amplitude.Size[0]))
		{
			// Set it
			Size1Value = (Samples.Base.Size[0] + Samples.Amplitude.Size[0]);
		}

		// Check base+amp
		if (Size2Value < Samples.Base.Size[1])
		{
			// Set it
			Size2Value = Samples.Base.Size[1];
		}
		if (Size2Value < (Samples.Base.Size[1] + Samples.Amplitude.Size[1]))
		{
			// Set it
			Size2Value = (Samples.Base.Size[1] + Samples.Amplitude.Size[1]);
		}

		// Check base+amp
		if (ScaleMaxValue < Samples.Base.Scale)
		{
			// Set it
			ScaleMaxValue = Samples.Base.Scale;
		}
		if (ScaleMaxValue < (Samples.Base.Scale + Samples.Amplitude.Scale))
		{
			// Set it
			ScaleMaxValue = (Samples.Base.Scale + Samples.Amplitude.Scale);
		}

		// Check base+amp
		if (ChildScaleMaxValue < Samples.Base.ChildScale)
		{
			// Set it
			ChildScaleMaxValue = Samples.Base.ChildScale;
		}
		if (ChildScaleMaxValue < (Samples.Base.ChildScale + Samples.Amplitude.ChildScale))
		{
			// Set it
			ChildScaleMaxValue = (Samples.Base.ChildScale + Samples.Amplitude.ChildScale);
		}

		// Check base+amp
		if (LightIntensityMaxValue < Samples.Base.LightIntensity)
		{
			// Set it
			LightIntensityMaxValue = Samples.Base.LightIntensity;
		}
		if (LightIntensityMaxValue < (Samples.Base.LightIntensity + Samples.Amplitude.LightIntensity))
		{
			// Set it
			LightIntensityMaxValue = (Samples.Base.LightIntensity + Samples.Amplitude.LightIntensity);
		}

		// Check base+amp
		if (LightRadiusMaxValue < Samples.Base.LightRadius)
		{
			// Set it
			LightRadiusMaxValue = Samples.Base.LightRadius;
		}
		if (LightRadiusMaxValue < (Samples.Base.LightRadius + Samples.Amplitude.LightRadius))
		{
			// Set it
			LightRadiusMaxValue = (Samples.Base.LightRadius + Samples.Amplitude.LightRadius);
		}

		// Check base+amp
		if (LightFOVMaxValue < Samples.Base.LightFOV)
		{
			// Set it
			LightFOVMaxValue = Samples.Base.LightFOV;
		}
		if (LightFOVMaxValue < (Samples.Base.LightFOV + Samples.Amplitude.LightFOV))
		{
			// Set it
			LightFOVMaxValue = (Samples.Base.LightFOV + Samples.Amplitude.LightFOV);
		}
	}

	// Output start
	Writer.WriteLineFmt("\trotGraph %f\n\t{", RotationMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (RotationMaxValue > 0) ? ((Samples.Base.RotationDelta * (VisSamples.size()) / RotationMaxValue) / 2.0f) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (RotationMaxValue > 0) ? (((Samples.Base.RotationDelta + Samples.Amplitude.RotationDelta) * (VisSamples.size()) / RotationMaxValue) / 2.0f) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tsizeGraph0 %f\n\t{", (Size1Value * 2));

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size1Value > 0) ? (Samples.Base.Size[0] / Size1Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size1Value > 0) ? ((Samples.Base.Size[0] + Samples.Amplitude.Size[0]) / Size1Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tsizeGraph1 %f\n\t{", (Size2Value * 2));

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size2Value > 0) ? (Samples.Base.Size[1] / Size2Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (Size2Value > 0) ? ((Samples.Base.Size[1] + Samples.Amplitude.Size[1]) / Size2Value) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tscaleGraph %f\n\t{", ScaleMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (ScaleMaxValue > 0) ? (Samples.Base.Scale / ScaleMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (ScaleMaxValue > 0) ? ((Samples.Base.Scale + Samples.Amplitude.Scale) / ScaleMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tchildSizeScaleGraph %f\n\t{", ChildScaleMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (ChildScaleMaxValue > 0) ? (Samples.Base.ChildScale / ChildScaleMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (ChildScaleMaxValue > 0) ? ((Samples.Base.ChildScale + Samples.Amplitude.ChildScale) / ChildScaleMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLine("\tcolorGraph 1\n\t{");

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f %f %f", TimeFrame, Samples.Base.Color[RIndex] / 255.0f, Samples.Base.Color[GIndex] / 255.0f, Samples.Base.Color[BIndex] / 255.0f);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f %f %f", TimeFrame, Samples.Amplitude.Color[RIndex] / 255.0f, Samples.Amplitude.Color[GIndex] / 255.0f, Samples.Amplitude.Color[BIndex] / 255.0f);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLine("\talphaGraph 1\n\t{");

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, Samples.Base.Color[AIndex] / 255.0f);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, Samples.Amplitude.Color[AIndex] / 255.0f);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tlightIntensityGraph %f\n\t{", LightIntensityMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (LightIntensityMaxValue > 0) ? (Samples.Base.LightIntensity / LightIntensityMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (LightIntensityMaxValue > 0) ? ((Samples.Base.LightIntensity + Samples.Amplitude.LightIntensity) / LightIntensityMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tlightRadiusGraph %f\n\t{", LightRadiusMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (LightRadiusMaxValue > 0) ? (Samples.Base.LightRadius / LightRadiusMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (LightRadiusMaxValue > 0) ? ((Samples.Base.LightRadius + Samples.Amplitude.LightRadius) / LightRadiusMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");

	// Output start
	Writer.WriteLineFmt("\tlightFovGraph %f\n\t{", LightFOVMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (LightFOVMaxValue > 0) ? (Samples.Base.LightFOV / LightFOVMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)VisSamples.size(); i++)
	{
		// Get it
		auto& Samples = VisSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(VisSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (LightFOVMaxValue > 0) ? ((Samples.Base.LightFOV + Samples.Amplitude.LightFOV) / LightFOVMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");
}

void BOTranslateGenericSample(TextWriter& Writer, const std::string& CurveName, const std::vector<FxFloatRange>& GenericSamples)
{
	// Calculate max value for this entry
	float GenericMaxValue = 0;

	// Loop and check
	for (auto& Samples : GenericSamples)
	{
		// Check base+amp
		if (GenericMaxValue < Samples.Base)
		{
			// Set it
			GenericMaxValue = Samples.Base;
		}
		if (GenericMaxValue < (Samples.Base + Samples.Amplitude))
		{
			// Set it
			GenericMaxValue = (Samples.Base + Samples.Amplitude);
		}
	}

	// Output start
	Writer.WriteLineFmt("\t%s %f\n\t{", CurveName.c_str(), GenericMaxValue);

	// Begin base
	Writer.WriteLine("\t\t{");

	for (uint32_t i = 0; i < (uint32_t)GenericSamples.size(); i++)
	{
		// Get it
		auto& Samples = GenericSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(GenericSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (GenericMaxValue > 0) ? (Samples.Base / GenericMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End base start amp
	Writer.WriteLine("\t\t}\n\t\t{");

	for (uint32_t i = 0; i < (uint32_t)GenericSamples.size(); i++)
	{
		// Get it
		auto& Samples = GenericSamples[i];

		// Get current timeframe
		float TimeFrame = 0;
		// Check
		if (i > 0)
		{
			// Set the frame
			TimeFrame = ((float)i / (float)(GenericSamples.size() - 1));
		}

		// Calculate curve point
		auto CurvePoint = (GenericMaxValue > 0) ? ((Samples.Base + Samples.Amplitude) / GenericMaxValue) : 0;
		// Format time
		Writer.WriteLineFmt("\t\t\t%f %f", TimeFrame, CurvePoint);
	}

	// End amp
	Writer.WriteLine("\t\t}\n\t};");
}

void BO3TranslateDynamicLight(TextWriter& Writer, const FxDynamicLight& Light)
{
	// Prepare to output the lighting definition
	auto NameBuffer = CoDAssets::GameInstance->ReadNullTerminatedString(Light.NamePtr);
	// Strip the hash from the name, if any
	if (NameBuffer.find("-") != std::string::npos)
		NameBuffer = NameBuffer.substr(0, NameBuffer.find_last_of("-"));

	// Output the name
	Writer.WriteLineFmt("\t\t%s\n\t\t{", NameBuffer.c_str());

	// Extract colors
	auto ColorData = ConvertLinearToSRGB(Light.EncodedColors);

	// TODO: Color data 2048 is from pow(2, stops);

	// Shape properties
	Writer.WriteLineFmt("\t\t\tENABLE_FALLOFF %s", ((Light.FallOffDistance > 0) ? "1" : "0"));
	Writer.WriteLineFmt("\t\t\tPRIMARY_NOSHADOWMAP %s", ((Light.PrimaryNoShadowMap == 0x1) ? "1" : "0"));
	Writer.WriteLineFmt("\t\t\tPRIMARY_TYPE %s", ((Light.PrimaryType == 0x4) ? "OMNI" : "SPOT"));
	Writer.WriteLineFmt("\t\t\t_color %f %f %f", ColorData.X, ColorData.Y, ColorData.Z);
	Writer.WriteLineFmt("\t\t\tangles %f %f %f", Light.Angles[0], Light.Angles[1], Light.Angles[2]);
	Writer.WriteLineFmt("\t\t\tbake_intensity_scale %f", 1.0);
	Writer.WriteLineFmt("\t\t\tbulbLength %f", Light.BulbLength);
	Writer.WriteLineFmt("\t\t\tculling_cutoff %f", (Light.CullingCutOff * 1080.0f));
	Writer.WriteLineFmt("\t\t\tculling_falloff %f", (Light.CullingFallOff * 1080.0f));
	Writer.WriteLineFmt("\t\t\tcut_on %f", Light.CutOn);
	Writer.WriteLineFmt("\t\t\tfalloffdistance %f", Light.FallOffDistance);
	Writer.WriteLineFmt("\t\t\tfar_edge %f", Light.FarEdge);
	Writer.WriteLineFmt("\t\t\tfov_outer %f", ((std::acosf(Light.FOVOuter) * (1.0f / 0.017453292f)) * 2.0f));
	Writer.WriteLineFmt("\t\t\tmin_light_cutoff %f", 0);
	Writer.WriteLineFmt("\t\t\tnear_edge %f", Light.NearEdge);
	Writer.WriteLineFmt("\t\t\tpenumbraRadius %f", Light.PenumbraRadius);
	Writer.WriteLineFmt("\t\t\tradius %f", Light.Radius);
	Writer.WriteLineFmt("\t\t\troundness %f", Light.Roundness);
	Writer.WriteLineFmt("\t\t\tculling_use_pure_radius %d", Light.CullingUsePureRadius);
	Writer.WriteLineFmt("\t\t\tPROBE_ONLY %d", Light.ProbeOnly);

	// Defs
	Writer.WriteLineFmt("\t\t\tdef_angle %f", (Light.DefAngle * (1.0f / 0.017453292f)));
	Writer.WriteLineFmt("\t\t\tdef_center %f %f", 0, 0);
	Writer.WriteLineFmt("\t\t\tdef_offset %f %f", Light.DefOffset[0], Light.DefOffset[1]);
	Writer.WriteLineFmt("\t\t\tdef_rotation %f", (Light.DefRotation * (1.0f / 0.017453292f)));
	Writer.WriteLineFmt("\t\t\tdef_scroll %f %f", Light.DefScroll[0], Light.DefScroll[1]);
	Writer.WriteLineFmt("\t\t\tdef_shear %f %f", 0, 0);
	Writer.WriteLineFmt("\t\t\tdef_tile %f %f", Light.DefTile[0], Light.DefTile[1]);

	// Shadows
	switch (Light.ShadowUpdate)
	{
	case 0: Writer.WriteLine("\t\t\tshadowUpdate AlwaysNoCache"); break;
	case 1: Writer.WriteLine("\t\t\tshadowUpdate Always"); break;
	case 2: Writer.WriteLine("\t\t\tshadowUpdate Never"); break;
	}
	switch (Light.ShadowMapScale)
	{
	case 1: Writer.WriteLine("\t\t\tshadowmapScale 2"); break;
	case 2: Writer.WriteLine("\t\t\tshadowmapScale 1"); break;
	case 3: Writer.WriteLine("\t\t\tshadowmapScale 0.5"); break;
	case 0: Writer.WriteLine("\t\t\tshadowmapScale 4"); break;
	}
	Writer.WriteLineFmt("\t\t\tspec_comp %f", (std::logf(Light.SpecularComp) / std::logf(2.0f)));

	// Ellipse
	Writer.WriteLineFmt("\t\t\tsuperellipse %f %f %f %f", Light.SuperEllipse[0], Light.SuperEllipse[1], Light.SuperEllipse[2], Light.SuperEllipse[3]);

	// Lighting
	Writer.WriteLineFmt("\t\t\tortho_effect %f", Light.OrthoEffect);
	Writer.WriteLineFmt("\t\t\tscriptable %d", 0);
	Writer.WriteLine("\t\t\tstops 11");
	Writer.WriteLineFmt("\t\t\tortho_effect %f", Light.OrthoEffect);
	Writer.WriteLineFmt("\t\t\tvolumetric %d", Light.Volumetric);
	Writer.WriteLineFmt("\t\t\tvolumetricCookies %d", Light.VolumetricCookies);
	Writer.WriteLineFmt("\t\t\tvolumetricIntensityBoost %f", (std::logf(Light.VolumetricIntensityBoost) / std::logf(2.0f)));
	Writer.WriteLineFmt("\t\t\tvolumetricSampleCount %d", Light.VolumetricSampleCount);

	// End the definition
	Writer.WriteLine("\t\t};");
}

void BOProduceElementName(TextWriter& Writer, uint32_t ElementType, uint32_t ElementIndex)
{
	// Begin the effect
	switch (ElementType)
	{
	case 0: Writer.WriteLineFmt("{\n\tname \"wraith_looping_def%d\";", ElementIndex); break;
	case 1: Writer.WriteLineFmt("{\n\tname \"wraith_oneshot_def%d\";", ElementIndex); break;
	case 2: Writer.WriteLineFmt("{\n\tname \"wraith_emission_def%d\";", ElementIndex); break;
	}
}

void FxIntRangeInverse(FxIntRange& Range)
{
	if (Range.Base < 0)
	{
		// Only invert if we had a > abs of amp
		Range.Base = (Range.Base + Range.Amplitude);
		Range.Amplitude = Range.Amplitude * -1;
	}
}

void FxFloatRangeInverse(FxFloatRange& Range)
{
	if (Range.Base < 0)
	{
		// Only invert if we had a > abs of amp
		Range.Base = (Range.Base + Range.Amplitude);
		Range.Amplitude = Range.Amplitude * -1.0f;
	}
}

float AngularVelToDegrees(float Value)
{
	// Angular velocity has an extra 1000 tacked on...
	return (float)(((Value * 180000.0f) / PIValue));
}

Quaternion UnpackVec4Quat(uint32_t Value)
{
	// Prepare to unpack the value
	float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	// Calculate bit changes
	int32_t ix = (int32_t)(Value & 0x1ff);
	int32_t iy = (int32_t)((Value >> 9) & 0x3ff);
	int32_t iz = (int32_t)((Value >> 19) & 0x3ff);

	// Check for overflow
	if (ix > 0xff) ix -= 0x200;
	if (iy > 0x1ff) iy -= 0x400;
	if (iz > 0x1ff) iz -= 0x400;

	// Bring into range
	x = ix / 255.0f;
	y = iy / 511.0f;
	z = iz / 511.0f;

	// Calculate W
	w = (float)(1.0f / std::sqrt(1 + x * x + y * y + z * z));

	// All values require W as a mod
	x *= w;
	y *= w;
	z *= w;

	// Calculate biggest index and shift values
	uint32_t BiggestIndex = 3 - ((Value >> 29) & 3);

	// Output the result
	switch (BiggestIndex)
	{
	case 0: return Quaternion(w, x, y, z);
	case 1: return Quaternion(z, w, x, y);
	case 2: return Quaternion(y, z, w, x);
	case 3: return Quaternion(x, y, z, w);
	default: return Quaternion::Identity();
	}
}

Vector3 ConvertLinearToSRGB(const float Values[3])
{
	// Calculate range
	double R = Values[0] / 2048;
	double G = Values[1] / 2048;
	double B = Values[2] / 2048;

	// Check Values
	if (R <= 0.0031308)
		R = R * 12.92;
	else
		R = 1.055 * std::pow(R, 1.0 / 2.4) - 0.055;
	if (G <= 0.0031308)
		G = G * 12.92;
	else
		G = 1.055 * std::pow(G, 1.0 / 2.4) - 0.055;
	if (B <= 0.0031308)
		B = B * 12.92;
	else
		B = 1.055 * std::pow(B, 1.0 / 2.4) - 0.055;

	// Return result
	return Vector3((float)R, (float)G, (float)B);
}