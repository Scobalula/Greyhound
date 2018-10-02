#pragma once

#include <cstdint>
#include <string>
#include <cstdio>
#include <memory>
#include <winhttp.h>

// Which system proxy settings to use
enum class ProxyType
{
	// Bypass proxy settings
	NoProxy,
	// Use system proxy
	DefaultProxy
};

// A downloaded data blob
struct DownloadMemoryResult
{
	uint8_t* DataBuffer;
	uint64_t BufferSize;

	DownloadMemoryResult()
	{
		DataBuffer = nullptr;
		BufferSize = 0;
	}

	~DownloadMemoryResult()
	{
		if (DataBuffer != nullptr)
			free(DataBuffer);
		DataBuffer = nullptr;
		BufferSize = 0;
	}
};

// A class that handles reading and writing remote files via HTTP(s)
class WebClient
{
private:
	// A handle to the internet resources
	HINTERNET InternetHandle;

public:
	WebClient(ProxyType Proxy = ProxyType::NoProxy);
	~WebClient();

	// Sets the timeout period in Ms
	void SetTimeout(uint32_t Milliseconds);

	// Download a file with the given URL to the disk
	void DownloadFile(const std::string& Url, const std::string& FileName);
	// Download a string from the given URL
	std::string DownloadString(const std::string& Url);
	// Download a file with the given URL to memory
	std::unique_ptr<DownloadMemoryResult> DownloadData(const std::string& Url);

	// Upload a file to the given URL from the disk
	bool UploadFile(const std::string& Url, const std::string& FileName);
	// Upload a block of memory to the given URL
	bool UploadData(const std::string& Url, const uint8_t* Buffer, uint64_t BufferSize);
};