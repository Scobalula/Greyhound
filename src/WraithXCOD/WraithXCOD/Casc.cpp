#include "stdafx.h"
#include "Casc.h"

#include "Hashing.h"
#include "FileSystems.h"
#include "Strings.h"
#include "TextReader.h"
#include "BinaryReader.h"
#include "Compression.h"
#include "MemoryReader.h"

// TODO: Heavy Improvements are required, this was implemented in less than a day for specific use in Greyhound with Call of Duty
// so I'd advise against using this unless you know exactly what you're doing

void Casc::Container::LoadIndexFiles()
{
    auto IndexFiles = FileSystems::GetFiles(DataPath, "*.idx");

    for (auto IndexFile : IndexFiles)
    {
#if _DEBUG
        printf("LoadIndexFiles(): Loading %s...\n", FileSystems::GetFileName(IndexFile).c_str());
#endif // DEBUG
        BinaryReader IndexReader;

        if (!IndexReader.Open(IndexFile, true))
            continue;

        auto HeaderBlock = IndexReader.Read<IndexFileBlock>();

        // Size must be equal to the size of the Index Header
        if (HeaderBlock.BlockSize == sizeof(IndexFileHeader))
        {
            // Parse Header
            auto Header = IndexReader.Read<IndexFileHeader>();

            // and validate (these validations are done in CoD, and so we expect them for CoD
            if (
                Header.ExtraBytes == 0 &&
                Header.EncodedSizeLength == 4 &&
                Header.StorageOffsetLength == 5 &&
                Header.EncodingKeyLength == 9
                )
            {
                // Align to the next block
                IndexReader.SetPosition((IndexReader.GetPosition() + 0x17) & 0xFFFFFFF0);


                auto DataBlock = IndexReader.Read<IndexFileBlock>();

                auto EntrySize = Header.EncodedSizeLength + Header.StorageOffsetLength + Header.EncodingKeyLength;
                auto EntryBuffer = new uint8_t[EntrySize];
                
                for (uint32_t bytesConsumed = 0; bytesConsumed < DataBlock.BlockSize; bytesConsumed += EntrySize)
                {
                    uint64_t BytesRead;
                    IndexReader.Read(EntryBuffer, EntrySize, BytesRead);

                    if (BytesRead != EntrySize)
                        break;

                    DataEntries[IndexKey(EntryBuffer, Header)] = IndexEntry(EntryBuffer, Header);
                }

                delete[] EntryBuffer;
                IndexReader.Close();
            }
#if _DEBUG
            else
            {
                printf("LoadIndexFiles(): Header Data is invalid\n");
            }
#endif // DEBUG

        }
    }
}

void Casc::Container::LoadDataFiles()
{
    // TODO: This needs a different implementation, destructor is called each time...
    auto DataFileNames = FileSystems::GetFiles(DataPath, "data.*");

    DataFiles.resize(DataFileNames.size());

    for (auto DataFileName : DataFileNames)
    {
        DataFile DataFile(DataFileName);

        if (DataFile.GetIndex() == -1)
            continue;
        if (DataFile.GetFileHandle() == INVALID_HANDLE_VALUE)
            continue;

        DataFiles[DataFile.GetIndex()] = DataFile;
    }
}

void Casc::Container::LoadBuildInfo()
{
    if (!FileSystems::FileExists(BuildInfoPath))
        throw std::exception("Failed to locate .build.info");

    TextReader Reader;

    Reader.Open(BuildInfoPath, true);

    int32_t BuildKeyIndex = 0;

    bool Success = true;

    for (size_t i = 0; i < 2; i++)
    {
        auto Line = Reader.ReadLine(Success);

        if (!Success)
            break;

        auto Split = Strings::SplitString(Line, '|', true);

        if (i == 0)
        {
            for(int32_t j = 0; j < (int32_t)Split.size(); j++)
            {
                if (Split[j] == "Build Key!HEX:16")
                {
                    BuildKeyIndex = j;
                    break;
                }
            }
        }
        else
        {
            BuildKey = Split.at(BuildKeyIndex);
            break;
        }
    }
}

void Casc::Container::LoadConfigInfo()
{
    if(BuildKey.size() < 4)
        throw std::exception("Invalid Build Key");

    auto FolderName = FileSystems::CombinePath(Path, FileSystems::CombinePath("Data\\config", FileSystems::CombinePath(BuildKey.substr(0, 2), BuildKey.substr(2, 2))));
    auto FileName = FileSystems::CombinePath(FolderName, BuildKey);

    if (!FileSystems::FileExists(FileName))
        throw std::exception("Failed to locate build config");

    TextReader Reader;

    Reader.Open(FileName, true);


    while (true)
    {
        auto Success = true;
        auto Line = Reader.ReadLine(Success);

        if (!Success)
            break;
        if (Strings::StartsWith(Line, "#"))
            continue;
        if (Strings::IsNullOrWhiteSpace(Line))
            continue;

        auto LineSplit = Strings::SplitString(Line, '=', true);

        if (Strings::Trim(LineSplit.at(0)) == "vfs-root")
        {
            auto ValueSplit = Strings::SplitString(LineSplit.at(1), ' ', true);

            VFSRootKey = ValueSplit.at(1);
            break;
        }
    }
}

Casc::Container::Container(const std::string& path)
{
    // For Call of Duty purposes, we assume these paths
    Path          = path;
    DataPath      = FileSystems::CombinePath(Path, "Data\\data");
    BuildInfoPath = FileSystems::CombinePath(Path, ".build.info");
    Closed        = false;

    LoadBuildInfo();
    LoadConfigInfo();
    LoadDataFiles();
    LoadIndexFiles();

    // Attempt to locate VFS Root
    auto LookUpKey = IndexKey(VFSRootKey, 9);
    auto Found = DataEntries.find(LookUpKey);

    // Attempt to load File System
    if (Found != DataEntries.end())
    {
        auto Entry = Found->second;

        auto Reader = OpenFile(Entry);
        // Consume for fast access
        Reader.Consume();

        // Validate header
        if (Reader.Read<uint32_t>() == 0x53465654)
        {
            // Load TVFS
            FileSystemHandler = std::make_unique<TVFSHandler>();
            FileSystemHandler->Parse(Reader);

            // Run a check against the entries we got back
            // and see if they are loaded in this CASC, some won't be
            // such as German files on a Polish install
            for (auto& File : FileSystemHandler->FileEntries)
            {
                // Valite if file eixsts
                for (auto& FileEntry : File.second.KeyEntries)
                {
                    if (DataEntries.find(FileEntry) == DataEntries.end())
                    {
                        // Doesn't Eixst
                        File.second.Exists = false;
                        break;
                    }
                }
            }

        }
    }
}

Casc::Container::~Container()
{
    Close();
}

void Casc::Container::Open(const std::string & path)
{
    DataFiles.clear();
    DataEntries.clear();
    FileSystemHandler = nullptr;

    // For Call of Duty purposes, we assume these paths
    Path = path;
    DataPath = FileSystems::CombinePath(Path, "Data\\data");
    BuildInfoPath = FileSystems::CombinePath(Path, ".build.info");
    Closed = false;

    LoadBuildInfo();
    LoadConfigInfo();
    LoadDataFiles();
    LoadIndexFiles();

    // Attempt to locate VFS Root
    auto LookUpKey = IndexKey(VFSRootKey, 9);
    auto Found = DataEntries.find(LookUpKey);

    // Attempt to load File System
    if (Found != DataEntries.end())
    {
        auto Entry = Found->second;

        auto Reader = OpenFile(Entry);
        // Consume for fast access
        Reader.Consume();

        // Validate header
        if (Reader.Read<uint32_t>() == 0x53465654)
        {
            // Load TVFS
            FileSystemHandler = std::make_unique<TVFSHandler>();
            FileSystemHandler->Parse(Reader);

            // Run a check against the entries we got back
            // and see if they are loaded in this CASC, some won't be
            // such as German files on a Polish install
            for (auto& File : FileSystemHandler->FileEntries)
            {
                // Valite if file eixsts
                for (auto& FileEntry : File.second.KeyEntries)
                {
                    if (DataEntries.find(FileEntry) == DataEntries.end())
                    {
                        // Doesn't Eixst
                        File.second.Exists = false;
                        break;
                    }
                }
            }

        }
    }
}

Casc::FileReader Casc::Container::OpenFile(const std::string& FileName)
{
    if (FileSystemHandler == nullptr)
        throw std::exception("Opening by File Name requires a File System Handler");

    // Run some checks
    auto Result = FileSystemHandler->FileEntries.find(FileName);

    if (Result == FileSystemHandler->FileEntries.end())
        throw std::exception("Failed to find File in File System");

    auto File = Result->second;

    if(!File.Exists)
        throw std::exception("Failed to find file in Casc");
    // Our virtual offset
    uint64_t VirtualOffset = 0;
    // New Reader with this container
    FileReader Reader(*this);
    // Parse each entry, some files are spread across files/entries
    for (auto& KeyEntry : File.KeyEntries)
    {
        IndexEntry& Entry = DataEntries.at(KeyEntry);
        DataFile& DataFile = DataFiles.at(Entry.ArchiveIndex);

        FileSpan Span(Entry.ArchiveIndex);

        DataFile.SetPosition(Entry.Offset);

        BlockTableHeader BLTEHeader{};

        if (DataFile.Read((void*)&BLTEHeader, sizeof(BLTEHeader)) == sizeof(BlockTableHeader) && BLTEHeader.HeaderSize > 0)
        {
            auto FrameCount = BLTEHeader.GetFrameCount();

            auto BLTEntries = std::make_unique<BlockTableEntry[]>(FrameCount);

            DataFile.Read((void*)BLTEntries.get(), FrameCount * sizeof(BlockTableEntry));

            auto ArchiveOffset = DataFile.GetPosition();

            Span.ArchiveOffset = ArchiveOffset;
            Span.VirtualStartOffset = VirtualOffset;
            Span.VirtualEndOffset = VirtualOffset;


            for (uint32_t i = 0; i < FrameCount; i++)
            {
                FileFrame Frame;

                Frame.ArchiveOffset = ArchiveOffset;
                Frame.EncodedSize = BLTEntries[i].GetEncodedSize();
                Frame.ContentSize = BLTEntries[i].GetContentSize();
                Frame.VirtualStartOffset = VirtualOffset;
                Frame.VirtualEndOffset = VirtualOffset + Frame.ContentSize;

                Span.VirtualEndOffset += Frame.ContentSize;

                ArchiveOffset += Frame.EncodedSize;
                VirtualOffset += Frame.ContentSize;

                Span.Frames.push_back(Frame);
            }
        }

        Reader.AddSpan(Span);
    }

    // Initialize Info
    Reader.SetLength(VirtualOffset);


    return Reader;
}



Casc::FileReader Casc::Container::OpenFile(IndexEntry Entry)
{
    // Lock for multithreading
    std::lock_guard<std::mutex> Guard(Mutex);
    // Check if storage is closed
    if (Closed)
        throw std::exception("Attempted to open file from a closed Container");

    uint64_t VirtualOffset = 0;

    FileReader Reader(*this);

    DataFile& DataFile = DataFiles.at(Entry.ArchiveIndex);

    FileSpan Span(Entry.ArchiveIndex);

    DataFile.SetPosition(Entry.Offset);

    BlockTableHeader BLTEHeader{};

    if (DataFile.Read((void*)&BLTEHeader, sizeof(BLTEHeader)) == sizeof(BlockTableHeader) && BLTEHeader.HeaderSize > 0)
    {
        auto FrameCount = BLTEHeader.GetFrameCount();

        auto BLTEntries = std::make_unique<BlockTableEntry[]>(FrameCount);

        DataFile.Read((void*)BLTEntries.get(), FrameCount * sizeof(BlockTableEntry));

        auto ArchiveOffset = DataFile.GetPosition();

        Span.ArchiveOffset = ArchiveOffset;
        Span.VirtualStartOffset = VirtualOffset;
        Span.VirtualEndOffset = VirtualOffset;


        for (uint32_t i = 0; i < FrameCount; i++)
        {
            FileFrame Frame;

            Frame.ArchiveOffset = ArchiveOffset;
            Frame.EncodedSize = BLTEntries[i].GetEncodedSize();
            Frame.ContentSize = BLTEntries[i].GetContentSize();
            Frame.VirtualStartOffset = VirtualOffset;
            Frame.VirtualEndOffset = VirtualOffset + Frame.ContentSize;

            Span.VirtualEndOffset += Frame.ContentSize;

            ArchiveOffset += Frame.EncodedSize;
            VirtualOffset += Frame.ContentSize;

            Span.Frames.push_back(Frame);
        }
    }

    Reader.AddSpan(Span);

    // Initialize Info
    Reader.SetLength(VirtualOffset);


    Reader.Create(VirtualOffset, false);


    return Reader;
}

const int32_t Casc::Container::ReadDataFile(const size_t ArchiveIndex, void * Pointer, const int64_t Offset, const int32_t Size)
{
    // Lock for multithreading
    std::lock_guard<std::mutex> Guard(Mutex);
    // Check if storage is closed
    if (Closed)
        return 0;


    DataFile& File = DataFiles[ArchiveIndex];

    File.SetPosition(Offset);

    return File.Read(Pointer, Size);
}

const std::map<std::string, Casc::FileSystem::Entry>& Casc::Container::GetFileEntries() const
{
    if(FileSystemHandler == nullptr)
        throw std::exception("No File System handler present");

    return FileSystemHandler->FileEntries;
}

void Casc::Container::Close()
{
    // Lock for multithreading
    std::lock_guard<std::mutex> Guard(Mutex);
    // Check if storage is closed
    if (Closed)
        return;

    DataFiles.clear();
    DataEntries.clear();
    FileSystemHandler = nullptr;
    Closed = true;
}

Casc::IndexEntry::IndexEntry() : Size(0), Offset(0), ArchiveIndex(0), EncodingKeySize(0)
{
    std::memset(EncodingKey, 0, 16);
}

Casc::IndexEntry::IndexEntry(const uint8_t* EntryBuffer, const IndexFileHeader& Header)
{
    uint64_t FileOffsetMask = (1 << Header.FileOffsetBits) - 1;
    uint64_t PackedOffsetAndIndex = 0;

    PackedOffsetAndIndex = (PackedOffsetAndIndex << 0x08) | EntryBuffer[0 + Header.EncodingKeyLength];
    PackedOffsetAndIndex = (PackedOffsetAndIndex << 0x08) | EntryBuffer[1 + Header.EncodingKeyLength];
    PackedOffsetAndIndex = (PackedOffsetAndIndex << 0x08) | EntryBuffer[2 + Header.EncodingKeyLength];
    PackedOffsetAndIndex = (PackedOffsetAndIndex << 0x08) | EntryBuffer[3 + Header.EncodingKeyLength];
    PackedOffsetAndIndex = (PackedOffsetAndIndex << 0x08) | EntryBuffer[4 + Header.EncodingKeyLength];

    Size = 0;
    Size = (Size << 0x08) | EntryBuffer[3 + Header.EncodingKeyLength + Header.StorageOffsetLength];
    Size = (Size << 0x08) | EntryBuffer[2 + Header.EncodingKeyLength + Header.StorageOffsetLength];
    Size = (Size << 0x08) | EntryBuffer[1 + Header.EncodingKeyLength + Header.StorageOffsetLength];
    Size = (Size << 0x08) | EntryBuffer[0 + Header.EncodingKeyLength + Header.StorageOffsetLength];

    ArchiveIndex = (int)(PackedOffsetAndIndex >> Header.FileOffsetBits);
    Offset = PackedOffsetAndIndex & FileOffsetMask;
    EncodingKeySize = Header.EncodingKeyLength;

    std::memset(EncodingKey, 0, 16);
    std::memcpy(EncodingKey, EntryBuffer, Header.EncodingKeyLength > 16 ? 16 : Header.EncodingKeyLength);
}

Casc::IndexKey::IndexKey()
{
    std::memset(EncodingKey, 0, 16);
}

Casc::IndexKey::IndexKey(const uint8_t* EntryBuffer, const IndexFileHeader & Header)
{
    EncodingKeySize = Header.EncodingKeyLength > 16 ? 16 : Header.EncodingKeyLength;

    std::memset(EncodingKey, 0, 16);
    std::memcpy(EncodingKey, EntryBuffer, EncodingKeySize);
}

Casc::IndexKey::IndexKey(const uint8_t* Key, const size_t KeySize)
{
    EncodingKeySize = KeySize > 16 ? 16 : KeySize;

    std::memset(EncodingKey, 0, 16);
    std::memcpy(EncodingKey, Key, EncodingKeySize);
}

Casc::IndexKey::IndexKey(const std::string& Key, const size_t KeySize)
{
    EncodingKeySize = KeySize > 16 ? 16 : KeySize;

    std::memset(EncodingKey, 0, 16);

    // mapping of ASCII characters to hex values
    const uint8_t hashmap[] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    // https://gist.github.com/vi/dd3b5569af8a26b97c8e20ae06e804cb
    for (uint8_t pos = 0; ((pos < (EncodingKeySize * 2)) && (pos < Key.size())); pos += 2)
    {
        auto idx0 = ((uint8_t)Key[pos + 0] & 0x1F) ^ 0x10;
        auto idx1 = ((uint8_t)Key[pos + 1] & 0x1F) ^ 0x10;
        EncodingKey[pos / 2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
    };
}

Casc::DataFile::DataFile() : FileHandle(INVALID_HANDLE_VALUE), ArchiveIndex(-1)
{
}

Casc::DataFile::DataFile(const std::string& Path)
{
    auto Index = Strings::SplitString(Path, '.');

    if (Index.size() < 2)
        ArchiveIndex = -1;
    else
        ArchiveIndex = std::stoi(Index[1]);

    FileHandle = CreateFileA(Path.c_str(),
        FILE_READ_DATA | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
}

Casc::DataFile::~DataFile()
{
    //if (FileHandle != INVALID_HANDLE_VALUE)
    //{
    //    CloseHandle(FileHandle);
    //}
}

const int32_t Casc::DataFile::Read(void * Pointer, int32_t Size)
{
    if (FileHandle == INVALID_HANDLE_VALUE)
        return -1;

    DWORD Result = 0;
    ReadFile(FileHandle, Pointer, (DWORD)Size, (LPDWORD)&Result, NULL);
    return (size_t)Result;
}

const HANDLE Casc::DataFile::GetFileHandle() const
{
    return FileHandle;
}

const int64_t Casc::DataFile::GetIndex() const
{
    return ArchiveIndex;
}

const uint64_t Casc::DataFile::SetPosition(uint64_t Position)
{
    if (FileHandle == INVALID_HANDLE_VALUE)
        return -1;

    LARGE_INTEGER Input;
    Input.QuadPart = (LONGLONG)Position;
    LARGE_INTEGER Result { 0 };
    SetFilePointerEx(FileHandle, Input, &Result, FILE_BEGIN);
    return Result.QuadPart;
}

const int64_t Casc::DataFile::GetPosition()
{
    if (FileHandle == INVALID_HANDLE_VALUE)
        return -1;

    LARGE_INTEGER Result{ 0 };
    SetFilePointerEx(FileHandle, { 0 }, &Result, FILE_CURRENT);
    return Result.QuadPart;
}

void Casc::FileReader::Create(uint64_t TotalLength, bool Consume)
{
    Length = TotalLength;
}

void Casc::FileReader::Consume()
{
    Cache = std::make_unique<uint8_t[]>(Length);
    CacheStartPosition = 0;
    CacheEndPosition = Length;

    for (auto& Span : Spans)
    {
        for (auto& Frame : Span.Frames)
        {
            auto TempBuffer = std::make_unique<uint8_t[]>(Frame.EncodedSize);

            Parent.ReadDataFile(Span.ArchiveIndex, (void*)TempBuffer.get(), Frame.ArchiveOffset, Frame.EncodedSize);


            switch (*TempBuffer.get())
            {
            case 0x4E:
                std::memcpy(TempBuffer.get() + 1, Cache.get() + Frame.VirtualStartOffset, Frame.ContentSize);
                break;
            case 0x5A:
                Compression::DecompressZLibBlock((int8_t*)TempBuffer.get() + 1, (int8_t*)Cache.get() + Frame.VirtualStartOffset, Frame.EncodedSize - 1, Frame.ContentSize);
                break;
            default:
                break;
            }
        }
    }
}

std::unique_ptr<uint8_t[]> Casc::FileReader::Read(uint64_t Length, uint64_t & Result)
{
    // Result
    DWORD LengthRead = 0;
    // We can read the block
    auto ResultBlock = std::make_unique<uint8_t[]>((size_t)Length);
    // Zero out the memory
    std::memset(ResultBlock.get(), 0, (size_t)Length);
    // Read it
    Result = (uint64_t)Read(ResultBlock.get(), 0, Length);
    // Return result
    return ResultBlock;
}

size_t Casc::FileReader::Read(uint8_t* buffer, size_t offset, size_t count)
{
    auto readStartPos = InternalPosition;

    // Our we outside the file?
    if (readStartPos >= Length)
        return 0;

    auto toRead = count;
    size_t consumed = 0;

    while (true)
    {
        auto readEndPos = readStartPos + offset;
        auto cacheAvailable = CacheEndPosition - readStartPos;

        if (cacheAvailable > 0 && Cache != nullptr)
        {
            // We can take from the cache
            if (CacheStartPosition <= readStartPos && CacheEndPosition > readStartPos)
            {
                auto p = (int)(readStartPos - CacheStartPosition);
                auto n = (int)(toRead <= (size_t)cacheAvailable) ? toRead : cacheAvailable;

                if (n <= 8)
                {
                    int byteCount = (int)n;
                    while (--byteCount >= 0)
                        buffer[offset + byteCount] = Cache[p + byteCount];
                }
                else
                {
                    std::memcpy(buffer + offset, Cache.get() + p, n);
                }

                toRead -= n;
                InternalPosition += n;
                offset += n;
                consumed += n;
            }
        }

        // We've satisfied what we need
        if (toRead == 0)
            break;

        readStartPos = InternalPosition;

        // Our we outside the file?
        if (readStartPos >= Length)
            break;

        // Find next span that is at the current position and request buffer
        auto span = FindSpan(readStartPos);
        auto frame = span->FindFrame(readStartPos);

        auto TempBuffer = std::make_unique<uint8_t[]>(frame->EncodedSize);

        if(Parent.ReadDataFile(span->ArchiveIndex, (void*)TempBuffer.get(), frame->ArchiveOffset, frame->EncodedSize) != frame->EncodedSize)
            break;

        // New Cache
        Cache = std::make_unique<uint8_t[]>(frame->ContentSize);
        CacheStartPosition = frame->VirtualStartOffset;
        CacheEndPosition = frame->VirtualEndOffset;

        switch (*TempBuffer.get())
        {
        case 0x4E:
            std::memcpy(Cache.get(), TempBuffer.get() + 1, frame->ContentSize);
            break;
        case 0x5A:
            Compression::DecompressZLibBlock((int8_t*)TempBuffer.get() + 1, (int8_t*)Cache.get(), frame->EncodedSize - 1, frame->ContentSize);
            break;
        default:
            break;
        }
    }

    return consumed;
}

Casc::TVFSHandler::PathTableNode Casc::TVFSHandler::ParsePathNode(FileReader& Reader)
{
    PathTableNode entry = {};

    auto buf = Reader.Peek<uint8_t>();

    if (buf == 0)
    {
        entry.Flags |= PathTableNodeFlags::PathSeparatorPre;
        Reader.Advance(1);
        buf = Reader.Peek<uint8_t>();
    }

    if (buf < 0x7F && buf != 0xFF)
    {
        Reader.Advance(1);
        Reader.Read((uint8_t*)entry.Name, 0, buf);
        entry.NameSize = buf;
        buf = Reader.Peek<uint8_t>();
    }

    if (buf == 0)
    {
        entry.Flags |= PathTableNodeFlags::PathSeparatorPost;
        Reader.Advance(1);
        buf = Reader.Peek<uint8_t>();
    }

    if (buf == 0xFF)
    {
        Reader.Advance(1);
        entry.Value = Reader.Read<uint32be_t>();
        entry.Flags |= PathTableNodeFlags::IsNodeValue;
    }
    else
    {
        entry.Flags |= PathTableNodeFlags::PathSeparatorPost;
    }

    return entry;

    return PathTableNode();
}

void Casc::TVFSHandler::AddFileEntry(FileReader& Reader, const std::string & Name, const uint32_t Offset)
{
    auto PathTableCurrent = Reader.GetPosition();

    Reader.SetPosition(Header.VFSTableOffset + Offset);

    auto SpanCount = Reader.Read<uint8_t>();

    FileSystem::Entry FileEntry(Name);

    for (uint8_t i = 0; i < SpanCount; i++)
    {
        auto RefFileOffset = (uint32_t)Reader.Read<uint32be_t>();
        auto sizeOfSpan    = (uint32_t)Reader.Read<uint32be_t>();
        auto CFTOffset     = ReadCFTOffset(Reader);

        auto VFSTableCurrent = Reader.GetPosition();
        Reader.SetPosition(Header.CFTTableOffset + CFTOffset);

        // Attempt to locate this entry, since some may not exist or may be online
        IndexKey Index(Header.EncodingKeySize);

        Reader.Read(Index.EncodingKey, 0, Header.EncodingKeySize);

        FileEntry.KeyEntries.push_back(Index);

        Reader.SetPosition(VFSTableCurrent);
    }

    FileEntries[FileEntry.Name] = FileEntry;

    Reader.SetPosition(PathTableCurrent);
}

void Casc::TVFSHandler::ParsePathTable(FileReader& Reader, int64_t end, std::string& Builder)
{
    auto CurrentPosition = Builder.size();

    while (Reader.GetPosition() < end)
    {
        auto Entry = ParsePathNode(Reader);

        if ((Entry.Flags & PathTableNodeFlags::PathSeparatorPre))
            Builder.append("\\");
        Builder.append(Entry.Name);
        if ((Entry.Flags & PathTableNodeFlags::PathSeparatorPost))
            Builder.append("\\");

        if ((Entry.Flags & PathTableNodeFlags::IsNodeValue))
        {
            if ((Entry.Value & 0x80000000) != 0)
            {
                auto FolderSize  = Entry.Value & 0x7FFFFFFF;
                auto FolderStart = Reader.GetPosition();
                auto FolderEnd   = FolderStart + FolderSize - 4;

                ParsePathTable(Reader, FolderEnd, Builder);
            }
            else
            {
                AddFileEntry(Reader, Builder, Entry.Value);
            }

            Builder.erase(CurrentPosition, Builder.size() - CurrentPosition);
        }
    }
}

uint32_t Casc::TVFSHandler::ReadCFTOffset(FileReader & Reader)
{
    uint8_t Buffer[4];

    if (Header.CFTTableSize > 0xFFFFFF)
    {
        Reader.Read(Buffer, 0, 4);
        return (uint32_t)((Buffer[0] << 24) | (Buffer[1] << 16) | (Buffer[2] << 8) | Buffer[3]);
    }
    else if (Header.CFTTableSize > 0xFFFF)
    {
        Reader.Read(Buffer, 0, 3);
        return (uint32_t)((Buffer[0] << 16) | (Buffer[1] << 8) | Buffer[2]);
    }
    else if (Header.CFTTableSize > 0xFF)
    {
        Reader.Read(Buffer, 0, 2);
        return (uint32_t)((Buffer[0] << 8) | Buffer[1]);
    }
    else
    {
        Reader.Read(Buffer, 0, 1);
        return Buffer[0];
    }
}

void Casc::TVFSHandler::Parse(FileReader& Reader)
{
    Reader.SetPosition(0);
    Reader.Read((uint8_t*)&Header, 0, sizeof(Header));

    if (
        Header.FormatVersion == 1 &&
        Header.HeaderSize == 0x26 &&
        Header.EncodingKeySize == 9 && 
        Header.PatchKeySize == 9
       )
    {
        Reader.SetPosition(Header.PathTableOffset);
       
        std::string Builder;
        Builder.reserve(MAX_PATH);
        ParsePathTable(Reader, Reader.GetPosition() + Header.PathTableSize, Builder);
    }
    
}
