#include "stdafx.h"

// The class we are implementing
#include "XNALaraExport.h"

// We need the textwriter, hashing, and filesystems classes
#include "TextWriter.h"
#include "Strings.h"
#include "FileSystems.h"

void XNALara::ExportXNA(const WraithModel& Model, const std::string& FileName)
{
	// Create a new writer
	auto Writer = TextWriter();
	// Open the model file
	Writer.Create(FileName);
	// Set buffer
	Writer.SetWriteBuffer(0x100000);
	// Write total bone count
	Writer.WriteLineFmt("%d", Model.BoneCount());
	// Loop through bones
	for (auto& Bone : Model.Bones)
	{
		// Write the name, parent, and position
		Writer.WriteLineFmt(
			"%s\n"
			"%d\n"
			"%f %f %f",
			Bone.TagName.c_str(),
			Bone.BoneParent,
			Bone.GlobalPosition.X, Bone.GlobalPosition.Y, Bone.GlobalPosition.Z);
	}
	// Cache submesh count
	auto SubmeshCount = Model.SubmeshCount();
	// Write submesh count
	Writer.WriteLineFmt("%d", SubmeshCount);
	// Loop through and output submeshes
	uint32_t SubmeshIndex = 0;
	// Iterate
	for (auto& Submesh : Model.Submeshes)
	{
		// Write submesh name and layer info
		Writer.WriteLineFmt(
			"WraithMesh%02d\n"
			"1\n1",
			SubmeshIndex);
		// Grab material reference
		const WraithMaterial& Material = (Submesh.MaterialIndicies[0] > -1) ? Model.Materials[Submesh.MaterialIndicies[0]] : WraithMaterial::DefaultMaterial;
		// Output material name, UV layer, and vertex count
		Writer.WriteLineFmt(
			"%s\n"
			"0\n"
			"%d",
			Material.MaterialName.c_str(),
			Submesh.VertexCount());
		// Iterate over verticies
		for (auto& Vertex : Submesh.Verticies)
		{
			// Write positions, normals, RGBA colors, and UVs
			Writer.WriteLineFmt(
				"%f %f %f\n"
				"%f %f %f\n"
				"255 255 255 255\n"
				"%f %f",
				// The data for the vertex
				Vertex.Position.X, Vertex.Position.Y, Vertex.Position.Z,
				Vertex.Normal.X, Vertex.Normal.Y, Vertex.Normal.Z,
				Vertex.UVLayers[0].U, Vertex.UVLayers[0].V);
			// Prepare to output weights
			uint32_t WeightIDs[4] = { 0, 0, 0, 0 };
			// Buffer for weight values
			float WeightValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			// Loop and apply (Only the first 4 weights are supported)
			for (uint32_t w = 0; w < std::min<uint32_t>(Vertex.WeightCount(), 4); w++)
			{
				// Apply the ID
				WeightIDs[w] = Vertex.Weights[w].BoneIndex;
				// Apply the value
				WeightValues[w] = Vertex.Weights[w].Weight;
			}
			// Output values
			Writer.WriteLineFmt(
				"%d %d %d %d\n"
				"%f %f %f %f",
				WeightIDs[0], WeightIDs[1], WeightIDs[2], WeightIDs[3],
				WeightValues[0], WeightValues[1], WeightValues[2], WeightValues[3]);
		}
		// Write face count
		Writer.WriteLineFmt("%d", Submesh.FacesCount());
		// Iterate over faces
		for (auto& Face : Submesh.Faces)
		{
			// Output indicies
			Writer.WriteLineFmt("%d %d %d", Face.Index1, Face.Index2, Face.Index3);
		}
		// Advance
		SubmeshIndex++;
	}
}