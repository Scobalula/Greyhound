#include "stdafx.h"
#include "XMBExport.h"


#include "MemoryWriter.h"
#include "BinaryWriter.h"
#include "Compression.h"
#include "Strings.h"

// 5 Byte *LZ4* Magic
const char XMBMagic[5] = { 0x2A, 0x4C, 0x5A, 0x34, 0x2A };

// Handles calculating byte padding, 
uint64_t ComputePadding(uint64_t Value)
{
    return (Value + 0x3) & 0xFFFFFFFFFFFFFC;
}

// Clamps a float to 16Bit Int between 32767 and -32768
int16_t ClampFloatToShort(float Value)
{
    return VectorMath::Clamp<int16_t>((int16_t)(32767 * VectorMath::Clamp(Value, -1.0f, 1.0f)), -32767, 32767);
}

// Writes a 16bit data block
void WriteMetaInt16Block(MemoryWriter& Writer, int16_t Hash, int16_t Value)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Value
    Writer.Write(Value);
}

// Writes an Unsigned 16bit data block
void WriteMetaUInt16Block(MemoryWriter& Writer, int16_t Hash, uint16_t Value)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Value
    Writer.Write(Value);
}

// Writes a 32bit data block
void WriteMetaInt32Block(MemoryWriter& Writer, int32_t Hash, int32_t Value)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Value
    Writer.Write(Value);
}

// Writes an Unsigned 32bit data block
void WriteMetaUInt32Block(MemoryWriter& Writer, int32_t Hash, uint32_t Value)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Value
    Writer.Write(Value);
}

// Writes a Float data block
void WriteMetaFloatBlock(MemoryWriter& Writer, int32_t Hash, float Value)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Value
    Writer.Write(Value);
}

// Writes a Vector 2 data block
void WriteMetaVec2Block(MemoryWriter& Writer, int32_t Hash, Vector2 Value)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Values
    Writer.Write(Value.X);
    Writer.Write(Value.Y);
}

// Writes a Vector 4 data block
void WriteMetaVec4Block(MemoryWriter& Writer, int32_t Hash, float X, float Y, float Z, float W)
{
    // Write Hash
    Writer.Write(Hash);
    // Write Values
    Writer.Write(X);
    Writer.Write(Y);
    Writer.Write(Z);
    Writer.Write(W);
}

// Writes an aligned string
void WriteStringAligned(MemoryWriter& Writer, const std::string& Value)
{
    // Write String
    Writer.WriteNullTerminatedString(Value);
    // Compute the Padding
    auto PadSize = ComputePadding(Value.size() + 1) - (Value.size() + 1);
    auto Padding = std::make_unique<uint8_t[]>(PadSize);
    // Write Padding
    Writer.Write((uint8_t*)Padding.get(), PadSize);
}

// Writes comment block 
void WriteCommentBlock(MemoryWriter& Writer, const std::string& Comment)
{
    // Write Hash
    Writer.Write(0xC355);
    // Write Comment Aligned
    WriteStringAligned(Writer, Comment);
}

// Writes model token block
void WriteModelBlock(MemoryWriter& Writer)
{
    // Write Hash
    Writer.Write(0x46C8);
}

// Writes model version block
void WriteVersionBlock(MemoryWriter& Writer, int16_t Version)
{
    // Write with Hash
    WriteMetaInt16Block(Writer, (int16_t)0x24D1, Version);
}

// Writes Bone Count block
void WriteBoneCountBlock(MemoryWriter& Writer, int16_t Count)
{
    // Write with Hash
    WriteMetaInt16Block(Writer, (int16_t)0x76BA, Count);
}

// Writes bone info block
void WriteBoneInfoBlock(MemoryWriter& Writer, const WraithBone& Bone, int32_t Index)
{
    // Write Hash
    Writer.Write(0xF099);
    // Write Indices
    Writer.Write(Index);
    Writer.Write(Bone.BoneParent);
    // Write Name Aligned
    WriteStringAligned(Writer, Bone.TagName);
}

// Writes bone index block
void WriteBoneIndexBlock(MemoryWriter& Writer, short Index)
{
    // Write with Hash
    WriteMetaInt16Block(Writer, (int16_t)0xDD9A, Index);
}

// Writes an XYZ Offset Block
void WriteOffsetBlock(MemoryWriter& Writer, Vector3 Offset)
{
    // Write Hash
    Writer.Write(0x9383);
    // Write XYZ
    Writer.Write(Offset.X);
    Writer.Write(Offset.Y);
    Writer.Write(Offset.Z);
}

// Writes an XYZ Scale Block
void WriteScaleBlock(MemoryWriter& Writer, Vector3 Offset)
{
    // Write Hash
    Writer.Write(0x1C56);
    // Write XYZ
    Writer.Write(Offset.X);
    Writer.Write(Offset.Y);
    Writer.Write(Offset.Z);
}

// Writes a Rotation Matrix Block
void WriteMatrixBlock(MemoryWriter& Writer, Matrix RotationMatrix)
{
    // Write Hash
    Writer.Write((int16_t)0xDCFD);
    // Write X
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(0, 0)));
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(0, 1)));
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(0, 2)));
    // Write Hash
    Writer.Write((int16_t)0xCCDC);
    // Write Y
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(1, 0)));
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(1, 1)));
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(1, 2)));
    // Write Hash
    Writer.Write((int16_t)0xFCBF);
    // Write Z
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(2, 0)));
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(2, 1)));
    Writer.Write(ClampFloatToShort(RotationMatrix.Mat(2, 2)));
}

// Writes a 16Bit Vertex Count Block
void WriteVertexCount16Block(MemoryWriter& Writer, uint16_t Count)
{
    // Write with Hash
    WriteMetaUInt16Block(Writer, (int16_t)0x950D, Count);
}

// Writes a 32Bit Vertex Count Block
void WriteVertexCount32Block(MemoryWriter& Writer, uint32_t Count)
{
    // Write with Hash
    WriteMetaUInt32Block(Writer, 0x2AEC, Count);
}

// Writes a 16Bit Vertex Index Block
void WriteVertexIndex16Block(MemoryWriter& Writer, uint16_t Index)
{
    // Write with Hash
    WriteMetaUInt16Block(Writer, (int16_t)0x8F03, Index);
}

// Writes a 32Bit Vertex Index Block
void WriteVertexIndex32Block(MemoryWriter& Writer, uint32_t Index)
{
    // Write with Hash
    WriteMetaUInt32Block(Writer, 0xB097, Index);
}

// Writes Vertex Weight Count Block
void WriteVertexWeightCountBlock(MemoryWriter& Writer, int16_t Count)
{
    // Write with Hash
    WriteMetaInt16Block(Writer, (int16_t)0xEA46, Count);
}

// Writes Vertex Weight Block
void WriteVertexWeightBlock(MemoryWriter& Writer, int16_t BoneIndex, float Weight)
{
    // Write with Hash
    WriteMetaInt16Block(Writer, (int16_t)0xF1AB, BoneIndex);
    // Write Weight
    Writer.Write(Weight);
}

// Writes Face Info Block
void WriteFaceInfoBlock(MemoryWriter& Writer, uint8_t MeshIndex, uint8_t MaterialIndex)
{
    // Write Hash
    Writer.Write((int16_t)0x562F);
    // Write Indices
    Writer.Write(MeshIndex);
    Writer.Write(MaterialIndex);
}

// Writes Face Vertex Normal Block
void WriteFaceVertexNormalBlock(MemoryWriter& Writer, Vector3 Normal)
{
    // Write Hash
    Writer.Write((int16_t)0x89EC);
    // Write Normal
    Writer.Write(ClampFloatToShort(Normal.X));
    Writer.Write(ClampFloatToShort(Normal.Y));
    Writer.Write(ClampFloatToShort(Normal.Z));
}

// Writes Color Block
void WriteFaceVertexColorBlock(MemoryWriter& Writer, uint8_t* Color)
{
    // Write Hash
    Writer.Write(0x6DD8);
    // Write RGBA
    Writer.Write(Color[0]);
    Writer.Write(Color[1]);
    Writer.Write(Color[2]);
    Writer.Write(Color[3]);
}

// Writes Face Vertex UV Block
void WriteFaceVertexUVBlock(MemoryWriter& Writer, uint16_t Layer, Vector2 UV)
{
    // Write with Hash
    Writer.Write((int16_t)0x1AD4);
    // Write Info
    Writer.Write(Layer);
    Writer.Write((float)UV.X);
    Writer.Write((float)UV.Y);
}

// Writes 16Bit Face Vertex
void WriteFaceVertex16(MemoryWriter& Writer, const WraithSubmesh& Mesh, uint32_t VertexIndex, uint32_t Offset)
{
    // Output Data
    WriteVertexIndex16Block(Writer, VertexIndex + Offset);
    WriteFaceVertexNormalBlock(Writer, Mesh.Verticies[VertexIndex].Normal);
    WriteFaceVertexColorBlock(Writer, (uint8_t*)&Mesh.Verticies[VertexIndex].Color);
    WriteFaceVertexUVBlock(Writer, 1, Mesh.Verticies[VertexIndex].UVLayers[0]);
}

// Writes 32Bit Face Vertex
void WriteFaceVertex32(MemoryWriter& Writer, const WraithSubmesh& Mesh, uint32_t VertexIndex, uint32_t Offset)
{
    // Output Data
    WriteVertexIndex32Block(Writer, VertexIndex + Offset);
    WriteFaceVertexNormalBlock(Writer, Mesh.Verticies[VertexIndex].Normal);
    WriteFaceVertexColorBlock(Writer, (uint8_t*)&Mesh.Verticies[VertexIndex].Color);
    WriteFaceVertexUVBlock(Writer, 1, Mesh.Verticies[VertexIndex].UVLayers[0]);
}

// Writes Bones and their positions/rotations
void WriteBones(MemoryWriter& Writer, const WraithModel& Model)
{
    // Write Total Bone Count
    WriteBoneCountBlock(Writer, Model.BoneCount());
    // The bone index
    uint32_t BoneIndex = 0;
    // Iterate over the bones
    for (auto& Bone : Model.Bones)
    {
        // Write Info
        WriteBoneInfoBlock(Writer, Bone, BoneIndex);
        // Advance
        BoneIndex++;
    }
    // The bone index
    BoneIndex = 0;
    // Iterate over the bones
    for (auto& Bone : Model.Bones)
    {
        // Grab rotation matrix
        Matrix RotationMatrix = Matrix::CreateFromQuaternion(Bone.GlobalRotation);
        // Write Info
        WriteBoneIndexBlock(Writer, BoneIndex);
        WriteOffsetBlock(Writer, Bone.GlobalPosition);
        WriteScaleBlock(Writer, Bone.BoneScale);
        WriteMatrixBlock(Writer, RotationMatrix);
        // Advance
        BoneIndex++;
    }
}

// Writes Mesh Data (Vertices + Faces)
void WriteMeshData(MemoryWriter& Writer, const WraithModel& Model)
{
    // Cache vertex count
    auto VertexCount = Model.VertexCount();
    // Cache face count
    auto FaceCount = Model.FaceCount();
    // Output vert count
    if (VertexCount > UINT16_MAX)
    {
        // Use integer 32
        WriteVertexCount32Block(Writer, VertexCount);
    }
    else
    {
        // Use integer 16
        WriteVertexCount16Block(Writer, (uint16_t)VertexCount);
    }
    // A unique list of material indicies
    std::unordered_set<int32_t> MaterialIndicies;
    // The vertex offset
    uint32_t VertexIndex = 0;
    // Iterate over submeshes
    for (auto& Submesh : Model.Submeshes)
    {
        // Add the material indicies for use later
        MaterialIndicies.insert(Submesh.MaterialIndicies[0]);
        // Iterate over verts and do positions and weights
        for (auto& Vertex : Submesh.Verticies)
        {
            // Write vertex
            if (VertexCount > UINT16_MAX)
            {
                WriteVertexIndex32Block(Writer, VertexIndex);
            }
            else
            {
                WriteVertexIndex16Block(Writer, VertexIndex);
            }
            // Write Offset
            WriteOffsetBlock(Writer, Vertex.Position);
            // Handle bone weights
            if (Vertex.WeightCount() == 1)
            {
                // Write Bone Count
                WriteVertexWeightCountBlock(Writer, 1);
                // Write Single Bone
                WriteVertexWeightBlock(Writer, Vertex.Weights[0].BoneIndex, 1.0);
            }
            else
            {
                // Write Bone Count
                WriteVertexWeightCountBlock(Writer, (int16_t)Vertex.WeightCount());
                // Loop
                for (auto& Weight : Vertex.Weights)
                {
                    // Output Weight
                    WriteVertexWeightBlock(Writer, Weight.BoneIndex, Weight.Weight);
                }
            }
            // Increase
            VertexIndex++;
        }
    }
    // the submesh index
    uint32_t SubmeshIndex = 0;
    // Reset
    VertexIndex = 0;
    // Write Count
    WriteMetaUInt32Block(Writer, 0xBE92, FaceCount);
    // Iterate over submeshes
    for (auto& Submesh : Model.Submeshes)
    {
        // Find the index of this submesh's material
        auto FindResult = MaterialIndicies.find(Submesh.MaterialIndicies[0]);
        // Check
        uint32_t IndexOfMaterial = (FindResult != MaterialIndicies.end()) ? (uint32_t)std::distance(MaterialIndicies.begin(), FindResult) : 0;
        // Loop through faces
        for (auto& Face : Submesh.Faces)
        {
            // Output the face information
            WriteFaceInfoBlock(Writer, (uint8_t)SubmeshIndex, (uint8_t)IndexOfMaterial);
            // Write vertex
            if (VertexCount > UINT16_MAX)
            {
                WriteFaceVertex32(Writer, Submesh, Face.Index3, VertexIndex);
                WriteFaceVertex32(Writer, Submesh, Face.Index1, VertexIndex);
                WriteFaceVertex32(Writer, Submesh, Face.Index2, VertexIndex);
            }
            else
            {
                WriteFaceVertex16(Writer, Submesh, Face.Index3, VertexIndex);
                WriteFaceVertex16(Writer, Submesh, Face.Index1, VertexIndex);
                WriteFaceVertex16(Writer, Submesh, Face.Index2, VertexIndex);
            }
        }
        // Increase
        VertexIndex += Submesh.VertexCount();
        // Advance
        SubmeshIndex++;
    }
}

// Writes Object Info Block
void WriteMetaObjectInfo(MemoryWriter& Writer, int16_t Index, std::string Name)
{
    // Write Hash
    Writer.Write((int16_t)0x87D4);
    // Write Index
    Writer.Write(Index);
    // Write Name
    WriteStringAligned(Writer, Name);
}

// Writes Objects/Meshes
void WriteObjects(MemoryWriter& Writer, const WraithModel& Model)
{
    // Write Count
    WriteMetaInt16Block(Writer, (int16_t)0x62AF, (int16_t)Model.SubmeshCount());
    // Loop
    for (uint32_t i = 0; i < Model.SubmeshCount(); i++)
    {
        WriteMetaObjectInfo(Writer, i, Strings::Format("WraithMesh_%d", i));
    }
}

// Writes material info block
void WriteMaterialInfoBlock(MemoryWriter& Writer, const WraithMaterial& Material, int16_t Index)
{
    // Write Hash
    Writer.Write((int16_t)0xA700);
    // Write Index
    Writer.Write(Index);
    // Write Name, Type, and Image
    WriteStringAligned(Writer, Material.MaterialName);
    WriteStringAligned(Writer, "lambert");

    // Ensure we don't exceed 128 characters, APE (and probably Linker) limits it...
    if(Material.DiffuseMapName.size() < 112)
        WriteStringAligned(Writer, Strings::Format("color:%s", Material.DiffuseMapName.c_str()));
    else
        WriteStringAligned(Writer, "");
}

// Writes material info block
void WriteMaterials(MemoryWriter& Writer, const WraithModel& Model)
{
    // Color Buffer
    uint8_t Color[4]{ 255, 255, 255, 255 };
    // Write Count
    WriteMetaInt16Block(Writer, (int16_t)0xA1B2, (int16_t)Model.MaterialCount());
    // Loop
    for (int i = 0; i < Model.MaterialCount(); i++)
    {
        // Write initial Info
        WriteMaterialInfoBlock(Writer, Model.Materials[i], (int16_t)i);
        // Write Default Info (Never used for T7, material set ups handle it)
        WriteFaceVertexColorBlock(Writer, (uint8_t*)&Color);
        WriteMetaVec4Block(Writer, 0x6DAB, 0, 0, 0, 1);
        WriteMetaVec4Block(Writer, 0x37FF, 0, 0, 0, 1);
        WriteMetaVec4Block(Writer, 0x4265, 0, 0, 0, 1);
        WriteMetaVec2Block(Writer, 0xC835, Vector2(0.8f, 0));
        WriteMetaVec2Block(Writer, 0xFE0C, Vector2(0, 0));
        WriteMetaVec2Block(Writer, 0x7E24, Vector2(6, 1));
        WriteMetaVec4Block(Writer, 0x317C, -1, -1, -1, 1);
        WriteMetaVec4Block(Writer, 0xE593, -1, -1, -1, 1);
        WriteMetaVec2Block(Writer, 0x7D76, Vector2(-1, -1));
        WriteMetaVec2Block(Writer, 0x83C7, Vector2(-1, -1));
        WriteMetaFloatBlock(Writer, 0x5CD2, -1.0f);
    }
}

void CodXMB::ExportXMB(const WraithModel& Model, const std::string& FileName)
{
    // Create a new writer (8MB of memory, reallocates if needed)
    auto Writer = MemoryWriter(0x800000);
    // Write header
    WriteCommentBlock(Writer, "Generated by Greyhound - A fork of Wraith Archon");
    WriteCommentBlock(Writer, "Please credit DTZxPorter & Scobalula for using it!");
    WriteModelBlock(Writer);
    WriteVersionBlock(Writer, 7);
    // Write Required Data
    WriteBones(Writer, Model);
    WriteMeshData(Writer, Model);
    WriteObjects(Writer, Model);
    WriteMaterials(Writer, Model);
    // Create Output File
    auto FileWriter = BinaryWriter();
    FileWriter.Create(FileName);
    // Create LZ4 Output
    auto CompressedSize = Compression::CompressionSizeLZ4((uint32_t)Writer.GetPosition());
    auto CompressedBuffer = std::make_unique<int8_t[]>(CompressedSize);
    auto ResultSize = Compression::CompressLZ4Block(Writer.GetCurrentStream(), CompressedBuffer.get(), Writer.GetPosition(), CompressedSize);
    // Write Header
    FileWriter.Write(XMBMagic);
    FileWriter.Write((uint32_t)Writer.GetPosition());
    // Write Buffer
    FileWriter.Write(CompressedBuffer.get(), ResultSize);
}
