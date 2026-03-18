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
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: BlackboardComp is null"));
		return EBTNodeResult::Failed;
	}

	UObject* PatrolManagerObj = BlackboardComp->GetValueAsObject(PatrolManagerKey.SelectedKeyName);
	if (!PatrolManagerObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: PatrolManager is null"));
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("BTTask_SetNextPatrolTarget: PatrolManager class = %s"), *PatrolManagerObj->GetClass()->GetName());

	AActor* CurrentTargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetPatrolActorKey.SelectedKeyName));

	int32 CurrentIndex = BlackboardComp->GetValueAsInt(CurrentPatrolIndexKey.SelectedKeyName);

	FMapProperty* MapProp = FindFProperty<FMapProperty>(PatrolManagerObj->GetClass(), PatrolMapPropertyName);
	if (!MapProp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: Map property '%s' not found"), *PatrolMapPropertyName.ToString());
		for (TFieldIterator<FProperty> It(PatrolManagerObj->GetClass()); It; ++It)
		{
			if (const FMapProperty* CandidateMap = CastField<FMapProperty>(*It))
			{
				UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: Found map property '%s' (Key=%s, Value=%s)"),
					*CandidateMap->GetName(),
					*CandidateMap->KeyProp->GetClass()->GetName(),
					*CandidateMap->ValueProp->GetClass()->GetName());
			}
		}
		return EBTNodeResult::Failed;
	}

	FIntProperty* KeyProp = CastField<FIntProperty>(MapProp->KeyProp);
	FObjectPropertyBase* ValueProp = CastField<FObjectPropertyBase>(MapProp->ValueProp);
	if (!KeyProp || !ValueProp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: Map key/value type mismatch"));
		return EBTNodeResult::Failed;
	}

	void* MapContainer = MapProp->ContainerPtrToValuePtr<void>(PatrolManagerObj);
	FScriptMapHelper MapHelper(MapProp, MapContainer);
	const int32 MapNum = MapHelper.Num();
	if (MapNum <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: Map is empty"));
		return EBTNodeResult::Failed;
	}

	TArray<int32> MapKeys;
	MapKeys.Reserve(MapNum);
	for (int32 MapIndex = 0; MapIndex < MapHelper.GetMaxIndex(); ++MapIndex)
	{
		if (!MapHelper.IsValidIndex(MapIndex))
		{
			continue;
		}

		const void* KeyPtr = MapHelper.GetKeyPtr(MapIndex);
		MapKeys.Add(KeyProp->GetPropertyValue(KeyPtr));
	}
	MapKeys.Sort();

	auto GetActorByKey = [&](int32 Key) -> AActor*
	{
		TArray<uint8> LocalKeyStorage;
		LocalKeyStorage.SetNumZeroed(KeyProp->ElementSize);
		KeyProp->InitializeValue(LocalKeyStorage.GetData());
		KeyProp->SetPropertyValue(LocalKeyStorage.GetData(), Key);

		const int32 LocalFoundIndex = MapHelper.FindMapIndexWithKey(LocalKeyStorage.GetData());
		KeyProp->DestroyValue(LocalKeyStorage.GetData());
		if (LocalFoundIndex == INDEX_NONE)
		{
			return nullptr;
		}

		void* LocalValuePtr = MapHelper.GetValuePtr(LocalFoundIndex);
		UObject* LocalTargetObj = ValueProp->GetObjectPropertyValue(LocalValuePtr);
		return Cast<AActor>(LocalTargetObj);
	};

	auto GetNextKey = [&](int32 Key) -> int32
	{
		if (MapKeys.Num() == 0)
		{
			return INDEX_NONE;
		}

		if (Key <= 0)
		{
			return MapKeys[0];
		}

		const int32 KeyIndex = MapKeys.IndexOfByKey(Key);
		if (KeyIndex == INDEX_NONE)
		{
			return MapKeys[0];
		}

		const int32 NextKeyIndex = (KeyIndex + 1) % MapKeys.Num();
		return MapKeys[NextKeyIndex];
	};

	if (CurrentIndex <= 0)
	{
		if (CurrentTargetActor)
		{
			int32 FoundKey = 0;
			for (int32 MapIndex = 0; MapIndex < MapHelper.GetMaxIndex(); ++MapIndex)
			{
				if (!MapHelper.IsValidIndex(MapIndex))
				{
					continue;
				}

				void* ValuePtr = MapHelper.GetValuePtr(MapIndex);
				UObject* ValueObj = ValueProp->GetObjectPropertyValue(ValuePtr);
				if (ValueObj == CurrentTargetActor)
				{
					const void* KeyPtr = MapHelper.GetKeyPtr(MapIndex);
					FoundKey = KeyProp->GetPropertyValue(KeyPtr);
					break;
				}
			}

			const int32 NextKey = GetNextKey(FoundKey);
			if (NextKey != INDEX_NONE)
			{
				AActor* NextTargetActor = GetActorByKey(NextKey);
				if (NextTargetActor)
				{
					BlackboardComp->SetValueAsObject(TargetPatrolActorKey.SelectedKeyName, NextTargetActor);
					BlackboardComp->SetValueAsInt(CurrentPatrolIndexKey.SelectedKeyName, NextKey);
					UE_LOG(LogTemp, Log, TEXT("BTTask_SetNextPatrolTarget: Initialized from EQS target. NextKey=%d"), NextKey);
					return EBTNodeResult::Succeeded;
				}
				UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: EQS 이후 다음 타겟(Key: %d)을 찾을 수 없습니다."), NextKey);
				return EBTNodeResult::Failed;
			}
		}

		const int32 NextKey = GetNextKey(0);
		if (NextKey == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: No valid keys in map"));
			return EBTNodeResult::Failed;
		}

		AActor* FallbackActor = GetActorByKey(NextKey);
		if (!FallbackActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: Fallback key %d not found"), NextKey);
			return EBTNodeResult::Failed;
		}

		BlackboardComp->SetValueAsObject(TargetPatrolActorKey.SelectedKeyName, FallbackActor);
		BlackboardComp->SetValueAsInt(CurrentPatrolIndexKey.SelectedKeyName, NextKey);
		UE_LOG(LogTemp, Log, TEXT("BTTask_SetNextPatrolTarget: Fallback to key %d"), NextKey);
		return EBTNodeResult::Succeeded;
	}

	const int32 NextIndex = GetNextKey(CurrentIndex);
	if (NextIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: No valid keys in map"));
		return EBTNodeResult::Failed;
	}

	// 2. 계산된 다음 인덱스를 기반으로 다음 목표 액터를 가져옵니다.
	AActor* NextTargetActor = GetActorByKey(NextIndex);
	if (!NextTargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("BTTask_SetNextPatrolTarget: Key %d not found"), NextIndex);
		return EBTNodeResult::Failed;
	}

	// 3. 블랙보드에 다음 타겟과 다음 인덱스를 정상적으로 갱신합니다.
	BlackboardComp->SetValueAsObject(TargetPatrolActorKey.SelectedKeyName, NextTargetActor);
	BlackboardComp->SetValueAsInt(CurrentPatrolIndexKey.SelectedKeyName, NextIndex);
	
	UE_LOG(LogTemp, Log, TEXT("BTTask_SetNextPatrolTarget: Set target to NextIndex=%d"), NextIndex);

	return EBTNodeResult::Succeeded;
}
