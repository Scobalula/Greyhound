#pragma once
#include "Hashing.h"

namespace Casc
{
#pragma pack(push, 1)
    struct BlockTableHeader
    {
        uint8_t EncodingKey[16];
        uint32_t ContentSize;
        uint16_t Flags;
        uint32_t JenkinsHash;
        uint32_t Checksum;
        uint32_t Signature;
        uint32_t HeaderSize;
        uint8_t TableFormat;
        uint8_t FrameCountBE[3];

        const uint32_t GetFrameCount()
        {
            return (FrameCountBE[0] << 16) | (FrameCountBE[1] << 8) | FrameCountBE[2];
        }
    };
#pragma pack(pop)

    struct BlockTableEntry
    {
    private:
        uint8_t EncodedSizeBE[4];
        uint8_t ContentSizeBE[4];
        uint8_t Hash[16];

    public:
        const uint32_t GetEncodedSize()
        {
            return (EncodedSizeBE[0] << 24) | (EncodedSizeBE[1] << 16) | (EncodedSizeBE[2] << 8) | EncodedSizeBE[3];
        }

        const uint32_t GetContentSize()
        {
            return (ContentSizeBE[0] << 24) | (ContentSizeBE[1] << 16) | (ContentSizeBE[2] << 8) | ContentSizeBE[3];
        }
    };
    struct uint32be_t
    {
        uint8_t Data[4];

        operator uint32_t() const
        {
            return (uint32_t)((Data[0] << 24) | (Data[1] << 16) | (Data[2] << 8) | Data[3]);
        }
    };
    struct IndexFileBlock
    {
        uint32_t BlockSize;
        uint32_t BlockHash;
    };

    struct IndexFileHeader
    {
        uint16_t    IndexVersion;
        uint8_t     BucketIndex;
        uint8_t     ExtraBytes;
        uint8_t     EncodedSizeLength;
        uint8_t     StorageOffsetLength;
        uint8_t     EncodingKeyLength;
        uint8_t     FileOffsetBits;
        uint64_t    SegmentSize;

    };

    class IndexKey
    {
    public:
        // Encoding Key
        uint8_t EncodingKey[16];
        // Encoding Key Size
        size_t EncodingKeySize;

        // Creates a new Index Key
        IndexKey();
        // Creates a new Index Key with the given Size
        IndexKey(const size_t KeySize) : EncodingKeySize(KeySize) {}
        // Creates a new Index Key with the given Key from an Index File
        IndexKey(const uint8_t* EntryBuffer, const IndexFileHeader& Header);
        // Creates a new Index Key with the given Key and Size
        IndexKey(const uint8_t* Key, const size_t KeySize);
        // Creates a new Index Key with the given Key and Size
        IndexKey(const std::string& Key, const size_t KeySize);

        // Compares the Keys for Equality
        bool operator==(const IndexKey& Compare) const
        {
            // Check sizes
            if (Compare.EncodingKeySize != EncodingKeySize)
                return false;

            return std::memcmp(Compare.EncodingKey, EncodingKey, EncodingKeySize > 16 ? 16 : EncodingKeySize) == 0;
        }
    };

    // A structure for Index Key Hashing for use in a Map
    struct IndexKeyHasher
    {
        // Generates a Hash for the given Key
        std::size_t operator() (const IndexKey & ToHash) const
        {
            return (size_t)Hashing::HashXXHashStream((int8_t*)ToHash.EncodingKey, ToHash.EncodingKeySize > 16 ? 16 : ToHash.EncodingKeySize);
        }
    };

    // A class to hold an Index Entry
    class IndexEntry
    {
    public:
        // Encoding Key
        uint8_t EncodingKey[16];
        // Encoding Key Size
        size_t EncodingKeySize;
        // Offset within Archive
        uint64_t Offset;
        // Full size of the Data within Archive
        uint64_t Size;
        // Archive Index
        uint64_t ArchiveIndex;

        // Creates a new Index Entry
        IndexEntry();
        // Creates a new Index Entry from the Buffer from the File
        IndexEntry(const uint8_t* EntryBuffer, const IndexFileHeader& Header);
    };

    // A class to hold a Data File
    class DataFile
    {
    private:
        // Data File Handle
        HANDLE FileHandle;
    public:
        // Creates a new Data File
        DataFile(const std::string& Path);
        // Closes the Data File
        ~DataFile();

        // Reads from the Data File
        const int32_t Read(void* Pointer, int32_t Size);
        // Sets the current position of the Data File
        const uint64_t SetPosition(uint64_t Position);
        // Gets the current position of the Data File
        const int64_t GetPosition();

        // Gets the File Handle of this Data File
        const HANDLE GetFileHandle() const;
    };

    // A class to hold a File Frame
    class FileFrame
    {
    public:
        // Virtual Start Offset
        uint64_t VirtualStartOffset;
        // Virtual End Offset
        uint64_t VirtualEndOffset;
        // Offset within the Data File
        uint64_t ArchiveOffset;
        // Encoded Size (compressed, including flag, etc)
        uint32_t EncodedSize;
        // Content Size
        uint32_t ContentSize;
    };

    class FileSpan
    {
    public:
        // Span Frames
        std::vector<FileFrame> Frames;
        // Virtual Start Offset
        uint64_t VirtualStartOffset;
        // Virtual End Offset
        uint64_t VirtualEndOffset;
        // Offset within the Data File
        uint64_t ArchiveOffset;
        // Data File Index
        uint64_t ArchiveIndex;

        // Creates a new Span
        FileSpan(uint64_t Index) : ArchiveIndex(Index) {}

        // Finds a frame by offset within this Span
        FileFrame* FindFrame(uint64_t Position)
        {
            for (auto& Frame : Frames)
            {
                if (Position >= Frame.VirtualStartOffset && Position < Frame.VirtualEndOffset)
                {
                    return &Frame;
                }
            }

            // Nothing Found
            return nullptr;
        }
    };

    class Container;

    class FileReader
    {
        // File Spans
        std::vector<FileSpan> Spans;
        // Buffer Cache
        std::unique_ptr<uint8_t[]> Cache;
        // Start Position of the Cache
        int64_t CacheStartPosition;
        // End Position of the Cache
        int64_t CacheEndPosition;
        // Our Internal Position
        int64_t InternalPosition;
        // The length of the File
        int64_t Length;
        // Parent Storage
        Container& Parent;
    public:
        // Initializes a new File Reader
        FileReader(Container& ParentContainer) : Length(-1), Parent(ParentContainer){}

        void Create(uint64_t TotalLength, bool Consume);

        FileSpan* FindSpan(uint64_t Position)
        {
            for (auto& Span : Spans)
            {
                if (Position >= Span.VirtualStartOffset && Position < Span.VirtualEndOffset)
                {
                    return &Span;
                }
            }

            return nullptr;
        }

        void Consume();

        // Read a block of data from the file with the given type
        template <class T>
        T Read()
        {
            // We must read the data based on type.
            T ResultValue;
            // Zero out the memory
            std::memset(&ResultValue, 0, sizeof(ResultValue));
            // Read the value from the process
            Read((uint8_t*)&ResultValue, 0, sizeof(ResultValue));
            // Return the result
            return ResultValue;
        }

        template <class T>
        uint8_t Peek()
        {
            auto Temp = InternalPosition;
            auto Result = Read<T>();
            InternalPosition = Temp;
            return Result;
        }

        // Read a block of data from the file with a given length
        std::unique_ptr<uint8_t[]> Read(uint64_t Length, uint64_t& Result);

        std::string ReadString(size_t size)
        {
            std::string result;
            result.resize(size);

            Read((uint8_t*)result.c_str(), 0, size);

            return result;
        }

        size_t Read(uint8_t* Pointer, size_t Offset, size_t Count);

        void AddSpan(const FileSpan& Span)
        {
            Spans.push_back(Span);
        }

        void Advance(uint64_t position)
        {
            InternalPosition += position;
        }

        void SetPosition(uint64_t position)
        {
            InternalPosition = position;
        }


        const int64_t GetPosition() const
        {
            return InternalPosition;
        }

        const int64_t GetLength() const
        {
            return Length;
        }

        void SetLength(int64_t TotalLength)
        {
            if (Length != -1)
                throw std::exception("File Length has already been set.");

            Length = TotalLength;
        }
    };

    class FileSystem
    {
    public:
        class Entry
        {
        public:
            // Full name of the Entry
            std::string Name;
            // Index Entries
            std::vector<IndexKey> KeyEntries;
            // Size of the Entry
            size_t Size;
            // Whether this file exists or not
            bool Exists;

            Entry() : Size(0), Exists(true) {}

            Entry(const std::string & EntryName) : Size(0), Exists(true), Name(EntryName) {}
        };

        std::map<std::string, Entry> FileEntries;

    public:
        // Parses the File System
        virtual void Parse(FileReader& Reader) = 0;
    };


    class TVFSHandler : public FileSystem
    {
    private:
        struct
        {
            uint32_t Signature;
            uint8_t FormatVersion;
            uint8_t HeaderSize;
            uint8_t EncodingKeySize;
            uint8_t PatchKeySize;
            uint32be_t Flags;
            uint32be_t PathTableOffset;
            uint32be_t PathTableSize;
            uint32be_t VFSTableOffset;
            uint32be_t VFSTableSize;
            uint32be_t CFTTableOffset;
            uint32be_t CFTTableSize;
            uint16_t MaxDepth;
        } Header;

        enum PathTableNodeFlags : uint32_t
        {
            None = 0x0000,
            PathSeparatorPre = 0x0001,
            PathSeparatorPost = 0x0002,
            IsNodeValue = 0x0004,
        };

        struct PathTableNode
        {
            char Name[MAX_PATH];
            uint8_t NameSize;
            uint32_t Flags;
            uint32_t Value;

            PathTableNode() : Flags(0), Value(0)
            {
                std::memset(Name, 0, sizeof(Name));
            }
        };
    private:
        PathTableNode ParsePathNode(FileReader& Reader);

        void AddFileEntry(FileReader& Reader, const std::string& Name, const uint32_t Offset);

        void ParsePathTable(FileReader& Reader, int64_t end, std::string& Builder);

        uint32_t ReadCFTOffset(FileReader& Reader);
    public:
        void Parse(FileReader& Reader);
    };

    class Container
    {
    private:
        // Main Path
        std::string Path;
        // Data Path
        std::string DataPath;
        // Data Path
        std::string BuildInfoPath;
        // Config Build Key
        std::string BuildKey;
        // VFS Root Key
        std::string VFSRootKey;
        // Data Entries
        std::unordered_map<IndexKey, IndexEntry, IndexKeyHasher> DataEntries;
        // Data Files
        std::unique_ptr<std::shared_ptr<DataFile>[]> DataFiles;
        // File System Handler
        std::unique_ptr<FileSystem> FileSystemHandler;
        // Container Mutex
        std::mutex Mutex;
        // If we're closed
        bool Closed;

        // Loads Indices Files
        void LoadIndexFiles();

        void LoadDataFiles();

        void LoadBuildInfo();

        void LoadConfigInfo();
    public:
        Container() { }
        Container(const std::string& path);
        ~Container();

        void Open(const std::string& path);

        FileReader OpenFile(const std::string& FileName);

        FileReader OpenFile(IndexEntry entry);
        // Reads from the Data File
        const int32_t ReadDataFile(const size_t ArchiveIndex, void* Pointer, const int64_t Offset, const int32_t Size);

        // Gets the Files
        const std::map<std::string, FileSystem::Entry>& GetFileEntries() const;

        void Close();
    };
}

