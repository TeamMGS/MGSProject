#include "AI/BTTask_SetNextPatrolTarget.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "UObject/UnrealType.h"
#include "GameFramework/Actor.h"

UBTTask_SetNextPatrolTarget::UBTTask_SetNextPatrolTarget()
{
	NodeName = TEXT("Set Next Patrol Target");
	PatrolMapPropertyName = TEXT("PatrolIndex");
}

EBTNodeResult::Type UBTTask_SetNextPatrolTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	UObject* PatrolManagerObj = BlackboardComp->GetValueAsObject(PatrolManagerKey.SelectedKeyName);
	if (!PatrolManagerObj)
	{
		return EBTNodeResult::Failed;
	}

	int32 CurrentIndex = BlackboardComp->GetValueAsInt(CurrentPatrolIndexKey.SelectedKeyName);
	if (CurrentIndex <= 0)
	{
		CurrentIndex = 1;
	}

	FMapProperty* MapProp = FindFProperty<FMapProperty>(PatrolManagerObj->GetClass(), PatrolMapPropertyName);
	if (!MapProp)
	{
		return EBTNodeResult::Failed;
	}

	FIntProperty* KeyProp = CastField<FIntProperty>(MapProp->KeyProp);
	FObjectPropertyBase* ValueProp = CastField<FObjectPropertyBase>(MapProp->ValueProp);
	if (!KeyProp || !ValueProp)
	{
		return EBTNodeResult::Failed;
	}

	void* MapContainer = MapProp->ContainerPtrToValuePtr<void>(PatrolManagerObj);
	FScriptMapHelper MapHelper(MapProp, MapContainer);
	const int32 MapNum = MapHelper.Num();
	if (MapNum <= 0)
	{
		return EBTNodeResult::Failed;
	}

	TArray<uint8> KeyStorage;
	KeyStorage.SetNumZeroed(KeyProp->ElementSize);
	KeyProp->InitializeValue(KeyStorage.GetData());
	KeyProp->SetPropertyValue(KeyStorage.GetData(), CurrentIndex);

	const int32 FoundIndex = MapHelper.FindMapIndexWithKey(KeyStorage.GetData());
	KeyProp->DestroyValue(KeyStorage.GetData());
	if (FoundIndex == INDEX_NONE)
	{
		return EBTNodeResult::Failed;
	}

	void* ValuePtr = MapHelper.GetValuePtr(FoundIndex);
	UObject* TargetObj = ValueProp->GetObjectPropertyValue(ValuePtr);
	AActor* TargetActor = Cast<AActor>(TargetObj);
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	BlackboardComp->SetValueAsObject(TargetPatrolActorKey.SelectedKeyName, TargetActor);

	int32 NextIndex = CurrentIndex + 1;
	if (NextIndex > MapNum)
	{
		NextIndex = 1;
	}
	BlackboardComp->SetValueAsInt(CurrentPatrolIndexKey.SelectedKeyName, NextIndex);

	return EBTNodeResult::Succeeded;
}
