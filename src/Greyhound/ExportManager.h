#pragma once

#include "Form.h"
#include "ListBase.h"
#include "ExportAsset.h"
#include "Settings.h"

typedef void (ExportProgressCallback)(uint32_t Progress, Forms::Form* MainForm, bool Finished);
typedef bool (CheckStatusCallback)(int32_t AssetIndex, Forms::Form* MainForm);

// Handles exporting assets from the various filesystems...
class ExportManager
{
public:

	// Our settings object
	static System::Settings Config;
	// Our application path
	static string ApplicationPath;
	// Our export path
	static string ExportPath;

	// Initializes the exporter (Load settings / paths)
	static void InitializeExporter();
	// Saves the current memory settings to disk
	static void SaveConfigToDisk();

	// The path used to export BSP models
	static string GetMapExportPath();
};