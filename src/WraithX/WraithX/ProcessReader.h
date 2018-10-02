#pragma once

#include <cstdint>
#include <string>
#include <Windows.h>

// A class that handles reading and scanning a process for data
class ProcessReader
{
private:
	// A handle to the process
	HANDLE ProcessHandle;

public:
	ProcessReader();
	~ProcessReader();

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
		// Read a block of memory from a process based on the type
		if (ProcessHandle != NULL)
		{
			// We must read the data based on type.
			T ResultValue;
			// Zero out the memory
			std::memset(&ResultValue, 0, sizeof(ResultValue));
			// Read the value from the process
			ReadProcessMemory(ProcessHandle, (void*)Offset, &ResultValue, sizeof(ResultValue), 0);
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