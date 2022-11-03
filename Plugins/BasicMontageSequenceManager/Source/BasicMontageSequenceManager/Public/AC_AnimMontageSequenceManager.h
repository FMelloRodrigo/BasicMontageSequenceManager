// Copyright 2022 - Rodrigo Mello

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "BMSM_Enums.h"
#include "BMSM_Structs.h"
#include "Engine/DataTable.h"
#include "BMSM_MontagesComponent.h"
#include "AC_AnimMontageSequenceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboCancel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackStarted, FBMSM_S_Montages, Montage);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BASICMONTAGESEQUENCEMANAGER_API UAC_AnimMontageSequenceManager : public UActorComponent, public IBMSM_MontagesComponent
{
	GENERATED_BODY()

public:	
	
	UAC_AnimMontageSequenceManager();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY (EditDefaultsOnly, Category = "Options")
	float MaxComboInterval;

	UPROPERTY(EditDefaultsOnly, Category = "Options")
	bool ResetComboOnChange;

	UPROPERTY(EditDefaultsOnly, Category = "Options")
	class UDataTable* CombosDataTable;

protected:
	
	virtual void BeginPlay() override;

public:	

	// Interface Functions
	virtual void I_Attack_Implementation(FName Name) override;
	virtual void I_ComboCancel_Implementation(float InTime) override;
	virtual void I_PlayMontageByStruct_Implementation(FBMSM_S_Montages Montage) override;
	virtual void I_MontageEnd_Implementation() override;

private:
	
	ACharacter* Character;
	
	UAnimInstance* AnimInstance;
	
	float DefaultWalkSpeed;

	UPROPERTY(Replicated)
	bool MidAttack;

	UPROPERTY(ReplicatedUsing = OnRep_MovementLocked)
	bool MovementLocked;

	UPROPERTY(Replicated)
	bool PlayingMontage;

	int32 CurrentAttackIndex;

	UPROPERTY(Replicated)
	int32 CurrentSectionIndex;

	FBMSM_S_AttacksMain CurrentAttack;

	FBMSM_S_CombosMain CurrentCombo;

	FTimerHandle ComboEndTimer;

	TMap <FName, FBMSM_S_CombosMain> ComboMap;


	// Functions

	void ComboMapBeginPlay();

	void Attack(FName ComboName);
	
	void MontageVerification();

	void AttackEnd();

	void ComboEndTimerEvent();

	void IncreaseAttackIndex();

	void AttackMontageEnd();

	void ComboCancelRecoveryEvent();

	void UpdateMovementLocked(bool Value);

	bool ComboHasChanged(FName ComboName);

	UFUNCTION(Server, Reliable)
	void SERVER_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(NetMulticast, Reliable)
	void MULTI_OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnRep_MovementLocked();

	UFUNCTION(Client, Reliable)
	void CLIENT_ComboCancel(float InTime);

	UFUNCTION(Server, Reliable)
	void SERVER_MontageStop(float InTime);

	UFUNCTION(NetMulticast, Reliable)
	void MULTI_MontageStop(float InTime);

	UFUNCTION(Client, Reliable)
	void CLIENT_PlayMontage(FBMSM_S_Montages MontageStruct);

	UFUNCTION(Server, Reliable)
	void SERVER_PlayMontage(FBMSM_S_Montages MontageStruct);

	UFUNCTION(NetMulticast, Reliable)
	void MULTI_PlayMontage(FBMSM_S_Montages MontageStruct);

	UFUNCTION(Server, Unreliable)
	void SERVER_UpdateSectionIndex(int32 Value);

	int32 GetLastComboIndex(TArray<FBMSM_S_AttacksMain> Array);

	int32 GetLastSectionIndex(TArray<FName> Array);

	int32 GetValidComboChangedIndex(FBMSM_S_CombosMain ComboStruct);

	// Delegates
	
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAssess = "true"))
	FOnAttackEnded OnAttackEnded;
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAssess = "true"))
	FOnComboCancel OnComboCancel;
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAssess = "true"))
	FOnAttackStarted OnAttackStarted;
	
};
