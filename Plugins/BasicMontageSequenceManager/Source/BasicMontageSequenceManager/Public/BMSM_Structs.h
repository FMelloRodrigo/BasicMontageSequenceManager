// Copyright 2022 - Rodrigo Mello
#pragma once

#include "Animation/AnimMontage.h"
#include "BMSM_Enums.h"
#include "Engine/DataTable.h"
#include "BMSM_Structs.generated.h"

USTRUCT(BlueprintType)
struct  FBMSM_S_Montages
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	UAnimMontage* MontageRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	float PlayRate{1};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	EMontageSlotTypes Slot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	TArray<FName> SectionJumps;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	bool LockMovement;

};

USTRUCT(BlueprintType)
struct FBMSM_S_AttacksMain
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	FName AttackName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Struct Variables")
	FBMSM_S_Montages MontageConfig;

};

USTRUCT(BlueprintType)
struct FBMSM_S_CombosMain : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct Variables")
	FName ComboName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct Variables")
	TArray<FBMSM_S_AttacksMain> Combo;


};

