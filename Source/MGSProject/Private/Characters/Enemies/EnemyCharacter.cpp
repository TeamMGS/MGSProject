/*
 * 파일명 : EnemyCharacter.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-02
 * 수정자 : 장대한
 * 수정일 : 2026-03-03
 */

#include "Characters/Enemies/EnemyCharacter.h"

#include "Components/Combat/EnemyCombatComponent.h"
#include "DataAssets/Startup/DA_StartupBase.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ASC/MGSAbilitySystemComponent.h"
#include "GAS/AttributeSets/CharacterAttributeSet.h"

AEnemyCharacter::AEnemyCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	//GetCharacterMovement()->RotationRate = FRotator();
	//GetCharacterMovement()->MaxWalkSpeed = ;
	//GetCharacterMovement()->BrakingDecelerationWalking = ;

	MGSAbilitySystemComponent = CreateDefaultSubobject<UMGSAbilitySystemComponent>(TEXT("MGSAbilitySystemComponent"));
	CharacterAttributeSet = CreateDefaultSubobject<UCharacterAttributeSet>(TEXT("CharacterAttributeSet"));
	
	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));
}

UPawnCombatComponent* AEnemyCharacter::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}

UMGSAbilitySystemComponent* AEnemyCharacter::GetMGSAbilitySystemComponent() const
{
	return MGSAbilitySystemComponent;
}

UCharacterAttributeSet* AEnemyCharacter::GetCharacterAttributeSet() const
{
	return CharacterAttributeSet;
}

void AEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (MGSAbilitySystemComponent)
	{
		MGSAbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
	InitEnemyStartupData();
}

void AEnemyCharacter::InitEnemyStartupData()
{
	if (StartupData.IsNull())
	{
		return;
	}
	
	// Startup Data를 비동기 로딩으로 불러온다.
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		StartupData.ToSoftObjectPath(),
		FStreamableDelegate::CreateLambda(
			[this]()
			{
				if (UDA_StartupBase* LoadedData = StartupData.Get())
				{
					if (UMGSAbilitySystemComponent* ASC = GetMGSAbilitySystemComponent())
					{
						LoadedData->GiveToAbilitySystemComponent(ASC);
					}
				}
			}
		)
	);
}
