#include "stdafx.h"

// The class we are implementing
#include "ProcessReader.h"

// We require the patterns utility functions
#include "Patterns.h"

ProcessReader::ProcessReader()
{
	// Init the handle
	ProcessHandle = NULL;
}

ProcessReader::~ProcessReader()
{
	// Detatch if possible
	Detatch();
}

bool ProcessReader::IsRunning()
{
	// Make sure we are attached
	if (ProcessHandle != NULL)
	{
		// Wait for it
		auto Result = WaitForSingleObject(ProcessHandle, 0);
		// Check result
		return (Result == WAIT_TIMEOUT);
	}
	// Not
	return false;
}

void ProcessReader::WaitForExit()
{
	// Wait for it
	WaitForSingleObject(ProcessHandle, INFINITE);
}

void ProcessReader::Detatch()
{
	// If the handle isn't null, close it
	if (ProcessHandle != NULL)
	{
		// Close it
		CloseHandle(ProcessHandle);
	}
	// Set it
	ProcessHandle = NULL;
}

bool ProcessReader::Attach(int ProcessID)
{
	// Attempt to attach to the following process, if we are already attached disconnect first
	if (ProcessHandle != NULL)
	{
		// Detatch first
		Detatch();
	}
	// Attemp to attach
	ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
	// Check
	if (ProcessHandle != NULL)
	{
		// Worked
		return true;
	}
	// Failed
	return false;
}

bool ProcessReader::Attach(const std::string& ProcessName)
{
	// Detatch if we aren't already
	if (ProcessHandle != NULL)
	{
		// Detatch first
		Detatch();
	}

	// Convert to wide
	auto WideName = Strings::ToUnicodeString(ProcessName);

	// Open the specified process
	PROCESSENTRY32 Entry;
	// Prepare the size
	Entry.dwSize = sizeof(PROCESSENTRY32);
	// Create a system snapshot
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	// Loop through results
	if (Process32First(Snapshot, &Entry) == TRUE)
	{
		do
		{
			// Compare the name
			if (_wcsnicmp(Entry.szExeFile, WideName.c_str(), WideName.size()) == 0)
			{
				// We found it
				ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Entry.th32ProcessID);
				// Clean up
				CloseHandle(Snapshot);
				// Stop
				return (ProcessHandle != NULL);
			}
		} while (Process32Next(Snapshot, &Entry));
	}
	// Clean up
	CloseHandle(Snapshot);
	// Failed
	return false;
}

bool ProcessReader::Connect(HANDLE ProcessHandleReference)
{
	// Detatch if we aren't already
	if (ProcessHandle != NULL)
	{
		// Detatch first
		Detatch();
	}
	// Set the new instance
	ProcessHandle = ProcessHandleReference;
	// Check
	if (ProcessHandle != NULL) { return true; }
	// Failed
	return false;
}

uintptr_t ProcessReader::GetMainModuleAddress()
{
	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// We can get it
		DWORD ProcessID = GetProcessId(ProcessHandle);
		// Create helper
		HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
		// Check it
		if (Snapshot == INVALID_HANDLE_VALUE)
		{
			// Cancel
			return NULL;
		}
		// Try and get the value
		uintptr_t MainModuleAddress = 0;
		// A holder for the info
		MODULEENTRY32 moduleData;
		// Set size
		moduleData.dwSize = sizeof(MODULEENTRY32);
		// Get first one
		if (!Module32First(Snapshot, &moduleData))
		{
			// Failed
			CloseHandle(Snapshot);
			// Cancel
			return NULL;
		}
		// Set it
		MainModuleAddress = (uintptr_t)moduleData.modBaseAddr;
		// Clean up
		CloseHandle(Snapshot);
		// Return it
		return MainModuleAddress;
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

uintptr_t ProcessReader::GetMainModuleMemorySize()
{
	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// We can get it
		DWORD ProcessID = GetProcessId(ProcessHandle);
		// Create helper
		HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ProcessID);
		// Check it
		if (Snapshot == INVALID_HANDLE_VALUE)
		{
			// Cancel
			return NULL;
		}
		// Try and get the value
		uintptr_t MainModuleMemory = 0;
		// A holder for the info
		MODULEENTRY32 moduleData;
		// Set size
		moduleData.dwSize = sizeof(MODULEENTRY32);
		// Get first one
		if (!Module32First(Snapshot, &moduleData))
		{
			// Failed
			CloseHandle(Snapshot);
			// Cancel
			return NULL;
		}
		// Set it
		MainModuleMemory = (uintptr_t)moduleData.modBaseSize;
		// Clean up
		CloseHandle(Snapshot);
		// Return it
		return MainModuleMemory;
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

uintptr_t ProcessReader::GetSizeOfCode()
{
	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// We must read from the main module address
		auto MainModuleAddress = GetMainModuleAddress();

		// Now we must read the PE information, starting with DOS_Header, NT_Header, then finally the optional entry
		auto DOSHeader = Read<IMAGE_DOS_HEADER>(MainModuleAddress);
		auto NTHeader = Read<IMAGE_NT_HEADERS>(MainModuleAddress + DOSHeader.e_lfanew);

		// Return code size
		return (uintptr_t)NTHeader.OptionalHeader.SizeOfCode;
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

std::string ProcessReader::GetProcessPath()
{
	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// We can get it
		CHAR ResultPath[MAX_PATH];
		// Result size
		DWORD ResiltSize = MAX_PATH;
		// Get module
		QueryFullProcessImageNameA(ProcessHandle, NULL, &ResultPath[0], &ResiltSize);
		// Return it
		return std::string(ResultPath);
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

HANDLE ProcessReader::GetCurrentProcess() const
{
	// Return it
	return ProcessHandle;
}

std::string ProcessReader::ReadNullTerminatedString(uintptr_t Offset)
{
	// Get the string if we are attached
	if (ProcessHandle != NULL)
	{
		// We can read
		std::stringstream ResultBuffer;
		// First char
		char CurrentChar = Read<char>(Offset);
		// Loop until null or EOF
		while (CurrentChar != 0)
		{
			// Increase
			Offset++;
			// Add
			ResultBuffer << CurrentChar;
			// Read again
			CurrentChar = Read<char>(Offset);
		}
		// Return it
		return ResultBuffer.str();
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

int8_t* ProcessReader::Read(uintptr_t Offset, uintptr_t Length, uintptr_t& Result)
{
	// Make sure we're attached
	if (ProcessHandle != NULL)
	{
		// We can read the block
		int8_t* ResultBlock = new int8_t[Length];
		// Zero out the memory
		std::memset(ResultBlock, 0, Length);
		// Length read
		SIZE_T LengthRead = 0;
		// Read it
		ReadProcessMemory(ProcessHandle, (void*)Offset, ResultBlock, Length, &LengthRead);
		// Set result
		Result = LengthRead;
		// Return result
		return ResultBlock;
	}
	// Failed to read it
	return nullptr;
}

intptr_t ProcessReader::Scan(const std::string& Pattern, bool UseExtendedMemoryScan)
{
	// Make sure we're attached
	if (ProcessHandle != NULL)
	{
		// Get the main module and code segment size, depending on flags
		auto MainModuleAttr = GetMainModuleAddress();
		auto MainModuleSize = (UseExtendedMemoryScan) ? GetMainModuleMemorySize() : GetSizeOfCode();
		// Scan
		return Scan(Pattern, MainModuleAttr, MainModuleSize);
	}
	// Failed
	return -1;
}

intptr_t ProcessReader::Scan(const std::string& Pattern, uintptr_t Offset, uintptr_t Length)
{
	// Make sure we're attached
	if (ProcessHandle != NULL)
	{
		// The resulting offset
		intptr_t ResultOffset = 0;

		// Pattern data
		std::string PatternBytes;
		std::string PatternMask;

		// Process the input patterm
		Patterns::ProcessPattern(Pattern, PatternBytes, PatternMask);

		// Total amount of data read
		uintptr_t DataRead = 0;

		// Scan the memory for it, read a chunk to scan
		int8_t* ChunkToScan = Read(Offset, Length, DataRead);

		// Scan the chunk
		intptr_t Result = (intptr_t)Patterns::ScanBlock(PatternBytes, PatternMask, (uintptr_t)ChunkToScan, DataRead);

		// Add the chunk offset itself (Since we're a process scanner)
		if (Result > -1) { Result += Offset; }

		// Clean up the memory block
		delete[] ChunkToScan;

		// Return it
		return Result;
	}
	// Failed to find it
	return -1;
}