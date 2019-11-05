// NOTE: This has been rewritten using a set of classes not yet implemented, do not use!
#include "stdafx.h"

// The class we are implementing
#include "FBXExport.h"

// We need the binarywriter class
#include "BinaryWriter.h"

// We need the Hashing class
#include "Hashing.h"

// FBX Magic (Kaydara FBX Binary)
const uint8_t FBXMagic[23] =
{
    0x4B, 0x61, 0x79, 0x64, 0x61, 0x72, 0x61, 0x20, 0x46, 0x42, 0x58, 0x20,
    0x42, 0x69, 0x6E, 0x61, 0x72, 0x79, 0x20, 0x20, 0x00, 0x1A, 0x00
};
// FBX File ID
const uint8_t FBXFileID[16] = {
    0x28, 0xB3, 0x2A, 0xEB, 0xB6, 0x24, 0xCC, 0xC2, 0xBF, 0xC8, 0xB0, 0x2A,
    0xA9, 0x2B, 0xFC, 0xF1
};
// FBX Foot ID
const uint8_t FBXFootID[16] = {
    0xFA, 0xBC, 0xAB, 0x09, 0xD0, 0xC8, 0xD4, 0x66, 0xB1, 0x76, 0xFB, 0x83,
    0x1C, 0xF7, 0x26, 0x7E
};
// FBX Foot Block
const uint8_t FBXFootBlock[136] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF8, 0x5A, 0x8C, 0x6A, 0xDE, 0xF5, 0xD9, 0x7E, 0xEC, 0xE9, 0x0C, 0xE3,
    0x75, 0x8F, 0x29, 0x0B
};

// Writes Element Header
void WriteElemHeader(BinaryWriter& Writer, const std::string& ElemName, uint32_t EndOffset, uint32_t PropCount, uint32_t PropSize)
{
    //Writer.Write(EndOffset);
    //Writer.Write(PropCount);
    //Writer.Write(PropSize);
    //Writer.Write((uint8_t)ElemName.size());
    //Writer.Write((uint8_t*)ElemName.c_str(), (uint32_t)ElemName.size());
}

#pragma region PropertyWriters
void WriteClassString(BinaryWriter& Writer, const std::string& Prop, const std::string& Class)
{
    //Writer.Write((uint8_t)0x53);
    //Writer.Write((uint32_t)Prop.size() + (uint32_t)Class.size() + 2);
    //Writer.Write((uint8_t*)Prop.c_str(), (uint32_t)Prop.size());
    //Writer.Write<uint16_t>(0x100);
    //Writer.Write((uint8_t*)Class.c_str(), (uint32_t)Class.size());
}

void WriteStringProperty(BinaryWriter& Writer, const std::string& Prop)
{
    //Writer.Write((uint8_t)0x53);
    //Writer.Write((uint32_t)Prop.size());
    //Writer.Write((uint8_t*)Prop.c_str(), (uint32_t)Prop.size());
}

void WriterInt32Property(BinaryWriter& Writer, const int32_t Prop)
{
    //Writer.Write((uint8_t)0x49);
    //Writer.Write(Prop);
}

void WriterInt64Property(BinaryWriter& Writer, const int64_t Prop)
{
    //Writer.Write((uint8_t)0x4C);
    //Writer.Write(Prop);
}
#pragma endregion


// Writes Int32 Elem
void WriteElemSingleInt32(BinaryWriter& Writer, const std::string& ElemName, int32_t Prop)
{
    //WriteElemHeader(Writer, ElemName, (uint32_t)Writer.GetPosition() + (uint32_t)ElemName.size() + 18, 1, 5);
    //Writer.Write((uint8_t)0x49);
    //Writer.Write(Prop);
}

// Writes String Elem
void WriteElemSingleString(BinaryWriter& Writer, const std::string& ElemName, const std::string& Prop)
{
    //WriteElemHeader(Writer, ElemName, (uint32_t)Writer.GetPosition() + (uint32_t)ElemName.size() + (uint32_t)Prop.size() + 18, 1, (uint32_t)Prop.size() + 5);
    //WriteStringProperty(Writer, Prop);
}

void WriteElemSingleByteArray(BinaryWriter& Writer, const std::string& ElemName, const uint8_t* Bytes, uint32_t Size)
{
    //WriteElemHeader(Writer, ElemName, (uint32_t)Writer.GetPosition() + (uint32_t)ElemName.size() + Size + 18, 1, Size + 5);
    //Writer.Write((uint8_t)0x52);
    //Writer.Write(Size);
    //Writer.Write(Bytes, Size);
}

void WritePropertyInteger(BinaryWriter& Writer, const std::string& PropName, const uint32_t Prop)
{
    //WriteElemHeader(Writer, "P", (uint32_t)Writer.GetPosition() + (uint32_t)PropName.size() + 49, 5, (uint32_t)PropName.size() + 35);
    //WriteStringProperty(Writer, PropName);
    //WriteStringProperty(Writer, "int");
    //WriteStringProperty(Writer, "Integer");
    //WriteStringProperty(Writer, "");
    //WriterInt32Property(Writer, Prop);
}

void WriteEmptyGroup(BinaryWriter& Writer, const std::string& GroupName)
{
    //// Write Model (Write Offset at the end)
    //WriteElemHeader(Writer, "Model", (uint32_t)(Writer.GetPosition() + GroupName.size() + 124), 3, GroupName.size() + 30);
    //WriterInt64Property(Writer, (int64_t)Hashing::HashCRC32StringInt(GroupName));
    //WriteClassString(Writer, GroupName, "Model");
    //WriteStringProperty(Writer, "Null");
    //WriteElemSingleInt32(Writer, "Version", 232);
    //WriteElemHeader(Writer, "Properties70", (uint32_t)(Writer.GetPosition() + 38), 0, 0);
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
}

void WriteDocuments(BinaryWriter& Writer, const std::string& FileName)
{
    //WriteElemHeader(Writer, "Documents", (uint32_t)(Writer.GetPosition() + 142), 0, 0);
    //WriteElemSingleInt32(Writer, "Count", 1);
    //WriteElemHeader(Writer, "Document", (uint32_t)(Writer.GetPosition() + 84), 3, 29);
    //WriterInt64Property(Writer, (int64_t)Hashing::HashCRC32StringInt(FileName));
    //WriteStringProperty(Writer, "");
    //WriteStringProperty(Writer, "Scene");
    //WriteElemSingleInt32(Writer, "RootNode", 0);
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
}

void WriteMaterial(BinaryWriter& Writer, const std::string& MaterialName)
{

}

// Writes the basic required nodes
void WriteFBXHeaderExtension(BinaryWriter& Writer)
{
    //// Write the magic
    //Writer.Write(FBXMagic);
    //// Write Version // FBX 2012
    //Writer.Write(7200);

    //// Write Required Elements (Constant Data so offsets remain the same)
    //// Output Header
    //WriteElemHeader(Writer, "FBXHeaderExtension", (uint32_t)(Writer.GetPosition() + 371), 0, 0);
    //WriteElemSingleInt32(Writer, "FBXHeaderVersion", 1003);
    //WriteElemSingleInt32(Writer, "FBXVersion", 7200); // FBX 2012
    //WriteElemSingleInt32(Writer, "EncryptionType", 0);
    //// Output Time Stamp
    //WriteElemHeader(Writer, "CreationTimeStamp", (uint32_t)(Writer.GetPosition() + 233), 0, 0);
    //WriteElemSingleInt32(Writer, "Version", 1000);
    //WriteElemSingleInt32(Writer, "Year", 1337);
    //WriteElemSingleInt32(Writer, "Month", 1337);
    //WriteElemSingleInt32(Writer, "Day", 1337);
    //WriteElemSingleInt32(Writer, "Hour", 1337);
    //WriteElemSingleInt32(Writer, "Minute", 1337);
    //WriteElemSingleInt32(Writer, "Second", 1337);
    //WriteElemSingleInt32(Writer, "Millisecond", 1337);
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
    //// Output Constant File Info
    //WriteElemSingleByteArray(Writer, "FileId", (uint8_t*)&FBXFileID, sizeof(FBXFileID));
    //WriteElemSingleString(Writer, "CreationTime", "1970-01-01 10:00:00:000");
    //// Output Createor
    //WriteElemSingleString(Writer, "Creator", "Exported via Greyhound");
    //// Output Global Settings
    //WriteElemHeader(Writer, "GlobalSettings", (uint32_t)(Writer.GetPosition() + 587), 0, 0);
    //WriteElemSingleInt32(Writer, "Version", 1000);
    //WriteElemHeader(Writer, "Properties70", (uint32_t)(Writer.GetPosition() + 522), 0, 0);
    //WritePropertyInteger(Writer, "UpAxis", 1);
    //WritePropertyInteger(Writer, "UpAxisSign", 1);
    //WritePropertyInteger(Writer, "FrontAxis", 2);
    //WritePropertyInteger(Writer, "FrontAxisSign", 1);
    //WritePropertyInteger(Writer, "CoordAxis", 0);
    //WritePropertyInteger(Writer, "CoordAxisSign", 1);
    //WritePropertyInteger(Writer, "OriginalUpAxis", 2);
    //WritePropertyInteger(Writer, "OriginalUpAxisSign", 1);
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
    //WriteElemHeader(Writer, "", 0, 0, 0); // Pad
}

void WriteDefinitions(BinaryWriter& Writer, const WraithModel& Model, const std::string& FileName)
{
//    // Output Documents
//    WriteDocuments(Writer, FileName);
//    // Output References
//    WriteElemHeader(Writer, "References", (uint32_t)(Writer.GetPosition() + 36), 0, 0);
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//    // Output Definitions
//    WriteElemHeader(Writer, "Definitions", (uint32_t)(Writer.GetPosition() + 507), 0, 0);
//    WriteElemSingleInt32(Writer, "Version", 100);
//    WriteElemSingleInt32(Writer, "Count", 8);
//    WriteElemHeader(Writer, "ObjectType", (uint32_t)(Writer.GetPosition() + 78), 1, 13);
//    WriteStringProperty(Writer, "GlobalSettings");
//    WriteElemSingleInt32(Writer, "Count", 1);
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//
//    WriteElemHeader(Writer, "ObjectType", (uint32_t)(Writer.GetPosition() + 110), 1, 13);
//    WriteStringProperty(Writer, "Model");
//    WriteElemSingleInt32(Writer, "Count", Model.SubmeshCount());
//    WriteElemHeader(Writer, "PropertyTemplate", (uint32_t)(Writer.GetPosition() + 41), 1, 13);
//    WriteStringProperty(Writer, "FbxNode");
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//
//    WriteElemHeader(Writer, "ObjectType", (uint32_t)(Writer.GetPosition() + 113), 1, 13);
//    WriteStringProperty(Writer, "Geometry");
//    WriteElemSingleInt32(Writer, "Count", Model.SubmeshCount());
//    WriteElemHeader(Writer, "PropertyTemplate", (uint32_t)(Writer.GetPosition() + 41), 1, 13);
//    WriteStringProperty(Writer, "FbxMesh");
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//
//    WriteElemHeader(Writer, "ObjectType", (uint32_t)(Writer.GetPosition() + 121), 1, 13);
//    WriteStringProperty(Writer, "Material");
//    WriteElemSingleInt32(Writer, "Count", Model.MaterialCount());
//    WriteElemHeader(Writer, "PropertyTemplate", (uint32_t)(Writer.GetPosition() + 49), 1, 13);
//    WriteStringProperty(Writer, "FbxSurfacePhong");
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//}
//
//void FBX::ExportFBX(const WraithModel& Model, const std::string& FileName)
//{
//    // Create a new writer
//    auto Writer = BinaryWriter();
//    // Open the file
//    Writer.Create(FileName);
//
//    // Data
//    std::map<uint64_t, uint64_t> Connections;
//    std::vector<uint64_t> MaterialHashes(Model.MaterialCount());
//
//    // Write Objects (Write Offset at the end)
//    auto ObjectsBeginOffset = Writer.GetPosition();
//    WriteElemHeader(Writer, "Objects", 0, 0, 0);
//    // Create Groups
//    WriteEmptyGroup(Writer, "Joints");
//    WriteEmptyGroup(Writer, "Meshes");
//    // Connect these to 0
//    Connections[0x825baae2] = 0;
//    Connections[0xbd1e6aa2] = 0;
//    // Iterate through submeshes
//    size_t MaterialIndex = 0;
//    for (auto& Material : Model.Materials)
//    {
//        // Get Hash
//        auto MaterialHash = (uint64_t)Hashing::HashCRC32StringInt(Material.MaterialName);
//        MaterialHashes[MaterialIndex] = MaterialHash;
//        // Write Material Node
//        WriteElemHeader(Writer, "Material", Writer.GetPosition() + 151 + Material.MaterialName.size(), 3, 29 + Material.MaterialName.size());
//        WriterInt64Property(Writer, MaterialHash);
//        WriteClassString(Writer, Material.MaterialName, "Material");
//        WriteStringProperty(Writer, "");
//        WriteElemSingleInt32(Writer, "Version", 102);
//        WriteElemSingleInt32(Writer, "MultiLayer", 0);
//        WriteElemSingleString(Writer, "ShadingModel", "phong");
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        MaterialIndex++;
//    }
//    // Iterate through submeshes
//    size_t SubmeshIndex = 0;
//    for (auto& Submesh : Model.Submeshes)
//    {
//        // Get Name
//        auto MeshName = Strings::Format("WraithMesh_%i", SubmeshIndex);
//        auto GeoHash = (uint64_t)Hashing::HashCRC32StringInt(Strings::Format("%s_Geo", MeshName.c_str()));
//        auto ModelHash = (uint64_t)Hashing::HashCRC32StringInt(Strings::Format("%s_Model", MeshName.c_str()));
//        // Add Connections
//        Connections[GeoHash] = ModelHash;
//        Connections[ModelHash] = 0x825baae2;
//        // Write Model Node
//        WriteElemHeader(Writer, "Model", Writer.GetPosition() + 79 + MeshName.size() + 7, 3, 23 + MeshName.size() + 7);
//        WriterInt64Property(Writer, ModelHash);
//        WriteClassString(Writer, MeshName, "Model");
//        WriteStringProperty(Writer, "Mesh");
//        WriteElemSingleInt32(Writer, "Version", 232);
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        // Begin Offset
//        auto BeginOffset = Writer.GetPosition();
//        // Write Model (Write Offset at the end)
//        WriteElemHeader(Writer, "Geometry", 0, 3, 23 + MeshName.size() + 10);
//        WriterInt64Property(Writer, GeoHash);
//        WriteClassString(Writer, MeshName, "Geometry");
//        WriteStringProperty(Writer, "Mesh");
//        WriteElemSingleInt32(Writer, "GeometryVersion", 124);
//        // Write Vertices
//        WriteElemHeader(Writer, "Vertices", Writer.GetPosition() + 34 + (8 * ((size_t)Submesh.VertexCount() * 3)), 1, 0);
//        Writer.Write((uint8_t)0x64);
//        Writer.Write(Submesh.VertexCount() * 3);
//        Writer.Write(0);
//        Writer.Write((uint32_t)(8 * ((size_t)Submesh.VertexCount() * 3)));
//        for (auto& Vertex : Submesh.Verticies)
//        {
//            Writer.Write((double)Vertex.Position.X);
//            Writer.Write((double)Vertex.Position.Y);
//            Writer.Write((double)Vertex.Position.Z);
//        }
//        // Write Face Indices
//        WriteElemHeader(Writer, "PolygonVertexIndex", Writer.GetPosition() + 44 + (4 * ((size_t)Submesh.FacesCount() * 3)), 1, 0);
//        Writer.Write((uint8_t)0x69);
//        Writer.Write(Submesh.FacesCount() * 3);
//        Writer.Write(0);
//        Writer.Write((uint32_t)(4 * ((size_t)Submesh.FacesCount() * 3)));
//        for (auto& Face : Submesh.Faces)
//        {
//            Writer.Write((int32_t)(Face.Index1));
//            Writer.Write((int32_t)(Face.Index2));
//            Writer.Write((int32_t)(~Face.Index3));
//        }
//        // Write Normals
//        auto NormalCount = Submesh.VertexCount() * 3;
//        auto NormalSize = NormalCount * 8;
//        WriteElemHeader(Writer, "LayerElementNormal", Writer.GetPosition() + 226 + NormalSize, 1, NormalSize + 13);
//        WriterInt32Property(Writer, 0);
//        WriteElemSingleInt32(Writer, "Version", 101);
//        WriteElemSingleString(Writer, "Name", "");
//        WriteElemSingleString(Writer, "MappingInformationType", "ByVertice");
//        WriteElemSingleString(Writer, "ReferenceInformationType", "Direct");
//        WriteElemHeader(Writer, "Normals", Writer.GetPosition() + 33 + NormalSize, 1, 0);
//        Writer.Write((uint8_t)0x64);
//        Writer.Write(NormalCount);
//        Writer.Write(0);
//        Writer.Write(NormalSize);
//        for (auto& Vertex : Submesh.Verticies)
//        {
//            Writer.Write((double)Vertex.Normal.X);
//            Writer.Write((double)Vertex.Normal.Y);
//            Writer.Write((double)Vertex.Normal.Z);
//        }
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        // Write UVs
//        auto UVCount = Submesh.VertexCount() * 2;
//        auto UVSize = UVCount * 8;
//        WriteElemHeader(Writer, "LayerElementUV", Writer.GetPosition() + 217 + UVSize, 1, 5);
//        WriterInt32Property(Writer, 0);
//        WriteElemSingleInt32(Writer, "Version", 101);
//        WriteElemSingleString(Writer, "Name", "");
//        WriteElemSingleString(Writer, "MappingInformationType", "ByVertice");
//        WriteElemSingleString(Writer, "ReferenceInformationType", "Direct");
//        WriteElemHeader(Writer, "UV", Writer.GetPosition() + 28 + UVSize, 1, UVSize + 13);
//        Writer.Write((uint8_t)0x64);
//        Writer.Write(UVCount);
//        Writer.Write(0);
//        Writer.Write(UVSize);
//        for (auto& Vertex : Submesh.Verticies)
//        {
//            Writer.Write((double)Vertex.UVLayers[0].U);
//            Writer.Write((double)Vertex.UVLayers[0].V);
//        }
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        // Write Layer Elem Info
//        WriteElemHeader(Writer, "Layer", Writer.GetPosition() + 269, 1, 5);
//        WriterInt32Property(Writer, 0);
//        WriteElemSingleInt32(Writer, "Version", 100);
//        WriteElemHeader(Writer, "LayerElement", Writer.GetPosition() + 106, 0, 0);
//        WriteElemSingleString(Writer, "Type", "LayerElementNormal");
//        WriteElemSingleInt32(Writer, "TypedIndex", 0);
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        WriteElemHeader(Writer, "LayerElement", Writer.GetPosition() + 102, 0, 0);
//        WriteElemSingleString(Writer, "Type", "LayerElementUV");
//        WriteElemSingleInt32(Writer, "TypedIndex", 0);
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//        // End Offset
//        auto EndOffset = Writer.GetPosition();
//        // Go back and write offset
//        Writer.SetPosition(BeginOffset);
//        Writer.Write((uint32_t)EndOffset);
//        Writer.SetPosition(EndOffset);
//        // Advance
//        SubmeshIndex++;
//    }
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//    // End Offset
//    auto EndOffset = Writer.GetPosition();
//    // Go back and write offset
//    Writer.SetPosition(ObjectsBeginOffset);
//    Writer.Write((uint32_t)EndOffset);
//    Writer.SetPosition(EndOffset);
//    // Write Connections
//    WriteElemHeader(Writer, "Connections", Writer.GetPosition() + (Connections.size() * 39) + 37, 0, 0);
//    // Iterate through connections
//    for (auto& Connection : Connections)
//    {
//        WriteElemHeader(Writer, "C", Writer.GetPosition() + 39, 3, 25);
//        WriteStringProperty(Writer, "00");
//        WriterInt64Property(Writer, Connection.first);
//        WriterInt64Property(Writer, Connection.second);
//    }
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//    WriteElemHeader(Writer, "", 0, 0, 0); // Pad
//    // Write Footer
//    Writer.Write((uint8_t*)&FBXFootID, sizeof(FBXFootID));
//    Writer.Write<uint64_t>(0);
//    Writer.Write<uint64_t>(0);
//    Writer.Write((uint8_t*)&FBXFootBlock, sizeof(FBXFootBlock));
}
