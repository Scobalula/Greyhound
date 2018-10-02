#include "stdafx.h"

// The class we are implementing
#include "SEAnimExport.h"

// We need the binarywriter class
#include "BinaryWriter.h"

// Specifies how the data is interpreted by the importer
enum class SEAnimAnimationType : uint8_t
{
	// Animation translations are set to this exact value each frame
	SEANIM_ABSOLUTE = 0,
	// This animation is applied to existing animation data in the scene
	SEANIM_ADDITIVE = 1,
	// Animation translations are based on rest position in scene
	SEANIM_RELATIVE = 2,
	// This animation is relative and contains delta data (Whole model movement) Delta tag name must be set!
	SEANIM_DELTA = 3
};

// Specifies the data present for each frame of every bone (Internal use only, matches specification v1.0.1)
enum class SEAnimDataPresenceFlags : uint8_t
{
	// These describe what type of keyframe data is present for the bones
	SEANIM_BONE_LOC = 1 << 0,
	SEANIM_BONE_ROT = 1 << 1,
	SEANIM_BONE_SCALE = 1 << 2,

	// If any of the above flags are set, then bone keyframe data is present, thus this comparing against this mask will return true
	SEANIM_PRESENCE_BONE = SEANIM_BONE_LOC | SEANIM_BONE_ROT | SEANIM_BONE_SCALE,

	// The file contains notetrack data
	SEANIM_PRESENCE_NOTE = 1 << 6,
	// The file contains a custom data block
	SEANIM_PRESENCE_CUSTOM = 1 << 7,
};

void SEAnim::ExportSEAnim(const WraithAnim& Animation, const std::string& FileName)
{
	// Create a new writer
	auto Writer = BinaryWriter();
	// Open the file
	Writer.Create(FileName);
	// The SEAnim magic
	char Magic[6] = { 'S', 'E', 'A', 'n', 'i', 'm' };
	// Write the magic
	Writer.Write(Magic);
	// Write the version
	Writer.Write<uint16_t>(0x1);
	// Write the header size
	Writer.Write<uint16_t>(0x1C);
	// Write the animation type
	switch (Animation.AnimType)
	{
	case WraithAnimationType::Absolute:
		Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_ABSOLUTE);
		break;
	case WraithAnimationType::Relative:
		Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_RELATIVE);
		break;
	case WraithAnimationType::Additive:
		Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_ADDITIVE);
		break;
	case WraithAnimationType::Delta:
		Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_DELTA);
		break;
	}
	// Write flags (Looped is the only flag for now)
	Writer.Write<uint8_t>(Animation.Looping ? (1 << 0) : 0);
	// Build data present flags
	{
		// Buffer
		uint8_t DataPresentFlags = 0x0;
		// Check for translations
		if (Animation.AnimationPositionKeys.size() > 0)
		{
			DataPresentFlags |= (uint8_t)SEAnimDataPresenceFlags::SEANIM_BONE_LOC;
		}
		// Check for rotations
		if (Animation.AnimationRotationKeys.size() > 0)
		{
			DataPresentFlags |= (uint8_t)SEAnimDataPresenceFlags::SEANIM_BONE_ROT;
		}
		// Check for scales
		if (Animation.AnimationScaleKeys.size() > 0)
		{
			DataPresentFlags |= (uint8_t)SEAnimDataPresenceFlags::SEANIM_BONE_SCALE;
		}
		// Check for notetracks
		if (Animation.AnimationNotetracks.size() > 0)
		{
			DataPresentFlags |= (uint8_t)SEAnimDataPresenceFlags::SEANIM_PRESENCE_NOTE;
		}
		// Write it
		Writer.Write<uint8_t>(DataPresentFlags);
	}
	// Write data property flags (Precision is the only one for now)
	Writer.Write<uint8_t>(0);
	// Write two reserved bytes
	Writer.Write<uint16_t>(0);
	// Write the framerate
	Writer.Write<float>(Animation.FrameRate);
	// We must fetch counts of each buffer, since it's automatically generated
	uint32_t FrameCountBuffer = Animation.FrameCount();
	// Bone count buffer
	uint32_t BoneCountBuffer = Animation.BoneCount();
	// The notification buffer
	uint32_t NotificationBuffer = Animation.NotificationCount();
	// Write count of frames
	Writer.Write<uint32_t>(FrameCountBuffer);
	// Write count of bones
	Writer.Write<uint32_t>(BoneCountBuffer);
	// Write modifier count
	Writer.Write<uint8_t>((uint8_t)Animation.AnimationBoneModifiers.size());
	// Write 3 reserved bytes
	uint8_t Reserved[3] = { 0x0, 0x0, 0x0 };
	// Write it
	Writer.Write(Reserved);
	// Write notification count
	Writer.Write<uint32_t>(NotificationBuffer);
	// Build unique tag data, in the order we need
	{
		// Get the bone tags present in the animation
		auto BoneTags = Animation.Bones();
		// Loop and write the tags
		for (auto& Tag : BoneTags)
		{
			// Write it
			Writer.WriteNullTerminatedString(Tag);
		}
		// Loop through modifiers
		for (auto& Modifier : Animation.AnimationBoneModifiers)
		{
			// Calculate the distance
			int16_t Index = (uint16_t)std::distance(BoneTags.begin(), BoneTags.find(Modifier.first));
			// Write the modifier
			if (BoneCountBuffer <= 0xFF)
			{
				// Write it as a byte
				Writer.Write<uint8_t>((uint8_t)Index);
			}
			else
			{
				// Write it as a short
				Writer.Write<uint16_t>((uint16_t)Index);
			}
			// Write the modifier value
			switch (Modifier.second)
			{
			case WraithAnimationType::Absolute:
				Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_ABSOLUTE);
				break;
			case WraithAnimationType::Relative:
				Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_RELATIVE);
				break;
			case WraithAnimationType::Additive:
				Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_ADDITIVE);
				break;
			case WraithAnimationType::Delta:
				Writer.Write<uint8_t>((uint8_t)SEAnimAnimationType::SEANIM_DELTA);
				break;
			}
		}
		// We must write the key info in the order of BoneTags
		for (auto& Tag : BoneTags)
		{
			// Write bone flags, 0 for now
			Writer.Write<uint8_t>(0x0);

			// Write translation keys first (if we have any)
			if (Animation.AnimationPositionKeys.size() > 0)
			{
				// Check if this bone has any translations
				if (Animation.AnimationPositionKeys.find(Tag) != Animation.AnimationPositionKeys.end())
				{
					// We have keys, write count based on total frames
					if (FrameCountBuffer <= 0xFF)
					{
						// Write as byte
						Writer.Write<uint8_t>((uint8_t)Animation.AnimationPositionKeys.at(Tag).size());
					}
					else if (FrameCountBuffer <= 0xFFFF)
					{
						// Write as short
						Writer.Write<uint16_t>((uint16_t)Animation.AnimationPositionKeys.at(Tag).size());
					}
					else
					{
						// Write as int
						Writer.Write<uint32_t>((uint32_t)Animation.AnimationPositionKeys.at(Tag).size());
					}
					// Output keys
					for (auto& Key : Animation.AnimationPositionKeys.at(Tag))
					{
						// Write frame index based on total frames
						if (FrameCountBuffer <= 0xFF)
						{
							// Write as byte
							Writer.Write<uint8_t>((uint8_t)Key.Frame);
						}
						else if (FrameCountBuffer <= 0xFFFF)
						{
							// Write as short
							Writer.Write<uint16_t>((uint16_t)Key.Frame);
						}
						else
						{
							// Write as int
							Writer.Write<uint32_t>((uint32_t)Key.Frame);
						}
						// Output the vector
						Writer.Write<Vector3>(Key.Value);
					}
				}
				else
				{
					// No keys for this bone, write the count based on total frames
					if (FrameCountBuffer <= 0xFF)
					{
						// Write as byte
						Writer.Write<uint8_t>(0x0);
					}
					else if (FrameCountBuffer <= 0xFFFF)
					{
						// Write as short
						Writer.Write<uint16_t>(0x0);
					}
					else
					{
						// Write as int
						Writer.Write<uint32_t>(0x0);
					}
				}
			}

			// Write rotation keys next (if we have any)
			if (Animation.AnimationRotationKeys.size() > 0)
			{
				// Check if this bone has any rotations
				if (Animation.AnimationRotationKeys.find(Tag) != Animation.AnimationRotationKeys.end())
				{
					// We have keys, write count based on total frames
					if (FrameCountBuffer <= 0xFF)
					{
						// Write as byte
						Writer.Write<uint8_t>((uint8_t)Animation.AnimationRotationKeys.at(Tag).size());
					}
					else if (FrameCountBuffer <= 0xFFFF)
					{
						// Write as short
						Writer.Write<uint16_t>((uint16_t)Animation.AnimationRotationKeys.at(Tag).size());
					}
					else
					{
						// Write as int
						Writer.Write<uint32_t>((uint32_t)Animation.AnimationRotationKeys.at(Tag).size());
					}
					// Output keys
					for (auto& Key : Animation.AnimationRotationKeys.at(Tag))
					{
						// Write frame index based on total frames
						if (FrameCountBuffer <= 0xFF)
						{
							// Write as byte
							Writer.Write<uint8_t>((uint8_t)Key.Frame);
						}
						else if (FrameCountBuffer <= 0xFFFF)
						{
							// Write as short
							Writer.Write<uint16_t>((uint16_t)Key.Frame);
						}
						else
						{
							// Write as int
							Writer.Write<uint32_t>((uint32_t)Key.Frame);
						}
						// Output the quaternion
						Writer.Write<Quaternion>(Key.Value);
					}
				}
				else
				{
					// No keys for this bone, write the count based on total frames
					if (FrameCountBuffer <= 0xFF)
					{
						// Write as byte
						Writer.Write<uint8_t>(0x0);
					}
					else if (FrameCountBuffer <= 0xFFFF)
					{
						// Write as short
						Writer.Write<uint16_t>(0x0);
					}
					else
					{
						// Write as int
						Writer.Write<uint32_t>(0x0);
					}
				}
			}

			// Write scale keys next (if we have any)
			if (Animation.AnimationScaleKeys.size() > 0)
			{
				// Check if this bone has any scales
				if (Animation.AnimationScaleKeys.find(Tag) != Animation.AnimationScaleKeys.end())
				{
					// We have keys, write count based on total frames
					if (FrameCountBuffer <= 0xFF)
					{
						// Write as byte
						Writer.Write<uint8_t>((uint8_t)Animation.AnimationScaleKeys.at(Tag).size());
					}
					else if (FrameCountBuffer <= 0xFFFF)
					{
						// Write as short
						Writer.Write<uint16_t>((uint16_t)Animation.AnimationScaleKeys.at(Tag).size());
					}
					else
					{
						// Write as int
						Writer.Write<uint32_t>((uint32_t)Animation.AnimationScaleKeys.at(Tag).size());
					}
					// Output keys
					for (auto& Key : Animation.AnimationScaleKeys.at(Tag))
					{
						// Write frame index based on total frames
						if (FrameCountBuffer <= 0xFF)
						{
							// Write as byte
							Writer.Write<uint8_t>((uint8_t)Key.Frame);
						}
						else if (FrameCountBuffer <= 0xFFFF)
						{
							// Write as short
							Writer.Write<uint16_t>((uint16_t)Key.Frame);
						}
						else
						{
							// Write as int
							Writer.Write<uint32_t>((uint32_t)Key.Frame);
						}
						// Output the vector
						Writer.Write<Vector3>(Key.Value);
					}
				}
				else
				{
					// No keys for this bone, write the count based on total frames
					if (FrameCountBuffer <= 0xFF)
					{
						// Write as byte
						Writer.Write<uint8_t>(0x0);
					}
					else if (FrameCountBuffer <= 0xFFFF)
					{
						// Write as short
						Writer.Write<uint16_t>(0x0);
					}
					else
					{
						// Write as int
						Writer.Write<uint32_t>(0x0);
					}
				}
			}
		}
	}
	// Output notetracks, if any
	if (NotificationBuffer > 0)
	{
		// We have notifications
		for (auto& Note : Animation.AnimationNotetracks)
		{
			// Iterate and write
			for (auto& Key : Note.second)
			{
				// Write the frame itself based on framecount
				if (FrameCountBuffer <= 0xFF)
				{
					// Write as byte
					Writer.Write<uint8_t>((uint8_t)Key);
				}
				else if (FrameCountBuffer <= 0xFFFF)
				{
					// Write as short
					Writer.Write<uint16_t>((uint16_t)Key);
				}
				else
				{
					// Write as int
					Writer.Write<uint32_t>((uint32_t)Key);
				}
				// Write flag name
				Writer.WriteNullTerminatedString(Note.first);
			}
		}
	}
}