/*
 * 파일명 : PlayerCombatComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#include "Components/Combat/PlayerCombatComponent.h"

#include "Weapon/BaseWeapon.h"

void UPlayerCombatComponent::RegisterSpawnedWeapon(FGameplayTag WeaponTag, ABaseWeapon* Weapon,
                                                   bool bRegisterAsEquippedWeapon)
{
	checkf(!CharacterCarriedWeaponMap.Contains(WeaponTag), TEXT("%s has already been as carried weapon"), *WeaponTag.ToString());
	check(Weapon);
	
	CharacterCarriedWeaponMap.Emplace(WeaponTag, Weapon);
	
	// 장착한 무기로 등록이 되면 현재 장착무기를 변경
	if (bRegisterAsEquippedWeapon)
	{
		CurrentEquippedWeaponTag = WeaponTag;
	}
}

ABaseWeapon* UPlayerCombatComponent::GetCharacterCarriedWeaponByTag(FGameplayTag WeaponTag) const
{
	// Map 컨테이너에 WeaponTag의 데이터가 있으면 ABaseWeapon 반환
	if (CharacterCarriedWeaponMap.Contains(WeaponTag))
	{
		if (ABaseWeapon* const* FoundWeapon = CharacterCarriedWeaponMap.Find(WeaponTag))
		{
			return *FoundWeapon;
		}
	}
	
	return nullptr;
}

ABaseWeapon* UPlayerCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}
	
	return GetCharacterCarriedWeaponByTag(CurrentEquippedWeaponTag);
}

ABaseWeapon* UPlayerCombatComponent::GetPlayerCarriedWeaponByTag(FGameplayTag Tag) const
{
	return Cast<ABaseWeapon>(GetCharacterCarriedWeaponByTag(Tag));
}
