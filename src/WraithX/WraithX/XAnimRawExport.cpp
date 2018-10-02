#include "stdafx.h"

// The class we are implementing
#include "XAnimRawExport.h"

// We need the binarywriter class
#include "BinaryWriter.h"

// Begin structures for writing

struct XAnimFlags
{
	bool Looping : 1;
	bool IsDelta : 1;
	bool LeftHandGripIK : 1;
	bool Streamable : 1;
	bool Reserved1 : 1;
	bool Reserved2 : 1;
	bool Reserved3 : 1;
	bool Reserved4 : 1;

	XAnimFlags()
	{
		// Defaults
		Looping = false;
		IsDelta = false;
		LeftHandGripIK = false;
		Streamable = false;
		Reserved1 = false;
		Reserved2 = false;
		Reserved3 = false;
		Reserved4 = false;
	}
};

struct XAnimHeader
{
	// The version number (either 17 or 19)
	uint16_t Version;
	// The total number of frames this animation can have
	uint16_t NumFrames;
	// The total number of animated bones
	uint16_t NumBones;
	// Flags that specify animation data
	XAnimFlags Flags;
	// The asset type configuration
	uint8_t AssetType;
	// The framerate this animation should play at
	uint16_t FrameRate;

	XAnimHeader()
	{
		// Defaults
		Version = 17;
		NumFrames = 0;
		NumBones = 0;
		AssetType = 0;
		FrameRate = 30;
	}
};

static_assert(sizeof(XAnimFlags) == 1, "XAnimFlags size mismatch, expected size of 1");
static_assert(sizeof(XAnimHeader) == 0xA, "XAnimHeader size mismatch, expected size of 10");

// End structures

void XAnimProduceQuaternions(BinaryWriter& Writer, uint32_t FrameSize, uint32_t LoopedFrameCount, const std::vector<WraithAnimFrame<Quaternion>>& QuatFrames, bool SimpleQuaternion = false)
{
	// Write all the frame indicies first (If we have more than 1 frame)
	if (QuatFrames.size() > 1 && QuatFrames.size() != LoopedFrameCount)
	{
		for (auto& Key : QuatFrames)
		{
			switch (FrameSize)
			{
			case 1: Writer.Write<uint8_t>((uint8_t)Key.Frame); break;
			case 2: Writer.Write<uint16_t>((uint16_t)Key.Frame); break;
			}
		}
	}

	// Loop and write unit (Normalized) quaternions
	for (auto& Key : QuatFrames)
	{
		auto QuaternionFrame = Key.Value;
		QuaternionFrame.Normalize();

		int16_t QuatX = (int16_t)(32767.0 * QuaternionFrame.X);
		int16_t QuatY = (int16_t)(32767.0 * QuaternionFrame.Y);
		int16_t QuatZ = (int16_t)(32767.0 * QuaternionFrame.Z);

		if (QuaternionFrame.W < 0.0)
		{
			QuatX = -QuatX;
			QuatY = -QuatY;
			QuatZ = -QuatZ;
		}

		if (!SimpleQuaternion)
		{
			Writer.Write<int16_t>(QuatX);
			Writer.Write<int16_t>(QuatY);
		}
		Writer.Write<int16_t>(QuatZ);
	}
}

void XAnimProduceTranslations(BinaryWriter& Writer, uint32_t FrameSize, uint32_t LoopedFrameCount, const std::vector<WraithAnimFrame<Vector3>>& PosFrames)
{
	// Write all the frame indicies first (If we have more than 1 frame)
	if (PosFrames.size() > 1 && PosFrames.size() != LoopedFrameCount)
	{
		for (auto& Key : PosFrames)
		{
			switch (FrameSize)
			{
			case 1: Writer.Write<uint8_t>((uint8_t)Key.Frame); break;
			case 2: Writer.Write<uint16_t>((uint16_t)Key.Frame); break;
			}
		}
	}

	if (PosFrames.size() == 1)
	{
		// We just write the translation
		Writer.Write<float>(PosFrames[0].Value.X);
		Writer.Write<float>(PosFrames[0].Value.Y);
		Writer.Write<float>(PosFrames[0].Value.Z);
	}
	else
	{
		// We must compress the translation
		float MinX = 0, MinY = 0, MinZ = 0;
		float SizeX = 1, SizeY = 1, SizeZ = 1;

		// Loop and calculate minimum
		for (auto& Key : PosFrames)
		{
			if (Key.Value.X < MinX) { MinX = Key.Value.X; }
			if (Key.Value.Y < MinY) { MinY = Key.Value.Y; }
			if (Key.Value.Z < MinZ) { MinZ = Key.Value.Z; }
		}
		
		// Loop and calculate size
		for (auto& Key : PosFrames)
		{
			float CalcX = (Key.Value.X - MinX) / SizeX;

			while ((CalcX * 65535.0) > 65535.0)
			{
				SizeX += 0.025f;
				CalcX = (Key.Value.X - MinX) / SizeX;
			}

			float CalcY = (Key.Value.Y - MinY) / SizeY;

			while ((CalcY * 65535.0) > 65535.0)
			{
				SizeY += 0.025f;
				CalcY = (Key.Value.Y - MinY) / SizeY;
			}

			float CalcZ = (Key.Value.Z - MinZ) / SizeZ;

			while ((CalcZ * 65535.0) > 65535.0)
			{
				SizeZ += 0.025f;
				CalcZ = (Key.Value.Z - MinZ) / SizeZ;
			}
		}

		// Output min/size table
		Writer.Write<uint8_t>(0);	// Precise Translations (Short)
		Writer.Write<float>(MinX);
		Writer.Write<float>(MinY);
		Writer.Write<float>(MinZ);
		Writer.Write<float>(SizeX);
		Writer.Write<float>(SizeY);
		Writer.Write<float>(SizeZ);

		// Output translations
		for (auto& Key : PosFrames)
		{
			float CalcX = (Key.Value.X - MinX) / SizeX;
			float CalcY = (Key.Value.Y - MinY) / SizeY;
			float CalcZ = (Key.Value.Z - MinZ) / SizeZ;

			Writer.Write<uint16_t>((uint16_t)(CalcX * 65535.0));
			Writer.Write<uint16_t>((uint16_t)(CalcY * 65535.0));
			Writer.Write<uint16_t>((uint16_t)(CalcZ * 65535.0));
		}
	}
}

void XAnimRaw::ExportXAnimRaw(const WraithAnim& Animation, const std::string& FileName, XAnimRawVersion Version)
{
	// Create a new writer
	auto Writer = BinaryWriter();
	// Open the file (Read / Write mode)
	Writer.Create(FileName, true);

	// Create the header and apply information
	XAnimHeader Header;
	Header.Version = (uint16_t)Version;
	Header.FrameRate = (uint16_t)Animation.FrameRate;

	// We must fetch counts of each buffer, since it's automatically generated, and delta data effects it.
	uint32_t FrameCountBuffer = Animation.FrameCount();
	// Bone count buffer
	uint32_t BoneCountBuffer = Animation.BoneCount();
	// The notification buffer
	uint32_t NotificationBuffer = Animation.NotificationCount();

	// Get the bone tag names
	auto BoneTagNames = Animation.Bones();

	// Specific flags
	if (Animation.Looping)
	{
		// Remove a frame, then set looping
		FrameCountBuffer--;
		Header.Flags.Looping = true;
	}

	// Apply count information
	Header.NumFrames = (uint16_t)FrameCountBuffer;
	Header.NumBones = (uint16_t)BoneCountBuffer;

	// Animation type specific information
	switch (Animation.AnimType)
	{
	case WraithAnimationType::Additive: Header.AssetType = 0; break;
	case WraithAnimationType::Absolute: Header.AssetType = 1; break;
	case WraithAnimationType::Relative: Header.AssetType = 2; break;
	case WraithAnimationType::Delta:
		Header.AssetType = 2;
		Header.Flags.IsDelta = true;
		Header.NumBones--;
		BoneTagNames.erase(Animation.DeltaTagName);
		break;
	}

	// Write the header
	Writer.Write((int8_t*)&Header, sizeof(XAnimHeader));

	// Calculate the size of data flags
	uint32_t FlagBufferSize = (((Header.NumBones - 1) >> 3) + 1);

	// Prepare temporary data so we can overwrite it later
	auto QuaternionFlipFlags = std::make_unique<uint8_t[]>(FlagBufferSize);
	auto SimpleQuaternionFlags = std::make_unique<uint8_t[]>(FlagBufferSize);

	// Clear it
	std::memset(QuaternionFlipFlags.get(), 0, FlagBufferSize);
	std::memset(SimpleQuaternionFlags.get(), 0, FlagBufferSize);

	// The size of a frame index
	auto FrameSize = (FrameCountBuffer > 255) ? 2 : 1;
	// The count of looped frames (Count + 1)
	auto LoopedFrameCount = (Header.Flags.Looping) ? (FrameCountBuffer + 1) : (FrameCountBuffer);

	// Prepare to write the delta frame data
	if (Header.Flags.IsDelta)
	{
		if (Animation.AnimationRotationKeys.find(Animation.DeltaTagName) != Animation.AnimationRotationKeys.end())
		{
			auto& BoneRotations = Animation.AnimationRotationKeys.at(Animation.DeltaTagName);
			// Write count
			Writer.Write<uint16_t>((uint16_t)BoneRotations.size());

			XAnimProduceQuaternions(Writer, FrameSize, LoopedFrameCount, BoneRotations, true);
		}
		else
		{
			// No delta rotations
			Writer.Write<uint16_t>(0);
		}

		if (Animation.AnimationPositionKeys.find(Animation.DeltaTagName) != Animation.AnimationPositionKeys.end())
		{
			// We have translation keys
			auto& BoneTranslations = Animation.AnimationPositionKeys.at(Animation.DeltaTagName);
			// Write count
			Writer.Write<uint16_t>((uint16_t)BoneTranslations.size());

			XAnimProduceTranslations(Writer, FrameSize, LoopedFrameCount, BoneTranslations);
		}
		else
		{
			// No delta translations
			Writer.Write<uint16_t>(0);
		}
	}

	// Store the current position to update later
	auto QuaternionFlagsPosition = Writer.GetPosition();

	// Write it
	Writer.Write(QuaternionFlipFlags.get(), FlagBufferSize);
	Writer.Write(SimpleQuaternionFlags.get(), FlagBufferSize);

	// Write the bone tags
	for (auto& Bone : BoneTagNames)
	{
		// Write the null-term string
		Writer.Write((uint8_t*)Bone.c_str(), (uint32_t)(Bone.size() + 1));
	}

	// Store the byte flag index
	int32_t SimpleQuaternionByteFlag = 0;

	// For all real bones, we must loop and write their data
	for (auto& Bone : BoneTagNames)
	{
		if (Animation.AnimationRotationKeys.find(Bone) != Animation.AnimationRotationKeys.end())
		{
			// We have rotation segment
			auto& BoneRotations = Animation.AnimationRotationKeys.at(Bone);
			// Write count
			Writer.Write<uint16_t>((uint16_t)BoneRotations.size());

			// Determine if we have actual quaternions
			if (BoneRotations.size() > 0)
			{
				XAnimProduceQuaternions(Writer, FrameSize, LoopedFrameCount, BoneRotations);
			}
			else
			{
				// Mark as a simple quaternion
				SimpleQuaternionFlags[(SimpleQuaternionByteFlag >> 3)] |= (1 << (SimpleQuaternionByteFlag & 7));
			}
		}
		else
		{
			// We don't have any (Mark as a simple quaternion)
			SimpleQuaternionFlags[(SimpleQuaternionByteFlag >> 3)] |= (1 << (SimpleQuaternionByteFlag & 7));
			Writer.Write<uint16_t>(0);
		}

		if (Animation.AnimationPositionKeys.find(Bone) != Animation.AnimationPositionKeys.end())
		{
			// We have translation keys
			auto& BoneTranslations = Animation.AnimationPositionKeys.at(Bone);
			// Write count
			Writer.Write<uint16_t>((uint16_t)BoneTranslations.size());

			XAnimProduceTranslations(Writer, FrameSize, LoopedFrameCount, BoneTranslations);
		}
		else
		{
			// We don't have any
			Writer.Write<uint16_t>(0);
		}

		// Advance the flag byte
		SimpleQuaternionByteFlag++;
	}

	// Write notetracks
	for (auto& Note : Animation.AnimationNotetracks)
	{
		if (Note.first == "end") { NotificationBuffer--; }
	}

	// Write the count
	Writer.Write<uint8_t>((uint8_t)NotificationBuffer);

	for (auto& Note : Animation.AnimationNotetracks)
	{
		if (Note.first != "end")
		{
			for (auto& NoteFrame : Note.second)
			{
				Writer.Write((uint8_t*)Note.first.c_str(), (uint32_t)(Note.first.size() + 1));
				Writer.Write<uint16_t>((uint16_t)NoteFrame);
			}
		}
	}

	// Seek back and rewrite the flags
	Writer.SetPosition(QuaternionFlagsPosition);
	Writer.Write(QuaternionFlipFlags.get(), FlagBufferSize);
	Writer.Write(SimpleQuaternionFlags.get(), FlagBufferSize);
}