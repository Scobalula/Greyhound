#include "stdafx.h"

// The class we are implementing
#include "Sound.h"

// We need the strings and math classes
#include "Strings.h"
#include "BinaryReader.h"
#include "VectorMath.h"
#include "FileSystems.h"
#include "MemoryReader.h"

// We don't need the FLAC DLL exports, we're a LIB
#define FLAC__NO_DLL 1

// We need the FLAC helper classes
#include "FLAC\stream_decoder.h"
#include "FLAC\stream_encoder.h"

// -- Structures used for transcoding

struct FLACStreamInfo
{
	// Basic stream info
	int8_t* InputBuffer;
	uint64_t InputBufferSize;
	uint64_t CurrentPosition;
	FILE* OutputFileHandle;

	// FLAC input info
	uint64_t TotalSamples;
	uint32_t SampleRate;
	uint32_t ChannelsCount;
	uint32_t Bps;
};

struct WAVStreamInfo
{
	// WAV input info
	uint64_t TotalSamples;
	uint32_t SeekLength;
	uint32_t RawDataSize;
	uint32_t FormatSize;
	uint32_t SampleRate;
	uint16_t ChannelsCount;
	uint16_t Bps;
};

// -- End transcode structures

// -- Begin FLAC write functions

inline void WriteLEUInt32(FILE* file, uint32_t x)
{
	// Write it
	fwrite(&x, 4, 1, file);
}

inline void WriteLEUInt16(FILE* file, uint16_t x)
{
	// Write it
	fwrite(&x, 2, 1, file);
}

// -- End FLAC write functions

// -- Begin FLAC callback functions

#pragma region FLACDecoder

FLAC__StreamDecoderSeekStatus FLACSeekStream(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;

	// Make sure we are within bounds
	if (absolute_byte_offset <= StreamData->InputBufferSize)
	{
		// Set position
		StreamData->CurrentPosition = absolute_byte_offset;
		// Done
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	}
	else
	{
		// Error
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
	}
}

FLAC__bool FLACEOFStream(const FLAC__StreamDecoder *decoder, void *client_data)
{
	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;
	// Check result
	if (StreamData->CurrentPosition >= StreamData->InputBufferSize)
	{
		// At end
		return true;
	}
	// Not at end
	return false;
}

FLAC__StreamDecoderLengthStatus FLACLengthStream(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;
	// Set result
	*stream_length = (FLAC__uint64)StreamData->InputBufferSize;
	// Done
	return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}

FLAC__StreamDecoderTellStatus FLACTellStream(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;
	// Set result
	*absolute_byte_offset = (FLAC__uint64)StreamData->CurrentPosition;
	// Done
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderReadStatus FLACReadFromStream(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;

	// Check size to read
	if (*bytes > 0)
	{
		// Prepare to read the data we need
		auto DataRequested = *bytes;

		// Remainder
		auto DataBufferRemainder = (int64_t)((int64_t)StreamData->InputBufferSize - (int64_t)StreamData->CurrentPosition);

		// Ensure we are within our bounds (Full read)
		if ((StreamData->CurrentPosition + DataRequested) <= StreamData->InputBufferSize)
		{
			// Copy input to buffer
			std::memcpy(buffer, StreamData->InputBuffer + StreamData->CurrentPosition, DataRequested);
			// Set current position
			StreamData->CurrentPosition += DataRequested;
			// Return continue
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}
		else if (DataBufferRemainder > 0)
		{
			// Copy the remainder of the buffer
			*bytes = (FLAC__uint64)DataBufferRemainder;
			// Copy from input
			std::memcpy(buffer, StreamData->InputBuffer + StreamData->CurrentPosition, DataBufferRemainder);
			// Set current position
			StreamData->CurrentPosition += DataBufferRemainder;
			// Return
			return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
		}
		else
		{
			// Set
			*bytes = 0;
			// We hit end too early, alert reader
			return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
		}
	}
	else
	{
		// Abort, no data requested
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	}
}

void FLACMetaDataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	// Unused
	(void)decoder, (void)client_data;

	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;

	// Check if it's streaming info
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
		// Set in our struct
		StreamData->TotalSamples = metadata->data.stream_info.total_samples;
		StreamData->SampleRate = metadata->data.stream_info.sample_rate;
		StreamData->ChannelsCount = metadata->data.stream_info.channels;
		StreamData->Bps = metadata->data.stream_info.bits_per_sample;
	}
}

void FLACErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	// Unused
	(void)decoder, (void)client_data, (void)status;
}

FLAC__StreamDecoderWriteStatus FLACWriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	// Unused
	(void)decoder;

	// First, grab our stream info
	FLACStreamInfo* StreamData = (FLACStreamInfo*)client_data;
	// Calculate total size
	const uint32_t TotalSize = (uint32_t)(StreamData->TotalSamples * StreamData->ChannelsCount * (StreamData->Bps / 8));

	// Verify that we got samples
	if (StreamData->TotalSamples == 0)
	{
		// Abort
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	// Check for first frame, if so, write the WAVfmt header
	if (frame->header.number.sample_number == 0)
	{
		// Format a WAV header for the output
		auto SizeWithHeader = TotalSize + 36;

		// Begin header
		fwrite("RIFF", 1, 4, StreamData->OutputFileHandle);
		fwrite(&SizeWithHeader, 4, 1, StreamData->OutputFileHandle);

		// Begin format segment
		fwrite("WAVEfmt ", 1, 8, StreamData->OutputFileHandle);

		// Output formatted segment
		WriteLEUInt32(StreamData->OutputFileHandle, 16);
		WriteLEUInt16(StreamData->OutputFileHandle, 1);
		WriteLEUInt16(StreamData->OutputFileHandle, (uint16_t)StreamData->ChannelsCount);
		WriteLEUInt32(StreamData->OutputFileHandle, StreamData->SampleRate);
		WriteLEUInt32(StreamData->OutputFileHandle, StreamData->SampleRate * StreamData->ChannelsCount * (StreamData->Bps / 8));
		WriteLEUInt16(StreamData->OutputFileHandle, (uint16_t)(StreamData->ChannelsCount * (StreamData->Bps / 8)));
		WriteLEUInt16(StreamData->OutputFileHandle, (uint16_t)StreamData->Bps);

		// Begin data segment
		fwrite("data", 1, 4, StreamData->OutputFileHandle);
		fwrite(&TotalSize, 4, 1, StreamData->OutputFileHandle);
	}

	// Write the 16bit PCM samples
	for (size_t i = 0; i < frame->header.blocksize; i++)
	{
		// Loop for channel count
		for (size_t c = 0; c < StreamData->ChannelsCount; c++)
		{
			// Write it
			WriteLEUInt16(StreamData->OutputFileHandle, (uint16_t)((int16_t)buffer[c][i]));
		}
	}

	// We're ready to continue
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

#pragma endregion

// -- End FLAC callback functions

bool Sound::ConvertSoundMemory(int8_t* SoundBuffer, uint64_t SoundSize, SoundFormat InFormat, const std::string& OutputFile, SoundFormat OutFormat)
{
	// Prepare to convert the buffer to the format required
	auto ConversionResult = false;

	// Check input
	switch (InFormat)
	{
		// Our input is a FLAC, we can Transcode to WAV/OGG
	case SoundFormat::FLAC_WithHeader:
		// Check output format
		if (OutFormat == SoundFormat::Standard_WAV)
		{
			// Transcode to WAV
			ConversionResult = TranscodeFLACToWav(SoundBuffer, SoundSize, OutputFile);
		}
		else if (OutFormat == SoundFormat::Standard_OGG)
		{
			// Transcode to OGG
			// TODO: OGG Transcode support
		}
		break;
		// Out input is a WAV, we can Transcode to FLAC/OGG
	case SoundFormat::WAV_WithHeader:
		// Check output format
		if (OutFormat == SoundFormat::Standard_FLAC)
		{
			// Transcode to FLAC
			ConversionResult = TranscodeWAVToFlac(SoundBuffer, SoundSize, OutputFile);
		}
		else if (OutFormat == SoundFormat::Standard_OGG)
		{
			// Transcode to OGG
			// TODO: OGG Transcode support
		}
		break;
	}

	// Return result
	return ConversionResult;
}

bool Sound::ConvertSoundFile(const std::string& InputFile, SoundFormat InFormat, const std::string& OutputFile, SoundFormat OutFormat)
{
	// Read the whole file to a buffer, then pass to memory function
	if (FileSystems::FileExists(InputFile))
	{
		// Allocate a reader
		auto Reader = BinaryReader();
		// Open file
		Reader.Open(InputFile);

		// Result size
		uint64_t ResultSize = 0;
		// Read a buffer
		auto DataBuffer = Reader.Read(Reader.GetLength(), ResultSize);

		// Make sure we got it
		if (DataBuffer != nullptr)
		{
			// Continue
			auto Result = ConvertSoundMemory(DataBuffer, ResultSize, InFormat, OutputFile, OutFormat);
			// Clean up
			delete[] DataBuffer;
			// Return result
			return Result;
		}
	}

	// Failed
	return false;
}

bool Sound::TranscodeFLACToWav(int8_t* SoundBuffer, uint64_t SoundSize, const std::string& OutputFile)
{
	// Prepare to transcode the FLAC buffer to a WAV file
	bool IsOk = true;
	FLAC__StreamDecoder* Decoder = nullptr;
	FLAC__StreamDecoderInitStatus InitStatus;

	// Prepare output file
	FILE* OutputHandle;
	// Open it
	fopen_s(&OutputHandle, OutputFile.c_str(), "wb");

	// No throw check
	if (OutputHandle == NULL)
	{
		// Just return failed, already in access
		return false;
	}

	// A flac stream info buffer used for transcoding everything
	auto FlacInfo = std::make_unique<FLACStreamInfo>();
	// Set properties
	FlacInfo->InputBuffer = SoundBuffer;
	FlacInfo->InputBufferSize = SoundSize;
	FlacInfo->OutputFileHandle = OutputHandle;
	FlacInfo->CurrentPosition = 0;

	// Initialize the decoder
	Decoder = FLAC__stream_decoder_new();

	// Check for failed
	if (Decoder == nullptr)
	{
		// Clean up file
		fclose(OutputHandle);
		// Failed
		return false;
	}

	// Disable MD5 verification checks
	FLAC__stream_decoder_set_md5_checking(Decoder, false);

	// Initialize the decoder with the buffer
	InitStatus = FLAC__stream_decoder_init_stream(Decoder, FLACReadFromStream, FLACSeekStream, FLACTellStream, FLACLengthStream, FLACEOFStream, FLACWriteCallback, FLACMetaDataCallback, FLACErrorCallback, FlacInfo.get());

	// Check for failed
	if (InitStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK) { IsOk = false; }

	// Continue if we're ok
	if (IsOk)
	{
		// Prepare to decode the FLAC buffer
		FLAC__stream_decoder_process_until_end_of_stream(Decoder);
	}

	// Clean up the decoder
	FLAC__stream_decoder_delete(Decoder);
	// Clean up file
	fclose(OutputHandle);

	// Only error on failed
	return IsOk;
}

bool Sound::TranscodeWAVToFlac(int8_t* SoundBuffer, uint64_t SoundSize, const std::string& OutputFile)
{
	// Prepare to transcode the WAV buffer to a FLAC file
	bool IsOk = true;
	FLAC__StreamEncoder* Encoder = nullptr;
	FLAC__StreamEncoderInitStatus InitStatus;

	// A wav stream info buffer used for transcoding everything
	auto WAVInfo = std::make_unique<WAVStreamInfo>();
	// Default
	WAVInfo->TotalSamples = 0;
	WAVInfo->SeekLength = 0;

	// Read WAV info
	auto WAVInfoReader = MemoryReader(SoundBuffer, SoundSize, true);
	// Whether or not reader is done
	bool KeepReading = true;
	
	// Loop to read
	while (KeepReading)
	{
		// Chunk ID
		uint32_t ChunkID = WAVInfoReader.Read<uint32_t>();

		// Check chunks
		switch (ChunkID)
		{
			// RIFF Chunk, skip total size
		case 0x46464952: WAVInfoReader.Advance(4); break;

			// WAVEFmt Chunk, read info
		case 0x45564157:
		{
			// Skip fmt 
			WAVInfoReader.Advance(4);

			// Read format info
			WAVInfo->FormatSize = WAVInfoReader.Read<uint32_t>();

			// Hold current pos
			auto CurrentPosition = WAVInfoReader.GetPosition();
			// Skip 2 bytes
			WAVInfoReader.Advance(2);

			// Read channel count
			WAVInfo->ChannelsCount = WAVInfoReader.Read<uint16_t>();
			// Read sample rate
			WAVInfo->SampleRate = WAVInfoReader.Read<uint32_t>();
			// Skip 6 bytes
			WAVInfoReader.Advance(6);
			// Read bps
			WAVInfo->Bps = WAVInfoReader.Read<uint16_t>();

			// Skip over chunk
			WAVInfoReader.SetPosition(CurrentPosition + WAVInfo->FormatSize);
		}
		break;

			// Data Chunk, read size and end, we're at the data
		case 0x61746164: WAVInfo->RawDataSize = WAVInfoReader.Read<uint32_t>(); KeepReading = false; break;
			// DPDS Chunk, read length
		case 0x73647064: WAVInfo->SeekLength = WAVInfoReader.Read<uint32_t>(); WAVInfoReader.Advance(WAVInfo->SeekLength); break;

			// Stop, we're at a bad chunk
		default: KeepReading = false; break;
		}
	}

	// Convert size to samples
	if (((WAVInfo->Bps / 8) * WAVInfo->ChannelsCount) > 0)
	{
		// We have valid data
		WAVInfo->TotalSamples = (WAVInfo->RawDataSize / ((WAVInfo->Bps / 8) * WAVInfo->ChannelsCount));
	}

	// Initialize the encoder
	Encoder = FLAC__stream_encoder_new();

	// Check for failed
	if (Encoder == nullptr)
	{
		// Failed
		return false;
	}

	// Configure the encoder
	FLAC__stream_encoder_set_verify(Encoder, false);
	FLAC__stream_encoder_set_compression_level(Encoder, 5);
	FLAC__stream_encoder_set_channels(Encoder, WAVInfo->ChannelsCount);
	FLAC__stream_encoder_set_bits_per_sample(Encoder, WAVInfo->Bps);
	FLAC__stream_encoder_set_sample_rate(Encoder, WAVInfo->SampleRate);

	// Setup the encoder
	InitStatus = FLAC__stream_encoder_init_file(Encoder, OutputFile.c_str(), nullptr, nullptr);

	// Check for failed
	if (InitStatus != FLAC__STREAM_ENCODER_INIT_STATUS_OK) { IsOk = false; }

	// Continue if we're ok
	if (IsOk)
	{
		// Total size to read
		size_t Left = (size_t)WAVInfo->TotalSamples;
		// Buffers
		auto Buffer = new FLAC__byte[1024 * ((WAVInfo->Bps / 8) * WAVInfo->ChannelsCount)];
		auto PCMBuffer = new FLAC__int32[1024 * 2];

		// Loop until EOF
		while (IsOk && Left)
		{
			// Calculate what we need
			size_t Need = (Left > 1024 ? (size_t)1024 : (size_t)Left);

			// Calculate data size to read
			auto SizeToRead = (WAVInfo->ChannelsCount * (WAVInfo->Bps / 8)) * Need;

			// Ensure we can read it
			if (WAVInfoReader.GetPosition() + SizeToRead <= WAVInfoReader.GetLength())
			{
				// Read
				WAVInfoReader.Read(SizeToRead, (int8_t*)Buffer);

				// Convert the data to PCM samples
				for (size_t i = 0; i < (Need * WAVInfo->ChannelsCount); i++)
				{
					// Convert to PCM
					PCMBuffer[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)Buffer[2 * i + 1] << 8) | (FLAC__int16)Buffer[2 * i]);
				}

				// Send off to encoder
				IsOk = (FLAC__stream_encoder_process_interleaved(Encoder, PCMBuffer, (uint32_t)Need) == 1);
			}
			else
			{
				// Failed
				IsOk = false;
			}

			// Advance
			Left -= Need;
		}

		// Clean up
		delete[] Buffer;
		delete[] PCMBuffer;
	}

	// Finalize the encoding and clean up
	FLAC__stream_encoder_finish(Encoder);
	FLAC__stream_encoder_delete(Encoder);

	// Result of decode
	return IsOk;
}

void Sound::WriteFLACHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildFLACHeader(FrameRate, ChannelsCount, FrameCount, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Write it
		Writer.Write(HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteFLACHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount)
{
	// Pass off
	WriteFLACHeaderToFile(*Writer.get(), FrameRate, ChannelsCount, FrameCount);
}

void Sound::WriteFLACHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildFLACHeader(FrameRate, ChannelsCount, FrameCount, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Copy the data
		std::memcpy(Buffer, HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteWAVHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildWAVHeader(FrameRate, ChannelsCount, TotalSize, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Write it
		Writer.Write(HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteWAVHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize)
{
	// Pass off
	WriteWAVHeaderToFile(*Writer.get(), FrameRate, ChannelsCount, TotalSize);
}

void Sound::WriteWAVHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildWAVHeader(FrameRate, ChannelsCount, TotalSize, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Copy the data
		std::memcpy(Buffer, HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteIMAHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildIMAHeader(FrameRate, ChannelsCount, BitsPerSample, BlockAlign, TotalSize, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Write it
		Writer.Write(HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteIMAHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize)
{
	// Pass off
	WriteIMAHeaderToFile(*Writer.get(), FrameRate, ChannelsCount, BitsPerSample, BlockAlign, TotalSize);
}

void Sound::WriteIMAHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildIMAHeader(FrameRate, ChannelsCount, BitsPerSample, BlockAlign, TotalSize, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Copy the data
		std::memcpy(Buffer, HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteMSADPCMHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildMSADPCMHeader(FrameRate, ChannelsCount, TotalSize, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Write it
		Writer.Write(HeaderBuffer.get(), ResultSize);
	}
}

void Sound::WriteMSADPCMHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize)
{
	// Pass off
	WriteMSADPCMHeaderToFile(*Writer.get(), FrameRate, ChannelsCount, TotalSize);
}

void Sound::WriteMSADPCMHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize)
{
	// The resulting header size
	uint32_t ResultSize = 0;

	// Build it
	auto HeaderBuffer = BuildMSADPCMHeader(FrameRate, ChannelsCount, TotalSize, ResultSize);

	// If we have a result size, write it to the stream
	if (HeaderBuffer != nullptr && ResultSize > 0)
	{
		// Copy the data
		std::memcpy(Buffer, HeaderBuffer.get(), ResultSize);
	}
}

const uint32_t Sound::GetMaximumFLACHeaderSize()
{
	// Same size always
	return 42;
}

const uint32_t Sound::GetMaximumWAVHeaderSize()
{
	// Same size always
	return 44;
}

const uint32_t Sound::GetMaximumIMAHeaderSize()
{
	// Same size always
	return 48;
}

const uint32_t Sound::GetMaximumMSADPCMHeaderSize()
{
	// Same size always
	return 78;
}

std::unique_ptr<int8_t[]> Sound::BuildFLACHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount, uint32_t& ResultSize)
{
	// Prepare to build a flac header
	uint32_t MaximumSize = GetMaximumFLACHeaderSize();
	// Allocate it
	auto HeaderBuffer = std::make_unique<int8_t[]>(MaximumSize);
	// Clear first
	std::memset(HeaderBuffer.get(), 0, MaximumSize);

	// Current position
	uint32_t CurrentPosition = 0;

	// Copy magic
	std::memcpy(HeaderBuffer.get(), "fLaC", 4);
	// Increase
	CurrentPosition += 4;

	// Constant values
	const uint16_t MinimumBlockSize = 0x80, MaximumBlockSize = 0, MinimumFrameSize = 0x22, MaximumFramesize = 0x4, SampleRate = 0x4;

	// Copy over

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &MinimumBlockSize, 1);
	std::memcpy(HeaderBuffer.get() + CurrentPosition + 1, &MaximumBlockSize, 2);
	// Increase 3
	CurrentPosition += 3;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &MinimumFrameSize, 1);
	std::memcpy(HeaderBuffer.get() + CurrentPosition + 1, &MaximumFramesize, 2);
	// Increase 3
	CurrentPosition += 3;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &SampleRate, 2);
	// Increase 2
	CurrentPosition += 2;

	// Skip 6 bytes padding
	CurrentPosition += 6;

	// Build flags
	{
		// Flags buffer
		uint64_t FlacFlags = 0;
		
		// Build flags
		FlacFlags += ((uint64_t)FrameRate << 44);
		FlacFlags += ((uint64_t)(ChannelsCount - 1) << 41);
		FlacFlags += ((uint64_t)(16 - 1) << 36);
		FlacFlags += ((uint64_t)FrameCount);

		// Swap
		FlacFlags = _byteswap_uint64(FlacFlags);

		// Write
		std::memcpy(HeaderBuffer.get() + CurrentPosition, &FlacFlags, 8);
		// Increase
		CurrentPosition += 8;
	}

	// Skip 16 byte checksum
	CurrentPosition += 16;

	// Set result
	ResultSize = MaximumSize;
	// Return it
	return HeaderBuffer;
}

std::unique_ptr<int8_t[]> Sound::BuildWAVHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize, uint32_t& ResultSize)
{
	// Prepare to build a wav header
	uint32_t MaximumSize = GetMaximumWAVHeaderSize();
	// Allocate it
	auto HeaderBuffer = std::make_unique<int8_t[]>(MaximumSize);
	// Clear first
	std::memset(HeaderBuffer.get(), 0, MaximumSize);

	// Current position
	uint32_t CurrentPosition = 0;

	// Copy magic
	std::memcpy(HeaderBuffer.get(), "RIFF", 4);
	// Increase
	CurrentPosition += 4;

	// Total length
	auto TotalLength = TotalSize + 36;
	// Apply
	std::memcpy(HeaderBuffer.get() + CurrentPosition, &TotalLength, 4);
	// Increase
	CurrentPosition += 4;

	// Copy second magic
	std::memcpy(HeaderBuffer.get() + CurrentPosition, "WAVEfmt ", 8);
	// Increase
	CurrentPosition += 8;

	// Constants
	const uint32_t FormatData = 16, Encoding = 1, BitsPerSample = 16, BlockAlign = (ChannelsCount * (16 / 8));
	const uint32_t AverageBps = (FrameRate * BlockAlign);

	// Copy over

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &FormatData, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &Encoding, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &ChannelsCount, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &FrameRate, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &AverageBps, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BlockAlign, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BitsPerSample, 2);
	// Increase 2
	CurrentPosition += 2;

	// Copy last magic
	std::memcpy(HeaderBuffer.get() + CurrentPosition, "data", 4);
	// Increase
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &TotalSize, 4);
	// Increase 4
	CurrentPosition += 4;

	// Set result
	ResultSize = MaximumSize;
	// Return it
	return HeaderBuffer;
}

std::unique_ptr<int8_t[]> Sound::BuildIMAHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize, uint32_t& ResultSize)
{
	// Prepare to build a ima header
	uint32_t MaximumSize = GetMaximumIMAHeaderSize();
	// Allocate it
	auto HeaderBuffer = std::make_unique<int8_t[]>(MaximumSize);
	// Clear first
	std::memset(HeaderBuffer.get(), 0, MaximumSize);

	// Current position
	uint32_t CurrentPosition = 0;

	// Copy magic
	std::memcpy(HeaderBuffer.get(), "RIFF", 4);
	// Increase
	CurrentPosition += 4;

	// Total length
	auto TotalLength = TotalSize + 40;
	// Apply
	std::memcpy(HeaderBuffer.get() + CurrentPosition, &TotalLength, 4);
	// Increase
	CurrentPosition += 4;

	// Copy second magic
	std::memcpy(HeaderBuffer.get() + CurrentPosition, "WAVEfmt ", 8);
	// Increase
	CurrentPosition += 8;

	// Constants
	const uint32_t FormatData = 20, Encoding = 0x11;
	const uint32_t AverageBps = (FrameRate * BlockAlign), CbSize = 2, SamplesPerBlock = (((BlockAlign - 4 * ChannelsCount) / (4 * ChannelsCount)) * 8 + 1);

	// Copy over

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &FormatData, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &Encoding, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &ChannelsCount, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &FrameRate, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &AverageBps, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BlockAlign, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BitsPerSample, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &CbSize, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &SamplesPerBlock, 2);
	// Increase 2
	CurrentPosition += 2;

	// Copy last magic
	std::memcpy(HeaderBuffer.get() + CurrentPosition, "data", 4);
	// Increase
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &TotalSize, 4);
	// Increase 4
	CurrentPosition += 4;

	// Set result
	ResultSize = MaximumSize;
	// Return it
	return HeaderBuffer;
}

std::unique_ptr<int8_t[]> Sound::BuildMSADPCMHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize, uint32_t& ResultSize)
{
	// Prepare to build a wav header
	uint32_t MaximumSize = GetMaximumMSADPCMHeaderSize();
	// Allocate it
	auto HeaderBuffer = std::make_unique<int8_t[]>(MaximumSize);
	// Clear first
	std::memset(HeaderBuffer.get(), 0, MaximumSize);

	// Current position
	uint32_t CurrentPosition = 0;

	// Copy magic
	std::memcpy(HeaderBuffer.get(), "RIFF", 4);
	// Increase
	CurrentPosition += 4;

	// Total length
	auto TotalLength = TotalSize + 70;
	// Apply
	std::memcpy(HeaderBuffer.get() + CurrentPosition, &TotalLength, 4);
	// Increase
	CurrentPosition += 4;

	// Copy second magic
	std::memcpy(HeaderBuffer.get() + CurrentPosition, "WAVEfmt ", 8);
	// Increase
	CurrentPosition += 8;

	// Constants
	const uint32_t FormatData = 50, Encoding = 2, BlockAlign = (ChannelsCount * 262), BitsPerSample = 4, ExtraData = 32;
	const uint32_t AverageBps = 45131, BlockSize = 512, CoeffCount = 7;

	// Copy over

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &FormatData, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &Encoding, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &ChannelsCount, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &FrameRate, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &AverageBps, 4);
	// Increase 4
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BlockAlign, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BitsPerSample, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &ExtraData, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &BlockSize, 2);
	// Increase 2
	CurrentPosition += 2;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &CoeffCount, 2);
	// Increase 2
	CurrentPosition += 2;

	// Coeff chart
	const int16_t CoeffList[14] = { 256, 0, 512, -256, 0, 0, 192, 64, 240, 0, 460, -208, 392, -232 };

	// Copy all
	std::memcpy(HeaderBuffer.get() + CurrentPosition, &CoeffList[0], 28);
	// Increase 28
	CurrentPosition += 28;

	// Copy last magic
	std::memcpy(HeaderBuffer.get() + CurrentPosition, "data", 4);
	// Increase
	CurrentPosition += 4;

	std::memcpy(HeaderBuffer.get() + CurrentPosition, &TotalSize, 4);
	// Increase 4
	CurrentPosition += 4;

	// Set result
	ResultSize = MaximumSize;
	// Return it
	return HeaderBuffer;
}