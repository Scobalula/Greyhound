#include "stdafx.h"

// The class we are implementing
#include "CoDXModelTranslator.h"

// We need the following WraithX classes
#include "ProcessReader.h"
#include "MemoryReader.h"
#include "HalfFloats.h"
#include "Strings.h"

// We need the following games for streamed lod support
#include "GameBlackOps3.h"
#include "GameBlackOps4.h"
#include "GameWorldWar2.h"
#include "GameModernWarfare4.h"

// Include generic structures
#include "DBGameGenerics.h"
#include "SettingsManager.h"

struct QSGfxVertexBuffer
{
    Vector3 Position;

    uint32_t BiNormal;
    uint8_t Color[4];
    uint32_t Pad1;
    Vector2 UV;
    uint32_t Tangent;
    uint32_t Normal;
};

std::unique_ptr<WraithModel> CoDXModelTranslator::TranslateXModel(const std::unique_ptr<XModel_t>& Model, uint32_t LodIndex, bool JustBones)
{
    // Check if we want Vertex Colors
    bool ExportColors = (SettingsManager::GetSetting("exportvtxcolor", "true") == "true");
    // Prepare to generate a WraithModel
    auto ModelResult = std::make_unique<WraithModel>();

    // Apply name
    ModelResult->AssetName = Model->ModelName;

    // Apply distances
    ModelResult->LodDistance = Model->ModelLods[LodIndex].LodDistance;
    ModelResult->LodMaxDistance = Model->ModelLods[LodIndex].LodMaxDistance;

    // Bone matrix size
    uintptr_t ReadDataSize = 0;
    // Calculated matricies size
    auto GlobalMatrixLength = (sizeof(DObjAnimMat) * (Model->BoneCount + Model->CosmeticBoneCount));
    auto LocalTranslationLength = (sizeof(Vector3) * ((Model->BoneCount + Model->CosmeticBoneCount) - Model->RootBoneCount));
    auto LocalRotationLength = (sizeof(QuatData) * ((Model->BoneCount + Model->CosmeticBoneCount) - Model->RootBoneCount));
    // Read global matrix data
    auto GlobalMatrixData = MemoryReader(CoDAssets::GameInstance->Read(Model->BaseMatriciesPtr, GlobalMatrixLength, ReadDataSize), ReadDataSize);
    auto LocalTranslationData = MemoryReader(CoDAssets::GameInstance->Read(Model->TranslationsPtr, LocalTranslationLength, ReadDataSize), ReadDataSize);
    auto LocalRotationData = MemoryReader(CoDAssets::GameInstance->Read(Model->RotationsPtr, LocalRotationLength, ReadDataSize), ReadDataSize);

    // Whether or not use bone was ticked
    bool NeedsLocalPositions = true;

    // Grab the bone ids pointer
    auto BoneIDs = Model->BoneIDsPtr;
    // Grab the bone parent pointer
    auto BoneParents = Model->BoneParentsPtr;

    // Prepare the model for bones
    ModelResult->PrepareBones(Model->BoneCount + Model->CosmeticBoneCount);

    // Iterate and build bones
    for (uint32_t i = 0; i < (Model->BoneCount + Model->CosmeticBoneCount); i++)
    {
        // Read the ID
        uint64_t BoneID = 0;

        // Check size
        switch (Model->BoneIndexSize)
        {
        case 2: BoneID = CoDAssets::GameInstance->Read<uint16_t>(BoneIDs); break;
        case 4: BoneID = CoDAssets::GameInstance->Read<uint32_t>(BoneIDs); break;
        }

        // Add the new bone
        auto& NewBone = ModelResult->AddBone();

        // Set the new bones name (Determine if we need something else)
        auto BoneName = CoDAssets::GameStringHandler(BoneID);
        // Check for an invalid tag name
        if (BoneName == "")
        {
            // Make a new bone name
            if (i == 0)
            {
                NewBone.TagName = "tag_origin";
            }
            else
            {
                NewBone.TagName = Strings::Format("no_tag_%d", i);
            }
        }
        else
        {
            // Set as read
            NewBone.TagName = BoneName;
        }
        
        // Read the parent ID (Default as a root bone)
        int32_t BoneParent = -1;

        // Check if we have parent ids yet
        if (i >= Model->RootBoneCount)
        {
            // We have a parent id to read
            switch (Model->BoneParentSize)
            {
            case 1: BoneParent = CoDAssets::GameInstance->Read<uint8_t>(BoneParents); break;
            case 2: BoneParent = CoDAssets::GameInstance->Read<uint16_t>(BoneParents); break;
            case 4: BoneParent = CoDAssets::GameInstance->Read<uint32_t>(BoneParents); break;
            }

            // Check if we're cosmetic bones
            if (i < Model->BoneCount)
            {
                // Subtract position
                BoneParent = (i - BoneParent);
            }
            else
            {
                // Absolute position
                BoneParent = BoneParent;
            }

            // Advance
            BoneParents += Model->BoneParentSize;
        }
        else
        {
            // Take i and subtract 1
            BoneParent = (i - 1);
        }

        // Set bone parent
        NewBone.BoneParent = BoneParent;

        // Read global data
        auto GlobalData = GlobalMatrixData.Read<DObjAnimMat>();

        // Assign global position
        NewBone.GlobalPosition = GlobalData.Translation;
        NewBone.GlobalRotation = GlobalData.Rotation;

        // Check if we aren't a root bone
        if (i > (Model->RootBoneCount - 1))
        {
            // We have local translations and rotations
            NewBone.LocalPosition = LocalTranslationData.Read<Vector3>();

            // Check if it was not zero
            if (NewBone.LocalPosition != Vector3(0, 0, 0))
            {
                // We don't need them
                NeedsLocalPositions = false;
            }
            
            // Read rotation data
            auto RotationData = LocalRotationData.Read<QuatData>();
            // Build rotation value
            if (Model->BoneRotationData == BoneDataTypes::DivideBySize)
            {
                // Set
                NewBone.LocalRotation = Quaternion((float)RotationData.RotationX / 32768.0f, (float)RotationData.RotationY / 32768.0f, (float)RotationData.RotationZ / 32768.0f, (float)RotationData.RotationW / 32768.0f);
            }
            else if (Model->BoneRotationData == BoneDataTypes::HalfFloat)
            {
                // Set
                NewBone.LocalRotation = Quaternion(HalfFloats::ToFloat((uint16_t)RotationData.RotationX), HalfFloats::ToFloat((uint16_t)RotationData.RotationY), HalfFloats::ToFloat((uint16_t)RotationData.RotationZ), HalfFloats::ToFloat((uint16_t)RotationData.RotationW));
            }
            else if (Model->BoneRotationData == BoneDataTypes::QuatPackingA)
            {
                // Set
                NewBone.LocalRotation = VectorPacking::QuatPackingA(*(uint64_t*)&RotationData);
            }
        }

        // Advance
        BoneIDs += Model->BoneIndexSize;
    }

    // Chean up bone data
    GlobalMatrixData.Close();
    LocalTranslationData.Close();
    LocalRotationData.Close();

    // If we're a viewmodel (!UseBones) && we have at least > 1 bone, we need to generate local positions from the globals
    if (NeedsLocalPositions && Model->BoneCount > 1)
    {
        // Generate
        ModelResult->GenerateLocalPositions(true, false);
    }

    // Check if we didn't parse any bones, if we didn't, we need can "inject" tag_origin (this is mostly for MW which has weird models
    // that are rare with thousands of bones)
    if (Model->BoneCount == 0)
    {
        // Add the new bone
        auto& NewBone = ModelResult->AddBone();
        NewBone.TagName = "tag_origin";
        NewBone.BoneParent = -1;
    }

    // Check if we just wanted the bones of the model (Used for hitbox logic)
    if (JustBones) { return ModelResult; }

    // Store game index (For faster lookup during conversion)
    auto CurrentGame = CoDAssets::GameID;

    // Grab reference to the lod we want to translate
    auto& LodReference = Model->ModelLods[LodIndex];

    // A list of unique material names
    std::map<std::string, uint32_t> UniqueMaterials;

    // Build unique material references (Right now, it's one material per submesh)
    for (uint32_t i = 0; i < (uint32_t)LodReference.Materials.size(); i++)
    {
        // Check if it exists
        auto FindResult = UniqueMaterials.find(LodReference.Materials[i].MaterialName);
        // Check if we have one
        if (FindResult != UniqueMaterials.end())
        {
            // We have it, calculate index and set
            LodReference.Submeshes[i].MaterialIndex = UniqueMaterials[LodReference.Materials[i].MaterialName];
        }
        else
        {
            // Add it
            auto& NewMat = ModelResult->AddMaterial();
            // Assign values
            NewMat.MaterialName = Strings::Replace(LodReference.Materials[i].MaterialName, "@", "_");

            // Assign image names
            for (auto& Image : LodReference.Materials[i].Images)
            {
                // Check
                switch (Image.ImageUsage)
                {
                case ImageUsageType::DiffuseMap:
                    NewMat.DiffuseMapName = Image.ImageName;
                    break;
                case ImageUsageType::NormalMap:
                    NewMat.NormalMapName = Image.ImageName;
                    break;
                case ImageUsageType::SpecularMap:
                    NewMat.SpecularMapName = Image.ImageName;
                    break;
                }
            }

            // Set
            LodReference.Submeshes[i].MaterialIndex = (uint32_t)(UniqueMaterials.size());

            // Add
            UniqueMaterials.insert(std::make_pair(LodReference.Materials[i].MaterialName, LodReference.Submeshes[i].MaterialIndex));
        }
    }

    // Check if we have a generic model, or a streamed one
    if (Model->IsModelStreamed)
    {
        // We have a streamed model, this is handled on a per-game basis, some information is already in the structures
        switch (CurrentGame)
        {
        case SupportedGames::BlackOps3:            GameBlackOps3::LoadXModel(LodReference, ModelResult); break;
        case SupportedGames::BlackOps4:            GameBlackOps4::LoadXModel(LodReference, ModelResult); break;
        case SupportedGames::WorldWar2:            GameWorldWar2::LoadXModel(LodReference, ModelResult); break;
        case SupportedGames::ModernWarfare4:    GameModernWarfare4::LoadXModel(LodReference, ModelResult); break;
        }
    }
    else
    {
        // We have a true generic model, no streamed, process data normally
        ModelResult->PrepareSubmeshes((uint32_t)LodReference.Submeshes.size());

        // Iterate over submeshes
        for (auto& Submesh : LodReference.Submeshes)
        {
            // Create and grab a new submesh
            auto& Mesh = ModelResult->AddSubmesh();

            // Set the material (COD has 1 per submesh)
            Mesh.AddMaterial(Submesh.MaterialIndex);

            // Prepare the mesh for the data
            Mesh.PrepareMesh(Submesh.VertexCount, Submesh.FaceCount);

            // Pre-allocate vertex weights (Data defaults to weight 1.0 on bone 0)
            auto VertexWeights = std::vector<WeightsData>(Submesh.VertexCount);

            // Setup weights, game specific code
            switch (CurrentGame)
            {
            case SupportedGames::WorldAtWar:
            case SupportedGames::BlackOps:
            case SupportedGames::BlackOps2:
            case SupportedGames::ModernWarfare:
            case SupportedGames::ModernWarfare2:
            case SupportedGames::ModernWarfare3:
            case SupportedGames::QuantumSolace:
                PrepareVertexWeightsA(VertexWeights, Submesh);
                break;
            case SupportedGames::Ghosts:
            case SupportedGames::AdvancedWarfare:
            case SupportedGames::ModernWarfareRemastered:
                PrepareVertexWeightsB(VertexWeights, Submesh);
                break;
            case SupportedGames::InfiniteWarfare:
                PrepareVertexWeightsC(VertexWeights, Submesh);
                break;
            }

            if (CurrentGame != SupportedGames::QuantumSolace)
            {
                // Mesh buffers size
                uintptr_t ReadDataSize = 0;
                // Calculated buffer size
                auto VerticiesLength = (sizeof(GfxVertexBuffer) * Submesh.VertexCount);
                auto FacesLength = (sizeof(GfxFaceBuffer) * Submesh.FaceCount);
                // Read mesh data
                auto VertexData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.VertexPtr, VerticiesLength, ReadDataSize), ReadDataSize);
                auto FaceData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.FacesPtr, FacesLength, ReadDataSize), ReadDataSize);

                // Iterate over verticies
                for (uint32_t i = 0; i < Submesh.VertexCount; i++)
                {
                    // Make a new vertex
                    auto& Vertex = Mesh.AddVertex();

                    // Read data
                    auto VertexInfo = VertexData.Read<GfxVertexBuffer>();
                    // Assign data
                    Vertex.Position = VertexInfo.Position;

                    // Unpack normal (Game specific)
                    switch (CurrentGame)
                    {
                    case SupportedGames::WorldAtWar:
                    case SupportedGames::BlackOps:
                    case SupportedGames::ModernWarfare:
                    case SupportedGames::ModernWarfare2:
                    case SupportedGames::ModernWarfare3:
                        // Apply UV layer (These games seems to have UVV before UVU) this works on all models
                        Vertex.AddUVLayer(HalfFloats::ToFloat(VertexInfo.UVVPos), HalfFloats::ToFloat(VertexInfo.UVUPos));
                        // Unpack vertex normal
                        Vertex.Normal = UnpackNormalA(VertexInfo.Normal);
                        break;
                    case SupportedGames::BlackOps2:
                        // Apply UV layer
                        Vertex.AddUVLayer(HalfFloats::ToFloat(VertexInfo.UVUPos), HalfFloats::ToFloat(VertexInfo.UVVPos));
                        // Unpack vertex normal
                        Vertex.Normal = UnpackNormalB(VertexInfo.Normal);
                        break;
                    case SupportedGames::Ghosts:
                    case SupportedGames::AdvancedWarfare:
                    case SupportedGames::ModernWarfareRemastered:
                    case SupportedGames::InfiniteWarfare:
                        // Apply UV layer
                        Vertex.AddUVLayer(HalfFloats::ToFloat(VertexInfo.UVUPos), HalfFloats::ToFloat(VertexInfo.UVVPos));
                        // Unpack vertex normal
                        Vertex.Normal = UnpackNormalC(VertexInfo.Normal);
                        break;
                    }

                    // Add Colors if we want them
                    if (ExportColors)
                    {
                        Vertex.Color[0] = VertexInfo.Color[0];
                        Vertex.Color[1] = VertexInfo.Color[1];
                        Vertex.Color[2] = VertexInfo.Color[2];
                        Vertex.Color[3] = VertexInfo.Color[3];
                    }
                    else
                    {
                        Vertex.Color[0] = 0xFF;
                        Vertex.Color[1] = 0xFF;
                        Vertex.Color[2] = 0xFF;
                        Vertex.Color[3] = 0xFF;
                    }

                    // Assign weights
                    auto& WeightValue = VertexWeights[i];

                    // Iterate
                    for (uint32_t w = 0; w < WeightValue.WeightCount; w++)
                    {
                        // Add new weight
                        Vertex.AddVertexWeight(WeightValue.BoneValues[w], WeightValue.WeightValues[w]);
                    }
                }

                // Iterate over faces
                for (uint32_t i = 0; i < Submesh.FaceCount; i++)
                {
                    // Read data
                    auto FaceIndicies = FaceData.Read<GfxFaceBuffer>();
                    // Add the face
                    Mesh.AddFace(FaceIndicies.Index1, FaceIndicies.Index2, FaceIndicies.Index3);
                }
            }
            else
            {
                // Mesh buffers size
                uintptr_t ReadDataSize = 0;
                // Calculated buffer size
                auto VerticiesLength = (sizeof(QSGfxVertexBuffer) * Submesh.VertexCount);
                auto FacesLength = (sizeof(GfxFaceBuffer) * Submesh.FaceCount);
                // Read mesh data
                auto VertexData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.VertexPtr, VerticiesLength, ReadDataSize), ReadDataSize);
                auto FaceData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.FacesPtr, FacesLength, ReadDataSize), ReadDataSize);

                // Iterate over verticies
                for (uint32_t i = 0; i < Submesh.VertexCount; i++)
                {
                    // Make a new vertex
                    auto& Vertex = Mesh.AddVertex();

                    // Read data
                    auto VertexInfo = VertexData.Read<QSGfxVertexBuffer>();
                    // Assign data
                    Vertex.Position = VertexInfo.Position;
                    Vertex.Normal = UnpackNormalA(VertexInfo.Tangent);
                    Vertex.AddUVLayer(VertexInfo.UV.X, VertexInfo.UV.Y);

                    // Add Colors if we want them
                    {
                        Vertex.Color[0] = 0xFF;
                        Vertex.Color[1] = 0xFF;
                        Vertex.Color[2] = 0xFF;
                        Vertex.Color[3] = 0xFF;
                    }

                    // Assign weights
                    auto& WeightValue = VertexWeights[i];

                    // Iterate
                    for (uint32_t w = 0; w < WeightValue.WeightCount; w++)
                    {
                        // Add new weight
                        Vertex.AddVertexWeight(WeightValue.BoneValues[w], WeightValue.WeightValues[w]);
                    }
                }

                // Iterate over faces
                for (uint32_t i = 0; i < Submesh.FaceCount; i++)
                {
                    // Read data
                    auto FaceIndicies = FaceData.Read<GfxFaceBuffer>();
                    // Add the face
                    Mesh.AddFace(FaceIndicies.Index1, FaceIndicies.Index2, FaceIndicies.Index3);
                }
            }
        }
    }

    // Return it
    return ModelResult;
}

int32_t CoDXModelTranslator::CalculateBiggestLodIndex(const std::unique_ptr<XModel_t>& Model)
{
    // Fetch lod count
    auto LodCount = (int32_t)Model->ModelLods.size();

    // This function is only valid if we have > 1 lod
    if (LodCount > 1)
    {
        // Perform calculation based on lod properties
        int32_t ResultIndex = 0;

        // Loop over total lods
        for (int32_t i = 0; i < LodCount; i++)
        {
            // Compare surface count and distance
            if ((Model->ModelLods[i].LodDistance < Model->ModelLods[ResultIndex].LodDistance) && (Model->ModelLods[i].Submeshes.size() >= Model->ModelLods[ResultIndex].Submeshes.size()))
            {
                // Set it as the newest result
                ResultIndex = i;
            }
        }
        
        // Return the biggest index
        return ResultIndex;
    }
    else if (LodCount == 0)
    {
        // There are no lods to export
        return -1;
    }

    // Default to the first lod
    return 0;
}

Vector3 CoDXModelTranslator::UnpackNormalA(uint32_t Normal)
{
    // Unpack a normal, used in [WAW, BO, MW, MW2, MW3]

    // Convert to packed structure
    auto PackedNormal = (GfxPackedUnitVec*)&Normal;

    // Decode the scale of the vector
    float DecodeScale = (float)((float)(uint8_t)PackedNormal->PackedBytes[3] - -192.0) / 32385.0f;

    // Make the vector
    return Vector3(
        (float)((float)(uint8_t)PackedNormal->PackedBytes[0] - 127.0) * DecodeScale,
        (float)((float)(uint8_t)PackedNormal->PackedBytes[1] - 127.0) * DecodeScale,
        (float)((float)(uint8_t)PackedNormal->PackedBytes[2] - 127.0) * DecodeScale);
}

Vector3 CoDXModelTranslator::UnpackNormalB(uint32_t Normal)
{
    // Unpack a normal, used in [BO2]

    // Convert to packed structure
    auto PackedNormal = (GfxPackedUnitVec*)&Normal;

    // Rebuild the float (As an integer value)
    uint32_t RBuiltX = ((PackedNormal->PackedInteger & 0x3FF) - 2 * (PackedNormal->PackedInteger & 0x200) + 0x40400000);
    uint32_t RBuiltY = (((PackedNormal->PackedInteger >> 10) & 0x3FF) - 2 * ((PackedNormal->PackedInteger >> 10) & 0x200) + 0x40400000);
    uint32_t RBuiltZ = (((PackedNormal->PackedInteger >> 20) & 0x3FF) - 2 * ((PackedNormal->PackedInteger >> 20) & 0x200) + 0x40400000);

    // Make the vector
    return Vector3(
        (float)(*reinterpret_cast<float*>(&RBuiltX) - 3.0) * 8208.0312f,
        (float)(*reinterpret_cast<float*>(&RBuiltY) - 3.0) * 8208.0312f,
        (float)(*reinterpret_cast<float*>(&RBuiltZ) - 3.0) * 8208.0312f);
}

Vector3 CoDXModelTranslator::UnpackNormalC(uint32_t Normal)
{
    // Unpack a normal, used in [Ghosts, AW, MWR, IW]

    // Convert to packed structure
    auto PackedNormal = (GfxPackedUnitVec*)&Normal;

    // Make the vector
    return Vector3(
        (float)((float)((float)(PackedNormal->PackedInteger & 0x3FF) / 1023.0) * 2.0) - 1.0f,
        (float)((float)((float)((PackedNormal->PackedInteger >> 10) & 0x3FF) / 1023.0) * 2.0) - 1.0f,
        (float)((float)((float)((PackedNormal->PackedInteger >> 20) & 0x3FF) / 1023.0) * 2.0) - 1.0f);
}

void CoDXModelTranslator::PrepareVertexWeightsA(std::vector<WeightsData>& Weights, const XModelSubmesh_t& Submesh)
{
    // Prepare weights, used in [WAW, BO, BO2, MW, MW2, MW3]

    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    // Prepare the simple, rigid weights
    for (uint32_t i = 0; i < Submesh.VertListcount; i++)
    {
        // Simple weights build, rigid, just apply the proper bone id
        uint32_t VertexCount = 0;
        uint32_t BoneIndex = 0;

        // Read rigid struct, QS does not have the pointer
        if (CoDAssets::GameID == SupportedGames::QuantumSolace)
        {
            auto RigidInfo = CoDAssets::GameInstance->Read<GfxRigidVertsQS>(Submesh.RigidWeightsPtr + (i * sizeof(GfxRigidVertsQS)));
            VertexCount = RigidInfo.VertexCount;
            BoneIndex = RigidInfo.BoneIndex / 64;
        }
        else
        {
            auto RigidInfo = CoDAssets::GameInstance->Read<GfxRigidVerts>(Submesh.RigidWeightsPtr + (i * sizeof(GfxRigidVerts)));
            VertexCount = RigidInfo.VertexCount;
            BoneIndex = RigidInfo.BoneIndex / 64;
        }

        // Apply bone ids properly
        for (uint32_t w = 0; w < VertexCount; w++)
        {
            // Apply
            Weights[WeightDataIndex].BoneValues[0] = BoneIndex;
            // Advance
            WeightDataIndex++;
        }
    }

    // Total weight data read
    uintptr_t ReadDataSize = 0;
    // Calculate the size of weights buffer
    auto WeightsDataLength = ((2 * Submesh.WeightCounts[0]) + (6 * Submesh.WeightCounts[1]) + (10 * Submesh.WeightCounts[2]) + (14 * Submesh.WeightCounts[3]));
    // Read the weight data
    auto WeightsData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.WeightsPtr, WeightsDataLength, ReadDataSize), ReadDataSize);

    // Prepare single bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[0]; w++)
    {
        // Apply
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        // Advance
        WeightDataIndex++;
    }

    // Prepare two bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[1]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 2;

        // Read IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2 and calculate 1
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - Weights[WeightDataIndex].WeightValues[1]);

        // Advance
        WeightDataIndex++;
    }

    // Prepare three bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[2]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 3;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2]));

        // Advance
        WeightDataIndex++;
    }

    // Prepare four bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[3]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 4;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3]));

        // Advance
        WeightDataIndex++;
    }
}

void CoDXModelTranslator::PrepareVertexWeightsB(std::vector<WeightsData>& Weights, const XModelSubmesh_t& Submesh)
{
    // Prepare weights, used in [Ghosts, AW, MWR]

    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    // Prepare the simple, rigid weights
    for (uint32_t i = 0; i < Submesh.VertListcount; i++)
    {
        // Simple weights build, rigid, just apply the proper bone id
        auto RigidInfo = CoDAssets::GameInstance->Read<GfxRigidVerts64>(Submesh.RigidWeightsPtr + (i * sizeof(GfxRigidVerts64)));
        // Apply bone ids properly
        for (uint32_t w = 0; w < RigidInfo.VertexCount; w++)
        {
            // Apply
            Weights[WeightDataIndex].BoneValues[0] = (RigidInfo.BoneIndex / 64);
            // Advance
            WeightDataIndex++;
        }
    }

    // Total weight data read
    uintptr_t ReadDataSize = 0;
    // Calculate the size of weights buffer
    auto WeightsDataLength = ((2 * Submesh.WeightCounts[0]) + (6 * Submesh.WeightCounts[1]) + (10 * Submesh.WeightCounts[2]) + (14 * Submesh.WeightCounts[3]) + (18 * Submesh.WeightCounts[4]) + (22 * Submesh.WeightCounts[5]) + (26 * Submesh.WeightCounts[6]) + (30 * Submesh.WeightCounts[7]));
    // Read the weight data
    auto WeightsData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.WeightsPtr, WeightsDataLength, ReadDataSize), ReadDataSize);

    // Prepare single bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[0]; w++)
    {
        // Apply
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        // Advance
        WeightDataIndex++;
    }

    // Prepare two bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[1]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 2;

        // Read IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2 and calculate 1
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - Weights[WeightDataIndex].WeightValues[1]);

        // Advance
        WeightDataIndex++;
    }

    // Prepare three bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[2]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 3;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2]));

        // Advance
        WeightDataIndex++;
    }

    // Prepare four bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[3]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 4;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3]));

        // Advance
        WeightDataIndex++;
    }

    // Prepare five bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[4]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 5;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4]));

        // Advance
        WeightDataIndex++;
    }

    // Prepare six bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[5]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 6;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (6)
        Weights[WeightDataIndex].BoneValues[5] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 6
        Weights[WeightDataIndex].WeightValues[5] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4] + Weights[WeightDataIndex].WeightValues[5]));

        // Advance
        WeightDataIndex++;
    }

    // Prepare seven bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[6]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 7;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (6)
        Weights[WeightDataIndex].BoneValues[5] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 6
        Weights[WeightDataIndex].WeightValues[5] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (7)
        Weights[WeightDataIndex].BoneValues[6] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 7
        Weights[WeightDataIndex].WeightValues[6] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4] + Weights[WeightDataIndex].WeightValues[5] + Weights[WeightDataIndex].WeightValues[6]));

        // Advance
        WeightDataIndex++;
    }

    // Prepare eight bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[7]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 8;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = (WeightsData.Read<uint16_t>() / 64);
        Weights[WeightDataIndex].BoneValues[1] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (6)
        Weights[WeightDataIndex].BoneValues[5] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 6
        Weights[WeightDataIndex].WeightValues[5] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (7)
        Weights[WeightDataIndex].BoneValues[6] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 7
        Weights[WeightDataIndex].WeightValues[6] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (8)
        Weights[WeightDataIndex].BoneValues[7] = (WeightsData.Read<uint16_t>() / 64);
        // Read value for 8
        Weights[WeightDataIndex].WeightValues[7] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4] + Weights[WeightDataIndex].WeightValues[5] + Weights[WeightDataIndex].WeightValues[6] + Weights[WeightDataIndex].WeightValues[7]));

        // Advance
        WeightDataIndex++;
    }
}

void CoDXModelTranslator::PrepareVertexWeightsC(std::vector<WeightsData>& Weights, const XModelSubmesh_t& Submesh)
{
    // Prepare weights, used in [IW]

    // The index of read weight data
    uint32_t WeightDataIndex = 0;

    // Prepare the simple, rigid weights
    for (uint32_t i = 0; i < Submesh.VertListcount; i++)
    {
        // Simple weights build, rigid, just apply the proper bone id
        auto RigidInfo = CoDAssets::GameInstance->Read<GfxRigidVerts64>(Submesh.RigidWeightsPtr + (i * sizeof(GfxRigidVerts64)));
        // Apply bone ids properly
        for (uint32_t w = 0; w < RigidInfo.VertexCount; w++)
        {
            // Apply
            Weights[WeightDataIndex].BoneValues[0] = RigidInfo.BoneIndex;
            // Advance
            WeightDataIndex++;
        }
    }

    // Total weight data read
    uintptr_t ReadDataSize = 0;
    // Calculate the size of weights buffer
    auto WeightsDataLength = ((4 * Submesh.WeightCounts[0]) + (8 * Submesh.WeightCounts[1]) + (12 * Submesh.WeightCounts[2]) + (16 * Submesh.WeightCounts[3]) + (20 * Submesh.WeightCounts[4]) + (24 * Submesh.WeightCounts[5]) + (28 * Submesh.WeightCounts[6]) + (32 * Submesh.WeightCounts[7]));
    // Read the weight data
    auto WeightsData = MemoryReader(CoDAssets::GameInstance->Read(Submesh.WeightsPtr, WeightsDataLength, ReadDataSize), ReadDataSize);

    // Prepare single bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[0]; w++)
    {
        // Apply
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare two bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[1]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 2;

        // Read IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2 and calculate 1
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - Weights[WeightDataIndex].WeightValues[1]);

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare three bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[2]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 3;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = WeightsData.Read<uint16_t>();
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2]));

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare four bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[3]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 4;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = WeightsData.Read<uint16_t>();
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = WeightsData.Read<uint16_t>();
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3]));

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare five bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[4]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 5;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = WeightsData.Read<uint16_t>();
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = WeightsData.Read<uint16_t>();
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = WeightsData.Read<uint16_t>();
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4]));

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare six bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[5]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 6;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = WeightsData.Read<uint16_t>();
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = WeightsData.Read<uint16_t>();
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = WeightsData.Read<uint16_t>();
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (6)
        Weights[WeightDataIndex].BoneValues[5] = WeightsData.Read<uint16_t>();
        // Read value for 6
        Weights[WeightDataIndex].WeightValues[5] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4] + Weights[WeightDataIndex].WeightValues[5]));

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare seven bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[6]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 7;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = WeightsData.Read<uint16_t>();
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = WeightsData.Read<uint16_t>();
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = WeightsData.Read<uint16_t>();
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (6)
        Weights[WeightDataIndex].BoneValues[5] = WeightsData.Read<uint16_t>();
        // Read value for 6
        Weights[WeightDataIndex].WeightValues[5] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (7)
        Weights[WeightDataIndex].BoneValues[6] = WeightsData.Read<uint16_t>();
        // Read value for 7
        Weights[WeightDataIndex].WeightValues[6] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4] + Weights[WeightDataIndex].WeightValues[5] + Weights[WeightDataIndex].WeightValues[6]));

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }

    // Prepare eight bone weights
    for (uint32_t w = 0; w < Submesh.WeightCounts[7]; w++)
    {
        // Set size
        Weights[WeightDataIndex].WeightCount = 8;

        // Read 2 IDs (1, 2)
        Weights[WeightDataIndex].BoneValues[0] = WeightsData.Read<uint16_t>();
        Weights[WeightDataIndex].BoneValues[1] = WeightsData.Read<uint16_t>();
        // Read value for 2
        Weights[WeightDataIndex].WeightValues[1] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (3)
        Weights[WeightDataIndex].BoneValues[2] = WeightsData.Read<uint16_t>();
        // Read value for 3
        Weights[WeightDataIndex].WeightValues[2] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (4)
        Weights[WeightDataIndex].BoneValues[3] = WeightsData.Read<uint16_t>();
        // Read value for 4
        Weights[WeightDataIndex].WeightValues[3] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (5)
        Weights[WeightDataIndex].BoneValues[4] = WeightsData.Read<uint16_t>();
        // Read value for 5
        Weights[WeightDataIndex].WeightValues[4] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (6)
        Weights[WeightDataIndex].BoneValues[5] = WeightsData.Read<uint16_t>();
        // Read value for 6
        Weights[WeightDataIndex].WeightValues[5] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (7)
        Weights[WeightDataIndex].BoneValues[6] = WeightsData.Read<uint16_t>();
        // Read value for 7
        Weights[WeightDataIndex].WeightValues[6] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Read 1 ID (8)
        Weights[WeightDataIndex].BoneValues[7] = WeightsData.Read<uint16_t>();
        // Read value for 8
        Weights[WeightDataIndex].WeightValues[7] = ((float)WeightsData.Read<uint16_t>() / 65536.0f);
        // Calculate first value
        Weights[WeightDataIndex].WeightValues[0] = (1.0f - (Weights[WeightDataIndex].WeightValues[1] + Weights[WeightDataIndex].WeightValues[2] + Weights[WeightDataIndex].WeightValues[3] + Weights[WeightDataIndex].WeightValues[4] + Weights[WeightDataIndex].WeightValues[5] + Weights[WeightDataIndex].WeightValues[6] + Weights[WeightDataIndex].WeightValues[7]));

        // Advance
        WeightDataIndex++;

        // Skip 2 padding
        WeightsData.Advance(2);
    }
}

std::unique_ptr<WraithModel> CoDXModelTranslator::TranslateXModelHitbox(const std::unique_ptr<XModel_t>& Model)
{
    // Ensure we have a hitbox loaded
    if (Model->BoneInfoPtr == 0) { return nullptr; }

    // Prepare to translate the model hitbox
    auto Result = TranslateXModel(Model, 0, true);

    // Ensure we translated it
    if (Result != nullptr)
    {
        // Grab the info ptr
        auto BoneInfoPtr = Model->BoneInfoPtr;

        // Iterate over bones and generate meshes
        for (size_t i = 0; i < (Result->Bones.size()); i++)
        {
            // Read the info
            auto BoneInfo = CoDAssets::GameInstance->Read<GfxBoneInfo>(BoneInfoPtr);

            // Check to skip
            if (VectorMath::EqualsSafe(std::begin(BoneInfo.Bounds[0]), std::end(BoneInfo.Bounds[0]), std::begin(BoneInfo.Bounds[1]), std::end(BoneInfo.Bounds[1])) == false)
            {
                // Add a new mesh
                auto& NewMesh = Result->AddSubmesh();
                // Prepare for explicit size
                NewMesh.PrepareMesh(8, 12);
                // Add material
                NewMesh.AddMaterial(-1);

                // Setup bounds
                std::vector<Vector3> VertPos =
                {
                    { BoneInfo.Bounds[0][0], BoneInfo.Bounds[0][1], BoneInfo.Bounds[0][2] },
                    { BoneInfo.Bounds[0][0], BoneInfo.Bounds[0][1], BoneInfo.Bounds[1][2] },
                    { BoneInfo.Bounds[0][0], BoneInfo.Bounds[1][1], BoneInfo.Bounds[0][2] },
                    { BoneInfo.Bounds[0][0], BoneInfo.Bounds[1][1], BoneInfo.Bounds[1][2] },
                    { BoneInfo.Bounds[1][0], BoneInfo.Bounds[0][1], BoneInfo.Bounds[0][2] },
                    { BoneInfo.Bounds[1][0], BoneInfo.Bounds[0][1], BoneInfo.Bounds[1][2] },
                    { BoneInfo.Bounds[1][0], BoneInfo.Bounds[1][1], BoneInfo.Bounds[0][2] },
                    { BoneInfo.Bounds[1][0], BoneInfo.Bounds[1][1], BoneInfo.Bounds[1][2] }
                };

                // Fetch the bone
                auto& Bone = Result->Bones[i];

                // Fetch the matrix
                auto Matrix = Matrix::CreateFromQuaternion(Bone.GlobalRotation);
                // Set translation
                Matrix.Mat(3, 0) = Bone.GlobalPosition.X;
                Matrix.Mat(3, 1) = Bone.GlobalPosition.Y;
                Matrix.Mat(3, 2) = Bone.GlobalPosition.Z;

                // Setup verticies
                for (auto& Vertex : VertPos)
                {
                    // Add
                    auto& Vert = NewMesh.AddVertex();
                    // Generate position
                    Vert.Position = Matrix::TransformVector(Vertex, Matrix);
                    Vert.AddUVLayer(0.5, 0.5);
                    Vert.Normal = Vector3(0.2f, 0.2f, 0.2f);
                    Vert.AddVertexWeight((uint32_t)i, 1.0f);
                }

                // Add faces
                NewMesh.AddFace(0, 2, 1);
                NewMesh.AddFace(1, 2, 3);
                NewMesh.AddFace(4, 5, 6);
                NewMesh.AddFace(6, 5, 7);
                NewMesh.AddFace(0, 1, 4);
                NewMesh.AddFace(4, 1, 5);
                NewMesh.AddFace(2, 6, 3);
                NewMesh.AddFace(3, 6, 7);
                NewMesh.AddFace(2, 0, 6);
                NewMesh.AddFace(6, 0, 4);
                NewMesh.AddFace(5, 1, 7);
                NewMesh.AddFace(7, 1, 3);
            }

            // Advance
            BoneInfoPtr += sizeof(GfxBoneInfo);
        }

        // We're done
        return Result;
    }

    // Failed to export
    return nullptr;
}