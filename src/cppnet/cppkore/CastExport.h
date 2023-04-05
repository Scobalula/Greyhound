#pragma once

#include <string>
#include <cstdint>

// We need to include external libraries for models
#include "WraithModel.h"
#include "WraithAnim.h"
#include <map>
#include <memory>

// A class that handles writing Cast model/anim files
class Cast
{
public:
	// Export a WraithModel to a Cast file
	static void ExportCastModel(const WraithModel& Model, const std::string& FileName, bool SupportsScale = false);
	// Export a WraithAnim to a Cast file
	static void ExportCastAnim(const WraithAnim& Anim, const std::string& FileName, bool SupportsScale = false);
};

enum class CastNodeId : uint32_t
{
	Root = 0x746F6F72,
	Model = 0x6C646F6D,
	Mesh = 0x6873656D,
	BlendShape = 0x68736C62,
	Skeleton = 0x6C656B73,
	Bone = 0x656E6F62,
	Animation = 0x6D696E61,
	Curve = 0x76727563,
	NotificationTrack = 0x6669746E,
	Material = 0x6C74616D,
	File = 0x656C6966,
};

enum class CastPropertyId : uint16_t
{
	Byte = 'b',
	Short = 'h',
	Integer32 = 'i',
	Integer64 = 'l',
	Float = 'f',
	Double = 'd',
	String = 's',
	Vector2 = 'v2',
	Vector3 = 'v3',
	Vector4 = 'v4'
};

class CastProperty
{
public:
	CastPropertyId Identifier;
	size_t Elements;
	std::vector<char> Buffer;

	CastProperty() : Identifier(CastPropertyId::Byte), Elements(0) {}

	template <class T>
	void Write(const T data)
	{
		Elements++;
		// Quick and dirty adding data to vector for now
		// TODO: Swap out for expand + memcpy or idk xoxoxoxo
		auto asChar = (const unsigned char*)&data;
		for (size_t i = 0; i < sizeof(T); i++)
		{
			Buffer.push_back(asChar[i]);
		}
	}

	void Write(const char* data, const size_t len)
	{
		Elements++;
		// Quick and dirty adding data to vector for now
		// TODO: Swap out for expand + memcpy or idk xoxoxoxo
		for (size_t i = 0; i < len; i++)
		{
			Buffer.push_back(data[i]);
		}
	}
};

class CastNode
{
public:
	CastNodeId Identifier;
	uint64_t Hash;
	std::map<std::string, std::unique_ptr<CastProperty>> Properties;
	std::vector<std::unique_ptr<CastNode>> Children;

	CastNode();
	CastNode(const CastNodeId id);
	CastNode(const CastNodeId id, const uint64_t hash);

	CastProperty* AddProperty(const std::string& propName, const CastPropertyId id)
	{
		auto prop = std::make_unique<CastProperty>();
		auto result = prop.get();
		prop->Identifier = id;
		Properties[propName] = std::move(prop);
		return result;
	}

	CastProperty* AddProperty(const std::string& propName, const CastPropertyId id, const size_t capacity)
	{
		auto prop = std::make_unique<CastProperty>();
		auto result = prop.get();
		prop->Identifier = id;
		prop->Buffer.reserve(capacity);
		Properties[propName] = std::move(prop);
		return result;
	}

	void SetProperty(const std::string& propName, const std::string& value)
	{
		auto prop = std::make_unique<CastProperty>();
		prop->Identifier = CastPropertyId::String;
		prop->Write(value.c_str(), value.length() + 1);
		Properties[propName] = std::move(prop);
	}

	template <class T>
	void SetProperty(const std::string& propName, const CastPropertyId id, const T value)
	{
		auto prop = std::make_unique<CastProperty>();
		prop->Identifier = id;
		prop->Write<T>(value);
		Properties[propName] = std::move(prop);
	}

	const size_t Size() const;

	CastNode* AddNode(const CastNodeId id);
	CastNode* AddNode(const CastNodeId id, const uint64_t hash);
};