#pragma once

#include <cstdint>
#include <memory>

// We need the binary writer class
#include "BinaryWriter.h"
#include "WraithBinaryWriter.h"

// A list of supported sound formats, input and output
enum class SoundFormat
{
    // Standard sound formats

    Standard_WAV,
    Standard_FLAC,
    Standard_OGG,

    // This is used only for InFormat, which will read the WAV format from the header in the stream
    WAV_WithHeader,
    // This is used only for InFormat, which will read the FLAC format from the header in the stream
    FLAC_WithHeader
};

class Sound
{
public:
    // -- Conversion functions

    // Converts a sound stream from memory to a file with the specified format
    static bool ConvertSoundMemory(int8_t* SoundBuffer, uint64_t SoundSize, SoundFormat InFormat, const std::string& OutputFile, SoundFormat OutFormat);
    // Converts a sound file to a file with the specified format
    static bool ConvertSoundFile(const std::string& InputFile, SoundFormat InFormat, const std::string& OutputFile, SoundFormat OutFormat);

    // -- WAV / FLAC header functions

    // Builds and writes a FLAC header with the specified format
    static void WriteFLACHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount);
    // Builds and writes a FLAC header with the specified format
    static void WriteFLACHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount);
    // Builds and writes a FLAC header with the specified format
    static void WriteFLACHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount);

    // Builds and writes a WAV header with the specified format
    static void WriteWAVHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize);
    // Builds and writes a WAV header with the specified format
    static void WriteWAVHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize);
    // Builds and writes a WAV header with the specified format
    static void WriteWAVHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize);

    // Builds and writes a IMA header with the specified format
    static void WriteIMAHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize);
    // Builds and writes a IMA header with the specified format
    static void WriteIMAHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize);
    // Builds and writes a IMA header with the specified format
    static void WriteIMAHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize);

    // Builds and writes a MSADPCM header with the specified format
    static void WriteMSADPCMHeaderToFile(BinaryWriter& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize);
    // Builds and writes a MSADPCM header with the specified format
    static void WriteMSADPCMHeaderToFile(const std::shared_ptr<BinaryWriter>& Writer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize);
    // Builds and writes a MSADPCM header with the specified format
    static void WriteMSADPCMHeaderToStream(int8_t* Buffer, uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize);

    // Gets the maximum size of a FLAC header
    static const uint32_t GetMaximumFLACHeaderSize();
    // Gets the maximum size of a WAV header
    static const uint32_t GetMaximumWAVHeaderSize();
    // Gets the maximum size of a IMA header
    static const uint32_t GetMaximumIMAHeaderSize();
    // Gets the maximum size of a MSADPCM header
    static const uint32_t GetMaximumMSADPCMHeaderSize();

private:

    // Transcodes a FLAC file (With header) to a compatible WAV output file
    static bool TranscodeFLACToWav(int8_t* SoundBuffer, uint64_t SoundSize, const std::string& OutputFile);
    // Transcodes a WAV file (With header) to a compatible FLAC output file
    static bool TranscodeWAVToFlac(int8_t* SoundBuffer, uint64_t SoundSize, const std::string& OutputFile);

    // Builds a FLAC file header
    static std::unique_ptr<int8_t[]> BuildFLACHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t FrameCount, uint32_t& ResultSize);
    // Builds a WAV file header
    static std::unique_ptr<int8_t[]> BuildWAVHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize, uint32_t& ResultSize);
    // Builds a IMA file header
    static std::unique_ptr<int8_t[]> BuildIMAHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t BitsPerSample, uint32_t BlockAlign, uint32_t TotalSize, uint32_t& ResultSize);
    // Builds a MSADPCM file header
    static std::unique_ptr<int8_t[]> BuildMSADPCMHeader(uint32_t FrameRate, uint32_t ChannelsCount, uint32_t TotalSize, uint32_t& ResultSize);
};