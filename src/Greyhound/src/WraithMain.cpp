#include "pch.h"
#include "WraithMain.h"

#include "UIXButton.h"
#include "UIXTextBox.h"
#include "UIXListView.h"
#include "WraithSettings.h"
#include <version.h>
#include <CoDAssets.h>
#include <Texture.h>
#include <CoDXModelTranslator.h>
#include <GreyhoundTheme.h>

const uint64_t FNVPrime = 0x100000001B3;
const uint64_t FNVOffset = 0xCBF29CE484222325;

// Generates a 64bit FNV Hash for the given string
uint64_t FNVHash(std::string data)
{
	uint64_t Result = FNVOffset;

	for (uint32_t i = 0; i < data.length(); i++)
	{
		Result ^= data[i];
		Result *= FNVPrime;
	}

	return Result & 0xFFFFFFFFFFFFFFF;
}

WraithMain::WraithMain()
	: Forms::Form(), IsInExportMode(false)
{
	g_pWraithMain = this;
	this->InitializeComponent();
}

void WraithMain::InitializeComponent()
{
	this->SuspendLayout();
	this->SetAutoScaleDimensions({ 6, 13 });
	this->SetAutoScaleMode(Forms::AutoScaleMode::Font);
	this->SetText("Greyhound");
	this->SetClientSize({ 781, 510 });
	this->SetSize({ 781, 510 });
	this->SetMinimumSize({ 782, 510 });
	this->SetStartPosition(Forms::FormStartPosition::CenterScreen);

	this->LoadGameButton = new UIX::UIXButton();
	this->LoadGameButton->SetSize({ 86, 29 });
	this->LoadGameButton->SetLocation({ 11, 431 });
	this->LoadGameButton->SetTabIndex(2);
	this->LoadGameButton->SetText("Load Game");
	this->LoadGameButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left);
	this->LoadGameButton->Click += &OnLoadGameClick;
	this->AddControl(this->LoadGameButton);

	//Will randomly flash sometimes, unsure why, considering this was basically directly ported from normal Greyhound???
	this->GameCube = new WraithCube();
	this->GameCube->SetSize({ 32,32 });
	this->GameCube->SetTabIndex(11);
	this->GameCube->SetLocation({ 723, 6 });
	this->GameCube->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Right);
	this->AddControl(this->GameCube);

	this->ClearButton = new UIX::UIXButton();
	this->ClearButton->SetSize({ 77, 29 });
	this->ClearButton->SetLocation({ 398, 431 });
	this->ClearButton->SetTabIndex(9);
	this->ClearButton->SetText("Clear");
	this->ClearButton->SetEnabled(false);
	this->ClearButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left);
	this->ClearButton->Click += &OnClearAllClick;
	this->AddControl(this->ClearButton);

	this->ClearSearchButton = new UIX::UIXButton();
	this->ClearSearchButton->SetSize({ 72, 23 });
	this->ClearSearchButton->SetLocation({ 381, 11 });
	this->ClearSearchButton->SetTabIndex(8);
	this->ClearSearchButton->SetText("Clear");
	this->ClearSearchButton->SetEnabled(false);
	this->ClearSearchButton->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->ClearSearchButton->Click += &OnClearClick;
	this->AddControl(this->ClearSearchButton);

	this->StatusLabel = new UIX::UIXLabel();
	this->StatusLabel->SetSize({ 253, 23 });
	this->StatusLabel->SetLocation({ 460, 11 });
	this->StatusLabel->SetTabIndex(7);
	this->StatusLabel->SetText("Assets Loaded: 0");
	this->StatusLabel->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Right);
	this->StatusLabel->SetTextAlign(Drawing::ContentAlignment::MiddleRight);
	this->AddControl(this->StatusLabel);

	this->SearchButton = new UIX::UIXButton();
	this->SearchButton->SetSize({ 81, 23 });
	this->SearchButton->SetLocation({ 293, 11 });
	this->SearchButton->SetTabIndex(6);
	this->SearchButton->SetText("Search");
	this->SearchButton->SetEnabled(false);
	this->SearchButton->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->SearchButton->Click += &OnSearchClick;
	this->AddControl(this->SearchButton);

	this->SearchBox = new UIX::UIXTextBox();
	this->SearchBox->SetSize({ 275, 23 });
	this->SearchBox->SetLocation({ 11, 11 });
	this->SearchBox->SetTabIndex(5);
	this->SearchBox->SetEnabled(false);
	this->SearchBox->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Left);
	this->SearchBox->KeyPress += &OnSearchKeyPressed;
	this->AddControl(this->SearchBox);

	this->ExportAllButton = new UIX::UIXButton();
	this->ExportAllButton->SetSize({ 86, 29 });
	this->ExportAllButton->SetLocation({ 305, 431 });
	this->ExportAllButton->SetTabIndex(5);
	this->ExportAllButton->SetText("Export All");
	this->ExportAllButton->SetEnabled(false);
	this->ExportAllButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left);
	this->ExportAllButton->Click += &OnExpAllClick;
	this->AddControl(this->ExportAllButton);

	this->ExportSelectedButton = new UIX::UIXButton();
	this->ExportSelectedButton->SetSize({ 101, 29 });
	this->ExportSelectedButton->SetLocation({ 197, 431 });
	this->ExportSelectedButton->SetTabIndex(4);
	this->ExportSelectedButton->SetText("Export Selected");
	this->ExportSelectedButton->SetEnabled(false);
	this->ExportSelectedButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left);
	this->ExportSelectedButton->Click += &OnExpClick;
	this->AddControl(this->ExportSelectedButton);

	this->LoadFileButton = new UIX::UIXButton();
	this->LoadFileButton->SetSize({ 86, 29 });
	this->LoadFileButton->SetLocation({ 104, 431 });
	this->LoadFileButton->SetTabIndex(3);
	this->LoadFileButton->SetText("Load File");
	this->LoadFileButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left);
	this->LoadFileButton->Click += &OnLoadFileClick;
	this->AddControl(this->LoadFileButton);

	this->SettingsButton = new UIX::UIXButton();
	this->SettingsButton->SetSize({ 96, 29 });
	this->SettingsButton->SetLocation({ 658, 431 });
	this->SettingsButton->SetTabIndex(1);
	this->SettingsButton->SetText("Settings");
	this->SettingsButton->Click += &OnSettingsClick;
	this->SettingsButton->SetAnchor(Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Right);
	this->AddControl(this->SettingsButton);

	this->AssetsListView = new UIX::UIXListView();
	this->AssetsListView->SetSize({ 743, 382 });
	this->AssetsListView->SetLocation({ 11, 41 });
	this->AssetsListView->SetTabIndex(0);
	this->AssetsListView->SetAnchor(Forms::AnchorStyles::Top | Forms::AnchorStyles::Bottom | Forms::AnchorStyles::Left | Forms::AnchorStyles::Right);
	this->AssetsListView->SetView(Forms::View::Details);
	this->AssetsListView->Columns.Add({ "Asset Name", 280 });
	this->AssetsListView->Columns.Add({ "Status", 120 });
	this->AssetsListView->Columns.Add({ "Type", 100 });
	this->AssetsListView->Columns.Add({ "Details", 220 });
	this->AssetsListView->SetVirtualMode(true);
	this->AssetsListView->SetFullRowSelect(true);
	this->AssetsListView->VirtualItemsSelectionRangeChanged += &OnSelectedIndicesChanged;
	this->AssetsListView->DoubleClick += &OnListDoubleClick;
	this->AssetsListView->MouseClick += &OnListRightClick;
	this->AssetsListView->KeyUp += &OnListKeyUp;
	this->AssetsListView->KeyPress += &OnListKeyPressed;
	this->AddControl(this->AssetsListView);

	this->ResumeLayout(false);
	this->PerformLayout();


	this->AssetsListView->RetrieveVirtualItem += &GetVirtualItem;
	this->SetBackColor({ 37, 37, 37 });
}

void WraithMain::LoadGameFile(string& File)
{
	this->StatusLabel->SetText("Processing file...");

	Threading::Thread Th([File](void* Data)
		{
			WraithMain* Main = (WraithMain*)Data;
			Main->AssetsListView->SetVirtualListSize(0);
			//Prepare to load the file, and report back if need be
			auto LoadFileResult = CoDAssets::BeginGameFileMode(File.ToCString());
			//Check if we had success
			if (LoadFileResult == LoadGameFileResult::Success)
			{
				//Get size
				auto AssetsCount = (uint32_t)CoDAssets::GameAssets->LoadedAssets.size();
				Main->StatusLabel->SetText(string::Format("Assets Loaded: %d", AssetsCount));
				Main->RefreshView();
				Main->LoadGameButton->SetEnabled(false);
				Main->LoadFileButton->SetEnabled(false);
				Main->ExportSelectedButton->SetEnabled(true);
				Main->ExportAllButton->SetEnabled(true);
				Main->ClearButton->SetEnabled(true);
				Main->SearchButton->SetEnabled(true);
				Main->SearchBox->SetEnabled(true);
			}
			else if (LoadFileResult == LoadGameFileResult::InvalidFile)
			{
				Main->Invoke([]()
					{
						Forms::MessageBox::Show("The file you have provided was invalid.", "Greyhound", Forms::MessageBoxButtons::OK, Forms::MessageBoxIcon::Warning);
					});
				Main->StatusLabel->SetText("Assets Loaded: 0");
				Main->LoadGameButton->SetEnabled(true);
				Main->LoadFileButton->SetEnabled(true);
				Main->ExportSelectedButton->SetEnabled(false);
				Main->ExportAllButton->SetEnabled(false);
				Main->ClearButton->SetEnabled(false);
				Main->SearchButton->SetEnabled(false);
				Main->SearchBox->SetEnabled(false);
			}
			else if (LoadFileResult == LoadGameFileResult::UnknownError)
			{
				Main->Invoke([]()
					{
						Forms::MessageBox::Show("An unknown error has occured while loading the file.", "Greyhound", Forms::MessageBoxButtons::OK, Forms::MessageBoxIcon::Warning);
					});
				Main->StatusLabel->SetText("Assets Loaded: 0");
				Main->StatusLabel->SetText("Assets Loaded: 0");
				Main->LoadGameButton->SetEnabled(true);
				Main->LoadFileButton->SetEnabled(true);
				Main->ExportSelectedButton->SetEnabled(false);
				Main->ExportAllButton->SetEnabled(false);
				Main->ClearButton->SetEnabled(false);
				Main->SearchButton->SetEnabled(false);
				Main->SearchBox->SetEnabled(false);
			}
		});

	Th.Start(this);
}

void WraithMain::ExportSelectedAssets()
{
	// Prepare to export all available assets
	List<uint32_t> Selected = this->AssetsListView->SelectedIndices();

	List<CoDAsset_t*> AssetsToExport(Selected.Count(), true);

	for (uint32_t i = 0; i < AssetsToExport.Count(); i++)
	{
		uint32_t& DisplayIndex = this->DisplayIndices[Selected[i]];
		auto& AssetUse = CoDAssets::GameAssets->LoadedAssets[DisplayIndex];
		// Grab and set
		//auto& AssetUse = CoDAssets::GameAssets->LoadedAssets[AssetIndex];

		// Set
		AssetUse->AssetLoadedIndex = Selected[i];
		AssetsToExport[i] = AssetUse;
	}

	if (Selected.Count() == 0)
	{
		Forms::MessageBox::Show("Please select at least one asset to export.", "Greyhound", Forms::MessageBoxButtons::OK, Forms::MessageBoxIcon::Information);
		return;
	}

	this->ProgressWindow = std::make_unique<WraithProgress>();
	Threading::Thread([this, &AssetsToExport] {
		//Export All
		CoDAssets::ExportSelection(AssetsToExport, &WraithMain::ExportProgressCallback, &WraithMain::CheckStatusCallback, this);
	}).Start();
	this->ProgressWindow->ShowDialog();
	this->AssetsListView->Refresh();
}

void WraithMain::ExportAllAssets()
{
	if (this->DisplayIndices.Count() == 0)
	{
		Forms::MessageBox::Show("Please find some assets to export.", "Greyhound", Forms::MessageBoxButtons::OK, Forms::MessageBoxIcon::Information);
		return;
	}
	List<CoDAsset_t*> AssetsToExport(this->DisplayIndices.Count(), false);

	// An index for the element position
	uint32_t AssetPosition = 0;
	// Load up the assets, just passing the array
	for (auto& AssetIndex : this->DisplayIndices)
	{
		// Grab and set
		auto& AssetUse = CoDAssets::GameAssets->LoadedAssets[AssetIndex];
		// Set
		AssetUse->AssetLoadedIndex = AssetIndex;
		AssetsToExport.Add(AssetUse);
	}
	this->ProgressWindow = std::make_unique<WraithProgress>();

	Threading::Thread([this, &AssetsToExport] {
		CoDAssets::ExportAllAssets(AssetsToExport, &WraithMain::ExportProgressCallback, &WraithMain::CheckStatusCallback, this);
	}).Start();

	this->ProgressWindow->ShowDialog();
	this->AssetsListView->Refresh();
}

void WraithMain::SearchForAssets()
{
	if (CoDAssets::GameAssets == nullptr)
		return;

	this->ClearSearchButton->SetEnabled(true);

	string SearchText = this->SearchBox->Text().ToLower();

	if (string::IsNullOrEmpty(SearchText))
	{
		this->ResetDisplayIndices();
		return;
	}

	bool isBlackList = SearchText.StartsWith("!");
	if (isBlackList)
		SearchText = SearchText.Substring(1);

	List<string> SearchMap = SearchText.Split(",");

	for (auto& Search : SearchMap)
		Search = Search.Trim();

	List<uint32_t> SearchResults;
	uint32_t CurrentIndex = 0;

	for (auto& Asset : CoDAssets::GameAssets->LoadedAssets)
	{
		string AssetNameLowercase = string(Asset->AssetName).ToLower();
		bool IsMatch = isBlackList;

		for (auto& Search : SearchMap)
		{
			bool Result = AssetNameLowercase.Contains(Search);

			if (!isBlackList && Result)
			{
				IsMatch = true;
				break;
			}
			else if (isBlackList && Result)
			{
				IsMatch = false;
				break;
			}
			// Second pass for Bo4, hash
			if (CoDAssets::GameID == SupportedGames::BlackOps4 || CoDAssets::GameID == SupportedGames::BlackOpsCW)
			{
				// Convert to hex string
				std::stringstream HashValue;
				HashValue << std::hex << FNVHash(Search.ToCString()) << std::dec;

				// If we match, add, then stop
				Result = string(Asset->AssetName.c_str()).Contains(HashValue.str().c_str());

				// Check match type
				if (!isBlackList && Result)
				{
					IsMatch = true;
					break;
				}
				else if (isBlackList && Result)
				{
					IsMatch = false;
					break;
				}
			}
		}

		if (IsMatch)
			SearchResults.Add(CurrentIndex);

		CurrentIndex++;
	}

	this->AssetsListView->SetVirtualListSize(0);

	this->DisplayIndices = std::move(SearchResults);

	this->AssetsListView->SetVirtualListSize(this->DisplayIndices.Count());
	this->AssetsListView->Refresh();

	this->StatusLabel->SetText(string::Format("Found %d assets", this->DisplayIndices.Count()));
}

void WraithMain::ExportSingleAsset()
{
	if (this->IsInExportMode)
		return;

	this->IsInExportMode = true;

	if (CoDAssets::GameAssets == nullptr)
		return;

	List<uint32_t> SelectedIndices = this->AssetsListView->SelectedIndices();

	if (SelectedIndices.Count() == 0)
		return;

	List<CoDAsset_t*> AssetsToExport(1, false);

	uint32_t& DisplayIndex = this->DisplayIndices[SelectedIndices[0]];
	auto& Asset = CoDAssets::GameAssets->LoadedAssets[DisplayIndex];
	auto i = std::vector<uint32_t>();

	i.push_back(DisplayIndex);

	Asset->AssetStatus = WraithAssetStatus::Processing;
	this->AssetsListView->Refresh();

	AssetsToExport[0] = Asset;

	this->ProgressWindow = nullptr;

	Threading::Thread([this, &AssetsToExport] {
		CoDAssets::ExportSelection(AssetsToExport, &WraithMain::ExportProgressCallback, &WraithMain::CheckStatusCallback, this);
	}).Start();
}

void WraithMain::ExportProgressChanged(uint32_t Progress, bool Finished)
{
	if (Finished)
		this->IsInExportMode = false;

	if (this->ProgressWindow == nullptr)
	{
		if (Finished)
			this->AssetsListView->Refresh();

		return;
	}

	this->ProgressWindow->UpdateProgress(Progress, Finished);
}

bool WraithMain::CheckStatus(int32_t AssetIndex)
{
	if (AssetIndex < 0)
		return (this->ProgressWindow != nullptr) ? this->ProgressWindow->IsCanceled() : false;

	CoDAssets::GameAssets->LoadedAssets[this->DisplayIndices[AssetIndex]]->AssetStatus = WraithAssetStatus::Exported;

	return (this->ProgressWindow != nullptr) ? this->ProgressWindow->IsCanceled() : false;
}

void WraithMain::SetAssetError(int32_t AssetIndex)
{
	CoDAssets::GameAssets->LoadedAssets[this->DisplayIndices[AssetIndex]]->AssetStatus = WraithAssetStatus::Error;
}

void WraithMain::DoPreviewSwap()
{
	if (!this->PreviewWindow || this->PreviewWindow->GetHandle() == nullptr)
		return;

	List<uint32_t> Selected = this->AssetsListView->SelectedIndices();

	if (Selected.Count() <= 0)
		return;

	if (CoDAssets::GameAssets == nullptr)
		return;

	auto& Asset = CoDAssets::GameAssets->LoadedAssets[this->DisplayIndices[Selected[0]]];

	if (Asset->AssetStatus != WraithAssetStatus::Loaded && Asset->AssetStatus != WraithAssetStatus::Exported && Asset->AssetStatus != WraithAssetStatus::Processing)
		return;

	switch (Asset->AssetType)
	{
	case WraithAssetType::Model:
	{
		//Building preview Asset::Model directly (as opposed to converting a WraithModel to Asset::Model) 
		//(is around 33% quicker - literally every single time)
		//Generating an Asset::Model has broken verticies in some games (looking at you cold war)
		//For some reason some verts have a disgustingly warped Y value
		//Using GetModelForPreview (Wraith model) on the ray rifle view model took ~205ms
		//To be completely honest, I couldn't really be bothered to spend hours rewriting existing glsl shader code
		//Otherwise using a WraithModel is completely reasonable.
		//Using BuildPreviewModel(full convert wraithmodel to Assets::Model) was consistently 880ms
		//Using BuildPreviewMdl (uses wraithmodel 'proxy' for vertex data (grr hate you cold war)) is was around 500ms
		auto t1 = std::chrono::high_resolution_clock::now();
		auto Mdl = CoDAssets::BuildPreviewMdl((CoDModel_t*)Asset);
		auto t2 = std::chrono::high_resolution_clock::now();
		auto ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		uint64_t time = ms_int.count();

		if (Mdl == nullptr)
			return;
		this->PreviewWindow->AssignPreviewModel(*Mdl.get(), string(Asset->AssetName), time);
	}
	break;
	case WraithAssetType::Image:
	{
		auto Texture = CoDAssets::BuildPreviewTexture((CoDImage_t*)Asset);
		if (Texture == nullptr)
			return;
		this->PreviewWindow->AssignPreviewImage(*Texture.get(), Asset->AssetName.c_str(), 0);
	}
	break;
	case WraithAssetType::Material:
	{
		auto Material = CoDAssets::BuildPreviewMaterial((CoDMaterial_t*)Asset);
		if (Material == nullptr)
			return;
		this->PreviewWindow->AssignPreviewImage(*Material.get(), Asset->AssetName.c_str(), 0);
	}
	break;
	}

}

std::unique_ptr<Assets::Texture> WraithMain::MaterialStreamCallback(Assets::MaterialSlotType type, uint64_t img)
{
	if (CoDAssets::GameAssets == nullptr)
		return nullptr;

	return CoDAssets::BuildPreviewTexture(type, img);
}

void WraithMain::RefreshView()
{
	string SearchText = this->SearchBox->Text();

	if (CoDAssets::GameAssets != nullptr)
	{
		this->AssetsListView->SetVirtualListSize(0);
		this->ResetDisplayIndices();
	}

	// restore the search box's text after refreshing
	// yes this makes the window flash when refreshing as it performs the search
	if (SearchText.Length() > 0)
	{
		this->SearchBox->SetText(SearchText);

		this->SearchForAssets();
	}
}

void WraithMain::ResetDisplayIndices()
{
	this->AssetsListView->SetVirtualListSize(0);

	this->SearchBox->SetText("");
	this->ClearSearchButton->SetEnabled(false);

	if (CoDAssets::GameAssets == nullptr)
		return;

	List<uint32_t> TempIndices(CoDAssets::GameAssets->LoadedAssets.size(), true);
	for (uint32_t i = 0; i < CoDAssets::GameAssets->LoadedAssets.size(); i++)
		TempIndices[i] = i;

	this->DisplayIndices = std::move(TempIndices);

	this->AssetsListView->SetVirtualListSize(this->DisplayIndices.Count());
	this->AssetsListView->Refresh();
	auto AssetsCount = (uint32_t)CoDAssets::GameAssets->LoadedAssets.size();
	this->StatusLabel->SetText(string::Format("Assets Loaded: %d", AssetsCount));
}

void WraithMain::OnLoadFileClick(Forms::Control* Sender)
{
	WraithMain* ThisPtr = (WraithMain*)Sender->FindForm();

	// Prepare to load a file, first, ask for one
	auto OpenFileD = OpenFileDialog::ShowFileDialog("Select a game file to load", "", "All files (*.*)|*.*;|Image Package Files (*.iwd, *.ipak, *.xpak)|*.iwd;*.ipak;*.xpak|Sound Package Files (*.sabs, *.sabl)|*.sabs;*.sabl;", Sender->FindForm());
	ThisPtr->LoadGameFile(OpenFileD);
}

void WraithMain::OnSettingsClick(Forms::Control* Sender)
{
	auto Settings = std::make_unique<WraithSettings>();
	Settings->ShowDialog((Forms::Form*)Sender->FindForm());
}

void WraithMain::OnExpClick(Forms::Control* Sender)
{
	((WraithMain*)Sender->FindForm())->ExportSelectedAssets();
}

void WraithMain::OnExpAllClick(Forms::Control* Sender)
{
	((WraithMain*)Sender->FindForm())->ExportAllAssets();
}

void WraithMain::OnSearchClick(Forms::Control* Sender)
{
	((WraithMain*)Sender->FindForm())->SearchForAssets();
}

void WraithMain::OnClearClick(Forms::Control* Sender)
{
	((WraithMain*)Sender->FindForm())->ResetDisplayIndices();
}

void WraithMain::OnLoadGameClick(Forms::Control* Sender)
{
	WraithMain* ThisPtr = (WraithMain*)Sender->FindForm();
	ThisPtr->LoadGameButton->SetEnabled(false);
	ThisPtr->LoadFileButton->SetEnabled(false);
	ThisPtr->ExportSelectedButton->SetEnabled(false);
	ThisPtr->ExportAllButton->SetEnabled(false);
	ThisPtr->ClearButton->SetEnabled(false);
	ThisPtr->SearchButton->SetEnabled(false);
	ThisPtr->SearchBox->SetEnabled(false);
	ThisPtr->LoadGameAsync();
}

void WraithMain::LoadGameAsync()
{
	Threading::Thread Th([](void* Data)
		{
			WraithMain* Main = (WraithMain*)Data;

			auto LoadGameResult = CoDAssets::BeginGameMode();

			// Check if we had success
			if (LoadGameResult == FindGameResult::Success)
			{
				// Get size
				auto AssetsCount = (uint32_t)CoDAssets::GameAssets->LoadedAssets.size();
				Main->StatusLabel->SetText(string::Format("Assets Loaded: %d", AssetsCount));
				Main->RefreshView();
				Main->LoadGameButton->SetEnabled(true);
				Main->LoadFileButton->SetEnabled(false);
				Main->ExportSelectedButton->SetEnabled(true);
				Main->ExportAllButton->SetEnabled(true);
				Main->ClearButton->SetEnabled(true);
				Main->SearchButton->SetEnabled(true);
				Main->SearchBox->SetEnabled(true);
				Main->Invoke([]() 
				{
					UpdateCubeIcon();
				});
			}
			else if (LoadGameResult == FindGameResult::NoGamesRunning)
			{
				Main->Invoke([]()
				{
					Forms::MessageBox::Show("No instances of any supported game were found. Please make sure the game is running first.", "Greyhound", Forms::MessageBoxButtons::OK, Forms::MessageBoxIcon::Warning);
				});
				Main->StatusLabel->SetText("Assets Loaded: 0");
				Main->LoadGameButton->SetEnabled(true);
				Main->LoadFileButton->SetEnabled(true);
				Main->ExportSelectedButton->SetEnabled(false);
				Main->ExportAllButton->SetEnabled(false);
				Main->ClearButton->SetEnabled(false);
				Main->SearchButton->SetEnabled(false);
				Main->SearchBox->SetEnabled(false);
			}
			else if (LoadGameResult == FindGameResult::FailedToLocateInfo)
			{
				Main->Invoke([]()
					{
						Forms::MessageBox::Show("This game is supported, but the current update is not. Please wait for an upcoming patch for support.", "Greyhound", Forms::MessageBoxButtons::OK, Forms::MessageBoxIcon::Warning);
					});
				Main->StatusLabel->SetText("Assets Loaded: 0");
				Main->LoadGameButton->SetEnabled(true);
				Main->LoadFileButton->SetEnabled(true);
				Main->ExportSelectedButton->SetEnabled(false);
				Main->ExportAllButton->SetEnabled(false);
				Main->ClearButton->SetEnabled(false);
				Main->SearchButton->SetEnabled(false);
				Main->SearchBox->SetEnabled(false);
			}
		});
	Th.Start(this);
}

void WraithMain::OnClearAllClick(Forms::Control* Sender)
{
	WraithMain* ThisPtr = (WraithMain*)Sender->FindForm();
	ThisPtr->LoadGameButton->SetEnabled(true);
	ThisPtr->LoadFileButton->SetEnabled(true);
	ThisPtr->ExportSelectedButton->SetEnabled(false);
	ThisPtr->ExportAllButton->SetEnabled(false);
	ThisPtr->ClearButton->SetEnabled(false);
	ThisPtr->SearchButton->SetEnabled(false);
	ThisPtr->SearchBox->SetEnabled(false);
	ThisPtr->OnClearAll();
}

void WraithMain::OnClearAll()
{
	//this->AssetsListView->SetVirtualListSize(0);
	Threading::Thread Th([](void* Data)
		{
			WraithMain* Main = (WraithMain*)Data;

			// Tell the assets pool to clean up everything
			CoDAssets::CleanUpGame();
			Main->ResetDisplayIndices();
			Main->StatusLabel->SetText("Assets Loaded: 0");
			Main->Invoke([]()
				{
					UpdateCubeIcon();
				});
		});
	Th.Start(this);
}

void WraithMain::OnListRightClick(const std::unique_ptr<MouseEventArgs>& EventArgs, Forms::Control* Sender)
{
	if (EventArgs->Button != Forms::MouseButtons::Right)
		return;

	WraithMain* ThisPtr = ((WraithMain*)Sender->FindForm());
	UIX::UIXListView* AssetsListView = ThisPtr->AssetsListView;

	List<uint32_t> SelectedIndices = AssetsListView->SelectedIndices();
	string endString = "";

	g_Logger.Info("selected asset names:\n");
	for (uint32_t i = 0; i < SelectedIndices.Count(); i++)
	{
		uint32_t& DisplayIndex = ThisPtr->DisplayIndices[SelectedIndices[i]];
		auto& Asset = CoDAssets::GameAssets->LoadedAssets[DisplayIndex];

		g_Logger.Info(Asset->AssetName + "\n");

		if (i != SelectedIndices.Count() - 1)
			endString += Asset->AssetName + "\n";
		else
			endString += Asset->AssetName;
	}

	clip::set_text(endString.ToCString());

	g_Logger.Info("copying %i asset name%s to clipboard\n", SelectedIndices.Count(), SelectedIndices.Count() == 1 ? "" : "s");
}

void WraithMain::OnListDoubleClick(Forms::Control* Sender)
{
	((WraithMain*)Sender->FindForm())->ExportSingleAsset();
}

void WraithMain::OnListKeyUp(const std::unique_ptr<KeyEventArgs>& EventArgs, Forms::Control* Sender)
{
	if (EventArgs->KeyCode() == Keys::E)
	{
		((WraithMain*)Sender->FindForm())->ExportSingleAsset();
	}
	else if (EventArgs->KeyCode() == Keys::P)
	{
		WraithMain* Form = (WraithMain*)Sender->FindForm();

		if (CoDAssets::GameAssets == nullptr)
			return;

		List<uint32_t> Selected = Form->AssetsListView->SelectedIndices();

		if (Selected.Count() <= 0)
			return;

		auto& Asset = CoDAssets::GameAssets->LoadedAssets[Form->DisplayIndices[Selected[0]]];

		switch (Asset->AssetType) {
		default:
		{
			if (Form->PreviewWindow == nullptr || Form->PreviewWindow->GetState(Forms::ControlStates::StateDisposed))
			{
				Form->PreviewWindow = std::make_unique<WraithPreview>();
				Form->PreviewWindow->SetMaterialStreamer([Form](Assets::MaterialSlotType type, uint64_t img)
				{
					return Form->MaterialStreamCallback(type, img);
				});
				Form->PreviewWindow->Show();

			}

			Form->PreviewWindow->BringToFront();
			Form->DoPreviewSwap();
			break;
		}
		}
	}
}

void WraithMain::OnListKeyPressed(const std::unique_ptr<KeyPressEventArgs>& EventArgs, Forms::Control* Sender)
{
	EventArgs->SetHandled(true);
}

void WraithMain::OnSearchKeyPressed(const std::unique_ptr<KeyPressEventArgs>& EventArgs, Forms::Control* Sender)
{
	if (EventArgs->KeyChar == '\r')
	{
		EventArgs->SetHandled(true);
		((WraithMain*)Sender->FindForm())->SearchForAssets();
	}
}

void WraithMain::OnSelectedIndicesChanged(const std::unique_ptr<Forms::ListViewVirtualItemsSelectionRangeChangedEventArgs>& EventArgs, Forms::Control* Sender)
{
	((WraithMain*)Sender->FindForm())->DoPreviewSwap();
}

void WraithMain::GetVirtualItem(const std::unique_ptr<Forms::RetrieveVirtualItemEventArgs>& EventArgs, Forms::Control* Sender)
{
	WraithMain* ThisPtr = (WraithMain*)Sender->FindForm();

	if (CoDAssets::GameAssets == nullptr)
		return;
	if (EventArgs->ItemIndex > (int32_t)ThisPtr->DisplayIndices.Count())
		return;

	uint32_t RemappedDisplayIndex = ThisPtr->DisplayIndices[EventArgs->ItemIndex];

	static const char* AssetTypes[] = { "Animation", "Image", "Model", "Sound", "Effect", "RawFile", "Material" };
	static const Drawing::Color AssetTypesColors[] =
	{
		Drawing::Color(0, 157, 220),  // Animation
		Drawing::Color(219, 80, 74),  // Image
		Drawing::Color(220, 75, 109), // Model
		Drawing::Color(202, 97, 195), // Sound
		Drawing::Color(27, 153, 139), // Effect
		Drawing::Color(211, 7, 247),  // RawFile
		Drawing::Color(216, 30, 91),  // Material,
	};

	static const char* AssetStatus[] = { "Loaded", "Exported", "NotLoaded", "Placeholder", "Processing", "Error"};
	static const Drawing::Color AssetStatusColors[] =
	{
		Drawing::Color(30, 144, 255), //Loaded
		Drawing::Color(50, 205, 50), //Exported
		Drawing::Color(33,  184, 235), //Not Loaded
		Drawing::Color(255,   165, 0), //Placeholder?
		Drawing::Color(255, 255, 255), //Processing
		Drawing::Color(255, 102, 102), //Error
	};

	auto& AssetList = CoDAssets::GameAssets->LoadedAssets;
	auto& Asset = AssetList[RemappedDisplayIndex];

	EventArgs->Style.ForeColor = Drawing::Color::White;
	EventArgs->Style.BackColor = Sender->BackColor();

	string infoText;

	switch (EventArgs->SubItemIndex)
	{
	case 0:
		EventArgs->Text = Asset->AssetName;
		break;
	case 1:
		EventArgs->Text = AssetStatus[(uint32_t)Asset->AssetStatus];
		EventArgs->Style.ForeColor = AssetStatusColors[(uint32_t)Asset->AssetStatus];
		break;
	case 2:
		EventArgs->Text = AssetTypes[(uint32_t)Asset->AssetType];
		//EventArgs->Style.ForeColor = AssetTypesColors[(uint32_t)Asset->AssetType];
		break;
	case 3:
		// Check type and format it
		switch (Asset->AssetType)
		{
		case WraithAssetType::Model:
			// Model info
			if (((CoDModel_t*)Asset)->CosmeticBoneCount == 0)
				infoText = string::Format("Bones: %d, LODs: %d", ((CoDModel_t*)Asset)->BoneCount, ((CoDModel_t*)Asset)->LodCount);
			else
				infoText = string::Format("Bones: %d, Cosmetics: %d, LODs: %d", ((CoDModel_t*)Asset)->BoneCount, ((CoDModel_t*)Asset)->CosmeticBoneCount, ((CoDModel_t*)Asset)->LodCount);
			break;
		case WraithAssetType::Animation:
			// Anim info
			infoText = string::Format("Framerate: %.2f, Frames: %d, Bones: %d", ((CoDAnim_t*)Asset)->Framerate, ((CoDAnim_t*)Asset)->FrameCount, ((CoDAnim_t*)Asset)->BoneCount);
			break;
		case WraithAssetType::Image:
			// Validate info (Some image resources may not have information available)
			if (((CoDImage_t*)Asset)->Width > 0)
			{
				// Image info
				infoText = string::Format("Width: %d, Height: %d", ((CoDImage_t*)Asset)->Width, ((CoDImage_t*)Asset)->Height);
			}
			else
			{
				// Image info not available
				infoText = "N/A";
			}
			break;
		case WraithAssetType::Sound:
		{
			// Validate info (Some image resources may not have information available)
			if (((CoDSound_t*)Asset)->Length > 0)
			{
				// Sound info
				auto Time = string::DurationToReadableTime(std::chrono::milliseconds(((CoDSound_t*)Asset)->Length));
				// Formatted time
				infoText = string::Format(Time);
			}
			else
			{
				// Sound info not available
				infoText = "N/A";
			}
			break;
		}
		case WraithAssetType::Material:
			// Rawfile info
			infoText = string::Format("Images: %llu", ((CoDMaterial_t*)Asset)->ImageCount);
			break;
		case WraithAssetType::RawFile:
			// Rawfile info
			infoText = string::Format("Size: 0x%llx", Asset->AssetSize);
			break;
		}
		EventArgs->Text = infoText;
		break;
	}
}

void WraithMain::ExportProgressCallback(uint32_t Progress, Forms::Form* MainForm, bool Finished)
{
	((WraithMain*)MainForm)->ExportProgressChanged(Progress, Finished);
}

bool WraithMain::CheckStatusCallback(int32_t AssetIndex, Forms::Form* MainForm)
{
	return ((WraithMain*)MainForm)->CheckStatus(AssetIndex);
}

void WraithMain::UpdateCubeIcon()
{
	// Update it to the game, or the default wraith icon
	if (CoDAssets::GameInstance != nullptr && CoDAssets::GameInstance->IsRunning())
	{
		// Extract the icon, cancel on failure
		auto GameIcon = IO::File::ExtractFileIcon(CoDAssets::GameInstance->GetProcessPath());
		// Load if not null
		if (GameIcon != NULL)
		{
			// Load into cube, then clean up
			g_pWraithMain->GameCube->LoadCubeIcon(GameIcon);
			DestroyIcon(GameIcon);
			return;
		}
	}
	auto GameIcon = IO::File::ExtractFileIcon(std::string(System::Environment::GetApplication()));
	if (GameIcon != NULL)
	{
		// Load into cube, then clean up
		g_pWraithMain->GameCube->LoadCubeIcon(GameIcon);
		DestroyIcon(GameIcon);
	}
}

WraithMain* g_pWraithMain;
