#include <conio.h>
#include <stdio.h>
#include <memory>
#include <map>
#include <array>
#include <chrono>

// Wraith application and api (Must be included before additional includes)
#include "WraithApp.h"
#include "WraithWindow.h"
#include "WraithTheme.h"
#include "WraithX.h"

// Our test GUI for controls
#include "TestWindow.h"

// Resource files
#include "resource.h"

// WraithX Includes
#include "ProcessReader.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "MemoryReader.h"
#include "TextReader.h"
#include "TextWriter.h"
#include "FileSystems.h"
#include "Patterns.h"
#include "WebClient.h"
#include "VectorMath.h"
#include "Compression.h"
#include "Encryption.h"
#include "WraithAsset.h"
#include "WraithAnim.h"
#include "WraithModel.h"
#include "Hashing.h"
#include "Image.h"
#include "Autherization.h"
#include "Instance.h"
#include "Systems.h"
#include "Console.h"
#include "Sound.h"
#include "WraithNameIndex.h"
#include "InjectionReader.h"

// WraithX exporter includes
#include "SEAnimExport.h"
#include "MayaExport.h"
#include "ValveSMDExport.h"
#include "OBJExport.h"
#include "XNALaraExport.h"
#include "XMEExport.h"

// Macro for debugging tests (No throws)
#define ASSERT_PRNT(a) if (a) { printf("DONE!\r\n"); TestsCompleted++; } else { printf("FAILED TEST!\r\n"); TestsFailed++; }

// Function for performance testing of functions
template<typename TimeT = std::chrono::milliseconds>
struct UnitTestTimer
{
	template<typename F, typename ...Args>
	static typename TimeT::rep ExecuteFunction(F func, Args&&... args)
	{
		auto start = std::chrono::high_resolution_clock::now();
		func(std::forward<Args>(args)...);
		auto duration = std::chrono::duration_cast<TimeT>(std::chrono::high_resolution_clock::now() - start);
		return duration.count();
	}
}; // UnitTestTimer<std::chrono::nanoseconds>::ExecuteFunction(test);

// Allow modern GUI controls
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// The WraithX App Instance
WraithApp WraithAppInstance;

// Handler for loading theme icons from resources
HICON LoadIconResource(WraithIconAssets AssetID)
{
	// Check the ID, if we have it, we can load it
	switch (AssetID)
	{
	case WraithIconAssets::ApplicationIcon:
		// Load the application icon
		return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON));
	case WraithIconAssets::ApplicationIconLarge:
		// Load the application icon large
		return (HICON)::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 64, 64, LR_SHARED);
	case WraithIconAssets::CheckboxCheckedIcon:
		// Load the checkbox checked icon
		return LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CHECKICON));
	}
	// Failed
	return NULL;
}

// Main entry point of app
int main(int argc, char** argv)
{
	// Arch
	auto Arch = (sizeof(uintptr_t) == 8) ? 64 : 32;

	// Hook theme callbacks
	WraithTheme::OnLoadIconResource = LoadIconResource;

	// Initialize the API (This must be done BEFORE running a WraithApp)
	if (!WraithX::InitializeAPI(true))
	{
		// Failed to initialize
		MessageBoxA(NULL, "A fatal error occured while initializing Wraith", "WraithX", MB_OK | MB_ICONEXCLAMATION);
		// Failed
		return -1;
	}

	// This will create the window, and start a message pump, disable for just tests
	WraithAppInstance.RunApplication(TestWindow());

	// Tests entry point
	printf("-- WraithX Unit Tests --\r\n");
	printf("-  Build %d.%d.%d, Running on %dbit architecture\r\n\r\n", WRAITHX_BUILD_MAJOR, WRAITHX_BUILD_MINOR, WRAITHX_BUILD_EXPONENT, Arch);

	// Disable buffer
	setvbuf(stdout, NULL, _IONBF, 0);

	// The amount of tests
	auto TestsCompleted = 0;
	auto TestsFailed = 0;

	// Process read test
#pragma region Read test

	printf(":  [1]\t\tProcess read test... ");
	{
		// Make a reader instance
		std::shared_ptr<ProcessReader> ProcessReadTest = std::make_shared<ProcessReader>();

		// Attach to the test process
		auto AttachRes = ProcessReadTest->Attach("WraithXTests.exe");

		// Get the address of the main module
		auto MainModule = ProcessReadTest->GetMainModuleAddress();

		// Read the PE header
		auto Result = ProcessReadTest->Read<int>(MainModule);

		// Validate
		ASSERT_PRNT(Result == 0x00905a4d);
	}

#pragma endregion
	// Process scan test
#pragma region Scan test

	printf(":  [2]\t\tProcess scan test... ");
	{
		// Make a reader instance
		std::shared_ptr<ProcessReader> ProcessReadTest = std::make_shared<ProcessReader>();

		// Attach to the test process
		auto AttachRes = ProcessReadTest->Attach("WraithXTests.exe");

		// Scan for a common string 'WraithX Unit Tests' (ascii bytes)
		auto ScanResult = ProcessReadTest->Scan("57 72 61 69 74 68 58 20 55 6E 69 74 20 54 65 73 74 73", true);

		// Validate (Will never be at 0 since PE header)
		ASSERT_PRNT(ScanResult > -1);
	}

#pragma endregion
	// Process topattern test
#pragma region Find (ToPattern) test

	printf(":  [3]\t\tProcess scan (ToPattern) test... ");
	{
		// Make a reader instance
		std::shared_ptr<ProcessReader> ProcessReadTest = std::make_shared<ProcessReader>();

		// Attach to the test process
		auto AttachRes = ProcessReadTest->Attach("WraithXTests.exe");

		// Get the address of the main module
		auto MainModule = ProcessReadTest->GetMainModuleAddress();

		// Scan for a common string 'WraithX Unit Tests' (ascii bytes)
		auto ScanResult = ProcessReadTest->Scan(Patterns::ToPattern<const char*>("WraithX Unit Tests"), true);

		// Validate (Will never be at 0 since PE header)
		ASSERT_PRNT(ScanResult > -1);
	}

#pragma endregion
	// Process running test
#pragma region Running test

	printf(":  [4]\t\tProcess running test... ");
	{
		// Make a reader instance
		std::shared_ptr<ProcessReader> ProcessReadTest = std::make_shared<ProcessReader>();

		// Attach to the test process
		auto AttachRes = ProcessReadTest->Attach("WraithXTests.exe");

		// Validate (MUST be true)
		ASSERT_PRNT(ProcessReadTest->IsRunning());
	}

#pragma endregion
	// Binary read test
#pragma region Read binary test

	printf(":  [5]\t\tBinary reading test... ");
	{
		// Make a reader instance
		std::shared_ptr<BinaryReader> BinaryReadTest = std::make_shared<BinaryReader>();

		// Open the test file
		BinaryReadTest->Open("Tests/BinaryRead.bin");

		// Check the magic
		auto Magic = BinaryReadTest->Read<int>();

		// Jump to string
		BinaryReadTest->SetPosition(0x4E);

		// Read
		auto Result = BinaryReadTest->ReadString(0x2b);

		// Compare data
		auto CompareData = "This program cannot be run in DOS mode.\r\r\n$";

		// Validate (MUST be true)
		ASSERT_PRNT(Magic == 0x00905a4d && (strncmp(Result.c_str(), CompareData, Result.size())) == 0);
	}

#pragma endregion
	// Binary scan test
#pragma region Read scan test

	printf(":  [6]\t\tBinary scan test... ");
	{
		// Make a reader instance
		std::shared_ptr<BinaryReader> BinaryReadTest = std::make_shared<BinaryReader>();

		// Open the test file
		BinaryReadTest->Open("Tests/BinaryRead.bin");

		// Find the pattern 'This program cannot be run in DOS mode' (ascii bytes)
		auto Offset = BinaryReadTest->Scan("54 68 69 73 20 70 72 6F 67 72 61 6D 20 63 61 6E 6E 6F 74 20 62 65 20 72 75 6E 20 69 6E 20 44 4F 53 20 6D 6F 64 65");

		// Validate (MUST be at 0x4E)
		ASSERT_PRNT(Offset == 0x4E);
	}

#pragma endregion
	// ToPattern int test
#pragma region ToPattern int test

	printf(":  [7]\t\tToPattern int test... ");
	{
		// Convert it
		auto Result = Patterns::ToPattern<int>(0x33445566);

		// Validate
		ASSERT_PRNT(Result == "66 55 44 33");
	}

#pragma endregion
	// ToPattern string test
#pragma region ToPattern string test

	printf(":  [8]\t\tToPattern string test... ");
	{
		// Convert it
		auto Result = Patterns::ToPattern<const char*>("This program cannot be run in DOS mode");

		// Validate (case insensitive since it doesn't matter)
		ASSERT_PRNT(_strnicmp(Result.c_str(), "54 68 69 73 20 70 72 6F 67 72 61 6D 20 63 61 6E 6E 6F 74 20 62 65 20 72 75 6E 20 69 6E 20 44 4F 53 20 6D 6F 64 65", Result.size()) == 0);
	}

#pragma endregion
	// Text readline test
#pragma region Text readline test

	printf(":  [9]\t\tText readline test... ");
	{
		// Make a reader instance
		std::shared_ptr<TextReader> TextReadTest = std::make_shared<TextReader>();

		// Open the test file
		TextReadTest->Open("Tests/TextRead.txt");

		// Read the line
		auto Offset = TextReadTest->ReadLine();

		// Validate
		ASSERT_PRNT(Offset == "Hello there!");
	}

#pragma endregion
	// Text parseline test
#pragma region Text parseline test

	printf(":  [10]\t\tText parseline test... ");
	{
		// Make a reader instance
		std::shared_ptr<TextReader> TextReadTest = std::make_shared<TextReader>();

		// Open the test file
		TextReadTest->Open("Tests/TextRead.txt");

		// Skip two lines
		TextReadTest->ReadLine();
		TextReadTest->ReadLine();

		float F1 = 0, F2 = 0, F3 = 0;
		int I1 = 0;

		// Parse the line
		TextReadTest->ParseLine("%f %f %f %d", &F1, &F2, &F3, &I1);

		// Validate (Last one should be fine, since it's just an integer)
		ASSERT_PRNT(I1 == 0x5);
	}

#pragma endregion
	// String format test
#pragma region String format test

	printf(":  [11]\t\tString format test... ");
	{
		// Format it
		auto Result = Strings::Format("%d %d, %s", 0x33, 0x44, "hello!");

		// Validate
		ASSERT_PRNT(Result == "51 68, hello!");
	}

#pragma endregion
	// String toint test
#pragma region String toint test

	printf(":  [12]\t\tString toint test... ");
	{
		// Value
		int32_t Number = -1;
		// Parse it
		auto Result = Strings::ToInteger("100", Number);

		// Validate
		ASSERT_PRNT(Result == true && Number == 100);
	}

#pragma endregion
	// String isdigit test
#pragma region String isdigit test

	printf(":  [13]\t\tString isdigit test... ");
	{
		// Parse it
		auto Result = Strings::IsDigits("123 home");

		// Validate
		ASSERT_PRNT(Result == false);
	}

#pragma endregion
	// String replace test
#pragma region String replace test

	printf(":  [14]\t\tString replace test... ");
	{
		// Parse it
		auto Result = Strings::Replace("Hello World!", "Hello", "Goodbye");

		// Validate
		ASSERT_PRNT(Result == "Goodbye World!");
	}

#pragma endregion
	// String whitespace test
#pragma region String whitespace test

	printf(":  [15]\t\tString whitespace test... ");
	{
		// Check it
		auto Result = Strings::IsNullOrWhiteSpace("");
		auto Result2 = Strings::IsNullOrWhiteSpace("ADAWD a  ");

		// Validate
		ASSERT_PRNT(Result == true && Result2 == false);
	}

#pragma endregion
	// File exists test
#pragma region File exists test

	printf(":  [16]\t\tFile exists test... ");
	{
		// Check it
		auto Result = FileSystems::FileExists("Tests/TextRead.txt");

		// Validate
		ASSERT_PRNT(Result == true);
	}

#pragma endregion
	// Dir exists test
#pragma region Dir exists test

	printf(":  [17]\t\tDir exists test... ");
	{
		// Check it
		auto Result = FileSystems::DirectoryExists("Tests");

		// Validate
		ASSERT_PRNT(Result == true);
	}

#pragma endregion
	// Path combine test
#pragma region Path combine test

	printf(":  [18]\t\tPath combine test... ");
	{
		// Check it
		auto Result = FileSystems::CombinePath("C:\\Tests\\", "IWin\\Lawl");
		auto Result2 = FileSystems::CombinePath("D:\\Wins", "LolHi");

		// Validate
		ASSERT_PRNT(Result == "C:\\Tests\\IWin\\Lawl" && Result2 == "D:\\Wins\\LolHi");
	}

#pragma endregion
	// Path rooted test
#pragma region Path rooted test

	printf(":  [19]\t\tPath rooted test... ");
	{
		// Check it
		auto Result = FileSystems::IsPathRooted("C:\\Wins");
		auto Result2 = FileSystems::IsPathRooted("Lawl");

		// Validate
		ASSERT_PRNT(Result == true && Result2 == false);
	}

#pragma endregion
	// Path getfiles test
#pragma region Path getfiles test

	printf(":  [20]\t\tPath getfiles test... ");
	{
		// Check it
		auto Result = FileSystems::GetFiles("Tests", "*.*");

		// Validate
		ASSERT_PRNT(Result.size() > 0);
	}

#pragma endregion
	// Path getdirs test
#pragma region Path getdirs test

	printf(":  [21]\t\tPath getdirs test... ");
	{
		// Check it
		auto Result = FileSystems::GetDirectories("Tests");

		// Validate
		ASSERT_PRNT(Result.size() == 1);
	}

#pragma endregion
	// Path getfilename test
#pragma region Path getfilename test

	printf(":  [22]\t\tPath getfilename test... ");
	{
		// Check it
		auto Result = FileSystems::GetFileName("C:\\Documents\\Cool shit\\images\\TestFileWins.jpg");

		// Validate
		ASSERT_PRNT(Result == "TestFileWins.jpg");
	}

#pragma endregion
	// Path getfilenameext test
#pragma region Path getfilenameext test

	printf(":  [23]\t\tPath getfilenameext test... ");
	{
		// Check it
		auto Result = FileSystems::GetFileNameWithoutExtension("C:\\Documents\\Cool shit\\images\\TestFileWins.jpg");

		// Validate
		ASSERT_PRNT(Result == "TestFileWins");
	}

#pragma endregion
	// Path getext test
#pragma region Path getext test

	printf(":  [24]\t\tPath getext test... ");
	{
		// Check it
		auto Result = FileSystems::GetExtension("C:\\Documents\\Cool shit\\images\\TestFileWins.jpg");

		// Validate
		ASSERT_PRNT(Result == ".jpg");
	}

#pragma endregion
	// Path readtoend test
#pragma region Path readtoend test

	printf(":  [25]\t\tText readtoend test... ");
	{
		// Make a reader instance
		std::shared_ptr<TextReader> TextReadTest = std::make_shared<TextReader>();

		// Open the test file
		TextReadTest->Open("Tests/TextRead.txt");

		// Read the whole file
		auto Result = TextReadTest->ReadToEnd();

		// Validate
		ASSERT_PRNT(Result.length() == 77);
	}

#pragma endregion
	// Write binary test
#pragma region Write binary test

	printf(":  [26]\t\tWrite binary test... ");
	{
		// Make a writer instance
		std::shared_ptr<BinaryWriter> WriteTest = std::make_shared<BinaryWriter>();

		// Had error
		bool HadError = false;

		// Try to write
		try
		{
			// Make a file
			WriteTest->Create("Tests/BinaryWriter.bin");

			// Write some data
			WriteTest->Write<int32_t>(0xDEADBEEF);
			WriteTest->Write<int32_t>(0x1234567);
			WriteTest->Write<float_t>(1.0f);
			WriteTest->Write<double_t>(1.0f);
			WriteTest->Write<int64_t>(0x7FFFFFFFFF);

			// Write note
			WriteTest->WriteNullTerminatedString("end");

			// Close
			WriteTest->Close();
		}
		catch (...)
		{
			HadError = true;
		}

		// Validate
		ASSERT_PRNT(HadError == false);
	}

#pragma endregion
	// Write text test
#pragma region Write text test

	printf(":  [27]\t\tWrite text test... ");
	{
		// Make a writer instance
		std::shared_ptr<TextWriter> WriteTest = std::make_shared<TextWriter>();

		// Had error
		bool HadError = false;

		// Try to write
		try
		{
			// Make a file
			WriteTest->Create("Tests/TextWriter.txt");

			// Write some data
			WriteTest->Write("HELLO ");
			WriteTest->WriteLine("WORLD");

			// Write formatted
			WriteTest->WriteLineFmt("%f %f %f %f %d %d", 1.2f, 1.3f, 1.5f, 1.7f, 66, 88);

			// Make a new line
			WriteTest->NewLine();

			// Close
			WriteTest->Close();
		}
		catch (...)
		{
			HadError = true;
		}

		// Validate
		ASSERT_PRNT(HadError == false);
	}

#pragma endregion
	// Compress lz4 test
#pragma region Compress lz4 test

	printf(":  [28]\t\tCompress lz4 test... ");
	{
		// Load up a buffer for compressing
		const uint8_t Buffer[] = { 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79 };

		// Calculate max size
		auto MaxBufferSize = Compression::CompressionSizeLZ4(9);

		// Pointer for the compressed data
		int8_t* CompressedBuffer = new int8_t[MaxBufferSize];
		// Set mem
		std::memset(CompressedBuffer, 0, MaxBufferSize);

		// Compress it
		auto Result = Compression::CompressLZ4Block((const int8_t*)&Buffer[0], CompressedBuffer, 9, MaxBufferSize);

		// Save to a file (For next test)
		std::shared_ptr<BinaryWriter> Writer = std::make_shared<BinaryWriter>();

		// Make new file
		Writer->Create("Tests/LZ4Test.bin");

		// Write result
		if (Result > 0)
		{
			Writer->Write(CompressedBuffer, Result);
		}

		// Close file
		Writer->Close();

		// Clean up
		delete[] CompressedBuffer;

		// Validate
		ASSERT_PRNT(Result > 0);
	}

#pragma endregion
	// Decompress lz4 test
#pragma region Decompress lz4 test

	printf(":  [29]\t\tDecompress lz4 test... ");
	{
		// Load up a buffer for decompressing
		const uint8_t CompressedBuffer[] = { 0x90, 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79 };

		// Pointer for the decompressed data
		int8_t* DecompressedBuffer = new int8_t[9];
		// Clear buffer
		std::memset(DecompressedBuffer, 0, 9);

		// Decompress it
		auto Result = Compression::DecompressLZ4Block((const int8_t*)&CompressedBuffer[0], DecompressedBuffer, 10, 9);

		// Check resulting values
		bool HasSuccess = (Result > 0 && DecompressedBuffer[0] == 'h' && DecompressedBuffer[1] == 'e' && DecompressedBuffer[2] == 'y');

		// Clean up
		delete[] DecompressedBuffer;

		// Validate
		ASSERT_PRNT(HasSuccess);
	}

#pragma endregion
	// Compress lzo1x test
#pragma region Compress lzo1x test

	printf(":  [30]\t\tCompress lzo1x test... ");
	{
		// Load up a buffer for compressing
		const uint8_t Buffer[] = { 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79 };

		// Calculate max size
		auto MaxBufferSize = Compression::CompressionSizeLZO1X(9);

		// Pointer for the compressed data
		int8_t* CompressedBuffer = new int8_t[MaxBufferSize];
		// Set mem
		std::memset(CompressedBuffer, 0, MaxBufferSize);

		// Compress it
		auto Result = Compression::CompressLZO1XBlock((const int8_t*)&Buffer[0], CompressedBuffer, 9, MaxBufferSize);

		// Save to a file (For next test)
		std::shared_ptr<BinaryWriter> Writer = std::make_shared<BinaryWriter>();

		// Make new file
		Writer->Create("Tests/LZO1XTest.bin");

		// Write result
		if (Result > 0)
		{
			Writer->Write(CompressedBuffer, Result);
		}

		// Close file
		Writer->Close();

		// Clean up
		delete[] CompressedBuffer;

		// Validate
		ASSERT_PRNT(Result > 0);
	}

#pragma endregion
	// Decompress lzo1x test
#pragma region Decompress lzo1x test

	printf(":  [31]\t\tDecompress lzo1x test... ");
	{
		// Load up a buffer for decompressing
		const uint8_t CompressedBuffer[] = { 0x1A, 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79, 0x11, 0x00, 0x00 };

		// Pointer for the decompressed data
		int8_t* DecompressedBuffer = new int8_t[9];
		// Clear buffer
		std::memset(DecompressedBuffer, 0, 9);

		// Decompress it
		auto Result = Compression::DecompressLZO1XBlock((const int8_t*)&CompressedBuffer[0], DecompressedBuffer, 13, 9);

		// Check resulting values
		bool HasSuccess = (Result > 0 && DecompressedBuffer[0] == 'h' && DecompressedBuffer[1] == 'e' && DecompressedBuffer[2] == 'y');

		// Clean up
		delete[] DecompressedBuffer;

		// Validate
		ASSERT_PRNT(HasSuccess);
	}

#pragma endregion
	// Compress zlib test
#pragma region Compress zlib test

	printf(":  [32]\t\tCompress zlib test... ");
	{
		// Load up a buffer for compressing
		const uint8_t Buffer[] = { 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79 };

		// Calculate max size
		auto MaxBufferSize = Compression::CompressionSizeZLib(9);

		// Pointer for the compressed data
		int8_t* CompressedBuffer = new int8_t[MaxBufferSize];
		// Set mem
		std::memset(CompressedBuffer, 0, MaxBufferSize);

		// Compress it
		auto Result = Compression::CompressZLibBlock((const int8_t*)&Buffer[0], CompressedBuffer, 9, MaxBufferSize);

		// Save to a file (For next test)
		std::shared_ptr<BinaryWriter> Writer = std::make_shared<BinaryWriter>();

		// Make new file
		Writer->Create("Tests/ZLibTest.bin");

		// Write result
		if (Result > 0)
		{
			Writer->Write(CompressedBuffer, Result);
		}

		// Close file
		Writer->Close();

		// Clean up
		delete[] CompressedBuffer;

		// Validate
		ASSERT_PRNT(Result > 0);
	}

#pragma endregion
	// Decompress zlib test
#pragma region Decompress zlib test

	printf(":  [33]\t\tDecompress zlib test... ");
	{
		// Load up a buffer for decompressing
		const uint8_t CompressedBuffer[] = { 0x78, 0x01, 0x01, 0x09, 0x00, 0xF6, 0xFF, 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79, 0x11, 0x73, 0x03, 0x99 };

		// Pointer for the decompressed data
		int8_t* DecompressedBuffer = new int8_t[9];
		// Clear buffer
		std::memset(DecompressedBuffer, 0, 9);

		// Decompress it
		auto Result = Compression::DecompressZLibBlock((const int8_t*)&CompressedBuffer[0], DecompressedBuffer, 20, 9);

		// Check resulting values
		bool HasSuccess = (Result > 0 && DecompressedBuffer[0] == 'h' && DecompressedBuffer[1] == 'e' && DecompressedBuffer[2] == 'y');

		// Clean up
		delete[] DecompressedBuffer;

		// Validate
		ASSERT_PRNT(HasSuccess);
	}

#pragma endregion
	// EncDec salsa20 test
#pragma region EncDec salsa20 test

	printf(":  [34]\t\tEncDec salsa20 test... ");
	{
		// Load up buffers for encrypting ('heytate!')
		const uint8_t Buffer[] = { 0x68, 0x65, 0x79, 0x74, 0x61, 0x74, 0x65, 0x21 };

		// Key and IV (128)
		uint8_t Key[16] = { 0x8c, 0x9f, 0x41, 0x52, 0xc3, 0x5b, 0xaf, 0x44, 0xbf, 0x21, 0x20, 0x02, 0x3e, 0xa7, 0xb9, 0x13 };
		uint8_t IV[8] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

		// Make key
		Salsa20Key KeyData;
		// Set
		KeyData.Key = &Key[0];
		KeyData.IV = &IV[0];
		KeyData.KeyLength = 128;

		// First, encrypt the buffer with the given key and iv
		auto Result = Encryption::Salsa20Block((int8_t*)&Buffer, 8, KeyData);
		// Next, decrypt the buffer
		auto Result2 = Encryption::Salsa20Block((int8_t*)&Buffer, 8, KeyData);

		// Result
		bool HasSuccess = (Result == 8 && Result2 == 8 && Buffer[0] == 'h' && Buffer[1] == 'e' && Buffer[2] == 'y');

		// Validate
		ASSERT_PRNT(HasSuccess);
	}

#pragma endregion
	// Base64 convert test
#pragma region Base64 convert test

	printf(":  [35]\t\tBase64 convert test... ");
	{
		// The base buffer
		std::vector<uint8_t> BaseBuffer;
		// Append message
		BaseBuffer.push_back('h');
		BaseBuffer.push_back('e');
		BaseBuffer.push_back('y');
		BaseBuffer.push_back('t');
		BaseBuffer.push_back('a');
		BaseBuffer.push_back('t');
		BaseBuffer.push_back('e');

		// Convert it
		auto Result = Strings::ToBase64String(BaseBuffer);

		// Next, convert back to buffer
		auto Result2 = Strings::FromBase64String(Result);

		// Validate
		ASSERT_PRNT(Result.size() == 12 && Result2.size() == 7);
	}

#pragma endregion
	// Vector vec2 test
#pragma region Vector vec2 test

	printf(":  [36]\t\tVector vec2 test... ");
	{
		// Make a vector (5, 10)
		Vector2 Vec(5, 10);

		// First, lets take the values
		auto XVal = Vec.X;
		auto YVal = Vec.Y;

		// Array test
		auto XArr = Vec[0];
		auto YArr = Vec[1];

		// Next, try a scalar operation
		Vec += 10;

		// Validate
		ASSERT_PRNT(Vec.X == 15 && Vec.Y == 20 && XVal == 5 && YVal == 10);
	}

#pragma endregion
	// Vector vec3 test
#pragma region Vector vec3 test

	printf(":  [37]\t\tVector vec3 test... ");
	{
		// Make a vector (5, 10, 20)
		Vector3 Vec(5, 10, 20);
		Vector3 Next(4, 3, 2);

		// First, lets take the values
		auto XVal = Vec.X;
		auto YVal = Vec.Y;
		auto ZVal = Vec.Z;

		// Array test
		auto XArr = Vec[0];
		auto YArr = Vec[1];
		auto ZArr = Vec[2];

		// Lerp
		auto Result = Vec.Lerp(0.3f, Next);

		// Next, try a scalar operation
		Vec += 10;

		// Validate
		ASSERT_PRNT(Vec.X == 15 && Vec.Y == 20 && Vec.Z == 30 && XVal == 5 && YVal == 10 && ZVal == 20);
	}

#pragma endregion
	// Vector degrad test
#pragma region Vector degrad test

	printf(":  [38]\t\tVector degrad test... ");
	{
		// Test values
		float Test1 = 360.0f;
		float Test2 = 1.5708f;

		// Convert
		auto Result = VectorMath::DegreesToRadians(Test1);
		auto Result2 = VectorMath::RadiansToDegrees(Test2);

		// Validate
		ASSERT_PRNT((Result2 > 90.0 && Result2 < 90.001) && (Result > 6.0 && Result < 6.9));
	}

#pragma endregion
	// Vector quat test
#pragma region Vector quat test

	printf(":  [39]\t\tVector quat test... ");
	{
		// Test values
		Quaternion Quat = Quaternion::FromEulerAngles(30, 30, 90);
		Quaternion Quat2(0.0f, 0.0f, 0.707f, 0.707f);

		// Convert to euler
		auto Result = Quat2.ToEulerAngles();

		// Validate
		ASSERT_PRNT(Result.X == 0.0000f && Result.Y == 0.0000f && Result.Z == 90.0000f);
	}

#pragma endregion
	// Vector matrix test
#pragma region Vector matrix test

	printf(":  [40]\t\tVector matrix test... ");
	{
		// Test values
		Quaternion Quat(0.442f, 0.162f, 0.647f, 0.600f);

		// Convert
		auto Matrx = Matrix::CreateFromQuaternion(Quat);

		Vector3 Vec(22.0f, 33.0f, 55.0f);

		// Transform
		auto Result = Matrix::TransformVector(Vec, Matrx);

		// Expected
		Vector3 Expected(23.6802692f, -4.93330669f, 63.3501053f);

		// Validate
		ASSERT_PRNT(Result == Expected);
	}

#pragma endregion
	// File createdir test
#pragma region File createdir test

	printf(":  [41]\t\tFile createdir test... ");
	{
		// Make it
		FileSystems::CreateDirectory("Tests\\NewDirTest");

		// Validate
		ASSERT_PRNT(FileSystems::DirectoryExists("Tests\\NewDirTest"));
	}

#pragma endregion
	// WraithAsset create test
#pragma region WraithAsset create test

	printf(":  [42]\t\tWraithAsset create test... ");
	{
		// Make it
		WraithAsset Asset;

		// Validate
		ASSERT_PRNT(Asset.AssetSize == -1 && Asset.AssetStatus == WraithAssetStatus::NotLoaded && Asset.AssetType == WraithAssetType::Unknown);
	}

#pragma endregion
	// WraithAnim create test
#pragma region WraithAnim create test

	printf(":  [43]\t\tWraithAnim create test... ");
	{
		// Make it
		WraithAnim Asset;

		// Generate a lot of tag names for simple tests
		std::vector<std::string> Bones;

		// Generate them
		for (int i = 0; i < 50; i++)
		{
			Bones.push_back(Strings::Format("bone%d", i));
		}

		// Add animation keys (50 bones, 200 keys per)
		for (int b = 0; b < 50; b++)
		{
			for (int i = 0; i < 200; i++)
			{
				Asset.AddTranslationKey(Bones[b], i, 0, 0.3f, 0.4f);
				Asset.AddRotationKey(Bones[b], i, 0, 0, 0, 1);
				Asset.AddScaleKey(Bones[b], i, 1, 1, 1);
			}
		}

		// Add a notetrack
		Asset.AddNoteTrack("Hello world!", 0);

		// Add modifier
		Asset.AddBoneModifier("bone3", WraithAnimationType::Relative);
		// Add modifier that isn't a normal bone
		Asset.AddBoneModifier("bone555555", WraithAnimationType::Relative);

		// Scale it
		Asset.ScaleAnimation(2.54f);

		// Validate
		ASSERT_PRNT(Asset.AssetSize == -1 && Asset.AssetStatus == WraithAssetStatus::NotLoaded && Asset.AssetType == WraithAssetType::Animation && Asset.FrameRate == 30.0f);
	}

#pragma endregion
	// SEAnim write test
#pragma region SEAnim write test

	printf(":  [44]\t\tSEAnim write test... ");
	{
		// Make it
		WraithAnim Asset;

		// Generate a lot of tag names for simple tests
		std::vector<std::string> Bones;

		// Generate them
		for (int i = 0; i < 25; i++)
		{
			Bones.push_back(Strings::Format("bone%d", i));
		}

		// Add animation keys (25 bones, 100 keys per)
		for (int b = 0; b < 25; b++)
		{
			for (int i = 0; i < 100; i++)
			{
				Asset.AddTranslationKey(Bones[b], i, 0, 0.3f, 0.4f);
				Asset.AddRotationKey(Bones[b], i, 0, 0, 0, 1);
				Asset.AddScaleKey(Bones[b], i, 1, 1, 1);
			}
		}

		// Add a notetrack
		Asset.AddNoteTrack("Hello world!", 0);

		// Add modifier
		Asset.AddBoneModifier("bone3", WraithAnimationType::Relative);
		// Add modifier that isn't a normal bone
		Asset.AddBoneModifier("bone555555", WraithAnimationType::Relative);

		// Write the SEAnim
		SEAnim::ExportSEAnim(Asset, "Tests/SEAnimTest.seanim");

		auto ResultHash = Hashing::HashSHA1File("Tests/SEAnimTest.seanim");

		// Validate
		ASSERT_PRNT(ResultHash == "c6aabd7abd2e8e216fb0b0f7a3a143e53513d3a0");
	}

#pragma endregion
	// Hash sha1 test
#pragma region Hash sha1 test

	printf(":  [45]\t\tHash sha1 test... ");
	{
		// Tests
		auto Result = Hashing::HashSHA1String("Hello world!");
		auto Result2 = Hashing::HashSHA1File("Tests/SEAnimRef.seanim");

		// Validate
		ASSERT_PRNT(Result == "d3486ae9136e7856bc42212385ea797094475802" && Result2 == "4a302d25294141d61feeb77ca8b626151eec9db0");
	}

#pragma endregion
	// Hash md5 test
#pragma region Hash md5 test

	printf(":  [46]\t\tHash md5 test... ");
	{
		// Tests
		auto Result = Hashing::HashMD5String("Hello world!");
		auto Result2 = Hashing::HashMD5File("Tests/SEAnimRef.seanim");

		// Validate
		ASSERT_PRNT(Result == "86fb269d190d2c85f6e0468ceca42a20" && Result2 == "9d5a7d224b3a1fb4a12729297c2a53d2");
	}

#pragma endregion
	// Auth gen test
#pragma region Auth gen test

	printf(":  [47]\t\tAuth gen test... ");
	{
		// Tests
		auto Result = Autherization::ComputeSystemID();

		// Validate
		ASSERT_PRNT(Result.size() > 0);
	}

#pragma endregion
	// WraithModel create test
#pragma region WraithModel create test

	printf(":  [48]\t\tWraithModel create test... ");
	{
		// Make it
		WraithModel Model;

		// Add bones
		for (auto i = 0; i < 10; i++)
		{
			// Make a bone
			auto& Bone = Model.AddBone();

			// Parent
			if (i > 0) { Bone.BoneParent = 0; }

			// Tag
			Bone.TagName = Strings::Format("bone%d", i);

			// Positions
		}

		// Build submesh
		{
			// Make a submesh
			auto& Mesh = Model.AddSubmesh();

			// No material for now
			Mesh.AddMaterial(-1);

			// Add verticies to make a cube
			{
				// A list of vectors
				std::array<Vector3, 8> Positions = { Vector3(16.0f, 16.0f, 16.0f), Vector3(16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, -16.0f), Vector3(-16.0f, -16.0f, -16.0f), Vector3(-16.0f, 16.0f, 16.0f), Vector3(-16.0f, -16.0f, 16.0f), Vector3(-16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, 16.0f) };

				// Add
				for (auto i = 0; i < 8; i++)
				{
					// Make a vert
					auto& Vertex = Mesh.AddVertex();
					// Set
					Vertex.Position = Positions[i];
					// UV
					Vertex.AddUVLayer(0.0f, 1.0f);
					// Weight
					Vertex.AddVertexWeight(0, 1.0f);
				}
			}

			// Add faces to make a cube
			Mesh.AddFace(1, 6, 3); // 2 0 1
			Mesh.AddFace(1, 3, 2); // 2 1 3
			Mesh.AddFace(7, 5, 4); // 6 4 5
			Mesh.AddFace(7, 4, 0); // 6 5 7
			Mesh.AddFace(7, 2, 3); // 10 8 9
			Mesh.AddFace(7, 3, 5); // 10 9 11
			Mesh.AddFace(7, 0, 1); // 14 12 13
			Mesh.AddFace(7, 1, 2); // 14 13 15
			Mesh.AddFace(4, 6, 1); // 18 16 17
			Mesh.AddFace(4, 1, 0); // 18 17 19
			Mesh.AddFace(3, 6, 4); // 22 20 21
			Mesh.AddFace(3, 4, 5); // 22 21 23
		}

		// Scale the model
		Model.ScaleModel(2.54f);

		// Generate global positions from our locals
		Model.GenerateGlobalPositions(true, true);

		// Properties to validate
		auto BoneCount = Model.BoneCount();
		auto VertexCount = Model.VertexCount();
		auto FaceCount = Model.FaceCount();

		// Validate
		ASSERT_PRNT(BoneCount == 10);
	}

#pragma endregion
	// Maya write test
#pragma region Maya write test

	printf(":  [49]\t\tMaya write test... ");
	{
		// Make it
		WraithModel Model;

		// Add bones
		for (auto i = 0; i < 10; i++)
		{
			// Make a bone
			auto& Bone = Model.AddBone();

			// Parent
			if (i > 0) { Bone.BoneParent = 0; }

			// Tag
			Bone.TagName = Strings::Format("bone%d", i);

			// Positions
		}

		// Build submesh
		{
			// Make a submesh
			auto& Mesh = Model.AddSubmesh();

			// No material for now
			Mesh.AddMaterial(-1);

			// Add verticies to make a cube
			{
				// A list of vectors
				std::array<Vector3, 8> Positions = { Vector3(16.0f, 16.0f, 16.0f), Vector3(16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, -16.0f), Vector3(-16.0f, -16.0f, -16.0f), Vector3(-16.0f, 16.0f, 16.0f), Vector3(-16.0f, -16.0f, 16.0f), Vector3(-16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, 16.0f) };
				std::array<Vector3, 8> Normals = { Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f) };

				// Loop and add
				for (auto i = 0; i < 8; i++)
				{
					// Make a vert
					auto& Vertex = Mesh.AddVertex();
					// Set
					Vertex.Position = Positions[i];
					Vertex.Normal = Normals[i];
					// UV
					Vertex.AddUVLayer(0.0f, 1.0f);
					// Weight
					Vertex.AddVertexWeight(0, 1.0f);

					// Custom weights
					if (i == 5)
					{
						Vertex.AddVertexWeight(2, 0.5f);
					}
					if (i == 7)
					{
						Vertex.AddVertexWeight(2, 0.5f);
						Vertex.AddVertexWeight(3, 0.1f);
					}
				}
			}

			// Weighting test
			{
				// Make a vert
				auto& Vertex = Mesh.AddVertex();

				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				// Weight
				Vertex.AddVertexWeight(1, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}

			// Add faces to make a cube
			Mesh.AddFace(1, 6, 3); // 2 0 1
			Mesh.AddFace(1, 3, 2); // 2 1 3
			Mesh.AddFace(7, 5, 4); // 6 4 5
			Mesh.AddFace(7, 4, 0); // 6 5 7
			Mesh.AddFace(7, 2, 3); // 10 8 9
			Mesh.AddFace(7, 3, 5); // 10 9 11
			Mesh.AddFace(7, 0, 1); // 14 12 13
			Mesh.AddFace(7, 1, 2); // 14 13 15
			Mesh.AddFace(4, 6, 1); // 18 16 17
			Mesh.AddFace(4, 1, 0); // 18 17 19
			Mesh.AddFace(3, 6, 4); // 22 20 21
			Mesh.AddFace(3, 4, 5); // 22 21 23
		}

		// Scale the model
		Model.ScaleModel(2.54f);

		// Generate global positions from our locals
		Model.GenerateGlobalPositions(true, true);

		// Write
		Maya::ExportMaya(Model, "Tests/MayaCube.ma");

		// Hash verify them
		auto MayaFileHash = Hashing::HashSHA1File("Tests/MayaCube.ma");
		auto MayaBindHash = Hashing::HashSHA1File("Tests/MayaCube_BIND.mel");

		// Validate
		ASSERT_PRNT(MayaFileHash == "b9f716e829b5756b0e635d33e30c8339315becf1" && MayaBindHash == "937d90f3ba6200fee72f12322c3d16be641f99e4");
	}

#pragma endregion
	// SMD write test
#pragma region SMD write test

	printf(":  [50]\t\tSMD write test... ");
	{
		// Make it
		WraithModel Model;

		// Add bones
		for (auto i = 0; i < 10; i++)
		{
			// Make a bone
			auto& Bone = Model.AddBone();

			// Parent
			if (i > 0) { Bone.BoneParent = 0; }

			// Tag
			Bone.TagName = Strings::Format("bone%d", i);

			// Positions
		}

		// Build submesh
		{
			// Make a submesh
			auto& Mesh = Model.AddSubmesh();

			// No material for now
			Mesh.AddMaterial(-1);

			// Add verticies to make a cube
			{
				// A list of vectors
				std::array<Vector3, 8> Positions = { Vector3(16.0f, 16.0f, 16.0f), Vector3(16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, -16.0f), Vector3(-16.0f, -16.0f, -16.0f), Vector3(-16.0f, 16.0f, 16.0f), Vector3(-16.0f, -16.0f, 16.0f), Vector3(-16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, 16.0f) };
				std::array<Vector3, 8> Normals = { Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f) };

				// Loop and add
				for (auto i = 0; i < 8; i++)
				{
					// Make a vert
					auto& Vertex = Mesh.AddVertex();
					// Set
					Vertex.Position = Positions[i];
					Vertex.Normal = Normals[i];
					// UV
					Vertex.AddUVLayer(0.0f, 1.0f);
					// Weight
					Vertex.AddVertexWeight(0, 1.0f);

					// Custom weights
					if (i == 5)
					{
						Vertex.AddVertexWeight(2, 0.5f);
					}
					if (i == 7)
					{
						Vertex.AddVertexWeight(2, 0.5f);
						Vertex.AddVertexWeight(3, 0.1f);
					}
				}
			}

			// Weighting test
			{
				// Make a vert
				auto& Vertex = Mesh.AddVertex();

				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				// Weight
				Vertex.AddVertexWeight(1, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}

			// Add faces to make a cube
			Mesh.AddFace(1, 6, 3); // 2 0 1
			Mesh.AddFace(1, 3, 2); // 2 1 3
			Mesh.AddFace(7, 5, 4); // 6 4 5
			Mesh.AddFace(7, 4, 0); // 6 5 7
			Mesh.AddFace(7, 2, 3); // 10 8 9
			Mesh.AddFace(7, 3, 5); // 10 9 11
			Mesh.AddFace(7, 0, 1); // 14 12 13
			Mesh.AddFace(7, 1, 2); // 14 13 15
			Mesh.AddFace(4, 6, 1); // 18 16 17
			Mesh.AddFace(4, 1, 0); // 18 17 19
			Mesh.AddFace(3, 6, 4); // 22 20 21
			Mesh.AddFace(3, 4, 5); // 22 21 23
		}

		// Scale the model
		Model.ScaleModel(2.54f);

		// Generate global positions from our locals
		Model.GenerateGlobalPositions(true, true);

		// Write
		ValveSMD::ExportSMD(Model, "Tests/SMDCube.smd");

		auto Result = Hashing::HashSHA1File("Tests/SMDCube.smd");

		// Validate
		ASSERT_PRNT(Result == "a56550d5c88415a8fe9986a34d8ecd70b7b80202");
	}

#pragma endregion
	// OBJ write test
#pragma region OBJ write test

	printf(":  [51]\t\tOBJ write test... ");
	{
		// Make it
		WraithModel Model;

		// Add bones
		for (auto i = 0; i < 10; i++)
		{
			// Make a bone
			auto& Bone = Model.AddBone();

			// Parent
			if (i > 0) { Bone.BoneParent = 0; }

			// Tag
			Bone.TagName = Strings::Format("bone%d", i);

			// Positions
		}

		// Build submesh
		{
			// Make a submesh
			auto& Mesh = Model.AddSubmesh();

			// No material for now
			Mesh.AddMaterial(-1);

			// Add verticies to make a cube
			{
				// A list of vectors
				std::array<Vector3, 8> Positions = { Vector3(16.0f, 16.0f, 16.0f), Vector3(16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, -16.0f), Vector3(-16.0f, -16.0f, -16.0f), Vector3(-16.0f, 16.0f, 16.0f), Vector3(-16.0f, -16.0f, 16.0f), Vector3(-16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, 16.0f) };
				std::array<Vector3, 8> Normals = { Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f) };

				// Loop and add
				for (auto i = 0; i < 8; i++)
				{
					// Make a vert
					auto& Vertex = Mesh.AddVertex();
					// Set
					Vertex.Position = Positions[i];
					Vertex.Normal = Normals[i];
					// UV
					Vertex.AddUVLayer(0.0f, 1.0f);
					// Weight
					Vertex.AddVertexWeight(0, 1.0f);

					// Custom weights
					if (i == 5)
					{
						Vertex.AddVertexWeight(2, 0.5f);
					}
					if (i == 7)
					{
						Vertex.AddVertexWeight(2, 0.5f);
						Vertex.AddVertexWeight(3, 0.1f);
					}
				}
			}

			// Weighting test
			{
				// Make a vert
				auto& Vertex = Mesh.AddVertex();

				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				// Weight
				Vertex.AddVertexWeight(1, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}

			// Add faces to make a cube
			Mesh.AddFace(1, 6, 3); // 2 0 1
			Mesh.AddFace(1, 3, 2); // 2 1 3
			Mesh.AddFace(7, 5, 4); // 6 4 5
			Mesh.AddFace(7, 4, 0); // 6 5 7
			Mesh.AddFace(7, 2, 3); // 10 8 9
			Mesh.AddFace(7, 3, 5); // 10 9 11
			Mesh.AddFace(7, 0, 1); // 14 12 13
			Mesh.AddFace(7, 1, 2); // 14 13 15
			Mesh.AddFace(4, 6, 1); // 18 16 17
			Mesh.AddFace(4, 1, 0); // 18 17 19
			Mesh.AddFace(3, 6, 4); // 22 20 21
			Mesh.AddFace(3, 4, 5); // 22 21 23
		}

		// Scale the model
		Model.ScaleModel(2.54f);

		// Generate global positions from our locals
		Model.GenerateGlobalPositions(true, true);

		// Write
		WavefrontOBJ::ExportOBJ(Model, "Tests/OBJCube.obj");

		auto Result = Hashing::HashSHA1File("Tests/OBJCube.obj");

		// Validate
		ASSERT_PRNT(Result == "afd4f541357d2ea4ca6fb9dd57e3aa6b89b01eb6");
	}

#pragma endregion
	// XME write test
#pragma region XME write test

	printf(":  [52]\t\tXME write test... ");
	{
		// Make it
		WraithModel Model;

		// Add bones
		for (auto i = 0; i < 10; i++)
		{
			// Make a bone
			auto& Bone = Model.AddBone();

			// Parent
			if (i > 0) { Bone.BoneParent = 0; }

			// Tag
			Bone.TagName = Strings::Format("bone%d", i);

			// Positions
		}

		// Build submesh
		{
			// Make a submesh
			auto& Mesh = Model.AddSubmesh();

			// No material for now
			Mesh.AddMaterial(-1);

			// Add verticies to make a cube
			{
				// A list of vectors
				std::array<Vector3, 8> Positions = { Vector3(16.0f, 16.0f, 16.0f), Vector3(16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, -16.0f), Vector3(-16.0f, -16.0f, -16.0f), Vector3(-16.0f, 16.0f, 16.0f), Vector3(-16.0f, -16.0f, 16.0f), Vector3(-16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, 16.0f) };
				std::array<Vector3, 8> Normals = { Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f) };

				// Loop and add
				for (auto i = 0; i < 8; i++)
				{
					// Make a vert
					auto& Vertex = Mesh.AddVertex();
					// Set
					Vertex.Position = Positions[i];
					Vertex.Normal = Normals[i];
					// UV
					Vertex.AddUVLayer(0.0f, 1.0f);
					// Weight
					Vertex.AddVertexWeight(0, 1.0f);

					// Custom weights
					if (i == 5)
					{
						Vertex.AddVertexWeight(2, 0.5f);
					}
					if (i == 7)
					{
						Vertex.AddVertexWeight(2, 0.5f);
						Vertex.AddVertexWeight(3, 0.1f);
					}
				}
			}

			// Weighting test
			{
				// Make a vert
				auto& Vertex = Mesh.AddVertex();

				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				// Weight
				Vertex.AddVertexWeight(1, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}

			// Add faces to make a cube
			Mesh.AddFace(1, 6, 3); // 2 0 1
			Mesh.AddFace(1, 3, 2); // 2 1 3
			Mesh.AddFace(7, 5, 4); // 6 4 5
			Mesh.AddFace(7, 4, 0); // 6 5 7
			Mesh.AddFace(7, 2, 3); // 10 8 9
			Mesh.AddFace(7, 3, 5); // 10 9 11
			Mesh.AddFace(7, 0, 1); // 14 12 13
			Mesh.AddFace(7, 1, 2); // 14 13 15
			Mesh.AddFace(4, 6, 1); // 18 16 17
			Mesh.AddFace(4, 1, 0); // 18 17 19
			Mesh.AddFace(3, 6, 4); // 22 20 21
			Mesh.AddFace(3, 4, 5); // 22 21 23
		}

		// Scale the model
		Model.ScaleModel(2.54f);

		// Generate global positions from our locals
		Model.GenerateGlobalPositions(true, true);

		// Write
		CodXME::ExportXME(Model, "Tests/XMECube.XMODEL_EXPORT");

		auto Result = Hashing::HashSHA1File("Tests/XMECube.XMODEL_EXPORT");

		// Validate
		ASSERT_PRNT(Result == "07fe81995428565d33fb4a7aa83651104695ef03");
	}

#pragma endregion
	// Read memory test
#pragma region Read memory test

	printf(":  [53]\t\tRead memory test... ");
	{
		// Tests
		auto Reader = std::make_unique<MemoryReader>();

		// Read the test data to a buffer, then setup the reader
		{
			// Read
			auto BReader = std::make_unique<BinaryReader>();
			BReader->Open("Tests/BinaryRead.bin");

			// Size
			uint64_t SizeRead = 0;

			// Make buffer
			auto Buffer = BReader->Read(0x200, SizeRead);

			// Load
			Reader->Setup(Buffer, SizeRead);

			// Clean up
			BReader.reset();
		}

		// Prepare to read data
		uint32_t Magic = Reader->Read<uint32_t>();

		// Jump to string
		Reader->SetPosition(0x4E);

		// Read
		auto ThisProgram = Reader->ReadNullTerminatedString();

		// Go back
		Reader->SetPosition(0x4E);

		// Read
		auto ThisProgram2 = Reader->ReadString(0x2b);

		// Compare data
		auto CompareData = "This program cannot be run in DOS mode.\r\r\n$";

		// Clean up
		Reader.reset();

		// Validate
		ASSERT_PRNT(Magic == 0x00905a4d && (strncmp(ThisProgram.c_str(), CompareData, ThisProgram.size())) == 0 && (strncmp(ThisProgram2.c_str(), CompareData, ThisProgram2.size())) == 0);
	}

#pragma endregion
	// Image transcode test
#pragma region Image transcode test

	printf(":  [54]\t\tImage transcode test... ");
	{
		// Prepare to transcode some images and test patches
		auto Result1 = Image::ConvertImageFile("Tests/ImageTest1.png", ImageFormat::Standard_PNG, "Tests/ImageRes1.dds", ImageFormat::DDS_BC1_UNORM);
		auto Result2 = Image::ConvertImageFile("Tests/ImageTest1.png", ImageFormat::Standard_PNG, "Tests/ImageRes2.tga", ImageFormat::Standard_TGA);
		auto Result3 = Image::ConvertImageFile("Tests/ImageTest1.png", ImageFormat::Standard_PNG, "Tests/ImageRes3.jpg", ImageFormat::Standard_JPEG);
		auto Result4 = Image::ConvertImageFile("Tests/ImageTest4.dds", ImageFormat::DDS_WithHeader, "Tests/ImageRes4.png", ImageFormat::Standard_PNG);
		auto Result5 = Image::ConvertImageFile("Tests/ImageTest5.dds", ImageFormat::DDS_WithHeader, "Tests/ImageRes5.png", ImageFormat::Standard_PNG, ImagePatch::Normal_Expand);

		// Verify the results
		auto Hash1 = Hashing::HashSHA1File("Tests/ImageRes1.dds") == "db53ad63f7c02b56cc6a689321b4ca9c256583b8";
		auto Hash2 = Hashing::HashSHA1File("Tests/ImageRes2.tga") == "680b289a5c208572bd3714e42da553b84aa73bc5";
		auto Hash3 = Hashing::HashSHA1File("Tests/ImageRes3.jpg") == "eeadb7f672b2ddf4c859cdcd13d7a0a06c9a62cc";
		auto Hash4 = Hashing::HashSHA1File("Tests/ImageRes4.png") == "898ab5d7f90334be0877adc4f63f5cd07ffd803b";
		auto Hash5 = Hashing::HashSHA1File("Tests/ImageRes5.png") == "eccfc6722e1e9bd1ee5c590a95c52084f45751cd";

		// Validate
		ASSERT_PRNT(Hash1 && Hash2 && Hash3 && Hash4 && Hash5);
	}

#pragma endregion
	// Image DDS test
#pragma region Image DDS test

	printf(":  [55]\t\tImage DDS test... ");
	{
		// Prepare to write some DDS headers
		auto TestWriter1 = std::make_shared<BinaryWriter>();
		// Setup
		TestWriter1->Create("Tests/ImageDDS1.dds");
		// Test 2
		auto TestWriter2 = std::make_shared<BinaryWriter>();
		// Setup
		TestWriter2->Create("Tests/ImageDDS2.dds");

		// Write
		Image::WriteDDSHeaderToFile(TestWriter1, 500, 500, 1, ImageFormat::DDS_BC1_UNORM);
		Image::WriteDDSHeaderToFile(TestWriter2, 200, 200, 1, ImageFormat::DDS_BC7_UNORM);

		// Close
		TestWriter1.reset();
		TestWriter2.reset();

		// Hash
		auto Hash1 = Hashing::HashSHA1File("Tests/ImageDDS1.dds") == "53e33f59d8ed0ae7a5c8510e1145feea5a857c72";
		auto Hash2 = Hashing::HashSHA1File("Tests/ImageDDS2.dds") == "f896e7a5f591d06e7c3c612fd3dcc9e1ce8d3dda";

		// Validate
		ASSERT_PRNT(Hash1 && Hash2);
	}

#pragma endregion
	// Decompress deflate test
#pragma region Decompress deflate test

	printf(":  [56]\t\tDecompress deflate test... ");
	{
		// Load up a buffer for decompressing
		const uint8_t CompressedBuffer[] = { 0xF3, 0x70, 0x8D, 0x0C, 0x71, 0x0C, 0x71, 0x0D, 0xF7, 0x70, 0x0C, 0x09, 0x0E, 0x0D, 0x00, 0x00 };

		// Pointer for the decompressed data
		int8_t* DecompressedBuffer = new int8_t[14];
		// Clear buffer
		std::memset(DecompressedBuffer, 0, 14);

		// Decompress it
		auto Result = Compression::DecompressDeflateBlock((const int8_t*)&CompressedBuffer[0], DecompressedBuffer, 16, 14);

		// Check resulting values
		bool HasSuccess = (Result > 0 && DecompressedBuffer[0] == 'H' && DecompressedBuffer[1] == 'E' && DecompressedBuffer[2] == 'Y');

		// Clean up
		delete[] DecompressedBuffer;

		// Validate
		ASSERT_PRNT(HasSuccess);
	}

#pragma endregion
	// Compress deflate test
#pragma region Compress deflate test

	printf(":  [57]\t\tCompress deflate test... ");
	{
		// Load up a buffer for compressing
		const uint8_t Buffer[] = { 0x68, 0x65, 0x79, 0x20, 0x72, 0x6F, 0x6C, 0x6C, 0x79 };

		// Calculate max size
		auto MaxBufferSize = Compression::CompressionSizeDeflate(9);

		// Pointer for the compressed data
		int8_t* CompressedBuffer = new int8_t[MaxBufferSize];
		// Set mem
		std::memset(CompressedBuffer, 0, MaxBufferSize);

		// Compress it
		auto Result = Compression::CompressDeflateBlock((const int8_t*)&Buffer[0], CompressedBuffer, 9, MaxBufferSize);

		// Save to a file (For next test)
		std::shared_ptr<BinaryWriter> Writer = std::make_shared<BinaryWriter>();

		// Make new file
		Writer->Create("Tests/DeflateTest.bin");

		// Write result
		if (Result > 0)
		{
			Writer->Write(CompressedBuffer, Result);
		}

		// Close file
		Writer->Close();

		// Clean up
		delete[] CompressedBuffer;

		// Validate
		ASSERT_PRNT(Result > 0);
	}

#pragma endregion
	// Hash xxhash test
#pragma region Hash xxhash test

	printf(":  [58]\t\tHash xxhash test... ");
	{
		// Tests
		auto Result = Hashing::HashXXHashString("Hello world!");
		std::string TestBuffer = "Whatisliveeeeee";
		auto Result2 = Hashing::HashXXHashStream((int8_t*)TestBuffer.c_str(), 14);

		// Validate
		ASSERT_PRNT(Result == 0x7f173f227ffd7db2 && Result2 == 0x104de5d92dcb4cc3);
	}

#pragma endregion
	// Single instance test
#pragma region Single instance test

	printf(":  [59]\t\tSingle instance test... ");
	{
		// Tests
		auto Result = Instance::BeginSingleInstance("WraithX");
		auto Result2 = Instance::BeginSingleInstance("WraithX");

		// Close
		Instance::EndSingleInstance();

		// Validate
		ASSERT_PRNT(Result == true && Result2 == false);
	}

#pragma endregion
	// String contains test
#pragma region String contains test

	printf(":  [60]\t\tString contains test... ");
	{
		// Tests
		auto Result = Strings::Contains("hello world", "world");
		auto Result2 = Strings::Contains("hello world", "pie");

		// Validate
		ASSERT_PRNT(Result == true && Result2 == false);
	}

#pragma endregion
	// XNA write test
#pragma region XNA write test

	printf(":  [61]\t\tXNA write test... ");
	{
		// Make it
		WraithModel Model;

		// Add bones
		for (auto i = 0; i < 10; i++)
		{
			// Make a bone
			auto& Bone = Model.AddBone();

			// Parent
			if (i > 0) { Bone.BoneParent = 0; }

			// Tag
			Bone.TagName = Strings::Format("bone%d", i);

			// Positions
		}

		// Build submesh
		{
			// Make a submesh
			auto& Mesh = Model.AddSubmesh();

			// No material for now
			Mesh.AddMaterial(-1);

			// Add verticies to make a cube
			{
				// A list of vectors
				std::array<Vector3, 8> Positions = { Vector3(16.0f, 16.0f, 16.0f), Vector3(16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, -16.0f), Vector3(-16.0f, -16.0f, -16.0f), Vector3(-16.0f, 16.0f, 16.0f), Vector3(-16.0f, -16.0f, 16.0f), Vector3(-16.0f, 16.0f, -16.0f), Vector3(16.0f, -16.0f, 16.0f) };
				std::array<Vector3, 8> Normals = { Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f) };

				// Loop and add
				for (auto i = 0; i < 8; i++)
				{
					// Make a vert
					auto& Vertex = Mesh.AddVertex();
					// Set
					Vertex.Position = Positions[i];
					Vertex.Normal = Normals[i];
					// UV
					Vertex.AddUVLayer(0.0f, 1.0f);
					// Weight
					Vertex.AddVertexWeight(0, 1.0f);

					// Custom weights
					if (i == 5)
					{
						Vertex.AddVertexWeight(2, 0.5f);
					}
					if (i == 7)
					{
						Vertex.AddVertexWeight(2, 0.5f);
						Vertex.AddVertexWeight(3, 0.1f);
					}
				}
			}

			// Weighting test
			{
				// Make a vert
				auto& Vertex = Mesh.AddVertex();

				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				// Weight
				Vertex.AddVertexWeight(1, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}
			{
				// Add
				auto& Vertex = Mesh.AddVertex();
				// UV
				Vertex.AddUVLayer(0.0f, 1.0f);

				Vertex.AddVertexWeight(0, 1.0f);

				// Set
				Vertex.Position = Vector3(16.0f, 16.0f, 16.0f); // 0
				Vertex.Normal = Vector3(0.0f, 0.0f, 1.0f);
			}

			// Add faces to make a cube
			Mesh.AddFace(1, 6, 3); // 2 0 1
			Mesh.AddFace(1, 3, 2); // 2 1 3
			Mesh.AddFace(7, 5, 4); // 6 4 5
			Mesh.AddFace(7, 4, 0); // 6 5 7
			Mesh.AddFace(7, 2, 3); // 10 8 9
			Mesh.AddFace(7, 3, 5); // 10 9 11
			Mesh.AddFace(7, 0, 1); // 14 12 13
			Mesh.AddFace(7, 1, 2); // 14 13 15
			Mesh.AddFace(4, 6, 1); // 18 16 17
			Mesh.AddFace(4, 1, 0); // 18 17 19
			Mesh.AddFace(3, 6, 4); // 22 20 21
			Mesh.AddFace(3, 4, 5); // 22 21 23
		}

		// Scale the model
		Model.ScaleModel(2.54f);

		// Generate global positions from our locals
		Model.GenerateGlobalPositions(true, true);

		// Write
		XNALara::ExportXNA(Model, "Tests/XNACube.mesh.ascii");

		auto Result = Hashing::HashSHA1File("Tests/XNACube.mesh.ascii");

		// Validate
		ASSERT_PRNT(Result == "317d48d9ce05b956808a205b1a7fcafe67c65931");
	}

#pragma endregion
	// String ends test
#pragma region String ends test

	printf(":  [62]\t\tString ends test... ");
	{
		// Parse it
		auto Result = Strings::EndsWith("hello world", "world");
		auto Result2 = Strings::EndsWith("hello world", "hello");

		// Validate
		ASSERT_PRNT(Result == true && Result2 == false);
	}

#pragma endregion
	// String begins test
#pragma region String begins test

	printf(":  [63]\t\tString begins test... ");
	{
		// Parse it
		auto Result = Strings::StartsWith("hello world", "world");
		auto Result2 = Strings::StartsWith("hello world", "hello");

		// Validate
		ASSERT_PRNT(Result == false && Result2 == true);
	}

#pragma endregion
	// System processes test
#pragma region System processes test

	printf(":  [64]\t\tSystem processes test... ");
	{
		// Parse it
		auto Processes = Systems::GetProcesses();
		// Check for common processes
		bool HadCSRSS = false;
		bool HadSystem = false;

		// Loop and check
		for (auto& Process : Processes)
		{
			// Check for csrss
			if (_stricmp(Process.ProcessName.c_str(), "csrss.exe") == 0)
			{
				HadCSRSS = true;
			}
			else if (_stricmp(Process.ProcessName.c_str(), "System") == 0)
			{
				HadSystem = true;
			}
		}

		// Validate
		ASSERT_PRNT(HadCSRSS == true && HadSystem == true);
	}

#pragma endregion
	// FLAC decode test
#pragma region FLAC decode test

	printf(":  [65]\t\tFLAC decode test... ");
	{
		// Decode the file to a WAV
		auto Result = Sound::ConvertSoundFile("Tests/TestFLAC1.flac", SoundFormat::FLAC_WithHeader, "Tests/OutWav1.wav", SoundFormat::Standard_WAV);
		
		// Hash result
		auto Hash = Hashing::HashSHA1File("Tests/OutWav1.wav");

		// Validate
		ASSERT_PRNT(Result && Hash == "9e1018ef748fc40c9f252d093c8f689ae890013c");
	}

#pragma endregion
	// FLAC encode test
#pragma region FLAC encode test

	printf(":  [66]\t\tFLAC encode test... ");
	{
		// Encode the file to a FLAC
		auto Result = Sound::ConvertSoundFile("Tests/TestWAV1.wav", SoundFormat::WAV_WithHeader, "Tests/OutFlac1.flac", SoundFormat::Standard_FLAC);

		// Hash result
		auto Hash = Hashing::HashSHA1File("Tests/OutFlac1.flac");

		// Validate
		ASSERT_PRNT(Result && Hash == "8921f4b4f0e988b101e5c21f185e626050dc94d4");
	}

#pragma endregion
	// FLAC header test
#pragma region FLAC header test

	printf(":  [67]\t\tFLAC header test... ");
	{
		// Writer
		auto Writer = BinaryWriter();
		// Open
		Writer.Create("Tests/OutFlacHeader.flac");
		// Encode the flac header
		Sound::WriteFLACHeaderToFile(Writer, 48000, 2, 40000);

		// Close
		Writer.Close();

		// Hash result
		auto Hash = Hashing::HashSHA1File("Tests/OutFlacHeader.flac");

		// Validate
		ASSERT_PRNT(Hash == "6301b9fe9c9eaf7d8f32295bbda63920fb1fd20a");
	}

#pragma endregion
	// WAV header test
#pragma region WAV header test

	printf(":  [68]\t\tWAV header test... ");
	{
		// Writer
		auto Writer = BinaryWriter();
		// Open
		Writer.Create("Tests/OutWavHeader.wav");
		// Encode the wav header
		Sound::WriteWAVHeaderToFile(Writer, 48000, 2, 43212);

		// Close
		Writer.Close();

		// Hash result
		auto Hash = Hashing::HashSHA1File("Tests/OutWavHeader.wav");

		// Validate
		ASSERT_PRNT(Hash == "b1a0922ede9b3758adb885deb129d9a7f1520d49");
	}

#pragma endregion
	// Name index test
#pragma region Name index test

	printf(":  [69]\t\tName index test... ");
	{
		// Load test file
		WraithNameIndex TestIndex("Tests/Test.wni");

		// Validate
		ASSERT_PRNT(TestIndex.NameDatabase.size() == 7436);
	}

#pragma endregion
	// Make index test
#pragma region Make index test

	printf(":  [70]\t\tMake index test... ");
	{
		// Make an index
		WraithNameIndex NameIndex;
		// Add entries
		NameIndex.NameDatabase[0x334455] = "Test1";
		NameIndex.NameDatabase[0x334454] = "Test2";
		NameIndex.NameDatabase[0x334452] = "Test3";

		// Save
		NameIndex.SaveIndex("Tests/Result1.wni");

		// Load up again
		WraithNameIndex ResultIndex("Tests/Result1.wni");

		// Validate
		ASSERT_PRNT(ResultIndex.NameDatabase.size() == 3);
	}

#pragma endregion
	// Console color read test
#pragma region Console color read test

	printf(":  [71]\t\tConsole color read test... ");
	{
		auto Foreground = Console::GetForegroundColor();
		auto Background = Console::GetBackgroundColor();

		// Validate
		ASSERT_PRNT((Foreground == ConsoleColor::Gray || Foreground == ConsoleColor::White) && Background == ConsoleColor::Black);
	}

#pragma endregion
	// Console color write test
#pragma region Console color write test

	printf(":  [72]\t\tConsole color write test... ");
	{
		// Store originals
		auto Foreground = Console::GetForegroundColor();
		auto Background = Console::GetBackgroundColor();

		// Set new ones
		Console::SetBackgroundColor(ConsoleColor::Red);
		Console::SetForegroundColor(ConsoleColor::Blue);

		// Get new values
		auto NewForeground = Console::GetForegroundColor();
		auto NewBackground = Console::GetBackgroundColor();

		// Reset
		Console::SetForegroundColor(Foreground);
		Console::SetBackgroundColor(Background);

		// Validate against the 'new' colors
		ASSERT_PRNT(NewForeground == ConsoleColor::Blue && NewBackground == ConsoleColor::Red);
	}

#pragma endregion
	// WebClient downloadstring test
#pragma region WebClient downloadstring test

	printf(":  [73]\t\tWebClient downloadstring test... ");
	{
		// Get it
		auto Downloader = WebClient();
		
		// Download it
		auto Result = Downloader.DownloadString("https://example.com/");

		// Validate against the title
		ASSERT_PRNT(Result.find("Example Domain") != std::string::npos);
	}

#pragma endregion
	// WebClient downloaddata test
#pragma region WebClient downloaddata test

	printf(":  [74]\t\tWebClient downloaddata test... ");
	{
		// Get it
		auto Downloader = WebClient();

		// Download it
		auto Result = Downloader.DownloadData("https://example.com/");

		// Validate against the title
		ASSERT_PRNT(Result->DataBuffer != nullptr && Result->BufferSize > 0);
	}

#pragma endregion

	// Clean up
	WraithX::ShutdownAPI(true);

	// All tests completed
	printf("\r\nCompleted (%d) tests, failed (%d) tests... Press any key to end...", TestsCompleted, TestsFailed);
	// Wait
	_getch();
	// Result
	return 0;
}