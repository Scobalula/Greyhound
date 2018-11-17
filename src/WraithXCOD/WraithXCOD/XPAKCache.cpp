#include "stdafx.h"

// The class we are implementing
#include "XPAKCache.h"

// We need the following classes
#include "FileSystems.h"
#include "Strings.h"
#include "Compression.h"
#include "BinaryReader.h"
#include "MemoryReader.h"
#include "Siren.h"

// We need the game files structs
#include "DBGameFiles.h"

XPAKCache::XPAKCache()
{
	// Default, attempt to load the siren lib
	Siren::Initialize(L"oo2core_6_win64.dll");
}

XPAKCache::~XPAKCache()
{
	// Clean up if need be
	Siren::Shutdown();
}

void XPAKCache::LoadPackageCache(const std::string& BasePath)
{
	// Call Base function first!
	CoDPackageCache::LoadPackageCache(BasePath);

	// We need to enumerate all files in this path, load them, and aquire hashes of the names
	auto PathXPAKFiles = FileSystems::GetFiles(BasePath, "*.xpak");

	// Iterate over file paths
	for (auto& XPAKFile : PathXPAKFiles)
	{
		// Some of the smaller blank XPAKs are causing this to return false, since the bigger XPAKs are fine now from what I can tell, just skip the bad ones
		this->LoadPackage(XPAKFile);
	}

	// We've finished loading, set status
	this->SetLoadedState();
}

bool XPAKCache::LoadPackage(const std::string& FilePath)
{
	// Call Base function first
	CoDPackageCache::LoadPackage(FilePath);

	// Add to package files
	auto PackageIndex = (uint32_t)PackageFilePaths.size();

	// Open the file
	auto Reader = BinaryReader();
	// Open it
	Reader.Open(FilePath);

	// Read the header
	auto Header = Reader.Read<BO3XPakHeader>();

	// Verify the magic and offset
	if (Header.Magic == 0x4950414b && Header.HashOffset < Reader.GetLength())
	{
		// Jump to hash offset
		Reader.SetPosition(Header.HashOffset);

		// Hash result size
		uint64_t HashResult = 0;
		// Read the hash data into a buffer
		auto HashData = MemoryReader(Reader.Read(Header.HashCount * sizeof(BO3XPakHashEntry), HashResult), HashResult);

		// Loop and setup entries
		for (uint64_t i = 0; i < Header.HashCount; i++)
		{
			// Read it
			auto Entry = HashData.Read<BO3XPakHashEntry>();

			// Prepare a cache entry
			PackageCacheObject NewObject;
			// Set data
			NewObject.Offset = Entry.Offset;
			NewObject.CompressedSize = Entry.Size;
			NewObject.UncompressedSize = Header.DataOffset;
			NewObject.PackageFileIndex = PackageIndex;

			// Append to database
			CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
		}

		// Append the file path
		PackageFilePaths.push_back(FilePath);

		// No issues
		return true;
	}

	// Failed 
	return false;
}

std::unique_ptr<uint8_t[]> XPAKCache::ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize)
{
	// Prepare to extract if found
	if (CacheObjects.find(CacheID) != CacheObjects.end())
	{
		// Take cache data, and extract from the XPAK (Uncompressed size = offset of data segment!)
		auto& CacheInfo = CacheObjects[CacheID];
		// Get the XPAK name
		auto& XPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];

		// Open the file
		auto Reader = BinaryReader();
		// Open it
		Reader.Open(XPAKFileName);

		// Hop to the beginning offset
		Reader.SetPosition(CacheInfo.Offset + CacheInfo.UncompressedSize);

		// A buffer for data read
		uint64_t DataRead = 0;
		// A buffer for total size
		uint64_t TotalDataSize = 0;

		// A buffer for the data, this will eventually be shipped off, it's 32mb of memory
		auto DataTemporaryBuffer = new int8_t[0x2000000];

		// Loop until we have all our data
		while (DataRead < CacheInfo.CompressedSize)
		{
			// Read the block header
			auto BlockHeader = Reader.Read<BO3XPakDataHeader>();

			// Loop for block count
			for (uint32_t i = 0; i < BlockHeader.Count; i++)
			{
				// Unpack the command information
				uint64_t BlockSize = (BlockHeader.Commands[i] & 0xFFFFFF);
				uint32_t CompressedFlag = (BlockHeader.Commands[i] >> 24);

				// Get current position
				uint64_t CurrentPosition = Reader.GetPosition();

				// Check the block type (3 = compressed (lz4), 8 = compressed (oodle), 0 = raw data, anything else = skip over!)
				if (CompressedFlag == 0x3)
				{
					// Read result
					uint64_t ReadSize = 0;
					// Read the block
					auto DataBlock = Reader.Read(BlockSize, ReadSize);

					// Check if we read data
					if (DataBlock != nullptr)
					{
						// Decompress the LZ4 block
						auto Result = Compression::DecompressLZ4Block((const int8_t*)DataBlock, DataTemporaryBuffer + TotalDataSize, (uint32_t)BlockSize, 0x2000000);

						// Append size
						TotalDataSize += Result;

						// Clean up the buffer
						delete[] DataBlock;
					}
				}
				else if (CompressedFlag == 0x8)
				{
					// Read result
					uint64_t ReadSize = 0;
					// Read the block
					auto DataBlock = Reader.Read(BlockSize, ReadSize);

					// Check if we read data
					if (DataBlock != nullptr)
					{
						// Read oodle decompressed size
						uint32_t DecompressedSize = *(uint32_t*)(DataBlock);

						// Decompress the Oodle block
						auto Result = Siren::Decompress((const uint8_t*)DataBlock + 4, (uint32_t)BlockSize - 4, (uint8_t*)DataTemporaryBuffer + TotalDataSize, DecompressedSize);

						// Append size
						TotalDataSize += Result;

						// Clean up the buffer
						delete[] DataBlock;
					}
				}
				else if (CompressedFlag == 0x0)
				{
					// Read result
					uint64_t ReadSize = 0;
					// Read the block
					auto DataBlock = Reader.Read(BlockSize, ReadSize);

					// Check if we read data
					if (DataBlock != nullptr)
					{
						// We just need to append it
						std::memcpy(DataTemporaryBuffer + TotalDataSize, DataBlock, BlockSize);

						// Append size
						TotalDataSize += BlockSize;

						// Clean up the buffer
						delete[] DataBlock;
					}
				}
				else
				{
					// As far as we care, any other flag value is padding (0xCF is one of them)
					Reader.Advance(BlockSize);
				}

				// We must append the block size and pad it properly (If it's the last block)
				uint64_t NextSegmentOffset = 0;
				uint64_t TotalBlockSize = 0;

				// Calculate
				if ((i + 1) < BlockHeader.Count)
				{
					// Not the last, standard
					TotalBlockSize = BlockSize;
					NextSegmentOffset = (CurrentPosition + BlockSize);
				}
				else
				{
					// We must pad this
					NextSegmentOffset = (((CurrentPosition + BlockSize) + 0x7F) & 0xFFFFFFFFFFFFF80);
					TotalBlockSize += (NextSegmentOffset - CurrentPosition);
				}

				// Append to data read (Cus it includes padding)
				DataRead += TotalBlockSize;

				// Jump to next segment
				Reader.SetPosition(NextSegmentOffset);
			}

			// We must append the size of a header
			DataRead += sizeof(BO3XPakDataHeader);
		}

		// If we got here, the result size is totaldatasize, we need to allocate a safe buffer, copy, then clean up properly
		auto ResultBuffer = std::make_unique<uint8_t[]>((uint32_t)TotalDataSize);
		// Copy over the buffer
		std::memcpy(ResultBuffer.get(), DataTemporaryBuffer, TotalDataSize);

		// Clean up
		delete[] DataTemporaryBuffer;

		// Set result size
		ResultSize = (uint32_t)TotalDataSize;

		// Return the safe buffer
		return ResultBuffer;
	}

	// Set
	ResultSize = 0;

	// Failed to find data
	return nullptr;
}