#include "stdafx.h"

// The class we are implementing
#include "WebClient.h"
#include "BinaryWriter.h"
#include "BinaryReader.h"
#include "Strings.h"

// Automatically link the library
#pragma comment(lib, "Winhttp.lib")

// The default useragent
const static wchar_t* UserAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Safari/537.36 WraithX";
// The default download buffer length (10mb)
const uint32_t DownloadBufferLength = 0xA00000;
// The default timeout length (100s)
const uint32_t TimeoutLength = 10000;

// TODO: We can report download / upload progress, download if content-length is specified (Default to indeterminate)
// TODO: Should use a universal callback w/ the flags

// A parsed url object
struct UriObject
{
	// The port type, defaults to default
	INTERNET_PORT PortType;
	// The hostname of the url
	std::wstring HostName;
	// The path, if any
	std::wstring Path;

	UriObject()
	{
		this->PortType = INTERNET_DEFAULT_PORT;
		this->HostName = L"";
		this->Path = L"";
	}
};

// Helper routine to parse a URL
const UriObject PreParseUrl(std::string Url)
{
	// The result
	auto Result = UriObject();

	// Check for and remove the HTTP(s):// from the Url, we don't want it
	if (_strnicmp(Url.c_str(), "https://", strlen("https://")) == 0)
	{
		Result.PortType = INTERNET_DEFAULT_HTTPS_PORT;
		Url = Url.substr(8);
	}
	else if (_strnicmp(Url.c_str(), "http://", strlen("http://")) == 0)
	{
		Result.PortType = INTERNET_DEFAULT_HTTP_PORT;
		Url = Url.substr(7);
	}
	else if (_strnicmp(Url.c_str(), "//", strlen("//")) == 0)
	{
		Url = Url.substr(7);
	}
	
	// Find path, if any
	auto PathPosition = Url.find_first_of("/");

	// Strip out hostname
	Result.HostName = Strings::ToUnicodeString(Url.substr(0, PathPosition));

	// Check for an existing path, Aka from '/' onward
	if (PathPosition != std::string::npos)
		Result.Path = Strings::ToUnicodeString(Url.substr(Url.find_first_of("/")));

	// Return it
	return Result;
}

WebClient::WebClient(ProxyType Proxy)
{
	// Open the HTTP instance with the provided configuration
	this->InternetHandle = WinHttpOpen(UserAgent, (Proxy == ProxyType::DefaultProxy) ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY : WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, NULL);

	// Ensure success
	if (this->InternetHandle == NULL)
	{
#if _DEBUG
		throw std::exception("Failed to initialize HTTP library");
#else
		throw std::exception();
#endif
	}

	// Set our default properties
	uint32_t DecodeOption = WINHTTP_DECOMPRESSION_FLAG_ALL;

	// Set them on the handle
	WinHttpSetOption(this->InternetHandle, WINHTTP_OPTION_DECOMPRESSION, &DecodeOption, sizeof(uint32_t));
	WinHttpSetTimeouts(this->InternetHandle, TimeoutLength, TimeoutLength, TimeoutLength, TimeoutLength);
}

WebClient::~WebClient()
{
	// Clean up
	if (this->InternetHandle != NULL)
		WinHttpCloseHandle(this->InternetHandle);
}

void WebClient::SetTimeout(uint32_t Milliseconds)
{
	// Set the specified option(s)
	WinHttpSetTimeouts(this->InternetHandle, Milliseconds, Milliseconds, Milliseconds, Milliseconds);
}

void WebClient::DownloadFile(const std::string& Url, const std::string& FileName)
{
	// Connect to the HTTP server
	auto Uri = PreParseUrl(Url);

	// Handles
	HINTERNET hConnect = NULL, hRequest = NULL;
	BOOL CanDownload = false;

	// Attempt to connect
	hConnect = WinHttpConnect(this->InternetHandle, Uri.HostName.c_str(), Uri.PortType, NULL);
	// On succes, make a request (secure if need be)
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", Uri.Path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (Uri.PortType == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : NULL);
	// On success, send the requrst
	if (hRequest)
		CanDownload = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, NULL, WINHTTP_NO_REQUEST_DATA, NULL, NULL, NULL);
	// On success, end request
	if (CanDownload)
		CanDownload = WinHttpReceiveResponse(hRequest, NULL);

	// If we can, read until we have no more data
	if (CanDownload)
	{
		DWORD BufferSize = 0, Downloaded = 0;

		// Create the output file
		auto OutputFile = BinaryWriter();
		// Create it
		OutputFile.Create(FileName);
		
		// Loop until we run out of data
		do
		{
			BufferSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &BufferSize))
				break;

			// Ensure we have data
			if (!BufferSize)
				break;

			// Buffer for some data
			auto TempBuffer = std::make_unique<char[]>(std::min<uint32_t>(BufferSize, DownloadBufferLength));
			// Read it
			WinHttpReadData(hRequest, (LPVOID)TempBuffer.get(), std::min<uint32_t>(BufferSize, DownloadBufferLength), &Downloaded);

			// Write to our file
			OutputFile.Write((int8_t*)TempBuffer.get(), Downloaded);

			// Check for more data
		} while (BufferSize > 0);
	}

	// Clean up
	if (hRequest)
		WinHttpCloseHandle(hRequest);
	if (hConnect)
		WinHttpCloseHandle(hConnect);
}

std::string WebClient::DownloadString(const std::string& Url)
{
	// Connect to the HTTP server
	auto Uri = PreParseUrl(Url);

	// Handles
	HINTERNET hConnect = NULL, hRequest = NULL;
	BOOL CanDownload = false;

	// Attempt to connect
	hConnect = WinHttpConnect(this->InternetHandle, Uri.HostName.c_str(), Uri.PortType, NULL);
	// On succes, make a request (secure if need be)
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", Uri.Path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (Uri.PortType == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : NULL);
	// On success, send the requrst
	if (hRequest)
		CanDownload = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, NULL, WINHTTP_NO_REQUEST_DATA, NULL, NULL, NULL);
	// On success, end request
	if (CanDownload)
		CanDownload = WinHttpReceiveResponse(hRequest, NULL);

	// If we can, read until we have no more data
	if (CanDownload)
	{
		DWORD BufferSize = 0, Downloaded = 0;

		// Create the output buffer
		std::stringstream Buffer;

		// Loop until we run out of data
		do
		{
			BufferSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &BufferSize))
				break;

			// Ensure we have data
			if (!BufferSize)
				break;

			// Buffer for some data (with a null char)
			auto TempBuffer = std::make_unique<char[]>(std::min<uint32_t>(BufferSize, DownloadBufferLength) + 1);
			// Read it
			WinHttpReadData(hRequest, (LPVOID)TempBuffer.get(), std::min<uint32_t>(BufferSize, DownloadBufferLength), &Downloaded);

			// Copy to our buffer
			Buffer << TempBuffer.get();

			// Check for more data
		} while (BufferSize > 0);

		// Clean up
		if (hRequest)
			WinHttpCloseHandle(hRequest);
		if (hConnect)
			WinHttpCloseHandle(hConnect);

		// Return result
		return Buffer.str();
	}

	// Clean up
	if (hRequest)
		WinHttpCloseHandle(hRequest);
	if (hConnect)
		WinHttpCloseHandle(hConnect);

	// Failed
	return "";
}

std::unique_ptr<DownloadMemoryResult> WebClient::DownloadData(const std::string& Url)
{
	// Connect to the HTTP server
	auto Uri = PreParseUrl(Url);

	// Handles
	HINTERNET hConnect = NULL, hRequest = NULL;
	BOOL CanDownload = false;

	// Attempt to connect
	hConnect = WinHttpConnect(this->InternetHandle, Uri.HostName.c_str(), Uri.PortType, NULL);
	// On succes, make a request (secure if need be)
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", Uri.Path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, (Uri.PortType == INTERNET_DEFAULT_HTTPS_PORT) ? WINHTTP_FLAG_SECURE : NULL);
	// On success, send the requrst
	if (hRequest)
		CanDownload = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, NULL, WINHTTP_NO_REQUEST_DATA, NULL, NULL, NULL);
	// On success, end request
	if (CanDownload)
		CanDownload = WinHttpReceiveResponse(hRequest, NULL);

	// If we can, read until we have no more data
	if (CanDownload)
	{
		DWORD BufferSize = 0, Downloaded = 0;

		// Create the output buffer
		auto Result = std::make_unique<DownloadMemoryResult>();

		// Loop until we run out of data
		do
		{
			BufferSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &BufferSize))
				break;

			// Ensure we have data
			if (!BufferSize)
				break;

			// Buffer for some data
			auto TempBuffer = std::make_unique<char[]>(std::min<uint32_t>(BufferSize, DownloadBufferLength));
			// Read it
			WinHttpReadData(hRequest, (LPVOID)TempBuffer.get(), std::min<uint32_t>(BufferSize, DownloadBufferLength), &Downloaded);

			// Move to our buffer
			if (Result->DataBuffer == nullptr)
				Result->DataBuffer = (uint8_t*)malloc(Downloaded);
			else
				Result->DataBuffer = (uint8_t*)realloc(Result->DataBuffer, (Result->BufferSize + Downloaded));

			// Copy the data over
			std::memcpy(Result->DataBuffer + Result->BufferSize, TempBuffer.get(), Downloaded);

			// Set the size
			Result->BufferSize += Downloaded;

			// Check for more data
		} while (BufferSize > 0);

		// Clean up
		if (hRequest)
			WinHttpCloseHandle(hRequest);
		if (hConnect)
			WinHttpCloseHandle(hConnect);

		// Return result
		return Result;
	}

	// Clean up
	if (hRequest)
		WinHttpCloseHandle(hRequest);
	if (hConnect)
		WinHttpCloseHandle(hConnect);

	// We failed to download data
	return nullptr;
}

bool WebClient::UploadFile(const std::string& Url, const std::string& FileName)
{
	// Connect to the HTTP server
	auto Uri = PreParseUrl(Url);

	// TODO: Finish uploading files
	return false;
}