#pragma once

#include <string>
#include <vector>

// Undefine the macros that the Windows headers include for easy swapping...
#undef MoveFile
#undef DeleteFile
#undef CopyFile
#undef CreateDirectory

// A class that handles the file system and paths
class FileSystems
{
public:
	// Constant variables
	static const char DirectorySeparatorChar = '\\';
	static const char AltDirectorySeparatorChar = '/';
	static const char VolumeSeparatorChar = ':';

	// Checks whether or not a file exists
	static bool FileExists(const std::string& File);
	// Deletes a file from the path
	static bool DeleteFile(const std::string& File);
	// Moves a file from one path to another
	static void MoveFile(const std::string& OriginalFile, const std::string& NewFile);
	// Copies a file from one path to another
	static void CopyFile(const std::string& OriginalFile, const std::string& NewFile);

	// Checks whether or not a directory exists
	static bool DirectoryExists(const std::string& Path);
	// Deletes a directory from the path
	static bool DeleteDirectory(const std::string& Path);

	// Create a directory with the given path
	static void CreateDirectory(const std::string& Path);

	// Get files from the specified path
	static std::vector<std::string> GetFiles(const std::string& Path, const std::string& SearchPattern);
	// Get directories from the specified path
	static std::vector<std::string> GetDirectories(const std::string& Path);

	// Combine two paths
	static std::string CombinePath(const std::string& Path1, const std::string& Path2);
	// Check if a path is rooted (C:\)
	static bool IsPathRooted(const std::string& Path);

	// Gets the name of a file from a path
	static std::string GetFileName(const std::string& Path);
	// Gets the name of a file from a path without the extension
	static std::string GetFileNameWithoutExtension(const std::string& Path);
	// Gets the name of a file from a path without any extensions
	static std::string GetFileNamePurgeExtensions(const std::string& Path);
	// Gets the extension of a file from a path
	static std::string GetExtension(const std::string& Path);
	// Gets the directory from a file path
	static std::string GetDirectoryName(const std::string& Path);
	// Gets the root from a path
	static std::string GetPathRoot(const std::string& Path);
	// Gets the root length from a path
	static uint32_t GetRootLength(const std::string& Path);

	// Gets the directory of the current program
	static std::string GetApplicationPath();

	// Gets the icon associated with the file
	static HICON ExtractFileIcon(const std::string& Path);
};