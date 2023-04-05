#pragma once
#include <vector>

// A class to hold an XAnim Buffer.
class CoDXAnimBuffer
{
private:
	// The complete xanim buffer as 1 memory blob.
	std::unique_ptr<uint8_t[]> Buffer;
public:
	// The data byte array.
	void* DataBytes;
	// The data byte array.
	void* DataShorts;
	// The data byte array.
	void* DataInts;
	// The data byte array.
	void* RandomDataBytes;
	// The data byte array.
	void* RandomDataShorts;
	// The data byte array.
	void* RandomDataInts;
	// The data byte array.
	void* Indices;
};

// A class to handle reading a CoD XAnim.
class CoDXAnimReader
{
public:
	// Deletes the xanim reader.
	~CoDXAnimReader();

	// The array of bone names.
	std::vector<std::string> BoneNames;
	// The array of notetracks by frame index.
	std::vector<std::pair<std::string, size_t>> Notetracks;
	// The data byte array.
	std::unique_ptr<uint8_t[]> DataBytes;
	// The data byte array.
	std::unique_ptr<uint8_t[]> DataShorts;
	// The data byte array.
	std::unique_ptr<uint8_t[]> DataInts;
	// The data byte array.
	std::unique_ptr<uint8_t[]> RandomDataBytes;
	// The data byte array.
	std::unique_ptr<uint8_t[]> RandomDataShorts;
	// The data byte array.
	std::unique_ptr<uint8_t[]> RandomDataInts;
	// The data byte array.
	std::unique_ptr<uint8_t[]> Indices;

	// Gets the buffer from the anim.
	uint8_t* GetBuffer();
};

#pragma once
