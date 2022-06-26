#include "stdafx.h"

// The class we are implementing
#include "CastExport.h"

// We need the binarywriter class
#include "BinaryWriter.h"

// We need the strings class
#include "Strings.h"

// We need the hashing class
#include "Hashing.h"

void WriteCastProperty(BinaryWriter& Writer, const std::string& Name, const CastProperty* Prop)
{
	Writer.Write((uint16_t)Prop->Identifier);
	Writer.Write((uint16_t)Name.size());
	Writer.Write((uint32_t)Prop->Elements);
	Writer.Write((const uint8_t*)Name.c_str(), (uint32_t)Name.size());
	if(Prop->Elements > 0)
		Writer.Write((const uint8_t*)&Prop->Buffer[0], (uint32_t)Prop->Buffer.size());
}

void WriteCastNode(BinaryWriter& Writer, const CastNode* Node)
{
	Writer.Write((uint32_t)Node->Identifier);
	Writer.Write((uint32_t)Node->Size());
	Writer.Write(Node->Hash);
	Writer.Write((uint32_t)Node->Properties.size());
	Writer.Write((uint32_t)Node->Children.size());

	for (auto& Prop : Node->Properties)
	{
		WriteCastProperty(Writer, Prop.first, Prop.second.get());
	}

	for (auto& Child : Node->Children)
	{
		WriteCastNode(Writer, Child.get());
	}
}

void WriteCastFile(BinaryWriter& Writer, const CastNode& Node)
{
	Writer.Write(0x74736163);
	Writer.Write(1);
	Writer.Write(1);
	Writer.Write(0);

	WriteCastNode(Writer, &Node);
}

void Cast::ExportCastModel(const WraithModel& Model, const std::string& FileName, bool SupportsScale)
{
	BinaryWriter Writer;

	if (!Writer.Create(FileName))
		return;

	CastNode CastRoot;
	auto CastModel = CastRoot.AddNode(CastNodeId::Model);
	auto CastSkeleton = CastModel->AddNode(CastNodeId::Skeleton);

	for (auto& Bone : Model.Bones)
	{
		auto CastBone = CastSkeleton->AddNode(CastNodeId::Bone);

		CastBone->SetProperty("n", Bone.TagName);
		CastBone->SetProperty("p", CastPropertyId::Integer32, (uint32_t)Bone.BoneParent);
		CastBone->SetProperty("lr", CastPropertyId::Vector4, Bone.LocalRotation);
		CastBone->SetProperty("lp", CastPropertyId::Vector3, Bone.LocalPosition);
		CastBone->SetProperty("wr", CastPropertyId::Vector4, Bone.GlobalRotation);
		CastBone->SetProperty("wp", CastPropertyId::Vector3, Bone.GlobalPosition);
		CastBone->SetProperty("s", CastPropertyId::Vector3, Bone.BoneScale);
	}

	auto BoneCount = Model.BoneCount();
	auto BoneIndexType = BoneCount <= 0xFFFF ? BoneCount <= 0xFF ? CastPropertyId::Byte : CastPropertyId::Short : CastPropertyId::Integer32;

	std::vector<uint64_t> MaterialHashes;

	for (auto& Material : Model.Materials)
	{
		auto CastMaterial = CastModel->AddNode(CastNodeId::Material, Hashing::HashXXHashString(Material.MaterialName));

		CastMaterial->SetProperty("n", Material.MaterialName);
		CastMaterial->SetProperty("t", "pbr");

		MaterialHashes.push_back(CastMaterial->Hash);

		auto CastDiffuse = CastMaterial->AddNode(CastNodeId::Material, Hashing::HashXXHashString(Material.DiffuseMapName));
		CastDiffuse->SetProperty("p", Material.DiffuseMapName);
		CastMaterial->SetProperty("albedo", CastPropertyId::Integer64, CastDiffuse->Hash);
		auto CastNormal = CastMaterial->AddNode(CastNodeId::Material, Hashing::HashXXHashString(Material.NormalMapName));
		CastNormal->SetProperty("p", Material.NormalMapName);
		CastMaterial->SetProperty("normal", CastPropertyId::Integer64, CastNormal->Hash);
		auto CastSpec = CastMaterial->AddNode(CastNodeId::Material, Hashing::HashXXHashString(Material.SpecularMapName));
		CastSpec->SetProperty("p", Material.SpecularMapName);
		CastMaterial->SetProperty("specular", CastPropertyId::Integer64, CastSpec->Hash);
	}

	for (auto& Mesh : Model.Submeshes)
	{
		auto CastMesh = CastModel->AddNode(CastNodeId::Mesh);

		auto VertCount = Mesh.VertexCount();
		auto FaceIndexType = VertCount <= 0xFFFF ? VertCount <= 0xFF ? CastPropertyId::Byte : CastPropertyId::Short : CastPropertyId::Integer32;

		auto Positions   = CastMesh->AddProperty("vp", CastPropertyId::Vector3, VertCount * sizeof(Vector3));
		auto Normals     = CastMesh->AddProperty("vn", CastPropertyId::Vector3, VertCount * sizeof(Vector3));
		auto Colors      = CastMesh->AddProperty("vc", CastPropertyId::Integer32, VertCount * sizeof(uint32_t));
		auto Materials   = CastMesh->AddProperty("m", CastPropertyId::Integer64, VertCount * sizeof(uint64_t));
		auto FaceIndices = CastMesh->AddProperty("f", FaceIndexType, VertCount * sizeof(uint32_t));
		auto BoneIndices = CastMesh->AddProperty("wb", BoneIndexType);
		auto BoneWeights = CastMesh->AddProperty("wv", CastPropertyId::Float, VertCount * sizeof(uint32_t));
		auto UVLayer     = CastMesh->AddProperty("u0", CastPropertyId::Vector2, VertCount * sizeof(float));
		
		uint8_t MaterialCountBuffer = (uint8_t)Mesh.MaterialCount();
		uint8_t MaxSkinInfluenceBuffer = 0;

		// Iterate to dynamically calculate max weight influence
		for (auto& Vertex : Mesh.Verticies)
		{
			if (Vertex.WeightCount() > MaxSkinInfluenceBuffer)
				MaxSkinInfluenceBuffer = (uint8_t)Vertex.WeightCount();
		}

		CastMesh->SetProperty("mi", CastPropertyId::Byte, MaxSkinInfluenceBuffer);
		CastMesh->SetProperty("ul", CastPropertyId::Byte, (uint8_t)1); // TODO: Multiple UV Layers, not needed rn now for CoD

		for (auto& Vertex : Mesh.Verticies)
		{
			Positions->Write(Vertex.Position);
			Normals->Write(Vertex.Normal);
			Colors->Write(*(uint32_t*)&Vertex.Color[0]);
			UVLayer->Write(Vertex.UVLayers[0]);

			for (uint32_t i = 0; i < MaxSkinInfluenceBuffer; i++)
			{
				// Write IDs
				auto WeightID = (i < Vertex.WeightCount()) ? Vertex.Weights[i].BoneIndex : 0;
				auto WeightValue = (i < Vertex.WeightCount()) ? Vertex.Weights[i].Weight : 0.0f;

				switch (BoneIndexType)
				{
				case CastPropertyId::Byte:
					BoneIndices->Write((uint8_t)WeightID);
					BoneWeights->Write(WeightValue); break;
				case CastPropertyId::Short:
					BoneIndices->Write((uint16_t)WeightID);
					BoneWeights->Write(WeightValue); break;
				case CastPropertyId::Integer32:
					BoneIndices->Write((uint32_t)WeightID);
					BoneWeights->Write(WeightValue); break;
				}
			}
		}

		for (auto& Face : Mesh.Faces)
		{
			// Skip degen faces, Maya don't be vibing to it
			if (Face.Index1 == Face.Index2 || Face.Index2 == Face.Index3 || Face.Index3 == Face.Index1)
				continue;

			switch (FaceIndexType)
			{
			case CastPropertyId::Byte:
				FaceIndices->Write((uint8_t)Face.Index1);
				FaceIndices->Write((uint8_t)Face.Index2);
				FaceIndices->Write((uint8_t)Face.Index3); break;
			case CastPropertyId::Short:
				FaceIndices->Write((uint16_t)Face.Index1);
				FaceIndices->Write((uint16_t)Face.Index2);
				FaceIndices->Write((uint16_t)Face.Index3); break;
			case CastPropertyId::Integer32:
				FaceIndices->Write((uint32_t)Face.Index1);
				FaceIndices->Write((uint32_t)Face.Index2);
				FaceIndices->Write((uint32_t)Face.Index3); break;
			}
		}

		Materials->Write(MaterialHashes[Mesh.MaterialIndicies[0]]);
	}

	WriteCastFile(Writer, CastRoot);
}

void Cast::ExportCastAnim(const WraithAnim& Anim, const std::string& FileName, bool SupportsScale)
{
	BinaryWriter Writer;

	if (!Writer.Create(FileName))
		return;

	CastNode CastRoot;
	auto CastAnim = CastRoot.AddNode(CastNodeId::Animation);
	//CastNode& CastSkeleton = CastModel.AddNode(CastNodeId::Skeleton);

	CastAnim->SetProperty("fr", CastPropertyId::Float, Anim.FrameRate);
	CastAnim->SetProperty("lo", CastPropertyId::Byte, Anim.Looping);

	for (auto& Positions : Anim.AnimationPositionKeys)
	{
		if (Positions.second.size() == 0)
			continue;

		auto XCurve = CastAnim->AddNode(CastNodeId::Curve);
		auto YCurve = CastAnim->AddNode(CastNodeId::Curve);
		auto ZCurve = CastAnim->AddNode(CastNodeId::Curve);

		XCurve->SetProperty("nn", Positions.first);
		YCurve->SetProperty("nn", Positions.first);
		ZCurve->SetProperty("nn", Positions.first);

		XCurve->SetProperty("kp", "tx");
		YCurve->SetProperty("kp", "ty");
		ZCurve->SetProperty("kp", "tz");

		auto Modifier = Anim.AnimationBoneModifiers.find(Positions.first);
		auto Type = Anim.AnimType;
		if (Modifier != Anim.AnimationBoneModifiers.end())
			Type = Modifier->second;

		switch (Type)
		{
		case WraithAnimationType::Absolute:
			XCurve->SetProperty("m", "absolute");
			YCurve->SetProperty("m", "absolute");
			ZCurve->SetProperty("m", "absolute"); break;
		case WraithAnimationType::Additive:
			XCurve->SetProperty("m", "additive");
			YCurve->SetProperty("m", "additive");
			ZCurve->SetProperty("m", "additive"); break;
		default:
			XCurve->SetProperty("m", "relative");
			YCurve->SetProperty("m", "relative");
			ZCurve->SetProperty("m", "relative"); break;
		}

		uint32_t LargestFrame = 0;

		for (auto& Position : Positions.second)
			if (Position.Frame > LargestFrame)
				LargestFrame = Position.Frame;

		auto KeyFrameBufferType = LargestFrame <= 0xFFFF ? LargestFrame <= 0xFF ? CastPropertyId::Byte : CastPropertyId::Short : CastPropertyId::Integer32;

		auto XKeyFrameBuffer = XCurve->AddProperty("kb", KeyFrameBufferType);
		auto YKeyFrameBuffer = YCurve->AddProperty("kb", KeyFrameBufferType);
		auto ZKeyFrameBuffer = ZCurve->AddProperty("kb", KeyFrameBufferType);
		auto XKeyValueBuffer = XCurve->AddProperty("kv", CastPropertyId::Float);
		auto YKeyValueBuffer = YCurve->AddProperty("kv", CastPropertyId::Float);
		auto ZKeyValueBuffer = ZCurve->AddProperty("kv", CastPropertyId::Float);

		for (auto& Position : Positions.second)
		{
			XKeyValueBuffer->Write(Position.Value.X);
			YKeyValueBuffer->Write(Position.Value.Y);
			ZKeyValueBuffer->Write(Position.Value.Z);

			switch (KeyFrameBufferType)
			{
			case CastPropertyId::Byte:
				XKeyFrameBuffer->Write((uint8_t)Position.Frame);
				YKeyFrameBuffer->Write((uint8_t)Position.Frame);
				ZKeyFrameBuffer->Write((uint8_t)Position.Frame); break;
			case CastPropertyId::Short:
				XKeyFrameBuffer->Write((uint16_t)Position.Frame);
				YKeyFrameBuffer->Write((uint16_t)Position.Frame);
				ZKeyFrameBuffer->Write((uint16_t)Position.Frame); break;
			case CastPropertyId::Integer32:
				XKeyFrameBuffer->Write((uint32_t)Position.Frame);
				YKeyFrameBuffer->Write((uint32_t)Position.Frame);
				ZKeyFrameBuffer->Write((uint32_t)Position.Frame); break;
			}
		}
	}

	for (auto& Rotations : Anim.AnimationRotationKeys)
	{
		if (Rotations.second.size() == 0)
			continue;

		auto Curve = CastAnim->AddNode(CastNodeId::Curve);

		Curve->SetProperty("nn", Rotations.first);
		Curve->SetProperty("kp", "rq");

		auto Modifier = Anim.AnimationBoneModifiers.find(Rotations.first);
		auto Type = Anim.AnimType;
		if (Modifier != Anim.AnimationBoneModifiers.end())
			Type = Modifier->second;

		switch (Type)
		{
		case WraithAnimationType::Absolute:
			Curve->SetProperty("m", "absolute"); break;
		case WraithAnimationType::Additive:
			Curve->SetProperty("m", "additive"); break;
		default:
			Curve->SetProperty("m", "relative"); break;
		}

		uint32_t LargestFrame = 0;

		for (auto& Rotation : Rotations.second)
			if (Rotation.Frame > LargestFrame)
				LargestFrame = Rotation.Frame;

		auto KeyFrameBufferType = LargestFrame <= 0xFFFF ? LargestFrame <= 0xFF ? CastPropertyId::Byte : CastPropertyId::Short : CastPropertyId::Integer32;

		auto KeyFrameBuffer = Curve->AddProperty("kb", KeyFrameBufferType);
		auto KeyValueBuffer = Curve->AddProperty("kv", CastPropertyId::Vector4);

		for (auto& Rotation : Rotations.second)
		{
			KeyValueBuffer->Write(Rotation.Value);

			switch (KeyFrameBufferType)
			{
			case CastPropertyId::Byte:
				KeyFrameBuffer->Write((uint8_t)Rotation.Frame); break;
			case CastPropertyId::Short:
				KeyFrameBuffer->Write((uint16_t)Rotation.Frame); break;
			case CastPropertyId::Integer32:
				KeyFrameBuffer->Write((uint32_t)Rotation.Frame); break;
			}
		}
	}

	for (auto& Note : Anim.AnimationNotetracks)
	{
		auto CastNote = CastAnim->AddNode(CastNodeId::NotificationTrack);
		CastNote->SetProperty("n", Note.first);
		auto KeyFrameBuffer = CastNote->AddProperty("kb", CastPropertyId::Integer32);
		for (auto& Key : Note.second)
			KeyFrameBuffer->Write(Key);
	}

	WriteCastFile(Writer, CastRoot);
}

CastNode::CastNode() : Identifier(CastNodeId::Root), Hash(0)
{
}

CastNode::CastNode(const CastNodeId id) : Identifier(id), Hash(0)
{

}

CastNode::CastNode(const CastNodeId id, const uint64_t hash) : Identifier(id), Hash(hash)
{
}

const size_t CastNode::Size() const
{
	size_t result = 24;

	for (auto& prop : Properties)
	{
		result += 8;
		result += prop.first.size();
		result += prop.second->Buffer.size();
	}

	for (auto& child : Children)
	{
		result += child->Size();
	}

	return result;
}

CastNode* CastNode::AddNode(const CastNodeId id)
{
	Children.emplace_back(std::make_unique<CastNode>(id));
	return Children.back().get();
}

CastNode* CastNode::AddNode(const CastNodeId id, const uint64_t hash)
{
	Children.emplace_back(std::make_unique<CastNode>(id, hash));
	return Children.back().get();
}
