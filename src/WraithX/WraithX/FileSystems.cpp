#include "stdafx.h"

// The class we are implementing
#include "FileSystems.h"

// We need the strings utilities
#include "Strings.h"

bool FileSystems::FileExists(const std::string& File)
{
    // Check if we have an actual path
    if (Strings::IsNullOrWhiteSpace(File)) { return false; }

    // Handle expanding the full path
    // TODO: Path expanding

    // Check whether the last char is a separator
    if (File[File.length()] == DirectorySeparatorChar || File[File.length()] == AltDirectorySeparatorChar)
    {
        // Not a file
        return false;
    }

    // Check the path using the FileAttributes data
    WIN32_FILE_ATTRIBUTE_DATA FileAttrs;
    // Fetch them
    auto Result = GetFileAttributesExA(File.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &FileAttrs);

    // Check result
    if (Result)
    {
        // We are a file if the following are met
        return (FileAttrs.dwFileAttributes != -1) && ((FileAttrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
    }

    // Failed
    return false;
}

bool FileSystems::DeleteFile(const std::string& File)
{
    // Check if we have an actual path
    if (Strings::IsNullOrWhiteSpace(File)) { return false; }

    // Handle expanding the full path
    // TODO: Path expanding

    // Attempt to delete the file
    auto Result = DeleteFileA(File.c_str());

    // Check if we deleted it
    if (Result && GetLastError() == 0) { return true; }

    // Failed to delete
    return false;
}

void FileSystems::MoveFile(const std::string& OriginalFile, const std::string& NewFile)
{
    // Check if we have an actual path (Original)
    if (Strings::IsNullOrWhiteSpace(OriginalFile)) { return; }

    // Check if we have an actual path (New)
    if (Strings::IsNullOrWhiteSpace(NewFile)) { return; }

    // Handle expanding both full paths
    // TODO: Path expanding

    // Make sure the original exists
    if (!FileExists(OriginalFile)) { return; }

    // Attempt to move the file
    auto Result = MoveFileA(OriginalFile.c_str(), NewFile.c_str());
}

void FileSystems::CopyFile(const std::string& OriginalFile, const std::string& NewFile)
{
    // Check if we have an actual path (Original)
    if (Strings::IsNullOrWhiteSpace(OriginalFile)) { return; }

    // Check if we have an actual path (New)
    if (Strings::IsNullOrWhiteSpace(NewFile)) { return; }

    // Handle expanding both full paths
    // TODO: Path expanding

    // Attempt to copy the file (overwriting if  exists)
    auto Result = CopyFileA(OriginalFile.c_str(), NewFile.c_str(), false);
}

bool FileSystems::DirectoryExists(const std::string& Path)
{
    // Check if we have an actual path
    if (Strings::IsNullOrWhiteSpace(Path)) { return false; }

    // Handle expanding both full paths
    // TODO: Path expanding
    
    // Check the path using the FileAttributes data
    WIN32_FILE_ATTRIBUTE_DATA FileAttrs;
    // Fetch them
    auto Result = GetFileAttributesExA(Path.c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &FileAttrs);

    // Check result
    if (Result)
    {
        // We are a folder if the following are met
        return (FileAttrs.dwFileAttributes != -1) && ((FileAttrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    }

    // Failed
    return false;
}

bool FileSystems::DeleteDirectory(const std::string& Path)
{
    // Check if we have an actual path
    if (Strings::IsNullOrWhiteSpace(Path)) { return false; }

    // Handle expanding both full paths
    // TODO: Path expanding

    // Make sure it exists first
    if (!DirectoryExists(Path)) { return false; }

    // Build search pattern (Path\\*)
    auto SearchPatern = (Path[Path.length() - 1] == DirectorySeparatorChar || Path[Path.length() - 1] == AltDirectorySeparatorChar) ? Path + "*" : Path + DirectorySeparatorChar + "*";

    // File info buffer
    WIN32_FIND_DATAA FileInfo;
    // We must recursively delete the files (and folders) before deleting the top level one
    auto FirstResult = FindFirstFileA(SearchPatern.c_str(), &FileInfo);

    // Iterate until we're at top-level
    do
    {
        // Check whether or not we are a directory
        auto isDir = (0 != (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

        // Check
        if (isDir)
        {
            // Skip over '.' '..'
            if (_strnicmp(FileInfo.cFileName, ".", 1) == 0 || _strnicmp(FileInfo.cFileName, "..", 2) == 0) { continue; }

            // Check if we can recurse
            auto shouldRecurse = (0 == (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT));

            // See if we should
            if (shouldRecurse)
            {
                // Make path
                auto newFullPath = CombinePath(Path, FileInfo.cFileName);
                // Delete it
                auto Result = DeleteDirectory(newFullPath);
                // Check
                if (!Result) { return false; }
            }
            else
            {
                // TODO: Windows API unmounts the volume, then deletes the point... https://referencesource.microsoft.com/#mscorlib/system/io/directory.cs,1409
            }
        }
        else
        {
            // We have a file, just delete it
            auto newFullPath = CombinePath(Path, FileInfo.cFileName);
            // Delete the file
            auto Result = DeleteFile(newFullPath);
            // Check
            if (!Result) { return false; }
        }

        // Keep iterating over files
    } while (FindNextFileA(FirstResult, &FileInfo));

    // If we made it here, delete top level folder
    auto Result = RemoveDirectoryA(Path.c_str());

    // Check if we made it
    if (Result) { return true; }

    // Failed to delete
    return false;
}

void FileSystems::CreateDirectory(const std::string& Path)
{
    // Make sure we have a path
    if (Strings::IsNullOrWhiteSpace(Path)) { return; }

    // Handle expanding full path
    // TODO: Path expanding

    // Make sure it doesn't exist
    if (DirectoryExists(Path)) { return; }

    // Cache full length
    int32_t FullLength = (int32_t)Path.length();

    // Calculate root length
    int32_t RootLength = (FullLength > 2 && Path[1] == VolumeSeparatorChar && (Path[2] == DirectorySeparatorChar || Path[2] == AltDirectorySeparatorChar)) ? 3 : 0;

    // We need to trim the trailing slash or the code will try to create 2 directories of the same name
    if (FullLength >= 2 && (Path[FullLength - 1] == DirectorySeparatorChar || Path[FullLength - 1] == AltDirectorySeparatorChar))
    {
        // Trim the slash
        FullLength--;
    }

    // A stack list of directories to make
    std::stack<std::string> DirStack;

    // We must calculate dir chunks
    if (FullLength > RootLength)
    {
        // Cache position minus one
        int32_t i = FullLength - 1;
        // Loop until end
        while (i >= RootLength)
        {
            // Add
            DirStack.push(Path.substr(0, i + 1));
            // Move to next
            while (i > RootLength && Path[i] != DirectorySeparatorChar && Path[i] != AltDirectorySeparatorChar)
            {
                // Move 1
                i--;
            }
            // Skip last one
            i--;
        }
    }

    // We must iterate until the stack is empty
    while (DirStack.size() > 0)
    {
        // Take the item
        auto DirMake = DirStack.top();

        // Create the path if we can
        auto Result = CreateDirectoryA(DirMake.c_str(), NULL);

        // Remove it
        DirStack.pop();
    }
}

std::vector<std::string> FileSystems::GetFiles(const std::string& Path, const std::string& SearchPattern)
{
    // A list of files
    std::vector<std::string> FilesResult;

    // Build search pattern
    auto& SearchBuild = (Path[Path.length() - 1] == DirectorySeparatorChar || Path[Path.length() - 1] == AltDirectorySeparatorChar) ? Path + SearchPattern : Path + DirectorySeparatorChar + SearchPattern;

    // File info buffer
    WIN32_FIND_DATAA FileInfo;
    // We must search for all matching files
    auto FirstResult = FindFirstFileA(SearchBuild.c_str(), &FileInfo);

    // Ensure we found one
    if (FirstResult != INVALID_HANDLE_VALUE)
    {
        // Make sure this is a file we need
        if (_strnicmp(FileInfo.cFileName, ".", 1) != 0 && _strnicmp(FileInfo.cFileName, "..", 2) != 0)
        {
            // Check type
            auto isDir = (0 != (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

            // Add if it's not a dir
            if (!isDir)
            {
                // Add it
                FilesResult.push_back(CombinePath(Path, FileInfo.cFileName));
            }
        }
        // Loop until end of files
        while (FindNextFileA(FirstResult, &FileInfo))
        {
            // Verify the file is what we need
            if (_strnicmp(FileInfo.cFileName, ".", 1) == 0 || _strnicmp(FileInfo.cFileName, "..", 2) == 0) { continue; }
            // Check type
            auto isDir = (0 != (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

            // Add if it's not a dir
            if (!isDir)
            {
                // Add it
                FilesResult.push_back(CombinePath(Path, FileInfo.cFileName));
            }
        }
        // Clear handle
        FindClose(FirstResult);
    }

    // Return it
    return FilesResult;
}

std::vector<std::string> FileSystems::GetDirectories(const std::string& Path)
{
    // A list of directories
    std::vector<std::string> FilesResult;

    // Build search pattern
    auto SearchBuild = (Path[Path.length() - 1] == DirectorySeparatorChar || Path[Path.length() - 1] == AltDirectorySeparatorChar) ? Path + "*" : Path + DirectorySeparatorChar + "*";

    // File info buffer
    WIN32_FIND_DATAA FileInfo;
    // We must search for all matching directories
    auto FirstResult = FindFirstFileA(SearchBuild.c_str(), &FileInfo);

    // Ensure we found one
    if (FirstResult != INVALID_HANDLE_VALUE)
    {
        // Make sure this is a directory we need
        if (_strnicmp(FileInfo.cFileName, ".", 1) != 0 && _strnicmp(FileInfo.cFileName, "..", 2) != 0)
        {
            // Check type
            auto isDir = (0 != (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

            // Add if it's a dir
            if (isDir)
            {
                // Add it
                FilesResult.push_back(CombinePath(Path, FileInfo.cFileName));
            }
        }
        // Loop until end of directories
        while (FindNextFileA(FirstResult, &FileInfo))
        {
            // Verify the directory is what we need
            if (_strnicmp(FileInfo.cFileName, ".", 1) == 0 || _strnicmp(FileInfo.cFileName, "..", 2) == 0) { continue; }
            // Check type
            auto isDir = (0 != (FileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));

            // Add if it's a dir
            if (isDir)
            {
                // Add it
                FilesResult.push_back(CombinePath(Path, FileInfo.cFileName));
            }
        }
    }

    // Return it
    return FilesResult;
}

std::string FileSystems::CombinePath(const std::string& Path1, const std::string& Path2)
{
    // Check if we have an actual paths for 1 and 2 (If both aren't return nothing)
    if (Strings::IsNullOrWhiteSpace(Path1) && Strings::IsNullOrWhiteSpace(Path2)) { return ""; }

    // Check lengths and combine
    if (Path2.length() == 0) { return Path1; }
    if (Path1.length() == 0) { return Path2; }

    // Check if path 2 is rooted
    if (IsPathRooted(Path2)) { return Path2; }

    // Otherwise merge the two paths
    auto LastChar = Path1[Path1.length() - 1];
    // Check if we have a separator
    if (LastChar != DirectorySeparatorChar && LastChar != AltDirectorySeparatorChar && LastChar != VolumeSeparatorChar)
    {
        // Merge with separator
        return Path1 + DirectorySeparatorChar + Path2;
    }

    // No separator, just merge
    return Path1 + Path2;
}

bool FileSystems::IsPathRooted(const std::string& Path)
{
    // Fetch length
    auto Length = Path.length();
    
    // Compare
    if ((Length >= 1 && (Path[0] == DirectorySeparatorChar || Path[0] == AltDirectorySeparatorChar)) || Length >= 2 && Path[1] == VolumeSeparatorChar)
    {
        // We are rooted
        return true;
    }

    // Not rooted
    return false;
}

std::string FileSystems::GetFileName(const std::string& Path)
{
    // Make sure we have an actual path
    if (Strings::IsNullOrWhiteSpace(Path)) { return ""; }

    // Cache the path length
    int32_t Length = (int32_t)Path.length();

    // Loop through and find the end
    for (int32_t i = Length; --i >= 0;)
    {
        // Get char
        auto Char = Path[i];
        // Compare with separators
        if (Char == DirectorySeparatorChar || Char == AltDirectorySeparatorChar || Char == VolumeSeparatorChar)
        {
            // Found it, return path
            return Path.substr(i + 1, Length - i - 1);
        }
    }

    // Failed to find it
    return Path;
}

std::string FileSystems::GetFileNameWithoutExtension(const std::string& Path)
{
    // First, get just the name
    auto NamePath = GetFileName(Path);
    
    // Make sure it isn't null
    if (Strings::IsNullOrWhiteSpace(Path)) { return ""; }

    // Find the extension
    auto ExtSeparator = NamePath.find_last_of(".");
    // See if we found it
    if (ExtSeparator == std::string::npos)
    {
        // No extension
        return NamePath;
    }
    else
    {
        // Clean up the extension
        return NamePath.substr(0, ExtSeparator);
    }
}

std::string FileSystems::GetFileNamePurgeExtensions(const std::string& Path)
{
    // First, get just the name
    auto NamePath = GetFileName(Path);

    // Make sure it isn't null
    if (Strings::IsNullOrWhiteSpace(Path)) { return ""; }

    // Find the extension
    auto ExtSeparator = NamePath.find_first_of(".");
    // See if we found it
    if (ExtSeparator == std::string::npos)
    {
        // No extension
        return NamePath;
    }
    else
    {
        // Clean up the extension
        return NamePath.substr(0, ExtSeparator);
    }
}

std::string FileSystems::GetExtension(const std::string& Path)
{
    // Make sure we have an actual path
    if (Strings::IsNullOrWhiteSpace(Path)) { return ""; }

    // Cache the length
    int32_t Length = (int32_t)Path.length();

    // Loop through and find it
    for (int32_t i = Length; --i >= 0;)
    {
        // Buffer the char
        auto Char = Path[i];

        // Check
        if (Char == '.')
        {
            // Make sure we have space
            if (i != Length - 1)
            {
                // Take from substring
                return Path.substr(i, Length - i);
            }
            else
            {
                // No extension
                return "";
            }
        }

        // Check if we're at the separator
        if (Char == DirectorySeparatorChar || Char == AltDirectorySeparatorChar || Char == VolumeSeparatorChar)
        {
            // Done
            break;
        }
    }

    // Failed to find extension
    return "";
}

std::string FileSystems::GetDirectoryName(const std::string& Path)
{
    // Make sure we have an actual path
    if (Strings::IsNullOrWhiteSpace(Path)) { return ""; }

    // Handle expanding full path
    // TODO: Path expanding

    // Cache full length
    int32_t FullLength = (int32_t)Path.length();

    // Calculate the root length
    int32_t RootLength = (FullLength > 2 && Path[1] == VolumeSeparatorChar && (Path[2] == DirectorySeparatorChar || Path[2] == AltDirectorySeparatorChar)) ? 3 : 0;

    // The iterator
    int32_t i = FullLength;
    // Check and loop
    if (i > RootLength)
    {
        // Iterate until end
        while (i > RootLength && Path[--i] != DirectorySeparatorChar && Path[i] != AltDirectorySeparatorChar);
        // Return the new dir
        return Path.substr(0, i);
    }

    // Failed to find it
    return "";
}

std::string FileSystems::GetPathRoot(const std::string& Path)
{
    // Make sure we have an actual path
    if (Strings::IsNullOrWhiteSpace(Path)) { return ""; }

    // Return the full root path via the root size, if any
    return Path.substr(0, GetRootLength(Path));
}

uint32_t FileSystems::GetRootLength(const std::string& Path)
{
    // Reserve space for the length, and path length (cached)
    int32_t i = 0;
    int32_t FullLength = (int32_t)Path.length();

    // Macro for each separator
    auto IsDirectorySeparator = [](const char C)
    {
        return (C == DirectorySeparatorChar || C == AltDirectorySeparatorChar);
    };

    // UNC names and relative paths
    if (FullLength >= 1 && IsDirectorySeparator(Path[0]))
    {
        // Separator
        i = 1;

        // Handle extended separation
        if (FullLength >= 2 && IsDirectorySeparator(Path[1]))
        {
            // At least two separators
            i = 2;
            int32_t n = 2;

            // Advance until we aren't separators
            while (i < FullLength && ((Path[i] != DirectorySeparatorChar && Path[i] != AltDirectorySeparatorChar) || --n > 0))
                i++;
        }
    }
    else if (FullLength >= 2 && Path[1] == VolumeSeparatorChar)
    {
        // C:
        i = 2;

        // Handle separator
        if (FullLength >= 3 && IsDirectorySeparator(Path[2]))
            i++;
    }
    
    // Return result
    return (uint32_t)i;
}

std::string FileSystems::GetApplicationPath()
{
    // A buffer
    WCHAR Buffer[MAX_PATH];
    // Clean it
    std::memset(Buffer, 0, (sizeof(WCHAR) * MAX_PATH));
    // Fetch it
    GetModuleFileName(GetModuleHandle(NULL), Buffer, MAX_PATH);
    // Return it, but get the directory name, not the exe name
    return GetDirectoryName(Strings::ToNormalString(std::wstring(Buffer)));
}

HICON FileSystems::ExtractFileIcon(const std::string& Path)
{
    // Result buffer
    HICON ResultIcon = NULL;

    // Attempt to load the highest resolution level first, from index 0 64/48/32/16
    if (SHDefExtractIconA(Path.c_str(), 0, 0, &ResultIcon, NULL, 64) == S_OK) { return ResultIcon; }
    if (SHDefExtractIconA(Path.c_str(), 0, 0, &ResultIcon, NULL, 48) == S_OK) { return ResultIcon; }
    if (SHDefExtractIconA(Path.c_str(), 0, 0, &ResultIcon, NULL, 32) == S_OK) { return ResultIcon; }
    if (SHDefExtractIconA(Path.c_str(), 0, 0, &ResultIcon, NULL, 16) == S_OK) { return ResultIcon; }

    // We failed, its null
    return NULL;
}