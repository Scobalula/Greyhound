#pragma once

#include <cstdint>

// -- Contains structures for various game file formats

#pragma region Black Ops 2

#pragma pack(push, 1)
struct BO2IPakHeader
{
	uint32_t Magic;
	uint32_t Version;
	uint32_t Size;
	uint32_t SegmentCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2IPakSegment
{
	uint32_t Type;
	uint32_t Offset;
	uint32_t Size;
	uint32_t EntryCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2IPakDataHeader
{
	// Count and offset are packed into a single integer
	uint32_t Offset : 24;
	uint32_t Count : 8;

	// The commands tell what each block of data does
	uint32_t Commands[31];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO2IPakEntry
{
	uint64_t Key;
	uint32_t Offset;
	uint32_t Size;
};
#pragma pack(pop)

#pragma endregion

#pragma region Black Ops 3

#pragma pack(push, 1)
struct BO3XPakHeader
{
	uint32_t Magic;
	uint16_t Unknown1;
	uint16_t Version;
	uint64_t Unknown2;
	uint64_t Size;
	uint64_t FileCount;
	uint64_t DataOffset;
	uint64_t DataSize;
	uint64_t HashCount;
	uint64_t HashOffset;
	uint64_t HashSize;
	uint64_t Unknown3;
	uint64_t UnknownOffset;
	uint64_t Unknown4;
	uint64_t IndexCount;
	uint64_t IndexOffset;
	uint64_t IndexSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XPakHashEntry
{
	uint64_t Key;
	uint64_t Offset;
	uint64_t Size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BO3XPakDataHeader
{
	// Count and offset
	uint32_t Count;
	uint32_t Offset;

	// The commands tell what each block of data does
	uint32_t Commands[30];
};
#pragma pack(pop)

#pragma endregion

#pragma region WWII

#pragma pack(push, 1)
struct WWIIXPTocHeader
{
	uint64_t Magic;
	uint32_t Version;
	uint32_t EntriesCount;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct WWIIXPTocEntry
{
	char Hash[16];
	uint64_t Offset;
	uint32_t Size;
	uint16_t PackageIndex;
};
#pragma pack(pop)

#pragma endregion

#pragma region SABStructures

#pragma pack(push, 1)
struct SABFileHeader
{
	uint32_t Magic;
	uint32_t Version;

	uint32_t SizeOfAudioEntry;
	uint32_t SizeOfHashEntry;
	uint32_t SizeOfNameEntry;

	uint32_t EntriesCount;

	uint8_t UnknownData[8];

	uint64_t FileSize;

	uint64_t EntryTableOffset;
	uint64_t HashTableOffset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SABv4Entry
{
	uint32_t Key;
	uint32_t Size;
	uint32_t SeekTableLength;
	uint32_t FrameCount;

	uint32_t Unknown1;

	uint64_t Offset;
	
	uint32_t FrameRate;
	uint8_t ChannelCount;

	uint8_t Looping;
	uint8_t Format;

	uint8_t Padding[9];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SABv14Entry
{
	uint32_t Key;
	uint32_t Size;
	uint32_t Offset;
	uint32_t FrameCount;
	uint8_t FrameRateIndex;
	uint8_t ChannelCount;
	uint8_t Looping;
	uint8_t Format;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SABv15Entry
{
	uint32_t Key;
	uint32_t Size;
	uint32_t FrameCount;

	uint32_t Unknown1;

	uint64_t Offset;

	uint8_t FrameRateIndex;
	uint8_t ChannelCount;
	uint8_t Looping;
	uint8_t Format;

	uint8_t Padding[8];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SABv21Entry
{
	uint64_t Key;
	uint64_t Unknown1;
	uint64_t Offset;

	uint32_t Size;
	uint32_t FrameCount;

	uint64_t Unknown2;	

	uint8_t FrameRateIndex;
	uint8_t ChannelCount;
	uint8_t Looping;
	uint8_t Format;

	uint8_t Padding[4];
};
#pragma pack(pop)

#pragma endregion