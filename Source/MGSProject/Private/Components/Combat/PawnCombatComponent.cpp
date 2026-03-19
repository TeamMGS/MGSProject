/*
 * 파일명 : PawnCombatComponent.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-12
 */

#include "Components/Combat/PawnCombatComponent.h"

#include "Characters/BaseCharacter.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MGSDebugHelper.h"
#include "Engine/LocalPlayer.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/WeaponAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "MGSStructType.h"
#include "Subsystems/ProjectilePoolWorldSubsystem.h"
#include "Weapon/BaseGun.h"
#include "Weapon/BaseWeapon.h"

void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag WeaponTag, ABaseWeapon* Weapon,
	const bool bRegisterAsEquippedWeapon)
{
	constexpr EObjectFlags TemplateFlags = RF_ClassDefaultObject | RF_ArchetypeObject;
	if (HasAnyFlags(TemplateFlags) ||
		(GetOwner() && GetOwner()->HasAnyFlags(TemplateFlags)) ||
		(Weapon && Weapon->HasAnyFlags(TemplateFlags)))
	{
		return;
	}

	// 이미 주무기, 보조무기가 장착되어 있으면 (=Key(태그)값이 존재하면)
	if (CharacterCarriedWeaponMap.Contains(WeaponTag))
	{
		const FString& Msg = FString::Printf(TEXT("[%s::%s] %s has already been carried as a weapon"), *GetNameSafe(GetOwner()), *GetName(), *WeaponTag.ToString());
		Debug::Print(Msg, FColor::Red);
		return;
	}
	// 장착할 무기(클래스)가 없으면
	if (!Weapon)
	{
		const FString& Msg = FString::Printf(TEXT("[%s::%s] Weapon class is missing"), *GetNameSafe(GetOwner()), *GetName());
		Debug::Print(Msg, FColor::Red);
		return;
	}

	// 소유 무기 목록에 추가
	CharacterCarriedWeaponMap.Emplace(WeaponTag, Weapon);
	// 무기에 Owner를 이 컴포넌트의 Owner로 설정
	Weapon->SetOwner(GetOwningPawn());

	if (const ABaseGun* SpawnedGun = Cast<ABaseGun>(Weapon))
	{
		// 런타임 정보 목록에 추가
		CharacterCarriedWeaponRuntimeStateMap.FindOrAdd(WeaponTag) = SpawnedGun->MakeDefaultRuntimeState();

		if (const UWorld* World = SpawnedGun->GetWorld())
		{
			if (UMGSProjectilePoolWorldSubsystem* ProjectilePoolSubsystem = World->GetSubsystem<UMGSProjectilePoolWorldSubsystem>())
			{
				// 무기 등록 시점에 풀링 월드 서브시스템에서 프리웜 진행
				ProjectilePoolSubsystem->PrewarmProjectileClass(SpawnedGun->GetProjectileClass());
			}
		}
	}
	
	// 바로 장비시키지 않을 경우 (홀스터 소켓에 장착 후 소유)
	if (!bRegisterAsEquippedWeapon)
	{
		AttachWeaponToSocket(Weapon, Weapon->GetHolsterSocketName());
		// 게임에서 숨기기
		Weapon->SetActorHiddenInGame(Weapon->GetHolsterSocketName().IsNone());
		// 콜리전 비활성화
		Weapon->SetActorEnableCollision(false);
		return;
	}

	// 등록 후 바로 장비
	EquipWeaponByTag(WeaponTag);
}

ABaseWeapon* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(const FGameplayTag& WeaponTag) const
{
	// 소유 무기 목록에서 태그(Key)와 일치하는 무기(Value=Class) 반환
	if (const TObjectPtr<ABaseWeapon>* FoundWeapon = CharacterCarriedWeaponMap.Find(WeaponTag))
	{
		return FoundWeapon->Get();
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

bool UPawnCombatComponent::EquipWeaponByTag(const FGameplayTag& WeaponTag)
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

	// 장착하려는 무기 태그가 현재 장착중인 무기의 태그와 같을 경우
	// ex) 태그: 주무기, 현재 장착중인 무기 태그: 주무기
	if (CurrentEquippedWeaponTag.MatchesTagExact(WeaponTag))
	{
		return true;
	}
	
	// 현재 장착 무기 해제
	UnequipCurrentWeapon();
	// 장착하려는 무기의 런타임 정보 등록
	ApplyWeaponRuntimeState(WeaponTag, TargetWeapon);
	// 장착하려는 무기의 어빌리티 부여 및 무기 입력 추가
	ApplyWeaponAbilities(TargetWeapon);
	// 장착하려는 무기를 장착 소켓에 부착
	AttachWeaponToSocket(TargetWeapon, TargetWeapon->GetEquippedSocketName());
	// 장착하려는 무기를 게임에서 보이도록
	TargetWeapon->SetActorHiddenInGame(false);
	
	// [03.11 김동석 추가] 애니메이션 레이어 링크
	if (ACharacter* OwningChar = Cast<ACharacter>(GetOwningPawn()))
	{
		if (UAnimInstance* AnimInst = OwningChar->GetMesh()->GetAnimInstance())
		{
			TSubclassOf<UAnimInstance> NewLayer = TargetWeapon->GetWeaponData().WeaponAnimLayer;
			if (NewLayer)
			{
				AnimInst->LinkAnimClassLayers(NewLayer);
			}
		}
	}
	
	// 이전 장착 무기 태그 캐싱
	const FGameplayTag& PreviousEquippedWeaponTag = CurrentEquippedWeaponTag;
	// 현재 장착 무기 태그 갱신
	CurrentEquippedWeaponTag = WeaponTag;
	// 무기 변경 이벤트 발생
	OnEquippedWeaponChanged.Broadcast(PreviousEquippedWeaponTag, CurrentEquippedWeaponTag);
	
	return true;
}

bool UPawnCombatComponent::UnequipCurrentWeapon()
{
	ABaseWeapon* CurrentWeapon = GetCharacterCurrentEquippedWeapon();
	if (!CurrentWeapon)
	{
		return false;
	}

	// [03.11 김동석 추가] 애니메이션 레이어 해제
	if (ACharacter* OwningChar = Cast<ACharacter>(GetOwningPawn()))
	{
		if (UAnimInstance* AnimInst = OwningChar->GetMesh()->GetAnimInstance())
		{
			TSubclassOf<UAnimInstance> LayerToUnlink = CurrentWeapon->GetWeaponData().WeaponAnimLayer;
			if (LayerToUnlink)
			{
				AnimInst->UnlinkAnimClassLayers(LayerToUnlink);
			}
		}
	}
	
	// 장착 해제하려는 무기의 런타임 정보 저장
	SaveCurrentWeaponRuntimeState();
	// 장착 해제하려는 무기의 어빌리티 제거 및 무기 입력 제거 
	RemoveWeaponAbilities(CurrentWeapon);
	// 장착 해제하려는 무기를 홀스터 소켓에 부착
	AttachWeaponToSocket(CurrentWeapon, CurrentWeapon->GetHolsterSocketName());
	// 장착 해제하려는 무기를 게임에서 숨김
	CurrentWeapon->SetActorHiddenInGame(CurrentWeapon->GetHolsterSocketName().IsNone());
	
	// 장착 해제하려는 무기 태그 캐싱
	const FGameplayTag PreviousEquippedWeaponTag = CurrentEquippedWeaponTag;
	// 현재 장착 무기 태그 비우기
	CurrentEquippedWeaponTag = FGameplayTag();
	// 무기 변경 이벤트 발생
	OnEquippedWeaponChanged.Broadcast(PreviousEquippedWeaponTag, CurrentEquippedWeaponTag);
	
	return true;
}

bool UPawnCombatComponent::PickupDroppedWeaponByTag(const FGameplayTag& WeaponTag, ABaseWeapon* DroppedWeapon)
{
	if (!WeaponTag.IsValid() || !DroppedWeapon)
	{
		return false;
	}

	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwningPawn());
	if (!OwningCharacter)
	{
		return false;
	}

	// 이미 자신 소유인 무기를 다시 줍는 경우면 처리하지 않음
	if (DroppedWeapon->GetOwner() == OwningCharacter)
	{
		return false;
	}

	
	
	// 주우려는 무기의 태그가 장비중인 태그과 같다면
	const bool bWasEquippedTargetTag = CurrentEquippedWeaponTag.MatchesTagExact(WeaponTag);
	if (bWasEquippedTargetTag)
	{
		// 현재 장비중인 무기를 해제
		UnequipCurrentWeapon();
	}

	// 떨어뜨릴 무기를 가져옴
	ABaseWeapon* ExistingWeapon = GetCharacterCarriedWeaponByTag(WeaponTag);
	if (ExistingWeapon)
	{
		// 떨어뜨릴 위치, 각도를 캐릭터 기준 Offset만큼 이동하여 계산
		const FVector CharacterDropOrigin =
			OwningCharacter->GetActorLocation() + OwningCharacter->GetActorForwardVector() * PickupSwapDropForwardOffset;
		const FVector DroppedWeaponLocation = CharacterDropOrigin + FVector(0.0f, 0.0f, PickupSwapDropHeightOffset);
		const FRotator DroppedWeaponRotation = OwningCharacter->GetActorRotation();
		
		// 떨어뜨릴 무기의 런타임 정보를 저장
		FWeaponRuntimeState ExistingWeaponRuntimeState;
		if (const FWeaponRuntimeState* ExistingRuntimeState = CharacterCarriedWeaponRuntimeStateMap.Find(WeaponTag))
		{
			ExistingWeaponRuntimeState = *ExistingRuntimeState;
			ExistingWeapon->SaveDroppedRuntimeState(ExistingWeaponRuntimeState);
		}

		// 떨어뜨릴 무기의 태그 제거
		CharacterCarriedWeaponMap.Remove(WeaponTag);
		// 떨어뜨릴 무기의 런타임 정보 제거
		CharacterCarriedWeaponRuntimeStateMap.Remove(WeaponTag);

		// 소켓에서 제거
		ExistingWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		// Owner 정리
		ExistingWeapon->SetOwner(nullptr);
		// 레벨에 떨어뜨림으로 게임에서 보이기
		ExistingWeapon->SetActorHiddenInGame(false);
		// 레벨에 떨어뜨릴 위치, 각도 지정
		ExistingWeapon->SetActorLocationAndRotation(DroppedWeaponLocation, DroppedWeaponRotation);
		// 무기가 레벨에 있을 때의 설정(물리, 콜리전)
		ExistingWeapon->SetAsWorldDroppedWeapon();
	}

	// 주운 무기를 슬롯에 등록합니다.
	DroppedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	// 주운 무기의 Owner 설정
	DroppedWeapon->SetOwner(GetOwningPawn());
	// 주운 무기를 처음에는 게임에서 안보이도록 설정 (장착 무기가 아닐 경우 가정)
	DroppedWeapon->SetActorHiddenInGame(false);
	// 소유 무기에 등록(Key: 태그, Value: Class)
	CharacterCarriedWeaponMap.Emplace(WeaponTag, DroppedWeapon);

	if (const ABaseGun* PickedGun = Cast<ABaseGun>(DroppedWeapon))
	{
		// 주운 무기의 런타임 정보 적재
		FWeaponRuntimeState PickedRuntimeState;
		if (DroppedWeapon->ConsumeDroppedRuntimeState(PickedRuntimeState))
		{
			CharacterCarriedWeaponRuntimeStateMap.FindOrAdd(WeaponTag) = PickedRuntimeState;
		}
		else
		{
			CharacterCarriedWeaponRuntimeStateMap.FindOrAdd(WeaponTag) = PickedGun->MakeDefaultRuntimeState();
		}
	}
	else
	{
		CharacterCarriedWeaponRuntimeStateMap.Remove(WeaponTag);
	}

	// 주우려는 무기의 태그가 장비중인 태그과 같다면
	if (bWasEquippedTargetTag)
	{
		// 즉시 장착
		return EquipWeaponByTag(WeaponTag);
	}

	// 주우려는 무기의 태그가 장비중인 태그가 아니라면
	// 홀스터 소켓에 장착
	AttachWeaponToSocket(DroppedWeapon, DroppedWeapon->GetHolsterSocketName());
	// 게임에서 숨기기
	DroppedWeapon->SetActorHiddenInGame(DroppedWeapon->GetHolsterSocketName().IsNone());
	
	return true;
}

bool UPawnCombatComponent::DropCarriedWeaponByTag(const FGameplayTag& WeaponTag, const FVector& WorldLocation, const FRotator& WorldRotation)
{
	if (!WeaponTag.IsValid())
	{
		return false;
	}

	ABaseWeapon* WeaponToDrop = GetCharacterCarriedWeaponByTag(WeaponTag);
	if (!WeaponToDrop)
	{
		return false;
	}

	const bool bWasEquipped = CurrentEquippedWeaponTag.MatchesTagExact(WeaponTag);
	if (bWasEquipped)
	{
		if (ACharacter* OwningChar = Cast<ACharacter>(GetOwningPawn()))
		{
			if (UAnimInstance* AnimInst = OwningChar->GetMesh()->GetAnimInstance())
			{
				TSubclassOf<UAnimInstance> LayerToUnlink = WeaponToDrop->GetWeaponData().WeaponAnimLayer;
				if (LayerToUnlink)
				{
					AnimInst->UnlinkAnimClassLayers(LayerToUnlink);
				}
			}
		}

		SaveCurrentWeaponRuntimeState();
		RemoveWeaponAbilities(WeaponToDrop);

		const FGameplayTag PreviousEquippedWeaponTag = CurrentEquippedWeaponTag;
		CurrentEquippedWeaponTag = FGameplayTag();
		OnEquippedWeaponChanged.Broadcast(PreviousEquippedWeaponTag, CurrentEquippedWeaponTag);
	}

	if (const FWeaponRuntimeState* ExistingRuntimeState = CharacterCarriedWeaponRuntimeStateMap.Find(WeaponTag))
	{
		WeaponToDrop->SaveDroppedRuntimeState(*ExistingRuntimeState);
	}

	CharacterCarriedWeaponMap.Remove(WeaponTag);
	CharacterCarriedWeaponRuntimeStateMap.Remove(WeaponTag);

	WeaponToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	WeaponToDrop->SetOwner(nullptr);
	WeaponToDrop->SetActorHiddenInGame(false);
	WeaponToDrop->SetActorLocationAndRotation(WorldLocation, WorldRotation);
	WeaponToDrop->SetAsWorldDroppedWeapon();

	return true;
}

void UPawnCombatComponent::ApplyWeaponRuntimeState(const FGameplayTag& WeaponTag, ABaseWeapon* Weapon)
{
	const ABaseGun* TargetGun = Cast<ABaseGun>(Weapon);
	const ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	UWeaponAttributeSet* WeaponAttributeSet = OwningCharacter ? OwningCharacter->GetWeaponAttributeSet() : nullptr;
	if (!TargetGun || !WeaponAttributeSet)
	{
		return;
	}

	// 장착하려는 무기의 WeaponAttributeSet 초기화
	TargetGun->InitializeWeaponAttributes(WeaponAttributeSet);
	
	// 소유 무기 런타임 정보 목록에서 태그(주무기, 보조무기)에 해당하는 런타임 정보 조회  
	if (const FWeaponRuntimeState* SavedRuntimeState = CharacterCarriedWeaponRuntimeStateMap.Find(WeaponTag))
	{
		// 장착하려는 무기에 런타임 정보 적용
		TargetGun->ApplyRuntimeState(WeaponAttributeSet, *SavedRuntimeState);
		return;
	}

	// 소유 무기 런타임 정보 목록에서 태그에 해당하는 런타임 정보를 찾지 못했다면
	// 런타임 정보 생성
	const FWeaponRuntimeState DefaultRuntimeState = TargetGun->MakeDefaultRuntimeState();
	// 소유 무기 런타임 정보 목록에 등록
	CharacterCarriedWeaponRuntimeStateMap.Add(WeaponTag, DefaultRuntimeState);
	// 장착하려는 무기에 런타임 정보 적용
	TargetGun->ApplyRuntimeState(WeaponAttributeSet, DefaultRuntimeState);
}

void UPawnCombatComponent::ApplyWeaponAbilities(ABaseWeapon* Weapon) const
{
	const ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	UMGSAbilitySystemComponent* ASC = OwningCharacter ? OwningCharacter->GetMGSAbilitySystemComponent() : nullptr;
	if (!ASC || !Weapon)
	{
		return;
	}

	if (Cast<APlayerCharacter>(OwningCharacter))
	{
		TArray<FGameplayAbilitySpecHandle> GrantedHandles;
		// ASC에 장착하려는 무기가 갖고 있는 Ability들 부여
		ASC->GrantWeaponAbilities(Weapon->GetWeaponData().WeaponAbilities, 1, GrantedHandles);
		// 장착하려는 무기에 GA Spec Handle 목록 저장
		Weapon->AssignGrantedAbilitySpecHandles(GrantedHandles);
		// 장착하려는 무기가 갖고 있는 IMC 매핑
		AddWeaponInputMappingContext(Weapon);
	}
	
	const FGameplayTag& EquipTag = Weapon->GetWeaponData().WeaponEquippedTag;
	if (EquipTag.IsValid())
	{
		// ASC에 무기가 갖고 있는 태그(주무기, 보조무기) 부여
		ASC->AddLooseGameplayTag(EquipTag);
	}
}

void UPawnCombatComponent::AddWeaponInputMappingContext(const ABaseWeapon* Weapon) const
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
		// 무기 IMC는 기본 IMC보다 높은 우선순위로 적용
		InputSubsystem->AddMappingContext(Weapon->GetWeaponData().WeaponInputMappingContext, 1);
	}
}

bool UPawnCombatComponent::AttachWeaponToSocket(ABaseWeapon* Weapon, const FName& SocketName) const
{
	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwningPawn());
	if (!OwningCharacter || !Weapon)
	{
		return false;
	}

	// 무기 소켓 장착 사전 설정
	Weapon->SetAsEquippedOrHolsteredWeapon();

	USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh();
	if (!CharacterMesh)
	{
		return false;
	}

	if (SocketName.IsNone())
	{
		// 소켓 이름이 비어있다면 현재 트랜스폼을 유지
		return true;
	}

	// 무기를 소켓에 적재 
	Weapon->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	
	return true;
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

	// 소유 무기 런타임 정보에 현재 장착중인 무기의 런타임 정보 갱신
	CharacterCarriedWeaponRuntimeStateMap.FindOrAdd(CurrentEquippedWeaponTag) = CurrentGun->MakeRuntimeState(WeaponAttributeSet);
}

void UPawnCombatComponent::RemoveWeaponAbilities(ABaseWeapon* Weapon) const
{
	const ABaseCharacter* OwningCharacter = Cast<ABaseCharacter>(GetOwningPawn());
	UMGSAbilitySystemComponent* ASC = OwningCharacter ? OwningCharacter->GetMGSAbilitySystemComponent() : nullptr;
	if (!ASC || !Weapon)
	{
		return;
	}

	if (Cast<APlayerCharacter>(OwningCharacter))
	{
		// 장착 해제하려는 무기가 갖고 있는 IMC 제거
		RemoveWeaponInputMappingContext(Weapon);
		TArray<FGameplayAbilitySpecHandle> SpecHandlesToRemove;
		// 장착 해제하려는 무기가 갖고 있는 GA Spec Handle 목록 가져오기(Move semantic)
		Weapon->ConsumeGrantedAbilitySpecHandles(SpecHandlesToRemove);
		// ASC에 무기가 갖고 있는 태그(주무기, 보조무기) 제거
		ASC->RemoveGrantedWeaponAbilities(SpecHandlesToRemove);
	}
	
	const FGameplayTag& EquipTag = Weapon->GetWeaponData().WeaponEquippedTag;
	if (EquipTag.IsValid())
	{
		// ASC에 무기가 갖고 있는 태그(주무기, 보조무기) 제거
		ASC->RemoveLooseGameplayTag(EquipTag);
	}
}

void UPawnCombatComponent::RemoveWeaponInputMappingContext(const ABaseWeapon* Weapon) const
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
		// 무기 IMC는 제거
		InputSubsystem->RemoveMappingContext(Weapon->GetWeaponData().WeaponInputMappingContext);
	}
}
