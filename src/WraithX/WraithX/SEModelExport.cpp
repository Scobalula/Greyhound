#include "stdafx.h"

// The class we are implementing
#include "SEModelExport.h"

// We need the binarywriter class
#include "BinaryWriter.h"

// Specifies the data present for this file container (spec v1.0.0)
enum class SEModelDataPresenceFlags : uint8_t
{
    // Whether or not this model contains a bone block
    SEMODEL_PRESENCE_BONE = 1 << 0,
    // Whether or not this model contains submesh blocks
    SEMODEL_PRESENCE_MESH = 1 << 1,
    // Whether or not this model contains inline material blocks
    SEMODEL_PRESENCE_MATERIALS = 1 << 2,

    // The file contains a custom data block
    SEMODEL_PRESENCE_CUSTOM = 1 << 7,
};

enum class SEModelBonePresenceFlags : uint8_t
{
    // Whether or not bones contain global-space matricies
    SEMODEL_PRESENCE_GLOBAL_MATRIX = 1 << 0,
    // Whether or not bones contain local-space matricies
    SEMODEL_PRESENCE_LOCAL_MATRIX = 1 << 1,

    // Whether or not bones contain scales
    SEMODEL_PRESENCE_SCALES = 1 << 2,
};

enum class SEModelMeshPresenceFlags : uint8_t
{
    // Whether or not meshes contain at least 1 uv map
    SEMODEL_PRESENCE_UVSET = 1 << 0,

    // Whether or not meshes contain vertex normals
    SEMODEL_PRESENCE_NORMALS = 1 << 1,

    // Whether or not meshes contain vertex colors (RGBA)
    SEMODEL_PRESENCE_COLOR = 1 << 2,

    // Whether or not meshes contain at least 1 weighted skin
    SEMODEL_PRESENCE_WEIGHTS = 1 << 3,
};

void SEModel::ExportSEModel(const WraithModel& Model, const std::string& FileName, bool SupportsScale)
{
    // Create a new writer
    auto Writer = BinaryWriter();
    // Open the file
    Writer.Create(FileName);
    // The SEAnim magic
    char Magic[7] = { 'S', 'E', 'M', 'o', 'd', 'e', 'l' };
    // Write the magic
    Writer.Write(Magic);
    // Write the version
    Writer.Write<uint16_t>(0x1);
    // Write header size
    Writer.Write<uint16_t>(0x14);

    // Build data present flags
    {
        // Buffer
        uint8_t DataPresentFlags = 0x0;
        // Check if we have bones
        if (!Model.Bones.empty())
        {
            DataPresentFlags |= (uint8_t)SEModelDataPresenceFlags::SEMODEL_PRESENCE_BONE;
        }
        // Check if we have meshes
        if (!Model.Submeshes.empty())
        {
            // If we have meshes, we have materials by default, since wraith embeds the definition blocks
            DataPresentFlags |= (uint8_t)SEModelDataPresenceFlags::SEMODEL_PRESENCE_MESH;
            DataPresentFlags |= (uint8_t)SEModelDataPresenceFlags::SEMODEL_PRESENCE_MATERIALS;
        }
        // Write it
        Writer.Write<uint8_t>(DataPresentFlags);
    }

    // Build bone data present flags
    {
        // Buffer
        uint8_t DataPresentFlags = 0x0;
        // We store local and global matricies
        DataPresentFlags |= (uint8_t)SEModelBonePresenceFlags::SEMODEL_PRESENCE_GLOBAL_MATRIX;
        DataPresentFlags |= (uint8_t)SEModelBonePresenceFlags::SEMODEL_PRESENCE_LOCAL_MATRIX;
        // Support scales
        if (SupportsScale)
            DataPresentFlags |= (uint8_t)SEModelBonePresenceFlags::SEMODEL_PRESENCE_SCALES;
        // Write it
        Writer.Write<uint8_t>(DataPresentFlags);
    }

    // Build mesh data present flags
    {
        // Buffer
        uint8_t DataPresentFlags = 0x0;
        // We store uvs, normals, colors, and weights
        DataPresentFlags |= (uint8_t)SEModelMeshPresenceFlags::SEMODEL_PRESENCE_UVSET;
        DataPresentFlags |= (uint8_t)SEModelMeshPresenceFlags::SEMODEL_PRESENCE_NORMALS;
        DataPresentFlags |= (uint8_t)SEModelMeshPresenceFlags::SEMODEL_PRESENCE_COLOR;
        DataPresentFlags |= (uint8_t)SEModelMeshPresenceFlags::SEMODEL_PRESENCE_WEIGHTS;
        // Write it
        Writer.Write<uint8_t>(DataPresentFlags);
    }

    // We must fetch counts of each buffer, since it's automatically generated
    uint32_t BoneCountBuffer = Model.BoneCount();
    // Mesh count buffer
    uint32_t MeshCountBuffer = Model.SubmeshCount();
    // The material count buffer
    uint32_t MaterialCountBuffer = Model.MaterialCount();

    // Write count of bones
    Writer.Write<uint32_t>(BoneCountBuffer);
    // Write count of submesh
    Writer.Write<uint32_t>(MeshCountBuffer);
    // Write count of materials
    Writer.Write<uint32_t>(MaterialCountBuffer);

    // Write 3 reserved bytes
    uint8_t Reserved[3] = { 0x0, 0x0, 0x0 };
    // Write it
    Writer.Write(Reserved);

    // Write the bone tags
    for (auto& Bone : Model.Bones)
        Writer.WriteNullTerminatedString(Bone.TagName);

    // Write the bone info
    for (auto& Bone : Model.Bones)
    {
        // Write bone flags, 0 for now
        Writer.Write<uint8_t>(0x0);

        // Write bone parent
        Writer.Write<int32_t>(Bone.BoneParent);

        // Write global matricies
        Writer.Write<Vector3>(Bone.GlobalPosition);
        Writer.Write<Quaternion>(Bone.GlobalRotation);
        // Write local matricies
        Writer.Write<Vector3>(Bone.LocalPosition);
        Writer.Write<Quaternion>(Bone.LocalRotation);

        // Write scale, if support
        if (SupportsScale)
            Writer.Write<Vector3>(Bone.BoneScale);
    }

    // Iterate and produce meshes
    for (auto& Mesh : Model.Submeshes)
    {
        // Write mesh flags, 0 for now
        Writer.Write<uint8_t>(0x0);

        // We must fetch counts of each buffer, since it's automatically generated
        uint32_t VertexCountBuffer = Mesh.VertexCount();
        uint32_t FaceCountBuffer = Mesh.FacesCount();
        uint8_t MaterialCountBuffer = (uint8_t)Mesh.MaterialCount();
        uint8_t MaxSkinInfluenceBuffer = 0;

        // Iterate to dynamically calculate max weight influence
        for (auto& Vertex : Mesh.Verticies)
        {
            if (Vertex.WeightCount() > MaxSkinInfluenceBuffer)
                MaxSkinInfluenceBuffer = (uint8_t)Vertex.WeightCount();
        }

        // Write material count
        Writer.Write<uint8_t>(MaterialCountBuffer);
        // Write weights count
        Writer.Write<uint8_t>(MaxSkinInfluenceBuffer);

        // Write vertex count
        Writer.Write<uint32_t>(VertexCountBuffer);
        // Write face count
        Writer.Write<uint32_t>(FaceCountBuffer);

        // Iterate and produce verticies
        for (auto& Vertex : Mesh.Verticies)
            Writer.Write<Vector3>(Vertex.Position);

        // Produce UVLayers
        for (auto& Vertex : Mesh.Verticies)
            for (auto& UVLayer : Vertex.UVLayers)
                Writer.Write<Vector2>(UVLayer);

        // Produce normals
        for (auto& Vertex : Mesh.Verticies)
            Writer.Write<Vector3>(Vertex.Normal);

        // Produce Colors
        for (auto& Vertex : Mesh.Verticies)
        {
            Writer.Write<uint8_t>(Vertex.Color[0]);
            Writer.Write<uint8_t>(Vertex.Color[1]);
            Writer.Write<uint8_t>(Vertex.Color[2]);
            Writer.Write<uint8_t>(Vertex.Color[3]);
        }

        // Produce weights
        for (auto& Vertex : Mesh.Verticies)
        {
            // Write weight values
            for (uint32_t i = 0; i < MaxSkinInfluenceBuffer; i++)
            {
                // Write IDs
                auto WeightID = (i < Vertex.WeightCount()) ? Vertex.Weights[i].BoneIndex : 0;
                auto WeightValue = (i < Vertex.WeightCount()) ? Vertex.Weights[i].Weight : 0.0f;

                // Write ID based on count
                if (BoneCountBuffer <= 0xFF)
                    Writer.Write<uint8_t>((uint8_t)WeightID);
                else if (BoneCountBuffer <= 0xFFFF)
                    Writer.Write<uint16_t>((uint16_t)WeightID);
                else
                    Writer.Write<uint32_t>((uint32_t)WeightID);

                // Write value
                Writer.Write<float>(WeightValue);
            }
        }

        // Iterate and produce faces
        for (auto& Face : Mesh.Faces)
        {
            // Write face indicies based on total vertex count
            if (VertexCountBuffer <= 0xFF)
            {
                // Write as byte
                Writer.Write<uint8_t>((uint8_t)Face.Index1);
                Writer.Write<uint8_t>((uint8_t)Face.Index2);
                Writer.Write<uint8_t>((uint8_t)Face.Index3);
            }
            else if (VertexCountBuffer <= 0xFFFF)
            {
                // Write as short
                Writer.Write<uint16_t>((uint16_t)Face.Index1);
                Writer.Write<uint16_t>((uint16_t)Face.Index2);
                Writer.Write<uint16_t>((uint16_t)Face.Index3);
            }
            else
            {
                // Write as int
                Writer.Write<uint32_t>((uint32_t)Face.Index1);
                Writer.Write<uint32_t>((uint32_t)Face.Index2);
                Writer.Write<uint32_t>((uint32_t)Face.Index3);
            }
        }

        // Output material reference indicies, one per UV layer, -1 indicates no material is assigned...
        for (auto& MaterialIndex : Mesh.MaterialIndicies)
            Writer.Write<int32_t>(MaterialIndex);
    }

    // Iterate and produce materials, used in mesh layers
    for (auto& Material : Model.Materials)
    {
        // Write the name
        Writer.WriteNullTerminatedString(Material.MaterialName);

        // Write isSimpleMaterial flag (True, Wraith only builds simple ones)
        Writer.Write<bool>(true);

        // Write each image reference (3 per each simple material)
        Writer.WriteNullTerminatedString(Material.DiffuseMapName);
        Writer.WriteNullTerminatedString(Material.NormalMapName);
        Writer.WriteNullTerminatedString(Material.SpecularMapName);
    }

    Writer.Write<uint64_t>(0xA646E656C424553);
    Writer.Write<uint64_t>(Model.BlendShapes.size());

    for (auto& BlendShapeName : Model.BlendShapes)
    {
        Writer.WriteNullTerminatedString(BlendShapeName);
    }

    // Iterate and produce meshes
    for (auto& Mesh : Model.Submeshes)
    {
        for (auto& Vertex : Mesh.Verticies)
        {
            Writer.Write<uint32_t>((uint32_t)Vertex.BlendShapeDeltas.size());

            for (auto& Delta : Vertex.BlendShapeDeltas)
            {
                Writer.Write<uint32_t>(Delta.first);
                Writer.Write<Vector3>(Delta.second);
            }
        }
    }
}