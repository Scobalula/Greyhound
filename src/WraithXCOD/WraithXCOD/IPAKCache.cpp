#include "stdafx.h"

// The class we are implementing
#include "IPAKCache.h"

// We need the following classes
#include "FileSystems.h"
#include "Strings.h"
#include "Compression.h"
#include "BinaryReader.h"

// We need the game files structs
#include "DBGameFiles.h"

IPAKCache::IPAKCache()
{
	// Defaults
}

IPAKCache::~IPAKCache()
{
	// Clean up if need be
}

void IPAKCache::LoadPackageCache(const std::string& BasePath)
{
	// Call Base function first!
	CoDPackageCache::LoadPackageCache(BasePath);

	// We need to enumerate all files in this path, load them, and aquire hashes of the names
	auto PathIPAKFiles = FileSystems::GetFiles(BasePath, "*.ipak");

	// Iterate over file paths
	for (auto& IPAKFile : PathIPAKFiles)
	{
		// Load it
		this->LoadPackage(IPAKFile);
	}

	// We've finished loading, set status
	this->SetLoadedState();
}

void IPAKCache::LoadPackage(const std::string& FilePath)
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
	auto Header = Reader.Read<BO2IPakHeader>();

	// Verify the magic
	if (Header.Magic == 0x4950414b)
	{
		// We have a valid file, prepare to read segments
		auto IPAKEntriesSegment = BO2IPakSegment();
		auto IPAKDataSegment = BO2IPakSegment();

		// Read segments until we find the info one
		for (uint32_t i = 0; i < Header.SegmentCount; i++)
		{
			// Read segment
			auto Segment = Reader.Read<BO2IPakSegment>();

			// Check for the info one, then break if found
			if (Segment.Type == 1) { IPAKEntriesSegment = Segment; }
			if (Segment.Type == 2) { IPAKDataSegment = Segment; }
		}

		// Jump to the offset, if we are sure it's right
		if (IPAKEntriesSegment.Type == 1)
		{
			// Hop to the offset
			Reader.SetPosition(IPAKEntriesSegment.Offset);

			// Loop and read entries
			for (uint32_t i = 0; i < IPAKEntriesSegment.EntryCount; i++)
			{
				// Read entry
				auto Entry = Reader.Read<BO2IPakEntry>();

				// Prepare a cache entry
				PackageCacheObject NewObject;
				// Set data
				NewObject.Offset = Entry.Offset;
				NewObject.CompressedSize = Entry.Size;
				NewObject.UncompressedSize = IPAKDataSegment.Offset;
				NewObject.PackageFileIndex = PackageIndex;

				// Append to database
				CacheObjects.insert(std::make_pair(Entry.Key, NewObject));
			}
		}

		// Append the file path
		PackageFilePaths.push_back(FilePath);
	}
}

std::unique_ptr<uint8_t[]> IPAKCache::ExtractPackageObject(uint64_t CacheID, uint32_t& ResultSize)
{
	// Prepare to extract if found
	if (CacheObjects.find(CacheID) != CacheObjects.end())
	{
		// Take cache data, and extract from the IPAK (Uncompressed size = offset of data segment!)
		auto& CacheInfo = CacheObjects[CacheID];
		// Get the IPAK name
		auto& IPAKFileName = PackageFilePaths[CacheInfo.PackageFileIndex];

		// Open the file
		auto Reader = BinaryReader();
		// Open it
		Reader.Open(IPAKFileName);

		// Hop to the beginning offset
		Reader.SetPosition(CacheInfo.Offset + CacheInfo.UncompressedSize);

		// A buffer for data read
		uint64_t DataRead = 0;
		// A buffer for total size
		uint64_t TotalDataSize = 0;

		// A buffer for the data, this will eventually be shipped off, it's 16mb of memory
		auto DataTemporaryBuffer = new int8_t[0x1000000];

		// Loop until we have all our data
		while (DataRead < CacheInfo.CompressedSize)
		{
			// Read the block header
			auto BlockHeader = Reader.Read<BO2IPakDataHeader>();

			// Loop for block count
			for (uint32_t i = 0; i < BlockHeader.Count; i++)
			{
				// Unpack the command information
				uint64_t BlockSize = (BlockHeader.Commands[i] & 0xFFFFFF);
				uint32_t CompressedFlag = (BlockHeader.Commands[i] >> 24);

				// Get current position
				uint64_t CurrentPosition = Reader.GetPosition();
				
				// Check the block type (1 = compressed, 0 = raw data, anything else = skip over!)
				if (CompressedFlag == 0x1)
				{
					// Read result
					uint64_t ReadSize = 0;
					// Read the block
					auto DataBlock = Reader.Read(BlockSize, ReadSize);

					// Check if we read data
					if (DataBlock != nullptr)
					{
						// Decompress the LZO block
						auto Result = Compression::DecompressLZO1XBlock((const int8_t*)DataBlock, DataTemporaryBuffer + TotalDataSize, (uint32_t)BlockSize, 0x1000000);

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
			DataRead += sizeof(BO2IPakDataHeader);
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