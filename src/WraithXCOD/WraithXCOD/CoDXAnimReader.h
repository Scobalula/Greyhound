#pragma once

// A class to handle reading a CoD XAnim.
class CoDXAnimReader
{
private:
	// The entire xanim data buffer.
	uint8_t* Buffer;
	// The size of the xanim data buffer.
	size_t BufferSize;
	// Whether or not we own the buffer.
	bool OwnsBuffer;


public:
	// Initializes the CoD Reader with a buffer.
	CoDXAnimReader(uint8_t* Buf, size_t BufSize, bool OwnsBuf);
	// Deletes the xanim reader.
	~CoDXAnimReader();

	// The array of bone names.
	std::vector<std::string> BoneNames;
	// The data byte array.
	uint8_t* DataBytes;
	// The data byte array.
	uint8_t* DataShorts;
	// The data byte array.
	uint8_t* DataInts;
	// The data byte array.
	uint8_t* RandomDataBytes;
	// The data byte array.
	uint8_t* RandomDataShorts;
	// The data byte array.
	uint8_t* RandomDataInts;
	// The data byte array.
	uint8_t* Indices;

	// Gets the buffer from the anim.
	uint8_t* GetBuffer();
};

