// Copyright 2022 - Rodrigo Mello

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BMSM_Structs.h"
#include "BMSM_Enums.h"
#include "BMSM_MontagesComponent.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBMSM_MontagesComponent : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BASICMONTAGESEQUENCEMANAGER_API IBMSM_MontagesComponent
{
	GENERATED_BODY()



	
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interface Events")
	void I_Attack(FName Name);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interface Events")
	void I_MontageEnd();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interface Events")
	void I_ComboCancel(float InTime);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interface Events")
	void I_PlayMontageByStruct(FBMSM_S_Montages Montage);

};
