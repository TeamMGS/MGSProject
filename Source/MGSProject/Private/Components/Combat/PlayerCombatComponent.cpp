/*
 * 파일명 : PlayerCombatComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Components/Combat/PlayerCombatComponent.h"

#include "Engine/EngineTypes.h"
#include "GAS/MGSGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon/BaseWeapon.h"

UPlayerCombatComponent::UPlayerCombatComponent()
{
	PrimaryWeaponTag = MGSGameplayTags::Weapon_Primary;
	SecondaryWeaponTag = MGSGameplayTags::Weapon_Secondary;
}

bool UPlayerCombatComponent::EquipPrimaryWeapon()
{
	return EquipWeaponByTag(PrimaryWeaponTag);
}

bool UPlayerCombatComponent::EquipSecondaryWeapon()
{
	return EquipWeaponByTag(SecondaryWeaponTag);
}

bool UPlayerCombatComponent::PickupDroppedWeapon(ABaseWeapon* DroppedWeapon)
{
	if (!DroppedWeapon)
	{
		return false;
	}

	const FGameplayTag DroppedWeaponTag = DroppedWeapon->GetWeaponTag();
	const bool bSupportedSlotTag = DroppedWeaponTag.MatchesTagExact(PrimaryWeaponTag) ||
								   DroppedWeaponTag.MatchesTagExact(SecondaryWeaponTag);
	if (!DroppedWeaponTag.IsValid() || !bSupportedSlotTag)
	{
		return false;
	}

	const bool bPickedUp = PickupDroppedWeaponByTag(DroppedWeaponTag, DroppedWeapon);
	if (bPickedUp)
	{
		NotifyDroppedWeaponOverlapEnd(DroppedWeapon);
	}

	return bPickedUp;
}

void UPlayerCombatComponent::NotifyDroppedWeaponOverlapBegin(ABaseWeapon* DroppedWeapon)
{
	if (!DroppedWeapon || DroppedWeapon->GetOwner() == GetOwner())
	{
		return;
	}

	NearbyDroppedWeapons.Add(DroppedWeapon);
	BroadcastNearbyDroppedWeaponChangedIfNeeded();
}

void UPlayerCombatComponent::NotifyDroppedWeaponOverlapEnd(ABaseWeapon* DroppedWeapon)
{
	if (!DroppedWeapon)
	{
		return;
	}

	NearbyDroppedWeapons.Remove(DroppedWeapon);
	BroadcastNearbyDroppedWeaponChangedIfNeeded();
}

bool UPlayerCombatComponent::HasNearbyDroppedWeapon() const
{
	return GetNearbyDroppedWeapon() != nullptr;
}

ABaseWeapon* UPlayerCombatComponent::GetNearbyDroppedWeapon() const
{
	if (ABaseWeapon* OverlapCandidate = ResolveBestNearbyDroppedWeapon())
	{
		return OverlapCandidate;
	}

	return FindNearbyDroppedWeaponBySweep();
}

void UPlayerCombatComponent::RefreshNearbyDroppedWeaponCandidate()
{
	BroadcastNearbyDroppedWeaponChangedIfNeeded();
}

ABaseWeapon* UPlayerCombatComponent::ResolveBestNearbyDroppedWeapon() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	ABaseWeapon* BestWeapon = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<ABaseWeapon>& NearbyWeapon : NearbyDroppedWeapons)
	{
		ABaseWeapon* Weapon = NearbyWeapon.Get();
		if (!Weapon || Weapon->GetOwner() == OwnerActor || !Weapon->GetWeaponTag().IsValid() || Weapon->IsHidden())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OwnerLocation, Weapon->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestWeapon = Weapon;
		}
	}

	return BestWeapon;
}

ABaseWeapon* UPlayerCombatComponent::FindNearbyDroppedWeaponBySweep() const
{
	const AActor* OwnerActor = GetOwner();
	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	if (!OwnerActor || !OwnerPawn)
	{
		return nullptr;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(const_cast<APawn*>(OwnerPawn));

	TArray<AActor*> OverlappedActors;
	const FVector SearchCenter = OwnerActor->GetActorLocation() + OwnerActor->GetActorForwardVector() * PickupSweepForwardOffset;
	const bool bFoundOverlap = UKismetSystemLibrary::SphereOverlapActors(
		World,
		SearchCenter,
		PickupSweepRadius,
		ObjectTypes,
		ABaseWeapon::StaticClass(),
		ActorsToIgnore,
		OverlappedActors);
	if (!bFoundOverlap)
	{
		return nullptr;
	}

	ABaseWeapon* BestWeapon = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (AActor* OverlappedActor : OverlappedActors)
	{
		ABaseWeapon* Weapon = Cast<ABaseWeapon>(OverlappedActor);
		if (!Weapon || Weapon->GetOwner() == OwnerActor || !Weapon->GetWeaponTag().IsValid() || Weapon->IsHidden())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OwnerActor->GetActorLocation(), Weapon->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestWeapon = Weapon;
		}
	}

	return BestWeapon;
}

void UPlayerCombatComponent::BroadcastNearbyDroppedWeaponChangedIfNeeded()
{
	for (auto It = NearbyDroppedWeapons.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	ABaseWeapon* CurrentNearbyDroppedWeapon = GetNearbyDroppedWeapon();
	if (LastNearbyDroppedWeapon.Get() == CurrentNearbyDroppedWeapon)
	{
		return;
	}

	LastNearbyDroppedWeapon = CurrentNearbyDroppedWeapon;
	OnNearbyDroppedWeaponChanged.Broadcast(CurrentNearbyDroppedWeapon);
}
