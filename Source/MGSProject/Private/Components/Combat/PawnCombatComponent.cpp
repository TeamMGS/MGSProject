/*
 * 파일명 : PawnCombatComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "Components/Combat/PawnCombatComponent.h"

#include "Characters/BaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "MGSStructType.h"
#include "Weapon/BaseGun.h"
#include "Weapon/BaseWeapon.h"

void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag WeaponTag, ABaseWeapon* Weapon,
	bool bRegisterAsEquippedWeapon)
{
	checkf(!CharacterCarriedWeaponMap.Contains(WeaponTag), TEXT("%s has already been as carried weapon"), *WeaponTag.ToString());
	check(Weapon);

	CharacterCarriedWeaponMap.Emplace(WeaponTag, Weapon);
	Weapon->SetOwner(GetOwningPawn());

	if (ABaseGun* SpawnedGun = Cast<ABaseGun>(Weapon))
	{
		CharacterCarriedWeaponRuntimeStateMap.FindOrAdd(WeaponTag) = SpawnedGun->MakeDefaultRuntimeState();
	}

	// 기본 등록 시에는 홀스터 소켓으로 붙여두고, 장착 등록이면 즉시 장착합니다.
	if (!bRegisterAsEquippedWeapon)
	{
		AttachWeaponToSocket(Weapon, Weapon->GetHolsterSocketName());
		Weapon->SetActorHiddenInGame(Weapon->GetHolsterSocketName().IsNone());
		Weapon->SetActorEnableCollision(false);
		return;
	}

	EquipWeaponByTag(WeaponTag);
}

ABaseWeapon* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(FGameplayTag WeaponTag) const
{
	if (CharacterCarriedWeaponMap.Contains(WeaponTag))
	{
		if (ABaseWeapon* const* FoundWeapon = CharacterCarriedWeaponMap.Find(WeaponTag))
		{
			return *FoundWeapon;
		}
	}

	return nullptr;
}

ABaseWeapon* UPawnCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}

	return GetCharacterCarriedWeaponByTag(CurrentEquippedWeaponTag);
}

ABaseWeapon* UPawnCombatComponent::GetPlayerCarriedWeaponByTag(FGameplayTag Tag) const
{
	return Cast<ABaseWeapon>(GetCharacterCarriedWeaponByTag(Tag));
}

bool UPawnCombatComponent::EquipWeaponByTag(FGameplayTag WeaponTag)
{
	if (!WeaponTag.IsValid())
	{
		return false;
	}

	ABaseWeapon* TargetWeapon = GetCharacterCarriedWeaponByTag(WeaponTag);
	if (!TargetWeapon)
	{
		return false;
	}

	if (CurrentEquippedWeaponTag.MatchesTagExact(WeaponTag))
	{
		// 같은 무기를 다시 장착하려는 경우는 성공으로 처리합니다.
		return true;
	}

	UnequipCurrentWeapon();
	ApplyWeaponRuntimeState(WeaponTag, TargetWeapon);

	AttachWeaponToSocket(TargetWeapon, TargetWeapon->GetEquippedSocketName());
	TargetWeapon->SetActorHiddenInGame(false);
	TargetWeapon->SetActorEnableCollision(false);

	ApplyWeaponAbilities(TargetWeapon);
	CurrentEquippedWeaponTag = WeaponTag;
	return true;
}

bool UPawnCombatComponent::UnequipCurrentWeapon()
{
	ABaseWeapon* CurrentWeapon = GetCharacterCurrentEquippedWeapon();
	if (!CurrentWeapon)
	{
		return false;
	}

	SaveCurrentWeaponRuntimeState();
	RemoveWeaponAbilities(CurrentWeapon);
	AttachWeaponToSocket(CurrentWeapon, CurrentWeapon->GetHolsterSocketName());
	CurrentWeapon->SetActorHiddenInGame(CurrentWeapon->GetHolsterSocketName().IsNone());
	CurrentWeapon->SetActorEnableCollision(false);
	CurrentEquippedWeaponTag = FGameplayTag();
	return true;
}

bool UPawnCombatComponent::AttachWeaponToSocket(ABaseWeapon* Weapon, FName SocketName) const
{
	ACharacter* OwningCharacter = Cast<ACharacter>(GetOwningPawn());
	if (!OwningCharacter || !Weapon)
	{
		return false;
	}

	USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh();
	if (!CharacterMesh)
	{
		return false;
	}

	if (SocketName.IsNone())
	{
		// 소켓 이름이 비어있다면 현재 트랜스폼을 유지합니다.
		return true;
	}

	Weapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	return true;
}

void UPawnCombatComponent::AddWeaponInputMappingContext(ABaseWeapon* Weapon) const
{
	if (!Weapon || !Weapon->GetWeaponData().WeaponInputMappingContext)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwningController());
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		// 무기 입력은 기본 입력보다 높은 우선순위로 적용합니다.
		InputSubsystem->AddMappingContext(Weapon->GetWeaponData().WeaponInputMappingContext, 1);
	}
}

void UPawnCombatComponent::RemoveWeaponInputMappingContext(ABaseWeapon* Weapon) const
{
	if (!Weapon || !Weapon->GetWeaponData().WeaponInputMappingContext)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwningController());
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		InputSubsystem->RemoveMappingContext(Weapon->GetWeaponData().WeaponInputMappingContext);
	}
}

void UPawnCombatComponent::ApplyWeaponAbilities(ABaseWeapon* Weapon) const
{
	const ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	UMGSAbilitySystemComponent* ASC = OwningCharacter ? OwningCharacter->GetMGSAbilitySystemComponent() : nullptr;
	if (!ASC || !Weapon)
	{
		return;
	}

	TArray<FGameplayAbilitySpecHandle> GrantedHandles;
	ASC->GrantWeaponAbilities(Weapon->GetWeaponData().WeaponAbilities, 1, GrantedHandles);
	Weapon->AssignGrantedAbilitySpecHandles(GrantedHandles);
	AddWeaponInputMappingContext(Weapon);
}

void UPawnCombatComponent::RemoveWeaponAbilities(ABaseWeapon* Weapon) const
{
	const ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	UMGSAbilitySystemComponent* ASC = OwningCharacter ? OwningCharacter->GetMGSAbilitySystemComponent() : nullptr;
	if (!ASC || !Weapon)
	{
		return;
	}

	RemoveWeaponInputMappingContext(Weapon);
	ASC->RemoveGrantedWeaponAbilities(Weapon->GetGrantedAbilitySpecHandlesMutable());
}

void UPawnCombatComponent::SaveCurrentWeaponRuntimeState()
{
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return;
	}

	const ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	const UWeaponAttributeSet* WeaponAttributeSet = OwningCharacter ? OwningCharacter->GetWeaponAttributeSet() : nullptr;
	ABaseGun* CurrentGun = Cast<ABaseGun>(GetCharacterCurrentEquippedWeapon());
	if (!CurrentGun || !WeaponAttributeSet)
	{
		return;
	}

	CharacterCarriedWeaponRuntimeStateMap.FindOrAdd(CurrentEquippedWeaponTag) = CurrentGun->MakeRuntimeState(WeaponAttributeSet);
}

void UPawnCombatComponent::ApplyWeaponRuntimeState(FGameplayTag WeaponTag, ABaseWeapon* Weapon)
{
	ABaseGun* TargetGun = Cast<ABaseGun>(Weapon);
	ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	UWeaponAttributeSet* WeaponAttributeSet = OwningCharacter ? OwningCharacter->GetWeaponAttributeSet() : nullptr;
	if (!TargetGun || !WeaponAttributeSet)
	{
		return;
	}

	TargetGun->InitializeWeaponAttributes(WeaponAttributeSet);

	if (const FWeaponRuntimeState* SavedRuntimeState = CharacterCarriedWeaponRuntimeStateMap.Find(WeaponTag))
	{
		TargetGun->ApplyRuntimeState(WeaponAttributeSet, *SavedRuntimeState);
		return;
	}

	const FWeaponRuntimeState DefaultRuntimeState = TargetGun->MakeDefaultRuntimeState();
	CharacterCarriedWeaponRuntimeStateMap.Add(WeaponTag, DefaultRuntimeState);
	TargetGun->ApplyRuntimeState(WeaponAttributeSet, DefaultRuntimeState);
}
