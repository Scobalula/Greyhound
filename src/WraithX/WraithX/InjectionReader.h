#pragma once

#include <cstdint>
#include <string>
#include <mutex>
#include <Windows.h>

// Sphinx commands
enum class SphinxCommand : uint32_t
{
	ReadProcessMemory = 0,
	GetMainModuleAddress,
	GetMainModuleMemorySize,
	GetSizeOfCode,
	ReadNullTerminatedString,
	ShutdownModule,
};

#pragma pack(push, 1)
// Sphinx data block
struct SphinxData
{
	SphinxCommand Command;

	uint64_t CommandPointer;
	uint64_t CommandSize;
};
#pragma pack(pop)

// A class that handles reading and scanning a process for data using an injected module
class InjectionReader
{
private:
	// A handle to the process
	HANDLE ProcessHandle;
	// A handle to sphinx
	HANDLE SphinxHandle;

	// A mutex for locking read and write operations
	std::mutex ReaderLock;

	// Attempt to inject 'WraithSphinx' into the process
	bool InjectSphinxModule();
	// Attempt to resolve the LoadLibraryAddress
	LPTHREAD_START_ROUTINE ResolveInjectionAddress(BOOL Is32BitProcess);

public:
	InjectionReader();
	~InjectionReader();

	// Attach this reader to a process with the given ID
	bool Attach(int ProcessID);
	// Attach this reader to a process with the given name
	bool Attach(const std::string& ProcessName);
	// Connect this reader to a process with the given handle
	bool Connect(HANDLE ProcessHandleReference);

	// Figure out whether or not the process is still running
	bool IsRunning();

	// Wait until the process exit's
	void WaitForExit();

	// Detatch from the currently attached process
	void Detatch();

	// Gets the address of the main module for this process
	uintptr_t GetMainModuleAddress();
	// Gets the amount of memory required to load this module
	uintptr_t GetMainModuleMemorySize();

	// Gets the size of 'code' the main module uses
	uintptr_t GetSizeOfCode();

	// Gets the full path of the process
	std::string GetProcessPath();

	// Gets the current handle of the process
	HANDLE GetCurrentProcess() const;

	template <class T>
	// Read a block of memory from the process with the given type
	T Read(uintptr_t Offset)
	{
		// Lock the mutex
		std::lock_guard<std::mutex> Lock(ReaderLock);

		// Read a block of memory from a process based on the type
		if (ProcessHandle != NULL)
		{
			// We must read the data based on type.
			T ResultValue;
			// Zero out the memory
			std::memset(&ResultValue, 0, sizeof(ResultValue));
			
			// Ignore invalid requests
			if (sizeof(ResultValue) > 0)
			{
				// Ask for a read command
				SphinxData ReadCommand;
				ReadCommand.Command = SphinxCommand::ReadProcessMemory;
				ReadCommand.CommandPointer = Offset;
				ReadCommand.CommandSize = sizeof(ResultValue);

				// Send the command
				DWORD ResultWrite = 0;
				WriteFile(SphinxHandle, &ReadCommand, sizeof(ReadCommand), &ResultWrite, NULL);

				// Read until we're hit the size buffer, or the handle becomes invalid...
				DWORD ReadCommandResult = 0;
				ReadFile(SphinxHandle, &ResultValue, sizeof(ResultValue), &ReadCommandResult, NULL);
			}
			
			// Return the result
			return ResultValue;
		}
		// Failed to perform read
#ifdef _DEBUG
		throw new std::exception("Not attached to any process");
#else
		throw new std::exception("");
#endif
	}
	// Read a null-terminated string from the process with the given offset
	std::string ReadNullTerminatedString(uintptr_t Offset);

	// Read a block of memory from the process with the given length
	int8_t* Read(uintptr_t Offset, uintptr_t Length, uintptr_t& Result);

	// Scan the process for a pattern (In the main modules code, or full memory if specified)
	intptr_t Scan(const std::string& Pattern, bool UseExtendedMemoryScan = false);
	// Scan the process for a pattern with the given offset and length
	intptr_t Scan(const std::string& Pattern, uintptr_t Offset, uintptr_t Length);
};