/*
 * 파일명 : MissionTarget.cpp
 */
#include "Mission/MissionTarget.h"

#include "Kismet/GameplayStatics.h"
#include "Mission/ExtractionPoint.h"
#include "Components/SphereComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/WidgetComponent.h"
#include "Characters/Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/Player/MGSPlayerController.h"
#include "Sound/SoundCue.h"

AMissionTarget::AMissionTarget()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	// 1. 기본 메쉬
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	// 2. 넓은 범위 콜리전 (InteractDetectionCollider)
	InteractDetectionCollider = CreateDefaultSubobject<USphereComponent>(TEXT("InteractDetectionCollider"));
	InteractDetectionCollider->SetupAttachment(Mesh);
	InteractDetectionCollider->SetSphereRadius(500.0f); // 넓은 범위 기본값 5미터
	InteractDetectionCollider->SetCollisionProfileName(TEXT("Trigger"));

	// 3. 빌보드 컴포넌트 (Inner_icon)
	Inner_icon = CreateDefaultSubobject<UBillboardComponent>(TEXT("Inner_icon"));
	Inner_icon->SetupAttachment(Mesh);
	Inner_icon->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // 물체 위쪽 1미터
	Inner_icon->SetVisibility(false); // 처음에는 숨김

	// 4. 좁은 범위 콜리전 (UITriggerCollider)
	UITriggerCollider = CreateDefaultSubobject<USphereComponent>(TEXT("UITriggerCollider"));
	UITriggerCollider->SetupAttachment(Mesh);
	UITriggerCollider->SetSphereRadius(200.0f); // 좁은 범위 기본값 2미터
	UITriggerCollider->SetCollisionProfileName(TEXT("Trigger"));

	// 5. 위젯 컴포넌트 (UI_Detail)
	UI_Detail = CreateDefaultSubobject<UWidgetComponent>(TEXT("UI_Detail"));
	UI_Detail->SetupAttachment(Mesh);
	UI_Detail->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f)); // 빌보드보다 조금 위
	UI_Detail->SetWidgetSpace(EWidgetSpace::Screen); // 화면 공간 UI
	UI_Detail->SetVisibility(false); // 처음에는 숨김
}

void AMissionTarget::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);

	// 오버랩 이벤트 바인딩
	InteractDetectionCollider->OnComponentBeginOverlap.AddDynamic(this, &AMissionTarget::OnDetectionBeginOverlap);
	InteractDetectionCollider->OnComponentEndOverlap.AddDynamic(this, &AMissionTarget::OnDetectionEndOverlap);

	UITriggerCollider->OnComponentBeginOverlap.AddDynamic(this, &AMissionTarget::OnUITriggerBeginOverlap);
	UITriggerCollider->OnComponentEndOverlap.AddDynamic(this, &AMissionTarget::OnUITriggerEndOverlap);
	
}

void AMissionTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Tick이 켜져 있다는 것은 상호작용 중이라는 뜻입니다.
	CurrentHoldTime += DeltaTime;

	// UI 업데이트 (위젯에 Percent 전달)
	UpdateInteractionUI();
	
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Interaction Started..."));
}

// 넓은 범위 진입: 아이콘 표시
void AMissionTarget::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APlayerCharacter>(OtherActor))
	{
		Inner_icon->SetVisibility(true);
	}
}

// 넓은 범위 이탈: 아이콘 숨김
void AMissionTarget::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APlayerCharacter>(OtherActor))
	{
		Inner_icon->SetVisibility(false);
	}
}

// 좁은 범위 진입: 아이콘 숨기고 디테일 UI 표시
void AMissionTarget::OnUITriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<APlayerCharacter>(OtherActor))
	{
		Inner_icon->SetVisibility(false);
		UI_Detail->SetVisibility(true);
	}
}

// 좁은 범위 이탈: 아이콘 다시 보여주고 디테일 UI 숨김
void AMissionTarget::OnUITriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<APlayerCharacter>(OtherActor))
	{
		Inner_icon->SetVisibility(true);
		UI_Detail->SetVisibility(false);
	}
}

void AMissionTarget::UpdateInteractionUI()
{
	if (!UI_Detail || !UI_Detail->GetWidget()) return;

	float Percent = FMath::Clamp(CurrentHoldTime / TargetInteractionTime, 0.0f, 1.0f);

	// 위젯의 블루프린트 함수 호출 방식 (가장 일반적)
	if (UUserWidget* ProgressBarWidget = UI_Detail->GetWidget())
	{
		// 블루프린트에 "UpdateProgress"라는 함수가 있다고 가정
		UFunction* Func = ProgressBarWidget->FindFunction(FName("UpdateProgress"));
		if (Func)
		{
			ProgressBarWidget->ProcessEvent(Func, &Percent);
		}
	}
}

void AMissionTarget::OnInteractionStarted(AActor* Interactor)
{
	CurrentHoldTime = 0.0f;
	SetActorTickEnabled(true);
	
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Interaction Started..."));
}

void AMissionTarget::OnInteractionSucceeded(AActor* Interactor)
{
	SetActorTickEnabled(false);
	
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Mission Target Destroyed! Mission Complete!"));

	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExtractionPoint::StaticClass(), OutActors);

	for (AActor* Actor : OutActors)
	{
		if (AExtractionPoint* EP = Cast<AExtractionPoint>(Actor))
		{
			EP->ActivateExtraction();
		}
	}
	AMGSPlayerController* MyPC = Cast<AMGSPlayerController>(GetWorld()->GetFirstPlayerController());
	if (MyPC != nullptr)
	{
		// 4. 플레이어 컨트롤러에 만들어둔 ChangeBGM 함수 호출!
		// (주의: 탈출지 헤더(.h)에 재생할 BGM 변수(ExtractionBGM 등)를 미리 선언하고 에디터에서 할당해둬야 합니다)
		MyPC->ChangeBGM(MyPC->ContainedBGM); 
	}
	
	Destroy();
}

void AMissionTarget::OnInteractionCanceled(AActor* Interactor)
{
	SetActorTickEnabled(false);
	CurrentHoldTime = 0.0f;
	UpdateInteractionUI(); // UI 초기화 (0%로)
	
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Interaction Canceled."));
}
