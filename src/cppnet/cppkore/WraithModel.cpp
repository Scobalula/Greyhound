#include "stdafx.h"

// The class we are implementing
#include "WraithModel.h"
#include "CodAssetType.h"
#include "Matrix.h"

WraithBone::WraithBone()
{
    // Defaults
    TagName = "";
    BoneParent = -1;
    LocalPosition = Vector3(0, 0, 0);
    LocalRotation = Math::Quaternion::Identity();
    GlobalPosition = Vector3(0, 0, 0);
    GlobalRotation = Math::Quaternion::Identity();
    BoneScale = Vector3(1, 1, 1);
    IsCosmetic = false;
}

WraithBone::~WraithBone()
{
    // Defaults
}

WraithFace::WraithFace()
{
    // Defaults
    Index1 = 0;
    Index2 = 0;
    Index3 = 0;
}

WraithFace::WraithFace(uint32_t Vert1, uint32_t Vert2, uint32_t Vert3)
{
    // Set them
    Index1 = Vert1;
    Index2 = Vert2;
    Index3 = Vert3;
}

WraithFace::~WraithFace()
{
    // Defaults
}

WraithVertexWeight::WraithVertexWeight()
{
    // Defaults
    BoneIndex = 0;
    Weight = 1.0f;
}

WraithVertexWeight::WraithVertexWeight(uint32_t Bone, float Value)
{
    // Set it
    BoneIndex = Bone;
    Weight = Value;
}

WraithVertexWeight::~WraithVertexWeight()
{
    // Defaults
}

WraithVertex::WraithVertex()
{
    // Defaults
    Position = Vector3(0, 0, 0);
    Normal = Vector3(0, 0, 0);
}

WraithVertex::~WraithVertex()
{
    // Clean up if need be
}

uint32_t WraithVertex::WeightCount() const
{
    // Return it
    return (uint32_t)Weights.size();
}

uint32_t WraithVertex::UVLayerCount() const
{
    // Return it
    return (uint32_t)UVLayers.size();
}

WraithSubmesh::WraithSubmesh()
{
    // Defaults

}

WraithSubmesh::~WraithSubmesh()
{
    // Clean up if we need to
}

uint32_t WraithSubmesh::VertexCount() const
{
    // Return it
    return (uint32_t)Verticies.size();
}

uint32_t WraithSubmesh::FacesCount() const
{
    // Return it
    return (uint32_t)Faces.size();
}

uint32_t WraithSubmesh::MaterialCount() const
{
    // Return it
    return (uint32_t)MaterialIndicies.size();
}

WraithVertex& WraithSubmesh::AddVertex()
{
    // Allocate it
    Verticies.emplace_back();
    // Return it
    return Verticies.back();
}

void WraithSubmesh::AddVertex(WraithVertex& Vertex)
{
    // Move it to the list
    Verticies.push_back(std::move(Vertex));
}

void WraithSubmesh::AddFace(uint32_t Index1, uint32_t Index2, uint32_t Index3)
{
    // Add the face
    Faces.emplace_back(Index1, Index2, Index3);
}

WraithFace& WraithSubmesh::AddFace()
{
    // Add a new one
    Faces.emplace_back();
    // Return
    return Faces.back();
}

void WraithSubmesh::AddMaterial(int32_t Index)
{
    // Add it
    MaterialIndicies.push_back(Index);
}

void WraithSubmesh::RemoveVertex(uint32_t Index)
{
    // Remove a vertex
    Verticies.erase(Verticies.begin() + Index);
}

void WraithSubmesh::RemoveFace(uint32_t Index)
{
    // Remove a face
    Faces.erase(Faces.begin() + Index);
}

void WraithSubmesh::RemoveMaterial(uint32_t Index)
{
    // Remove a material
    MaterialIndicies.erase(MaterialIndicies.begin() + Index);
}

void WraithSubmesh::PrepareMesh(uint32_t VertexCount, uint32_t FaceCount)
{
    // Resize
    Verticies.reserve(VertexCount);
    // Resize
    Faces.reserve(FaceCount);
}

void WraithVertex::AddVertexWeight(uint32_t BoneIndex, float Weight)
{
    // Add the weight
    Weights.emplace_back(BoneIndex, Weight);
}

void WraithVertex::AddUVLayer(float UVU, float UVV)
{
    // Make it
    UVLayers.emplace_back(UVU, UVV);
}

void WraithVertex::RemoveVertexWeight(uint32_t Index)
{
    // Remove
    Weights.erase(Weights.begin() + Index);
}

void WraithVertex::RemoveUVLayer(uint32_t Index)
{
    // Remove
    UVLayers.erase(UVLayers.begin() + Index);
}

void WraithSubmesh::ScaleSubmesh(float ScaleFactor)
{
    // Scale the mesh
    for (auto& Vertex : Verticies)
    {
        // Scale it's position
        Vertex.Position *= ScaleFactor;
    }
}

WraithMaterial::WraithMaterial()
{
    // Defaults
    MaterialName = "wraith_material";
    DiffuseMapName = "";
    NormalMapName = "";
    SpecularMapName = "";
}

WraithMaterial::WraithMaterial(const std::string Name, const std::string Diffuse, const std::string Normal, const std::string Specular)
{
    // Set
    MaterialName = Name;
    DiffuseMapName = Diffuse;
    NormalMapName = Normal;
    SpecularMapName = Specular;
}

WraithMaterial::~WraithMaterial()
{
    // Defaults
}

// Declare the default material, based on no material existing
const WraithMaterial WraithMaterial::DefaultMaterial = WraithMaterial("wraith_no_material", "", "", "");



WraithModel::WraithModel()
{
    // Defaults
    AssetType = WraithAssetType::Model;

    // Default distances
    LodDistance = 100.0f;
    LodMaxDistance = 200.0f;
}

WraithModel::~WraithModel()
{
    // Clean up if we need to
}

uint32_t WraithModel::BoneCount() const
{
    // Return the count
    return (uint32_t)Bones.size();
}

uint32_t WraithModel::VertexCount() const
{
    // Buffer
    uint32_t Buffer = 0;

    // Iterate and append
    for (auto& Submesh : Submeshes)
    {
        // Add
        Buffer += (uint32_t)Submesh.Verticies.size();
    }

    // Return it
    return Buffer;
}

uint32_t WraithModel::FaceCount() const
{
    // Buffer
    uint32_t Buffer = 0;

    // Iterate and append
    for (auto& Submesh : Submeshes)
    {
        // Add
        Buffer += (uint32_t)Submesh.Faces.size();
    }

    // Return it
    return Buffer;
}

uint32_t WraithModel::SubmeshCount() const
{
    // Return it
    return (uint32_t)Submeshes.size();
}

uint32_t WraithModel::MaterialCount() const
{
    // Return it
    return (uint32_t)Materials.size();
}

WraithMaterial& WraithModel::AddMaterial()
{
    // Add a new one
    Materials.emplace_back();
    // Return it
    return Materials.back();
}

void WraithModel::AddMaterial(WraithMaterial& Material)
{
    // Move it
    Materials.push_back(std::move(Material));
}

WraithBone& WraithModel::AddBone()
{
    // Make a new one
    Bones.emplace_back();
    // Return it
    return Bones.back();
}

void WraithModel::AddBone(WraithBone& Bone)
{
    // Move it
    Bones.push_back(std::move(Bone));
}

WraithSubmesh& WraithModel::AddSubmesh()
{
    // Make a new one
    Submeshes.emplace_back();
    // Return it
    return Submeshes.back();
}

void WraithModel::AddSubmesh(WraithSubmesh& Submesh)
{
    // Move it
    Submeshes.push_back(std::move(Submesh));
}

void WraithModel::RemoveBone(uint32_t Index)
{
    // Remove it
    Bones.erase(Bones.begin() + Index);
}

void WraithModel::RemoveSubmesh(uint32_t Index)
{
    // Remove it
    Submeshes.erase(Submeshes.begin() + Index);
}

void WraithModel::RemoveMaterial(uint32_t Index)
{
    // Remove it
    Materials.erase(Materials.begin() + Index);
}

void WraithModel::ScaleModel(float ScaleFactor)
{
    // Iterate over bones and submeshes

    // Bones
    for (auto& Bone : Bones)
    {
        // Scale it
        Bone.LocalPosition *= ScaleFactor;
        Bone.GlobalPosition *= ScaleFactor;
    }

    // Submeshes
    for (auto& Submesh : Submeshes)
    {
        // Scale it
        Submesh.ScaleSubmesh(ScaleFactor);
    }
}

void WraithModel::PrepareSubmeshes(uint32_t SubmeshCount)
{
    // Prepare it
    Submeshes.reserve(SubmeshCount);
}

void WraithModel::PrepareBones(uint32_t BoneCount)
{
    // Prepare it
    Bones.reserve(BoneCount);
}

void WraithModel::GenerateLocalPositions(bool Translations, bool Rotations)
{
    // Generate local positions from globals
    auto BoneCount = (uint32_t)Bones.size();

    // Loop through
    for (uint32_t i = 0; i < BoneCount; i++)
    {
        // Make sure we aren't at origin
        if (Bones[i].BoneParent != -1)
        {
            // Local positions can be generated by the following algo
            if (Translations)
            {
                // Parent global matrix
                Math::Matrix ParentGlobalMatrix = Math::Matrix::CreateFromQuaternion(~Bones[Bones[i].BoneParent].GlobalRotation);
                // Set the new local position
                Bones[i].LocalPosition = Math::Matrix::TransformVector(Bones[i].GlobalPosition - Bones[Bones[i].BoneParent].GlobalPosition, ParentGlobalMatrix);
            }

            // Local rotation is inverse parent global * our global
            if (Rotations) { Bones[i].LocalRotation = (~Bones[Bones[i].BoneParent].GlobalRotation * Bones[i].GlobalRotation); }
        }
        else
        {
            // The global position is the local one
            if (Translations) { Bones[i].LocalPosition = Bones[i].GlobalPosition; }
            // The global rotation is the local one
            if (Rotations) { Bones[i].LocalRotation = Bones[i].GlobalRotation; }
        }
    }
}

void WraithModel::GenerateGlobalPositions(bool Translations, bool Rotations)
{
    // Generate global positions from locals
    auto BoneCount = (uint32_t)Bones.size();

    // Loop through
    for (uint32_t i = 0; i < BoneCount; i++)
    {
        // Make sure we aren't at origin
        if (Bones[i].BoneParent != -1)
        {
            // Global positions can be calculated by the following algo
            if (Translations)
            {
                // Generate a localize position quat
                auto LocalPositionQuat = Math::Quaternion(Bones[i].LocalPosition.X, Bones[i].LocalPosition.Y, Bones[i].LocalPosition.Z, 0);
                // Grab the parent global rotation
                auto& ParentGlobalRotation = Bones[Bones[i].BoneParent].GlobalRotation;
                // Multiply the parent rotation b the local position quat
                auto RotationResult = ParentGlobalRotation * LocalPositionQuat;
                // Multiply the result by the conjugate of the parent
                auto InverseResult = RotationResult * ~ParentGlobalRotation;
                // Grab parent global position
                auto& ParentGlobalPosition = Bones[Bones[i].BoneParent].GlobalPosition;

                // Apply the new global position
                Bones[i].GlobalPosition = Vector3((ParentGlobalPosition.X + InverseResult.X), (ParentGlobalPosition.Y + InverseResult.Y), (ParentGlobalPosition.Z + InverseResult.Z));
            }

            // Global rotation is parent global * our local
            if (Rotations) { Bones[i].GlobalRotation = (Bones[Bones[i].BoneParent].GlobalRotation * Bones[i].LocalRotation); }
        }
        else
        {
            // The local position is the global one
            if (Translations) { Bones[i].GlobalPosition = Bones[i].LocalPosition; }
            // The local rotation is the global one
            if (Rotations) { Bones[i].GlobalRotation = Bones[i].LocalRotation; }
        }
    }
}