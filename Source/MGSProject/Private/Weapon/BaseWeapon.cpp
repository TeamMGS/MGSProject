/*
 * 파일명 : BaseWeapon.cpp
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-10
 */

#include "Weapon/BaseWeapon.h"

#include "Characters/Player/PlayerCharacter.h"
#include "Components/Combat/PlayerCombatComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

ABaseWeapon::ABaseWeapon()
{
	// 성능을 위해 Tick을 사용하지 않습니다.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(GetRootComponent());
	WeaponCollisionBox->SetBoxExtent(FVector(60.0f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WeaponCollisionBox->SetGenerateOverlapEvents(true);
	WeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleWeaponOverlapBegin);
	WeaponCollisionBox->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleWeaponOverlapEnd);

	// 월드에 배치/드랍된 무기는 기본적으로 물리 적용 상태로 시작합니다.
	SetAsWorldDroppedWeapon();
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	// BP 직렬화 값이 C++ 기본값을 덮어써도 런타임에서 상태를 강제 적용합니다.
	// Owner가 있어도 캐릭터에 실제 부착된 상태가 아니면 드랍 무기로 취급해야 합니다.
	const AActor* OwnerActor = GetOwner();
	const bool bIsAttachedToOwner = OwnerActor && (GetAttachParentActor() == OwnerActor || IsAttachedTo(OwnerActor));
	if (bIsAttachedToOwner)
	{
		SetAsEquippedOrHolsteredWeapon();
		return;
	}

	SetAsWorldDroppedWeapon();
}

void ABaseWeapon::SetAsWorldDroppedWeapon()
{
	if (!WeaponMesh)
	{
		return;
	}

	// 물리 킴
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	WeaponMesh->SetSimulatePhysics(true);
	// 중력 킴
	WeaponMesh->SetEnableGravity(true);
	SetActorEnableCollision(true);
	WeaponCollisionBox->SetBoxExtent(FVector(60.0f));
	WeaponCollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WeaponCollisionBox->SetGenerateOverlapEvents(true);
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponCollisionBox->UpdateOverlaps();
}

void ABaseWeapon::SetAsEquippedOrHolsteredWeapon()
{
	if (!WeaponMesh)
	{
		return;
	}

	if (WeaponCollisionBox && WeaponCollisionBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		TArray<AActor*> OverlappingActors;
		WeaponCollisionBox->GetOverlappingActors(OverlappingActors, APlayerCharacter::StaticClass());
		for (AActor* OverlappingActor : OverlappingActors)
		{
			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OverlappingActor);
			UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter ? PlayerCharacter->GetPlayerCombatComponent() : nullptr;
			if (PlayerCombatComponent)
			{
				PlayerCombatComponent->NotifyDroppedWeaponOverlapEnd(this);
			}
		}
	}

	// 물리 끔
	WeaponMesh->SetSimulatePhysics(false);
	// 중력 끔
	WeaponMesh->SetEnableGravity(false);
	// 콜리전 제거
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorEnableCollision(false);
}

void ABaseWeapon::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles)
{
	GrantedAbilitySpecHandles = SpecHandles;
}

void ABaseWeapon::ConsumeGrantedAbilitySpecHandles(TArray<FGameplayAbilitySpecHandle>& OutSpecHandles)
{
	// MoveTemp : rvalue로 캐스팅 후 이동 대입을 타게 함 (내부 버퍼를 복사 없이 넘겨 성능 향상)
	OutSpecHandles = MoveTemp(GrantedAbilitySpecHandles);
	GrantedAbilitySpecHandles.Reset();
}

void ABaseWeapon::SaveDroppedRuntimeState(const FWeaponRuntimeState& RuntimeState)
{
	DroppedRuntimeState = RuntimeState;
	bHasDroppedRuntimeState = true;
}

bool ABaseWeapon::ConsumeDroppedRuntimeState(FWeaponRuntimeState& OutRuntimeState)
{
	if (!bHasDroppedRuntimeState)
	{
		return false;
	}

	OutRuntimeState = DroppedRuntimeState;
	bHasDroppedRuntimeState = false;
	return true;
}

float ABaseWeapon::GetPickupDetectionDebugRadius() const
{
	if (!WeaponCollisionBox)
	{
		return 0.0f;
	}

	return WeaponCollisionBox->GetScaledBoxExtent().GetMax();
}

void ABaseWeapon::HandleWeaponOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter ? PlayerCharacter->GetPlayerCombatComponent() : nullptr;
	if (!PlayerCombatComponent)
	{
		return;
	}

	PlayerCombatComponent->NotifyDroppedWeaponOverlapBegin(this);
}

void ABaseWeapon::HandleWeaponOverlapEnd(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	(void)OverlappedComponent;
	(void)OtherComp;
	(void)OtherBodyIndex;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	UPlayerCombatComponent* PlayerCombatComponent = PlayerCharacter ? PlayerCharacter->GetPlayerCombatComponent() : nullptr;
	if (!PlayerCombatComponent)
	{
		return;
	}

	PlayerCombatComponent->NotifyDroppedWeaponOverlapEnd(this);
}
