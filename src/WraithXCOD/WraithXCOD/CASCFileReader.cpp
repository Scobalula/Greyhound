#include "CASCFileReader.h"

CASCFileReader::CASCFileReader(HANDLE Storage, std::string fileName) : FileHandle(nullptr)
{
    if (!CascOpenFile(Storage, fileName.c_str(), NULL, NULL, &FileHandle))
    {
        FileHandle = nullptr;
    }
}

CASCFileReader::~CASCFileReader()
{
    CascCloseFile(FileHandle);
}

uint64_t CASCFileReader::GetLength() const
{
    uint64_t Result = 0;
    CascGetFileSize64(FileHandle, &Result);
    return Result;
}

uint64_t CASCFileReader::GetPosition() const
{
    TCascFile* hf;

    hf = TCascFile::IsValid(FileHandle);
    if (hf == NULL)
        return 0;

    return hf->FilePointer;
}

void CASCFileReader::SetPosition(uint64_t Offset)
{
    CascSetFilePointer64(FileHandle, Offset, NULL, FILE_BEGIN);
}

void CASCFileReader::Advance(uint64_t Length)
{
    CascSetFilePointer64(FileHandle, Length, NULL, FILE_CURRENT);
}

std::string CASCFileReader::ReadNullTerminatedString()
{
    // Make sure we are loaded
    if (FileHandle != nullptr)
    {
        // We can read
        std::stringstream ResultBuffer;
        // First char
        char CurrentChar = Read<char>();
        // Loop until null or EOF
        while (CurrentChar != 0)
        {
            // Add
            ResultBuffer << CurrentChar;
            // Read again
            CurrentChar = Read<char>();
        }
        // Return it
        return ResultBuffer.str();
    }
    // Failed to perform read
#ifdef _DEBUG
    throw new std::exception("No file is open");
#else
    throw new std::exception("");
#endif
}

std::unique_ptr<uint8_t[]> CASCFileReader::Read(uint64_t Length, uint64_t& Result)
{
    // Make sure we are loaded
    if (FileHandle != nullptr)
    {
        // Result
        DWORD LengthRead = 0;
        // We can read the block
        auto ResultBlock = std::make_unique<uint8_t[]>((size_t)Length);
        // Zero out the memory
        std::memset(ResultBlock.get(), 0, (size_t)Length);
        // Read it
        CascReadFile(FileHandle, ResultBlock.get(), (DWORD)Length, &LengthRead);
        // Set result
        Result = LengthRead;
        // Return result
        return ResultBlock;
    }
    // Failed
    return nullptr;
}

void CASCFileReader::Read(uint8_t* Buffer, uint64_t Length, uint64_t& Result)
{
    // Make sure we are loaded
    if (FileHandle != nullptr)
    {
        // Result
        DWORD LengthRead = 0;
        // Read it
        CascReadFile(FileHandle, Buffer, (DWORD)Length, &LengthRead);
        // Set result
        Result = LengthRead;
    }
}
