// Copyright 2022 - Rodrigo Mello


#include "AC_AnimMontageSequenceManager.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BMSM_AnimationEvents.h"
#include "UObject/ScriptDelegates.h"


UAC_AnimMontageSequenceManager::UAC_AnimMontageSequenceManager()
{	
	PrimaryComponentTick.bCanEverTick = false;

	MaxComboInterval = 0.25f;

	ResetComboOnChange = false;

	SetIsReplicated(true);
}

void UAC_AnimMontageSequenceManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const

{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAC_AnimMontageSequenceManager, MovementLocked);
	DOREPLIFETIME(UAC_AnimMontageSequenceManager, CurrentSectionIndex);
	DOREPLIFETIME(UAC_AnimMontageSequenceManager, PlayingMontage);
}

void UAC_AnimMontageSequenceManager::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<ACharacter>(GetOwner());

	AnimInstance = (Character->GetMesh()->GetAnimInstance());

	DefaultWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;

	ComboMap.Empty();

	AnimInstance->OnMontageBlendingOut.AddDynamic(this, &UAC_AnimMontageSequenceManager::SERVER_OnMontageEnded);

	ComboMapBeginPlay();

}

void UAC_AnimMontageSequenceManager::ComboMapBeginPlay()
{
	if (CombosDataTable)
	{
		TArray<FName> DTRows = CombosDataTable->GetRowNames();

		for (FName& LoopName : DTRows)
		{
			FBMSM_S_CombosMain* Item = CombosDataTable->FindRow<FBMSM_S_CombosMain>(LoopName, "");
			ComboMap.Add(Item->ComboName, *Item);
		}

	}	
}

void UAC_AnimMontageSequenceManager::Attack(FName ComboName)
{
	
	if (ComboMap.Contains(ComboName))
	{
		if (!PlayingMontage && !MidAttack)
		{	
			if (ComboHasChanged(ComboName))
			{
				SERVER_UpdateSectionIndex(0);

				// Reset Current Attack Index, if ResetComboOnChange is true, otherwise continue the current Attack Index
				ResetComboOnChange ? CurrentAttackIndex = 0 : CurrentAttackIndex = GetValidComboChangedIndex(*ComboMap.Find(ComboName));

			}
			
			CurrentCombo = *ComboMap.Find(ComboName);
			
			GetOwner()->GetWorldTimerManager().ClearTimer(ComboEndTimer);

			int32 NextComboIndex = CurrentCombo.Combo.IsValidIndex(CurrentAttackIndex) ? NextComboIndex = CurrentAttackIndex : NextComboIndex = 0;

			CurrentAttack = CurrentCombo.Combo[NextComboIndex];

			MidAttack = true;

			MontageVerification();
			
		}
	}
}

void UAC_AnimMontageSequenceManager::MontageVerification()
{
	UAnimMontage* CurrentMontage = CurrentAttack.MontageConfig.MontageRef;
	if (CurrentMontage)
	{
		OnAttackStarted.Broadcast(CurrentAttack.MontageConfig);

		SERVER_PlayMontage(CurrentAttack.MontageConfig);

	}
	else
	{	
		// Cancel current attack if next montage is not valid
		CLIENT_ComboCancel(0.1);
	}

}

void UAC_AnimMontageSequenceManager::CLIENT_PlayMontage_Implementation(FBMSM_S_Montages MontageStruct)
{
	MidAttack ? CLIENT_ComboCancel(0.1f) : SERVER_PlayMontage(MontageStruct);
}

void UAC_AnimMontageSequenceManager::SERVER_PlayMontage_Implementation(FBMSM_S_Montages MontageStruct)
{	
	PlayingMontage = true;

	MULTI_PlayMontage(MontageStruct);

	UpdateMovementLocked(MontageStruct.LockMovement);	
}


void UAC_AnimMontageSequenceManager::MULTI_PlayMontage_Implementation(FBMSM_S_Montages MontageStruct)
{	
	if (AnimInstance->Implements<UBMSM_AnimationEvents>())
	{
		IBMSM_AnimationEvents::Execute_UpdateMontageSlot(AnimInstance, MontageStruct.Slot);
	}

	UAnimMontage* CurrentMontage = MontageStruct.MontageRef;
	float PlayRate = MontageStruct.PlayRate;
	TArray<FName> SectionJumps = MontageStruct.SectionJumps;
	
	if (CurrentMontage)
	{
		AnimInstance->Montage_Play(CurrentMontage, PlayRate);

		if (SectionJumps.Num() > 0)
		{
			AnimInstance->Montage_JumpToSection(SectionJumps[CurrentSectionIndex], CurrentMontage);
		}
	}
	
}

void UAC_AnimMontageSequenceManager::SERVER_OnMontageEnded_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	
	MULTI_OnMontageEnded(Montage, bInterrupted);
	
	if (!bInterrupted)
	{
		UpdateMovementLocked(false);
	}


}

void UAC_AnimMontageSequenceManager::MULTI_OnMontageEnded_Implementation(UAnimMontage* Montage, bool bInterrupted)
{
	PlayingMontage = false;
	
	if (!bInterrupted)
	{	
		if (AnimInstance->Implements<UBMSM_AnimationEvents>())
		{
			IBMSM_AnimationEvents::Execute_UpdateMontageSlot(AnimInstance, EMontageSlotTypes::EMST_Default);
		}
		
		if (Montage == CurrentAttack.MontageConfig.MontageRef)
		{	
			AttackMontageEnd();
		}
	}

}

void UAC_AnimMontageSequenceManager::AttackMontageEnd()
{
	if (MidAttack)
	{
		AttackEnd();
	}
}
void UAC_AnimMontageSequenceManager::OnRep_MovementLocked()
{
	if (MovementLocked)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 0;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	}	
}
void UAC_AnimMontageSequenceManager::CLIENT_ComboCancel_Implementation(float InTime)

{
	CurrentAttackIndex = 0;
	
	SERVER_UpdateSectionIndex(0);
	
	GetOwner()->GetWorldTimerManager().ClearTimer(ComboEndTimer);

	OnComboCancel.Broadcast();

	SERVER_MontageStop(InTime);

	// Timers aren't activated if time = 0
	float RecoveryTime = InTime >= 0.1f ? RecoveryTime = InTime : RecoveryTime = 0.1f;

	GetOwner()->GetWorldTimerManager().SetTimer(ComboEndTimer, this, &UAC_AnimMontageSequenceManager::ComboCancelRecoveryEvent, RecoveryTime , false);

}

void UAC_AnimMontageSequenceManager::ComboCancelRecoveryEvent()
{
	MidAttack = false;
}

void UAC_AnimMontageSequenceManager::SERVER_MontageStop_Implementation(float InTime)
{
	UpdateMovementLocked(false);

	PlayingMontage = false;

	MULTI_MontageStop(InTime);

}
void UAC_AnimMontageSequenceManager::MULTI_MontageStop_Implementation(float InTime)
{
	AnimInstance->Montage_Stop(InTime);
}


void UAC_AnimMontageSequenceManager::AttackEnd()
{
	bool HasNextSection = GetLastSectionIndex(CurrentAttack.MontageConfig.SectionJumps) > CurrentSectionIndex; 
	
	if (MidAttack)
	{
		HasNextSection ? SERVER_UpdateSectionIndex(CurrentSectionIndex + 1) : IncreaseAttackIndex();

		UpdateMovementLocked(false);
		MidAttack = false;
		PlayingMontage = false;
		
		GetOwner()->GetWorldTimerManager().SetTimer(ComboEndTimer, this, &UAC_AnimMontageSequenceManager::ComboEndTimerEvent, MaxComboInterval, false);
		
		OnAttackEnded.Broadcast();
	}

}

void UAC_AnimMontageSequenceManager::ComboEndTimerEvent()
{
	CLIENT_ComboCancel(MaxComboInterval);
}

void UAC_AnimMontageSequenceManager::IncreaseAttackIndex()
{
	bool IsLastCombo = CurrentAttackIndex < GetLastComboIndex(CurrentCombo.Combo); 
	
	SERVER_UpdateSectionIndex(0);
	
	IsLastCombo ? CurrentAttackIndex++ : CurrentAttackIndex = 0;
}

bool UAC_AnimMontageSequenceManager::ComboHasChanged(FName ComboName)
{
	return (CurrentAttackIndex > 0) && (CurrentCombo.ComboName != ComboName); 
}

// Interface Functions

void UAC_AnimMontageSequenceManager::I_Attack_Implementation(FName Name)
{
	Attack(Name);
}

void UAC_AnimMontageSequenceManager::I_ComboCancel_Implementation(float InTime)
{
	CLIENT_ComboCancel(InTime);
}

void UAC_AnimMontageSequenceManager::I_PlayMontageByStruct_Implementation(FBMSM_S_Montages Montage)
{
	if (!PlayingMontage)
	{
		CLIENT_PlayMontage(Montage);
	}
}

void UAC_AnimMontageSequenceManager::I_MontageEnd_Implementation()
{
	AttackMontageEnd();
}

void  UAC_AnimMontageSequenceManager::UpdateMovementLocked(bool Value)
{
	if (GetOwner()->HasAuthority())
	{
		MovementLocked = Value;
		OnRep_MovementLocked();
	}	
}

void UAC_AnimMontageSequenceManager::SERVER_UpdateSectionIndex_Implementation(int32 Value)
{
	CurrentSectionIndex = Value;
}

int32 UAC_AnimMontageSequenceManager::GetLastComboIndex(TArray<FBMSM_S_AttacksMain> Array)
{
	return int32(Array.Num() - 1);
}

int32 UAC_AnimMontageSequenceManager::GetLastSectionIndex(TArray<FName> Array)
{
	return int32(Array.Num() - 1);
}

int32 UAC_AnimMontageSequenceManager::GetValidComboChangedIndex(FBMSM_S_CombosMain ComboStruct)
{
	// Returns the a safe CurrentAttackIndex if this combo index is greater than the last

	int32 ReturnIndex;

	const int32 CurrentComboLastIndex = GetLastComboIndex(ComboStruct.Combo);

	CurrentAttackIndex > CurrentComboLastIndex ? ReturnIndex = CurrentComboLastIndex : ReturnIndex = CurrentAttackIndex;

	return ReturnIndex;
}