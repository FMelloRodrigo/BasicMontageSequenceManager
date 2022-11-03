// Copyright 2022 - Rodrigo Mello

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BMSM_Enums.h"
#include "BMSM_AnimationEvents.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBMSM_AnimationEvents : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BASICMONTAGESEQUENCEMANAGER_API IBMSM_AnimationEvents
{
	GENERATED_BODY()



	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Events")
	void UpdateMontageSlot(EMontageSlotTypes Slot);
};
