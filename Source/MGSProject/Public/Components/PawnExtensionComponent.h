/*
 * 파일명 : PawnExtensionComponent.h
 * 생성자 : 장대한
 * 생성일 : 2026-03-01
 * 수정자 : 장대한
 * 수정일 : 2026-03-02
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnExtensionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MGSPROJECT_API UPawnExtensionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	template<class T>
	T* GetOwningPawn() const
	{
		// 컴파일에서 에러가 발생하면 해당 텍스트표시 T에 파라미터는 APawn에서 파생되어야 한다.
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' Template Parameter get GetPawn must be derived from APawn");
		return CastChecked<T>(GetOwner());
	}
	
	APawn* GetOwningPawn() const
	{
		return GetOwningPawn<APawn>();
	}
	
	template<class T>
	T* GetOwningController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' Template Parameter get GetController must be derived from AController");
		return GetOwningPawn<APawn>()->GetController();
	}
	
	AController* GetOwningController() const
	{
		return GetOwningController<AController>();
	}
	
};
