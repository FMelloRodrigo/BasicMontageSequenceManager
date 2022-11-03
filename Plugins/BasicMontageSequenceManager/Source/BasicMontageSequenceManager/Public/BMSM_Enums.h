// Copyright 2022 - Rodrigo Mello
#pragma once

UENUM(BlueprintType)
enum class EMontageSlotTypes : uint8
{
	EMST_Default UMETA(DisplayName = "Default"),
	EMST_UpperBody UMETA(DisplayName = "UpperBody"),
	EMST_LowerBody UMETA(DisplayName = "LowerBody"),

	EMST_MAX UMETA(DisplayName = "DefaultMAX")
};