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
	
	// 시작 어빌리티 두 그룹을 ASC에 부여
	GrantAbilities(ActivateOnGivenAbilities, ASC, Level);
	GrantAbilities(ReactiveAbilities, ASC, Level);
	
	if (!StartupGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& BP_Effect : StartupGameplayEffects)
		{
			if (!BP_Effect)
			{
				continue;
			}
			
			// GameplayEffect 클래스의 CDO를 가져와 자기 자신에게 적용
			UGameplayEffect* EffectCDO = BP_Effect->GetDefaultObject<UGameplayEffect>();
			ASC->ApplyGameplayEffectToSelf(EffectCDO, Level, ASC->MakeEffectContext());
		}
	}
}

void UDA_StartupBase::GrantAbilities(const TArray<TSubclassOf<UBaseGameplayAbility>> GAs,
	UMGSAbilitySystemComponent* InASC, int32 Level)
{
	// 부여할 어빌리티가 없으면 종료
	if (GAs.IsEmpty())
	{
		return;
	}
	
	for (const TSubclassOf<UBaseGameplayAbility> Ability : GAs)
	{
		// 어빌리티 스펙 생성 및 공통 메타데이터 설정
		FGameplayAbilitySpec Spec(Ability);
		Spec.SourceObject = InASC->GetAvatarActor();
		Spec.Level = Level;
		
		// 설정된 스펙을 ASC에 최종 부여
		InASC->GiveAbility(Spec);
	}
}
