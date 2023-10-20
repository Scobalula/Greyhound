#pragma once
// We need the following WraithX classes
#include <WraithModel.h>

// We need the cod assets
#include "CoDAssets.h"

// We need the following std libs
#include <memory>

class CoDXModelBonesHelper
{
public:
	// Translates an in-game XModel to a WraithModel
	static void ReadXModelBones(const std::unique_ptr<XModel_t>& Model, const XModelLod_t& ModelLOD, const std::unique_ptr<WraithModel>& ResultModel);
};