/*
 * 파일명 : MissionTarget.h
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/MGSInteractableInterface.h"
#include "MissionTarget.generated.h"

class USphereComponent;
class UBillboardComponent;
class UWidgetComponent;

UCLASS()
class MGSPROJECT_API AMissionTarget : public AActor, public IMGSInteractableInterface
{
	GENERATED_BODY()
	
public:
	AMissionTarget();
	
	// IMGSInteractableInterface 구현
	virtual void OnInteractionStarted(AActor* Interactor) override;
	virtual void OnInteractionSucceeded(AActor* Interactor) override;
	virtual void OnInteractionCanceled(AActor* Interactor) override;
	virtual float GetInteractionDuration() const override { return 2.0f; }

protected:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	// 컴포넌트 오버랩 이벤트 핸들러
	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnUITriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnUITriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// 기본 메쉬
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// 넓은 범위 콜리전 (아이콘 표시용)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> InteractDetectionCollider;

	// 빌보드 컴포넌트 (아이콘)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBillboardComponent> Inner_icon;

	// 좁은 범위 콜리전 (디테일 UI 표시용)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> UITriggerCollider;

	// 위젯 컴포넌트 (디테일 UI)
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWidgetComponent> UI_Detail;
	
private:
	float CurrentHoldTime = 0.0f;
	float TargetInteractionTime = 2.0f; // 목표 시간

	// 위젯의 프로그레스 바를 업데이트하는 헬퍼 함수
	void UpdateInteractionUI();

};
