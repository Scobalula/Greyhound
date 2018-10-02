#pragma once

#include <cstdint>
#include <string>
#include <vector>

// We need the WraithAsset as a base
#include "WraithAsset.h"

// We need the vector 3, quaternion classes
#include "VectorMath.h"

// A class that represents a bone
class WraithBone
{
public:
	// Creates a new WraithBone
	WraithBone();
	~WraithBone();

	// The tag name of the bone
	std::string TagName;
	// The index of this bones parent in the WraithModel bone list (-1 means connected to the root)
	int32_t BoneParent;
	// The localized position of this bone
	Vector3 LocalPosition;
	// The localized rotation of this bone
	Quaternion LocalRotation;
	// The global position of this bone
	Vector3 GlobalPosition;
	// The global rotation of this bone
	Quaternion GlobalRotation;
	// The bone scale, defaults to 1.0
	Vector3 BoneScale;
};

// A class that represents a face
class WraithFace
{
public:
	// Creates a new WraithFace
	WraithFace();
	// Creates a new WraithFace with the specified vertex indicies
	WraithFace(uint32_t Vert1, uint32_t Vert2, uint32_t Vert3);
	~WraithFace();

	// -- Indicies

	// The first face index
	uint32_t Index1;
	// The second face index
	uint32_t Index2;
	// The last face index
	uint32_t Index3;
};

// A class that represents a vertex weight
class WraithVertexWeight
{
public:
	// Creates a WraithVertexWeight
	WraithVertexWeight();
	// Creates a WraithVertexWeight with the specified values
	WraithVertexWeight(uint32_t Bone, float Value);
	~WraithVertexWeight();

	// The bone index
	uint32_t BoneIndex;
	// The vertex weight (Out of 1.0)
	float Weight;
};

// A class that represents a vertex
class WraithVertex
{
public:
	// Creates a new WraithVertex
	WraithVertex();
	~WraithVertex();
	
	// -- Vertex data

	// The position of this vertex
	Vector3 Position;
	// A list of weights for this vertex
	std::vector<WraithVertexWeight> Weights;
	// The vertex normal of this vertex
	Vector3 Normal;
	// A list of UV layers for this vertex
	std::vector<Vector2> UVLayers;
	// Vertex Colors (RGBA)
	uint8_t Color[4];

	// -- Properties

	// The count of weights
	uint32_t WeightCount() const;
	// The count of UV layers
	uint32_t UVLayerCount() const;

	// -- Adding data

	// Add a weight to the vertex
	void AddVertexWeight(uint32_t BoneIndex, float Weight);
	// Add a UV layer to the vertex
	void AddUVLayer(float UVU, float UVV);

	// -- Removing data

	// Remove a weight with it's index
	void RemoveVertexWeight(uint32_t Index);
	// Remove a UV layer with it's index
	void RemoveUVLayer(uint32_t Index);
};

// A class that represents a submesh
class WraithSubmesh
{
public:
	// Creates a new WraithSubmesh
	WraithSubmesh();
	~WraithSubmesh();

	// -- Submesh data

	// A list of verticies in order for this submesh
	std::vector<WraithVertex> Verticies;
	// A list of faces in order for this submesh
	std::vector<WraithFace> Faces;
	// The material index used for each UV layer, in order, -1 means we have no material for this
	std::vector<int32_t> MaterialIndicies;

	// -- Properties

	// Get the count of verticies in the submesh
	uint32_t VertexCount() const;
	// Get the count of faces in the submesh
	uint32_t FacesCount() const;
	// Get the count of materials in the submesh
	uint32_t MaterialCount() const;

	// -- Adding data functions

	// Add a new vertex, returns a reference to the vertex for editing
	WraithVertex& AddVertex();
	// Adds a new vertex, moving it to the mesh
	void AddVertex(WraithVertex& Vertex);
	// Add a new face, returns a reference to the face for editing
	WraithFace& AddFace();
	// Add a new face with the given indicies
	void AddFace(uint32_t Index1, uint32_t Index2, uint32_t Index3);
	// Add a new material index, -1 means no material found
	void AddMaterial(int32_t Index);

	// -- Removing data functions

	// Remove a vertex by it's index
	void RemoveVertex(uint32_t Index);
	// Remove a face by it's index
	void RemoveFace(uint32_t Index);
	// Remove a material by it's index
	void RemoveMaterial(uint32_t Index);

	// -- Utility functions

	// Scale the verticies by a given scalefactor
	void ScaleSubmesh(float ScaleFactor);

	// Prepares the submesh for a lot of faces / verticies
	void PrepareMesh(uint32_t VertexCount, uint32_t FaceCount);
};

// A class that represents a material
class WraithMaterial
{
public:
	// Creates a new WraithMaterial
	WraithMaterial();
	// Creates a new WraithMaterial with the specified settings
	WraithMaterial(const std::string Name, const std::string Diffuse, const std::string Normal, const std::string Specular);
	~WraithMaterial();

	// -- Material data

	// The name of the material
	std::string MaterialName;
	// The file name of the diffuse map for this material
	std::string DiffuseMapName;
	// The file name of the normal map for this material
	std::string NormalMapName;
	// The file name of the specular map for this material
	std::string SpecularMapName;

	// The default material, when none exists
	static const WraithMaterial DefaultMaterial;
};

// A class that represents a model
class WraithModel : public WraithAsset
{
public:
	// Creates a new WraithModel
	WraithModel();
	~WraithModel();

	// -- Model data

	// A list of bones in this model
	std::vector<WraithBone> Bones;
	// A list of submeshes in this model
	std::vector<WraithSubmesh> Submeshes;
	// A list of materials for this model
	std::vector<WraithMaterial> Materials;

	// -- Model properties

	// Gets the number of bones in this model
	uint32_t BoneCount() const;
	// Gets the total number of verticies in this model
	uint32_t VertexCount() const;
	// Gets the total number of faces in this model
	uint32_t FaceCount() const;
	// Gets the total number of submeshes in this model
	uint32_t SubmeshCount() const;
	// Gets the total number of materials in this model
	uint32_t MaterialCount() const;

	// -- Model lod properties

	// Lod viewing distance
	float LodDistance;
	// Lod max viewing distance
	float LodMaxDistance;

	// -- Adding data functions

	// Adds a material to the model, returns a reference to it for editing
	WraithMaterial& AddMaterial();
	// Adds a material to the model, moving it
	void AddMaterial(WraithMaterial& Material);
	// Adds a bone to the model, returns a reference to it for editing
	WraithBone& AddBone();
	// Adds a bone to the model, moving it
	void AddBone(WraithBone& Bone);
	// Adds a submesh to the model, returns a reference to it for editing
	WraithSubmesh& AddSubmesh();
	// Adds a submesh to the model, moving it
	void AddSubmesh(WraithSubmesh& Submesh);

	// -- Removing data functions

	// Remove a bone from the model with it's index
	void RemoveBone(uint32_t Index);
	// Remove a submesh from the model with it's index
	void RemoveSubmesh(uint32_t Index);
	// Remove a material from the model with it's index
	void RemoveMaterial(uint32_t Index);

	// -- Utility functions

	// Scales every bone and vertex with the given scalefactor
	void ScaleModel(float ScaleFactor);

	// Prepares the model for a lot of submeshes
	void PrepareSubmeshes(uint32_t SubmeshCount);
	// Prepares the model for a lot of bones
	void PrepareBones(uint32_t BoneCount);

	// Generate local positions from global ones
	void GenerateLocalPositions(bool Translations, bool Rotations);
	// Generate global positions from local ones
	void GenerateGlobalPositions(bool Translations, bool Rotations);
};