#include "stdafx.h"

// The class we are implementing.
#include "CoDCDNCache.h"

#include "Strings.h"
#include "FileSystems.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"

CoDCDNCache::CoDCDNCache()
{
	Loaded = false;
}

bool CoDCDNCache::Load(const std::string& name)
{
	std::string cdnDir = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "cdn_cache");

	// Set up CDN Cache file paths for future use.
	InfoFilePath = FileSystems::CombinePath(cdnDir, name + ".cdn_info");
	DataFilePath = FileSystems::CombinePath(cdnDir, name + ".cdn_data");

	FileSystems::CreateDirectory(cdnDir);

	// At this point we are loaded, we've created the required information for
	// the build methods to work with.
	Loaded = true;

	BinaryReader reader;

	// Verify our info file can be opened and is valid, not necessarily a fatal
	// issue as we'll generate it later.
	if (!reader.Open(InfoFilePath))
		return false;
	if (reader.Read<uint32_t>() != 0x494E4443)
		return false;
	if (reader.Read<uint32_t>() != 1)
		return false;

	auto numEntries = reader.Read<uint32_t>();

	for (uint32_t i = 0; i < numEntries; i++)
	{
		auto entry = reader.Read<CoDCDNCacheEntry>();
		Entries[entry.Hash] = entry;
	}
}

bool CoDCDNCache::Save()
{
	if (!Loaded)
		return false;

	BinaryWriter writer;

	if (!writer.Create(InfoFilePath))
		return false;

	writer.Write<uint32_t>(0x494E4443);
	writer.Write<uint32_t>(1);
	writer.Write<uint32_t>((uint32_t)Entries.size());

	for (auto& entry : Entries)
	{
		writer.Write(entry.second);
	}
}

bool CoDCDNCache::Add(const uint64_t hash, const uint8_t* buffer, const size_t bufferSize)
{
	if (!Loaded)
		return false;

	BinaryWriter writer;

	if (!writer.Open(DataFilePath))
		return false;

	CoDCDNCacheEntry entry{};

	entry.Hash = hash;
	entry.Offset = writer.GetLength();;
	entry.Size = bufferSize;

	Entries[entry.Hash] = entry;

	if (!Save())
	{
		return false;
	}

	// Deliver the payload to the data file.
	writer.SetPosition(entry.Offset);
	writer.Write(buffer, (uint32_t)bufferSize);
	return true;
}

std::unique_ptr<uint8_t[]> CoDCDNCache::Extract(const uint64_t hash, uint32_t expectedSize, size_t& bufferSize)
{
	if (!Loaded)
		return false;

	auto find = Entries.find(hash);

	if (find == Entries.end())
		return nullptr;

	auto entry = find->second;

	// If we have a size mismatch then something happened either
	// when downloading from CDN or the size of this entry has changed
	if (entry.Size != expectedSize)
		return nullptr;

	BinaryReader reader;

	if (!reader.Open(DataFilePath))
		return false;

	auto result = std::make_unique<uint8_t[]>(entry.Size);

	reader.SetPosition(entry.Offset);

	reader.Read(result.get(), entry.Size, bufferSize);

	if (bufferSize != entry.Size)
	{
		bufferSize = 0;
		return nullptr;
	}

	return result;
}