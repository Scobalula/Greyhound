#pragma once

#include <Kore.h>

#include "UIXButton.h"
#include "UIXTextBox.h"
#include "UIXLabel.h"
#include "UIXListView.h"

#include "ExportAsset.h"
#include "ExportManager.h"
#include "WraithPreview.h"
#include "WraithProgress.h"
#include <WraithCube.h>

class WraithMain : public Forms::Form
{
public:
	WraithMain();
	virtual ~WraithMain() = default;

	// Reloads the view
	void RefreshView();
	// Resets display indices
	void ResetDisplayIndices();
	// Internal routine to load a file
	void LoadGameFile(string& File);
	void LoadGameAsync();
	void OnClearAll();

	void SetAssetError(int32_t AssetIndex);
protected:
	static void OnLoadFileClick(Forms::Control* Sender);
	static void OnSettingsClick(Forms::Control* Sender);
	static void OnExpClick(Forms::Control* Sender);
	static void OnExpAllClick(Forms::Control* Sender);
	static void OnSearchClick(Forms::Control* Sender);
	static void OnClearClick(Forms::Control* Sender);
	static void OnLoadGameClick(Forms::Control* Sender);
	static void OnClearAllClick(Forms::Control* Sender);
	static void OnListRightClick(const std::unique_ptr<MouseEventArgs>& EventArgs, Forms::Control* Sender);
	static void OnListDoubleClick(Forms::Control* Sender);
	static void OnListKeyUp(const std::unique_ptr<KeyEventArgs>& EventArgs, Forms::Control* Sender);
	static void OnListKeyPressed(const std::unique_ptr<KeyPressEventArgs>& EventArgs, Forms::Control* Sender);
	static void OnSearchKeyPressed(const std::unique_ptr<KeyPressEventArgs>& EventArgs, Forms::Control* Sender);
	static void OnSelectedIndicesChanged(const std::unique_ptr<Forms::ListViewVirtualItemsSelectionRangeChangedEventArgs>& EventArgs, Forms::Control* Sender);
	static void GetVirtualItem(const std::unique_ptr<Forms::RetrieveVirtualItemEventArgs>& EventArgs, Forms::Control* Sender);

	// A trampoline for the callback, then, invoke normal method...
	static void ExportProgressCallback(uint32_t Progress, Forms::Form* MainForm, bool Finished);
	static bool CheckStatusCallback(int32_t AssetIndex, Forms::Form* MainForm);

	static void UpdateCubeIcon();

private:
	// Internal routine to setup the component
	void InitializeComponent();
	// Internal routine to export selected assets
	void ExportSelectedAssets();
	// Internal routine to export all assets
	void ExportAllAssets();
	// Internal routine to search for assets
	void SearchForAssets();
	// Internal routine to export a single asset
	void ExportSingleAsset();

	// A lock for exporting
	bool IsInExportMode;

	// Internal routine to change the progress value
	void ExportProgressChanged(uint32_t Progress, bool Finished);
	bool CheckStatus(int32_t AssetIndex);
	void DoPreviewSwap();

	std::unique_ptr<Assets::Texture> MaterialStreamCallback(Assets::MaterialSlotType type, uint64_t img);

	// Internal controls reference
	UIX::UIXButton* ClearSearchButton;
	UIX::UIXLabel* StatusLabel;
	UIX::UIXButton* SearchButton;
	UIX::UIXTextBox* SearchBox;
	UIX::UIXButton* ExportAllButton;
	UIX::UIXButton* ExportSelectedButton;
	UIX::UIXButton* LoadFileButton;
	UIX::UIXButton* SettingsButton;
	UIX::UIXButton* ClearButton;
	UIX::UIXButton* LoadGameButton;
	UIX::UIXListView* AssetsListView;
	WraithCube* GameCube;

	// Converted assets for the list...
	//std::unique_ptr<List<WraithAsset>> LoadedAssets;
	
	// List of display indices for the list...
	List<uint32_t> DisplayIndices;

	// Preview window
	std::unique_ptr<WraithPreview> PreviewWindow;
	// Progress window
	std::unique_ptr<WraithProgress> ProgressWindow;

	// Internal load path
	List<string> LoadPath;
};

extern WraithMain* g_pWraithMain;