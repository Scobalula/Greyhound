#include "stdafx.h"

// The class we are implementing
#include "InjectionReader.h"

// We require the patterns utility functions
#include "Patterns.h"
#include "BinaryReader.h"
#include "FileSystems.h"

InjectionReader::InjectionReader()
{
	// Init the handle
	ProcessHandle = NULL;
	SphinxHandle = NULL;
}

InjectionReader::~InjectionReader()
{
	// Detatch if posible
	Detatch();
}

bool InjectionReader::IsRunning()
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

void InjectionReader::WaitForExit()
{
	// Wait for it
	WaitForSingleObject(ProcessHandle, INFINITE);
}

void InjectionReader::Detatch()
{
	// Lock the mutex
	std::lock_guard<std::mutex> Lock(ReaderLock);

	// Close the sphinx handle if it's still open...
	if (SphinxHandle != NULL && SphinxHandle != INVALID_HANDLE_VALUE)
	{
		// Ask for a close command
		SphinxData CloseCommand;
		CloseCommand.Command = SphinxCommand::ShutdownModule;
		CloseCommand.CommandPointer = 0;
		CloseCommand.CommandSize = 0;

		// Send the command
		DWORD ResultWrite = 0;
		WriteFile(SphinxHandle, &CloseCommand, sizeof(CloseCommand), &ResultWrite, NULL);
		CloseHandle(SphinxHandle);
	}

	// If the handle isn't null, close it
	if (ProcessHandle != NULL)
		CloseHandle(ProcessHandle);

	// Set it
	ProcessHandle = NULL;
}

bool InjectionReader::Attach(int ProcessID)
{
	// Attempt to attach to the following process, if we are already attached disconnect first
	if (ProcessHandle != NULL)
	{
		// Detatch first
		Detatch();
	}
	// Attemp to attach
	ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);

	// Check result
	if (ProcessHandle != NULL)
		return InjectSphinxModule();

	// Failed
	return false;
}

bool InjectionReader::Attach(const std::string& ProcessName)
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
				// Stop, if we got one, inject the module
				if (ProcessHandle != NULL)
					return InjectSphinxModule();
			}
		} while (Process32Next(Snapshot, &Entry));
	}
	// Clean up
	CloseHandle(Snapshot);
	// Failed
	return false;
}

bool InjectionReader::Connect(HANDLE ProcessHandleReference)
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
	if (ProcessHandle != NULL)
		return InjectSphinxModule();

	// Failed
	return false;
}

LPTHREAD_START_ROUTINE InjectionReader::ResolveInjectionAddress(BOOL Is32BitProcess)
{
	// Prepare to resolve the address, if we're not 32bit, we just return the normal address
	if (!Is32BitProcess)
		return (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	// Otherwise, this will be hard, we must get the base address of kernel32.dll first, then prepare to parse the on-disk dll
	HMODULE ModHandles[1024];
	DWORD ResultRead;

	// Get a list of all 32bit modules
	if (!EnumProcessModulesEx(ProcessHandle, ModHandles, sizeof(ModHandles), &ResultRead, LIST_MODULES_32BIT))
	{
		// Failed to enumerate modules
#if _DEBUG
		printf("Failed to enumerate 32bit process modules!\n");
#endif
		return nullptr;
	}

	// The base module is stored here
	HMODULE Kernel32Wow = nullptr;

	// Iterate result modules and find kernel32
	for (uint32_t i = 0; i < (ResultRead / sizeof(HMODULE)); i++)
	{
		// Fetch the module name
		char ModNameTemp[MAX_PATH];
		if (GetModuleFileNameExA(ProcessHandle, ModHandles[i], ModNameTemp, sizeof(ModNameTemp)))
		{
			// Check the name against the dll name
			if (Strings::EndsWith(Strings::ToLower(std::string(ModNameTemp)), "kernel32.dll"))
			{
				Kernel32Wow = ModHandles[i];
				break;
			}
		}
	}

	// Check if we found it
	if (Kernel32Wow == nullptr)
	{
#if _DEBUG
		printf("Failed to find kernel32 in remote process!\n");
#endif
		return nullptr;
	}

	// Resolve the SysWow64 path
	char SysWowPath[MAX_PATH] = { 0 };
	GetSystemWow64DirectoryA(SysWowPath, sizeof(SysWowPath));

	// Combine the module path
	auto ModulePath = FileSystems::CombinePath(SysWowPath, "kernel32.dll");

	// Load the kernel32.dll module into memory for parsing of exports
	HANDLE ExeHandle = CreateFileA(ModulePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (ExeHandle == INVALID_HANDLE_VALUE)
	{
#if _DEBUG
		printf("Failed to open kernel32.dll!\n");
#endif
		return nullptr;
	}

	HANDLE ExeMapping = CreateFileMappingA(ExeHandle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (ExeMapping == NULL)
	{
		CloseHandle(ExeHandle);
#if _DEBUG
		printf("Failed to map kernel32.dll into memory!\n");
#endif
		return nullptr;
	}

	LPVOID ExeBuffer = MapViewOfFile(ExeMapping, FILE_MAP_READ, 0, 0, 0);
	if (ExeBuffer == NULL)
	{
		CloseHandle(ExeMapping);
		CloseHandle(ExeHandle);
#if _DEBUG
		printf("Failed to open the kernel32.dll map view!\n");
#endif
		return nullptr;
	}

	// Prepare to parse the PE structure just to the exports
	PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)ExeBuffer;
	PIMAGE_NT_HEADERS32 NTHeader = (PIMAGE_NT_HEADERS32)((uintptr_t)ExeBuffer + (uintptr_t)DosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER FirstSectionHeader = (PIMAGE_SECTION_HEADER)((PBYTE)&NTHeader->OptionalHeader + NTHeader->FileHeader.SizeOfOptionalHeader);
	PIMAGE_EXPORT_DIRECTORY ExportDirectory = nullptr;
	PULONG Functions = nullptr, Names = nullptr;
	PUSHORT Ordinals = nullptr;

	// Store the section index
	uint32_t SectionIndex = 0;

	// Iterate and find our section
	for (SectionIndex = 0; SectionIndex < NTHeader->FileHeader.NumberOfSections; SectionIndex++)
	{
		// Check if our segment is within range
		if (FirstSectionHeader[SectionIndex].VirtualAddress <= NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
			&& NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress < (FirstSectionHeader[SectionIndex].VirtualAddress + FirstSectionHeader[SectionIndex].Misc.VirtualSize))
		{
			// Resolve the directory
			ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)ExeBuffer + FirstSectionHeader[SectionIndex].PointerToRawData +
				NTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress - FirstSectionHeader[SectionIndex].VirtualAddress);

			// Resolve each segment (Fn, Name, Ordinal) from the directory
			Functions = (PULONG)((PBYTE)ExeBuffer + FirstSectionHeader[SectionIndex].PointerToRawData +
				ExportDirectory->AddressOfFunctions - FirstSectionHeader[SectionIndex].VirtualAddress);
			Names = (PULONG)((PBYTE)ExeBuffer + FirstSectionHeader[SectionIndex].PointerToRawData +
				ExportDirectory->AddressOfNames - FirstSectionHeader[SectionIndex].VirtualAddress);
			Ordinals = (PUSHORT)((PBYTE)ExeBuffer + FirstSectionHeader[SectionIndex].PointerToRawData +
				ExportDirectory->AddressOfNameOrdinals - FirstSectionHeader[SectionIndex].VirtualAddress);

			// Exit the loop, we found it
			break;
		}
	}

	// Check if we found it...
	if (ExportDirectory == nullptr)
	{
#if _DEBUG
		printf("Failed to find kernel32.dll export directory!\n");
#endif
		return nullptr;
	}

	// Store the actual result address
	LPTHREAD_START_ROUTINE ResultAddress = nullptr;

	// Iterate and find our function
	for (uint32_t ordinal = 0; (ordinal < ExportDirectory->NumberOfFunctions) && (ResultAddress == nullptr); ordinal++)
	{
		// We must iterate and find a proper ordinal match
		for (uint32_t i = 0; i < ExportDirectory->NumberOfNames; i++)
		{
			// Check for match
			if (ordinal == Ordinals[i])
			{
				// We have a name for this one, check it for the one we want
				auto FunctionName = (char*)((PBYTE)ExeBuffer + FirstSectionHeader[SectionIndex].PointerToRawData +
					Names[i] - FirstSectionHeader[SectionIndex].VirtualAddress);

				// Check if it's a match
				if (_strnicmp(FunctionName, "LoadLibraryA", strlen("LoadLibraryA")) == 0)
				{
					// We found it, add to the base address
					ResultAddress = (LPTHREAD_START_ROUTINE)((uintptr_t)Kernel32Wow + Functions[Ordinals[i]]);

					// Exit the loop
					break;
				}
			}
		}
	}

	// Clean up
	UnmapViewOfFile(ExeBuffer);
	CloseHandle(ExeMapping);
	CloseHandle(ExeHandle);

	// Return result
	return ResultAddress;
}

bool InjectionReader::InjectSphinxModule()
{
	// Prepare to inject the module, it must be located at app_dir\\WraithSphinx32.dll / WraithSphinx64.dll
	BOOL Is32BitProcess = false;
	if (!IsWow64Process(ProcessHandle, &Is32BitProcess))
	{
#if _DEBUG
		printf("Failed to query process bitness!\n");
#endif
		return false;
	}

	// Get the full module path
	auto ModulePath = FileSystems::CombinePath(FileSystems::GetApplicationPath(), "WraithSphinx") + ((Is32BitProcess) ? "32.dll" : "64.dll");

	// We can inject the proper module here...
	auto PathBuffer = VirtualAllocEx(ProcessHandle, NULL, (size_t)(ModulePath.size() + 1), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (PathBuffer == NULL)
	{
#if _DEBUG
		printf("Failed to allocate memory for Dll path!\n");
#endif
		return false;
	}

	// Write the Dll full path here...
	auto WriteResult = WriteProcessMemory(ProcessHandle, PathBuffer, ModulePath.c_str(), (size_t)(ModulePath.size() + 1), NULL);
	if (WriteResult == FALSE)
	{
#if _DEBUG
		printf("Failed to write the Dll path!\n");
#endif
		return false;
	}

	// Create the thread call
	auto LoadLibCall = ResolveInjectionAddress(Is32BitProcess);
	if (LoadLibCall == NULL)
	{
#if _DEBUG
		printf("Failed to resolve LoadLibraryA address!\n");
#endif
		return false;
	}

	DWORD ThreadID = 0;
	// Create the new thread
	auto ThreadHandle = CreateRemoteThread(ProcessHandle, NULL, 0, LoadLibCall, PathBuffer, 0, &ThreadID);
	if (ThreadHandle == NULL)
	{
#if _DEBUG
		printf("Failed to create the remote thread!\n");
#endif
		return false;
	}

	// Wait for the thread to exit...
	WaitForSingleObject(ThreadHandle, INFINITE);

	// Connect to the pipe first
	SphinxHandle = CreateFile(L"\\\\.\\pipe\\WraithSphinx", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	// Ensure success, if busy wait
	while (SphinxHandle == INVALID_HANDLE_VALUE && GetLastError() == ERROR_PIPE_BUSY)
	{
		// We may need to wait until not busy
		SphinxHandle = CreateFile(L"\\\\.\\pipe\\WraithSphinx", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	}

	// Validate success
	if (SphinxHandle == INVALID_HANDLE_VALUE)
	{
#if _DEBUG
		printf("Failed to connect to Sphinx module!\n");
#endif
		return false;
	}

	// Success
	return true;
}

uintptr_t InjectionReader::GetMainModuleAddress()
{
	// Lock the mutex
	std::lock_guard<std::mutex> Lock(ReaderLock);

	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// Ask for a main mod addr command
		SphinxData ReadCommand;
		ReadCommand.Command = SphinxCommand::GetMainModuleAddress;
		ReadCommand.CommandPointer = 0;
		ReadCommand.CommandSize = 0;

		// Send the command
		DWORD ResultWrite = 0;
		WriteFile(SphinxHandle, &ReadCommand, sizeof(ReadCommand), &ResultWrite, NULL);

		// Read the resulting address uint64_t
		uint64_t MainModuleAddress = 0;
		DWORD ReadCommandResult = 0;
		ReadFile(SphinxHandle, &MainModuleAddress, sizeof(uint64_t), &ReadCommandResult, NULL);

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

uintptr_t InjectionReader::GetMainModuleMemorySize()
{
	// Lock the mutex
	std::lock_guard<std::mutex> Lock(ReaderLock);

	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// Ask for a main mod mem size command
		SphinxData ReadCommand;
		ReadCommand.Command = SphinxCommand::GetMainModuleMemorySize;
		ReadCommand.CommandPointer = 0;
		ReadCommand.CommandSize = 0;

		// Send the command
		DWORD ResultWrite = 0;
		WriteFile(SphinxHandle, &ReadCommand, sizeof(ReadCommand), &ResultWrite, NULL);

		// Read the resulting address uint64_t
		uint64_t MainModuleMemory = 0;
		DWORD ReadCommandResult = 0;
		ReadFile(SphinxHandle, &MainModuleMemory, sizeof(uint64_t), &ReadCommandResult, NULL);

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

uintptr_t InjectionReader::GetSizeOfCode()
{
	// Lock the mutex
	std::lock_guard<std::mutex> Lock(ReaderLock);

	// Get the main module if we are attached
	if (ProcessHandle != NULL)
	{
		// Ask for a main mod code size command
		SphinxData ReadCommand;
		ReadCommand.Command = SphinxCommand::GetSizeOfCode;
		ReadCommand.CommandPointer = 0;
		ReadCommand.CommandSize = 0;

		// Send the command
		DWORD ResultWrite = 0;
		WriteFile(SphinxHandle, &ReadCommand, sizeof(ReadCommand), &ResultWrite, NULL);

		// Read the resulting address uint64_t
		uint64_t SizeOfCode = 0;
		DWORD ReadCommandResult = 0;
		ReadFile(SphinxHandle, &SizeOfCode, sizeof(uint64_t), &ReadCommandResult, NULL);

		// Return it
		return SizeOfCode;
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

std::string InjectionReader::GetProcessPath()
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

HANDLE InjectionReader::GetCurrentProcess() const
{
	// Return it
	return ProcessHandle;
}

std::string InjectionReader::ReadNullTerminatedString(uintptr_t Offset)
{
	// Lock the mutex
	std::lock_guard<std::mutex> Lock(ReaderLock);

	// Get the string if we are attached
	if (ProcessHandle != NULL)
	{
		// Ask for a read command
		SphinxData ReadCommand;
		ReadCommand.Command = SphinxCommand::ReadNullTerminatedString;
		ReadCommand.CommandPointer = Offset;
		ReadCommand.CommandSize = 0;

		// Send the command
		DWORD ResultWrite = 0;
		WriteFile(SphinxHandle, &ReadCommand, sizeof(ReadCommand), &ResultWrite, NULL);

		// Read until we're hit the size buffer, or the handle becomes invalid...
		uint64_t StringLength = 0;
		DWORD ReadCommandResult = 0;
		ReadFile(SphinxHandle, &StringLength, sizeof(uint64_t), &ReadCommandResult, NULL);

		// Only read if size > 0
		if (StringLength > 0)
		{
			// Now, allocate and read the string
			auto Result = std::string();
			// Allocate the buffer
			Result.resize(StringLength);

			// Read the string
			ReadFile(SphinxHandle, &Result[0], (DWORD)StringLength, &ReadCommandResult, NULL);

			// Return the result
			return Result;
		}

		// Nothing, blank string
		return std::string();
	}
	// Failed to perform read
#ifdef _DEBUG
	throw new std::exception("Not attached to any process");
#else
	throw new std::exception("");
#endif
}

int8_t* InjectionReader::Read(uintptr_t Offset, uintptr_t Length, uintptr_t& Result)
{
	// Lock the mutex
	std::lock_guard<std::mutex> Lock(ReaderLock);

	// Make sure we're attached
	if (ProcessHandle != NULL)
	{
		// We can read the block
		int8_t* ResultBlock = new int8_t[Length];
		// Zero out the memory
		std::memset(ResultBlock, 0, Length);

		// Ignore invalid requests... (A length of 0 denotes async ReadFile, so we must IGNORE them)
		if (Length > 0)
		{
			// Ask for a read command
			SphinxData ReadCommand;
			ReadCommand.Command = SphinxCommand::ReadProcessMemory;
			ReadCommand.CommandPointer = Offset;
			ReadCommand.CommandSize = Length;

			// Send the command
			DWORD ResultWrite = 0;
			WriteFile(SphinxHandle, &ReadCommand, sizeof(ReadCommand), &ResultWrite, NULL);

			// Read until we're hit the size buffer, or the handle becomes invalid...
			DWORD ReadCommandResult = 0;
			ReadFile(SphinxHandle, ResultBlock, (DWORD)Length, &ReadCommandResult, NULL);
		}

		// Set result
		Result = Length;

		// Return result
		return ResultBlock;
	}
	// Failed to read it
	return nullptr;
}

intptr_t InjectionReader::Scan(const std::string& Pattern, bool UseExtendedMemoryScan)
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

intptr_t InjectionReader::Scan(const std::string& Pattern, uintptr_t Offset, uintptr_t Length)
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