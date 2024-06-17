#include "stdafx.h"

// The class we are implementing
#include "GLTFExport.h"

// We need the binarywriter class
#include "BinaryWriter.h"

// We need the strings class
#include "Strings.h"

// We need the Tiny GLTF Header
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

template <class T>
void WriteToBuffer(tinygltf::Buffer& GltfBuffer, const T data)
{
	auto asChar = (const unsigned char*)&data;

	for (size_t i = 0; i < sizeof(T); i++)
	{
		GltfBuffer.data.push_back(asChar[i]);
	}
}

int CreateView(tinygltf::Model& GltfModel, int Buffer, size_t Offset, size_t Length, size_t Stride, int Target)
{
	int Index = (int)GltfModel.bufferViews.size();

	tinygltf::BufferView GltfView;

	GltfView.buffer = Buffer;
	GltfView.byteOffset = Offset;
	GltfView.byteLength = Length;
	GltfView.byteStride = Stride;
	GltfView.target = Target;

	GltfModel.bufferViews.push_back(GltfView);

	return Index;
}


int CreateAccessor(tinygltf::Model& GltfModel, int View, size_t Offset, size_t Length, size_t stride, int CompType, int Type)
{
	int Index = (int)GltfModel.accessors.size();

	tinygltf::Accessor GltfAccessor;

	GltfAccessor.bufferView = View;
	GltfAccessor.byteOffset = Offset;
	GltfAccessor.count = Length;
	GltfAccessor.componentType = CompType;
	GltfAccessor.type = Type;

	GltfModel.accessors.push_back(GltfAccessor);

	return Index;
}

int CreateAccessor(tinygltf::Model& GltfModel, int View, size_t Offset, size_t Length, size_t stride, int CompType, int Type, double MinX, double MinY, double MinZ, double MinW, double MaxX, double MaxY, double MaxZ, double MaxW)
{
	int Index = (int)GltfModel.accessors.size();

	tinygltf::Accessor GltfAccessor;

	GltfAccessor.bufferView = View;
	GltfAccessor.byteOffset = Offset;
	GltfAccessor.count = Length;
	GltfAccessor.componentType = CompType;
	GltfAccessor.type = Type;

	switch (Type)
	{
	case TINYGLTF_TYPE_SCALAR:
		GltfAccessor.minValues.push_back(MinX);
		GltfAccessor.maxValues.push_back(MaxX);
		break;
	case TINYGLTF_TYPE_VEC2:
		GltfAccessor.minValues.push_back(MinX);
		GltfAccessor.minValues.push_back(MinY);
		GltfAccessor.maxValues.push_back(MaxX);
		GltfAccessor.maxValues.push_back(MaxY);
		break;
	case TINYGLTF_TYPE_VEC3:
		GltfAccessor.minValues.push_back(MinX);
		GltfAccessor.minValues.push_back(MinY);
		GltfAccessor.minValues.push_back(MinZ);
		GltfAccessor.maxValues.push_back(MaxX);
		GltfAccessor.maxValues.push_back(MaxY);
		GltfAccessor.maxValues.push_back(MaxZ);
		break;
	case TINYGLTF_TYPE_VEC4:
		GltfAccessor.minValues.push_back(MinX);
		GltfAccessor.minValues.push_back(MinY);
		GltfAccessor.minValues.push_back(MinZ);
		GltfAccessor.minValues.push_back(MinW);
		GltfAccessor.maxValues.push_back(MaxX);
		GltfAccessor.maxValues.push_back(MaxY);
		GltfAccessor.maxValues.push_back(MaxZ);
		GltfAccessor.maxValues.push_back(MaxW);
		break;
	}

	GltfModel.accessors.push_back(GltfAccessor);

	return Index;
}

void GLTF::ExportGLTF(const WraithModel& Model, const std::string& FileName, bool SupportsScale, bool writeBinary)
{
	// TODO: Adjust types based off counts, so we're not writing 4 bytes per face
	// or joint when it isn't necessary.

	tinygltf::Scene Scene;
	tinygltf::Model GltfModel;
	tinygltf::Skin GltfSkin;
	tinygltf::TinyGLTF loader;

	// Current Indices
	size_t NodeIndex = 0;

	Scene.nodes.push_back(0);

	// Assign asset
	GltfModel.asset.generator = "Greyhound";
	GltfModel.asset.version = "2.0";
	GltfModel.defaultScene = 0;

	// Our main buffer
	tinygltf::Buffer GltfBuffer;

	// Bones
	auto MatricesView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Model.Bones.size() * 64, 0, 0);
	auto MatricesAccessor = CreateAccessor(GltfModel, MatricesView, 0, Model.Bones.size(), 0, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_MAT4);

	// Set up the skin
	GltfSkin.inverseBindMatrices = MatricesAccessor;

	// Create a matrix for up axis conversion
	Matrix GlobalTransform;
	GlobalTransform.Mat(0, 0) = 1.0f;
	GlobalTransform.Mat(1, 2) = 1.0f;
	GlobalTransform.Mat(2, 1) = 1.0f;

	// Write bones
	for (auto& Bone : Model.Bones)
	{
		// Create a node for this bone.
		tinygltf::Node GltfNode;
		GltfNode.name = Bone.TagName;
		GltfNode.translation.push_back(Bone.LocalPosition.X);
		GltfNode.translation.push_back(Bone.LocalPosition.Y);
		GltfNode.translation.push_back(Bone.LocalPosition.Z);
		GltfNode.rotation.push_back(Bone.LocalRotation.X);
		GltfNode.rotation.push_back(Bone.LocalRotation.Y);
		GltfNode.rotation.push_back(Bone.LocalRotation.Z);
		GltfNode.rotation.push_back(Bone.LocalRotation.W);
		// If we have a bone parent, add to the children of the parent.
		if(Bone.BoneParent != -1)
			GltfModel.nodes[Bone.BoneParent].children.push_back((int)GltfModel.nodes.size());
		// Get the global matrix and add it to our inverse matrices.
		Matrix Transform = Matrix::CreateFromQuaternion(Bone.GlobalRotation);
		Transform.Mat(3, 0) = Bone.GlobalPosition.X;
		Transform.Mat(3, 1) = Bone.GlobalPosition.Y;
		Transform.Mat(3, 2) = Bone.GlobalPosition.Z;
		// Now push inverted
		WriteToBuffer(GltfBuffer, Transform.Inverse());
		// Finally add to skin.
		GltfSkin.joints.push_back((int)GltfModel.nodes.size());
		GltfModel.nodes.push_back(GltfNode);
	}

	// Texture Info
	std::map<std::string, int> TextureMap;

	// Build unique textures
	for (auto& Material : Model.Materials)
	{
		// Pre-check
		if (Material.DiffuseMapName.size() != 0 && TextureMap.find(Material.DiffuseMapName) == TextureMap.end())
		{
			// Build our image source and texture.
			tinygltf::Image Image;
			Image.uri = Material.DiffuseMapName;
			tinygltf::Texture Texture;
			Texture.source = (int)GltfModel.images.size();
			// Add image
			GltfModel.images.push_back(Image);
			// Add to our final lists for materials to reference from
			TextureMap[Material.DiffuseMapName] = GltfModel.textures.size();
			GltfModel.textures.push_back(Texture);
		}
		if (Material.NormalMapName.size() != 0 && TextureMap.find(Material.NormalMapName) == TextureMap.end())
		{
			// Build our image source and texture.
			tinygltf::Image Image;
			Image.uri = Material.NormalMapName;
			tinygltf::Texture Texture;
			Texture.source = (int)GltfModel.images.size();
			// Add image
			GltfModel.images.push_back(Image);
			// Add to our final lists for materials to reference from
			TextureMap[Material.NormalMapName] = (int)GltfModel.textures.size();
			GltfModel.textures.push_back(Texture);
		}
	}

	// Build Materials
	for (auto& Material : Model.Materials)
	{
		tinygltf::Material GltfMaterial;

		// Check for a texture
		if (Material.DiffuseMapName.size() > 0)
		{
			// Assign the actual texture
			GltfMaterial.pbrMetallicRoughness.baseColorTexture.index = TextureMap[Material.DiffuseMapName];
		}
		else
		{
			// Assign a random color instead for danka viewing.
			GltfMaterial.pbrMetallicRoughness.baseColorFactor[0] = ((std::rand() % 100) / 100.0);
			GltfMaterial.pbrMetallicRoughness.baseColorFactor[1] = ((std::rand() % 100) / 100.0);
			GltfMaterial.pbrMetallicRoughness.baseColorFactor[2] = ((std::rand() % 100) / 100.0);
		}
		// Check for normal map
		if (Material.NormalMapName.size() > 0)
		{
			GltfMaterial.normalTexture.index = TextureMap[Material.NormalMapName];
		}

		GltfMaterial.pbrMetallicRoughness.metallicFactor = 0;
		GltfMaterial.pbrMetallicRoughness.roughnessFactor = 0.75;
		GltfMaterial.name = Material.MaterialName;
		GltfModel.materials.push_back(GltfMaterial);
	}

	// Build meshes
	for (auto& Mesh : Model.Submeshes)
	{
		tinygltf::Mesh GltfMesh;
		tinygltf::Primitive GltfPrim;

		size_t MaxSkinInfluenceBuffer = 0;

		Vector3 MinPos(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		Vector3 MaxPos(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

		// Iterate to dynamically calculate max weight influence
		for (auto& Vertex : Mesh.Verticies)
		{
			if (Vertex.WeightCount() > MaxSkinInfluenceBuffer)
				MaxSkinInfluenceBuffer = Vertex.WeightCount();

			// Check Min/Max
			MinPos.X = std::min(Vertex.Position.X, MinPos.X);
			MinPos.Y = std::min(Vertex.Position.Y, MinPos.Y);
			MinPos.Z = std::min(Vertex.Position.Z, MinPos.Z);
			MaxPos.X = std::max(Vertex.Position.X, MaxPos.X);
			MaxPos.Y = std::max(Vertex.Position.Y, MaxPos.Y);
			MaxPos.Z = std::max(Vertex.Position.Z, MaxPos.Z);
		}

		if (Model.BlendShapes.size() > 0)
		{
			tinygltf::Value::Array TargetNames;

			for (auto& Shape : Model.BlendShapes)
			{
				TargetNames.emplace_back(Shape);
			}

			GltfMesh.extras = tinygltf::Value(
			{
				{ "targetNames", tinygltf::Value(TargetNames) }
			});
		}

		size_t NumWeights = ((MaxSkinInfluenceBuffer - 1) / 4) + 1;

		// Indices
		auto IndicesView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Faces.size() * 3 * 4, 0, TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
		auto IndicesAccessor = CreateAccessor(GltfModel, IndicesView, 0, Mesh.Faces.size() * 3, 0, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR, 0, 0, 0, 0, Mesh.Verticies.size() -1, 0, 0, 0);
		GltfPrim.indices = IndicesAccessor;
		GltfPrim.mode = TINYGLTF_MODE_TRIANGLES;

		for (auto& Face : Mesh.Faces)
		{
			WriteToBuffer(GltfBuffer, Face.Index1);
			WriteToBuffer(GltfBuffer, Face.Index3);
			WriteToBuffer(GltfBuffer, Face.Index2);
		}

		// Positions
		auto PositionsView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Verticies.size() * 12, 0, TINYGLTF_TARGET_ARRAY_BUFFER);
		auto PositionsAccessor = CreateAccessor(GltfModel, PositionsView, 0, Mesh.Verticies.size(), 0, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3,
			MinPos.X,
			MinPos.Y,
			MinPos.Z,
			0.0,
			MaxPos.X,
			MaxPos.Y,
			MaxPos.Z,
			0.0);
		GltfPrim.attributes["POSITION"] = PositionsAccessor;

		for (auto& Vertex : Mesh.Verticies)
		{
			WriteToBuffer(GltfBuffer, Vertex.Position);
		}

		if (Model.BlendShapes.size() > 0)
		{
			for (size_t i = 0; i < Model.BlendShapes.size(); i++)
			{
				// Add target for the actual mesh data
				auto ShapePositionsView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Verticies.size() * 12, 0, TINYGLTF_TARGET_ARRAY_BUFFER);
				auto ShapePositionsAccessor = CreateAccessor(GltfModel, ShapePositionsView, 0, Mesh.Verticies.size(), 0, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3,
					0.0,
					0.0,
					0.0,
					0.0,
					0.0,
					0.0,
					0.0,
					0.0);
				GltfPrim.targets.push_back(
				{
					{ "POSITION", ShapePositionsAccessor }
				});

				// We'll need to add each vertex for this blendshape
				// This could do with some optimization, for both space
				// and performance, as it requires looping each vertex
				// per shape, this ties in with better optimizing the 
				// buffer to pack data with a stride
				for (auto& Vertex : Mesh.Verticies)
				{
					Vector3 ShapeValue;

					for (auto& Delta : Vertex.BlendShapeDeltas)
					{
						if (Delta.first == i)
						{
							ShapeValue = Delta.second;
							break;
						}
					}

					WriteToBuffer(GltfBuffer, ShapeValue * 2.54f);
				}
			}
		}

		// Normals
		auto NormalsView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Verticies.size() * 12, 0, TINYGLTF_TARGET_ARRAY_BUFFER);
		auto NormalsAccessor = CreateAccessor(GltfModel, NormalsView, 0, Mesh.Verticies.size(), 0, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3);
		GltfPrim.attributes["NORMAL"] = NormalsAccessor;

		for (auto& Vertex : Mesh.Verticies)
		{
			WriteToBuffer(GltfBuffer, Vertex.Normal);
		}

		// UVs
		auto UVsView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Verticies.size() * 8, 0, TINYGLTF_TARGET_ARRAY_BUFFER);
		auto UVsAccessor = CreateAccessor(GltfModel, UVsView, 0, Mesh.Verticies.size(), 0, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2);
		GltfPrim.attributes["TEXCOORD_0"] = UVsAccessor;

		for (auto& Vertex : Mesh.Verticies)
		{
			WriteToBuffer(GltfBuffer, Vertex.UVLayers[0]);
		}

		// For weights, we'll need to add each "channel"
		// TODO: Could use stride to optimize this so we can write in 1 go.
		for (size_t c = 0; c < NumWeights; c++)
		{
			auto JointsView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Verticies.size() * 8, 0, TINYGLTF_TARGET_ARRAY_BUFFER);
			auto JointsAccessor = CreateAccessor(GltfModel, JointsView, 0, Mesh.Verticies.size(), 0, TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, TINYGLTF_TYPE_VEC4);
			GltfPrim.attributes[Strings::Format("JOINTS_%llu", c)] = JointsAccessor;

			for (auto& Vertex : Mesh.Verticies)
			{
				for (size_t w = 0; w < 4; w++)
				{
					size_t idx = (c * 4) + w;

					if (idx >= Vertex.Weights.size())
					{
						WriteToBuffer(GltfBuffer, (uint16_t)0);
					}
					else
					{
						WriteToBuffer(GltfBuffer, (uint16_t)Vertex.Weights[idx].BoneIndex);
					}
				}
			}

			auto WeightsView = CreateView(GltfModel, 0, GltfBuffer.data.size(), Mesh.Verticies.size() * 16, 0, TINYGLTF_TARGET_ARRAY_BUFFER);
			auto WeightsAccessor = CreateAccessor(GltfModel, WeightsView, 0, Mesh.Verticies.size(), 0, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC4);
			GltfPrim.attributes[Strings::Format("WEIGHTS_%llu", c)] = WeightsAccessor;

			for (auto& Vertex : Mesh.Verticies)
			{
				for (size_t w = 0; w < 4; w++)
				{
					size_t idx = (c * 4) + w;

					if (idx >= Vertex.Weights.size())
					{
						WriteToBuffer(GltfBuffer, 0.0f);
					}
					else
					{
						WriteToBuffer(GltfBuffer, Vertex.Weights[idx].Weight);
					}
				}
			}
		}

		// Add material
		GltfPrim.material = Mesh.MaterialIndicies[0];

		// Add node
		tinygltf::Node GltfMeshNode;
		GltfMeshNode.mesh = (int)GltfModel.meshes.size();
		GltfMeshNode.skin = 0;
		GltfModel.nodes.push_back(GltfMeshNode);

		// Add to our model
		GltfMesh.primitives.push_back(GltfPrim);
		GltfModel.meshes.push_back(GltfMesh);
	}

#if _DEBUG
	tinygltf::Model model;
	std::string err;
	std::string warn;
	// assume ascii glTF.
	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, FileName);

	if (!warn.empty()) {
		std::cout << "warn : " << warn << std::endl;
	}
	if (!ret) {
		if (!err.empty())
		{
			std::cerr << err << std::endl;
		}
	}
#endif

	GltfModel.buffers.push_back(GltfBuffer);
	GltfModel.skins.push_back(GltfSkin);
	GltfModel.scenes.push_back(Scene);
	loader.WriteGltfSceneToFile(&GltfModel, FileName, false, false, true, writeBinary);
}