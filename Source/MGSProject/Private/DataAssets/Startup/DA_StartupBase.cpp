/*
 * 파일명 : DA_Startup.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-04
 */

#include "DataAssets/Startup/DA_StartupBase.h"

#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/GA/BaseGameplayAbility.h"

void UDA_StartupBase::GiveToAbilitySystemComponent(UMGSAbilitySystemComponent* ASC, int32 Level)
{
	// ASC 유효성 검사
	check(ASC);
	
	// ASC에 자동 활성화 및 반응형 GA 목록 부여
	GrantAbilities(ActivateOnGivenAbilities, ASC, Level);
	GrantAbilities(ReactiveAbilities, ASC, Level);
	
	// 자동 활성화 GE 목록이 비어있지 않으면
	if (!StartupGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& BP_Effect : StartupGameplayEffects)
		{
			if (!BP_Effect)
			{
				continue;
			}
			
			// GameplayEffect 클래스의 CDO(Class Default Object)를 가져와 ASC 자신에게 적용
			UGameplayEffect* EffectCDO = BP_Effect->GetDefaultObject<UGameplayEffect>();
			ASC->ApplyGameplayEffectToSelf(EffectCDO, Level, ASC->MakeEffectContext());
		}
	}
}

void UDA_StartupBase::GrantAbilities(const TArray<TSubclassOf<UBaseGameplayAbility>> GAs,
	UMGSAbilitySystemComponent* InASC, int32 Level)
{
	// GA 목록이 비어있으면
	if (GAs.IsEmpty())
	{
		return;
	}
	
	// GA 목록 순회
	for (const TSubclassOf<UBaseGameplayAbility> Ability : GAs)
	{
		// Spec 생성
		FGameplayAbilitySpec Spec(Ability);
		Spec.SourceObject = InASC->GetAvatarActor();
		Spec.Level = Level;
		
		// ASC에 어빌리티 부여
		InASC->GiveAbility(Spec);
	}
}
