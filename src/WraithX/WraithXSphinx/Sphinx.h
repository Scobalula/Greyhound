#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <cstdio>
#include <cstdint>

// Include just the injection reader class, for the structs
#include "InjectionReader.h"

// The global pipe instance
HANDLE PipeHandle = NULL;

DWORD WINAPI SphinxExecute(LPVOID lpParam)
{
#if _DEBUG
	printf("Begin sphinx listener\n");
#endif

	// The current sphinx command
	SphinxData DataBuffer;
	bool CanContinue = true;

	// Loop until the pipe is closed
	if (PipeHandle != INVALID_HANDLE_VALUE)
	{
		// Wait for a command request, or, if we connected early, just go
		while (ConnectNamedPipe(PipeHandle, NULL) != FALSE || GetLastError() == ERROR_PIPE_CONNECTED)
		{
			while (CanContinue)
			{
				// Read the command first
				DWORD SizeRead = 0;
				ReadFile(PipeHandle, &DataBuffer, sizeof(DataBuffer), &SizeRead, NULL);

				// Validate it
				if (SizeRead == sizeof(DataBuffer))
				{
					// We have a command, prepare to do something with it
#if _DEBUG
					printf("Command [%d] [0x%llX] [0x%llX]\n", DataBuffer.Command, DataBuffer.CommandPointer, DataBuffer.CommandSize);
#endif

					// Our command functions
					auto RPMCommand = [&]()
					{
						if (DataBuffer.CommandSize > 0)
						{
							// Read the region
							DWORD ResultBuffer = 0;
							WriteFile(PipeHandle, (void*)(DataBuffer.CommandPointer), (DWORD)DataBuffer.CommandSize, &ResultBuffer, NULL);

							// Fill remainder if it was invalid, or not finished...
							if ((DWORD)DataBuffer.CommandSize > ResultBuffer)
							{
								// Calculate remainder
								DWORD Needs = ((DWORD)DataBuffer.CommandSize - ResultBuffer);

#if _DEBUG
								printf("Needs to finish: [0x%X]\n", Needs);
#endif

								// TODO: This isn't safe, we need a better way to do this...
								// Fill the output w/ blank data so that we can safely continue...
								auto temp = new char[Needs];
								std::memset(temp, 0, Needs);
								WriteFile(PipeHandle, temp, Needs, &ResultBuffer, NULL);
								delete[] temp;
							}
						}
					};
					auto MainModAddrCommand = [&]()
					{
						// Fetch the main module's address
						auto MainModuleAddress = (uint64_t)GetModuleHandle(NULL);

						// Write the result...
						DWORD ResultBuffer = 0;
						WriteFile(PipeHandle, &MainModuleAddress, sizeof(uint64_t), &ResultBuffer, NULL);
					};
					auto MainModMemSizeCommand = [&]()
					{
						// Fetch the main modules memory size
						uint64_t MainModuleMemSize = 0;

						// Fetch snapshot
						HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
						// Check it
						if (Snapshot == INVALID_HANDLE_VALUE)
						{
							// Write the result...
							DWORD ResultBuffer = 0;
							WriteFile(PipeHandle, &MainModuleMemSize, sizeof(uint64_t), &ResultBuffer, NULL);
							return;
						}

						// A holder for the info
						MODULEENTRY32 moduleData;
						// Set size
						moduleData.dwSize = sizeof(MODULEENTRY32);
						// Get first one
						if (Module32First(Snapshot, &moduleData))
							MainModuleMemSize = (uint64_t)moduleData.modBaseSize;

						// Clean up
						CloseHandle(Snapshot);

						// Write the result...
						DWORD ResultBuffer = 0;
						WriteFile(PipeHandle, &MainModuleMemSize, sizeof(uint64_t), &ResultBuffer, NULL);
					};
					auto SizeOfCodeCommand = [&]()
					{
						// Fetch the main modules code size
						uint64_t SizeOfCode = 0;

						// Load module information
						auto Mod = (HMODULE)GetModuleHandle(NULL);

						// Load PE information
						auto DOSHeader = (const IMAGE_DOS_HEADER*)(Mod);
						auto NTHeader = (const IMAGE_NT_HEADERS*)((const uint8_t*)(DOSHeader)+DOSHeader->e_lfanew);

						// Set size of code
						SizeOfCode = NTHeader->OptionalHeader.SizeOfCode;

						// Write the result...
						DWORD ResultBuffer = 0;
						WriteFile(PipeHandle, &SizeOfCode, sizeof(uint64_t), &ResultBuffer, NULL);
					};
					auto ReadNullTermString = [&]()
					{
						// We can read the string directly, but, we must write the size!
						auto ResultString = (DataBuffer.CommandPointer > 0) ? std::string((const char*)DataBuffer.CommandPointer) : std::string();

						// Write the size first, then write the whole string
						uint64_t StringSize = ResultString.size();

						// Write the result
						DWORD ResultBuffer = 0;
						WriteFile(PipeHandle, &StringSize, sizeof(uint64_t), &ResultBuffer, NULL);
						
						// If we had a string
						if (StringSize > 0)
						{
							// Write the string
							WriteFile(PipeHandle, ResultString.c_str(), (DWORD)StringSize, &ResultBuffer, NULL);
						}
					};
					auto ShutdownCommand = [&]()
					{
						// Stop execution
						CanContinue = false;
					};

					// Check the command, and perform the task
					switch (DataBuffer.Command)
					{
					case SphinxCommand::ReadProcessMemory: RPMCommand(); break;
					case SphinxCommand::GetMainModuleAddress: MainModAddrCommand(); break;
					case SphinxCommand::GetSizeOfCode: SizeOfCodeCommand(); break;
					case SphinxCommand::GetMainModuleMemorySize: MainModMemSizeCommand(); break;
					case SphinxCommand::ReadNullTerminatedString: ReadNullTermString(); break;
					case SphinxCommand::ShutdownModule: ShutdownCommand(); break;
					}

#if _DEBUG
					printf("Finished command!\n");
#endif
				}
				else if (GetLastError() == ERROR_BROKEN_PIPE || GetLastError() == ERROR_ACCESS_DENIED)
				{
					// Done here, the owner disconnected
					CanContinue = false;
				}
				else
				{
					// Failed
#if _DEBUG
					printf("Received invalid data, size: %d Error: 0x%X\n", SizeRead, GetLastError());
#endif
				}
			}
		}

		// Close out for now
		DisconnectNamedPipe(PipeHandle);
	}

	// Ask to unload the module (Changes w/ 32 and 64bit)
#if _WIN64
	FreeLibraryAndExitThread(GetModuleHandleA("WraithSphinx64.dll"), 0);
#else
	FreeLibraryAndExitThread(GetModuleHandleA("WraithSphinx32.dll"), 0);
#endif

	// Success
	return 0;
}

void WINAPI SphinxInitialize()
{
	// Prepare the module
#if _DEBUG
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	printf("WraithSphinx module has loaded!\n");
#endif

	// Start the named pipe server
	PipeHandle = CreateNamedPipe(L"\\\\.\\pipe\\WraithSphinx", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024 * 16, 1024 * 16, NMPWAIT_USE_DEFAULT_WAIT, NULL);

#if _DEBUG
	printf("Created pipe: 0x%p\n", PipeHandle);
#endif

	// Run logic
	CreateThread(NULL, 0, SphinxExecute, NULL, 0, NULL);
}

void WINAPI SphinxShutdown()
{
#if _DEBUG
	printf("Shutting down module...\n");
#endif

	// Shutdown the pipe
	if (PipeHandle != INVALID_HANDLE_VALUE)
		CloseHandle(PipeHandle);

#if _DEBUG
	fclose(stdout);
	FreeConsole();
#endif
}