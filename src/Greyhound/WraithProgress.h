#pragma once

#include <Kore.h>
#include "UIXButton.h"
#include "UIXLabel.h"
#include "UIXProgressBar.h"

class WraithProgress : public Forms::Form
{
public:
	WraithProgress();
	virtual ~WraithProgress() = default;

	// Updates the progress
	void UpdateProgress(uint32_t Progress, bool Finished);
	// Gets whether or not we canceled it
	bool IsCanceled();
	// Sets whether or not to automatically close
	void SetAutoClose(bool Value);

private:
	// Internal routine to setup the component
	void InitializeComponent();

	// Internal event on finish click
	static void OnFinishClick(Forms::Control* Sender);
	static void OnCancelClick(Forms::Control* Sender);

	// Cancels the progress
	void CancelProgress();

	// Internal controls reference
	UIX::UIXLabel* ExportLabel;
	UIX::UIXButton* CancelButton;
	UIX::UIXButton* FinishButton;
	UIX::UIXProgressBar* ExportProgressBar;

	// Whether or not we can close
	bool CanClose;
	bool Canceled;
	bool AutoClose;
};
