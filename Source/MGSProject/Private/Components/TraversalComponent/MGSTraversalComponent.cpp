/*
 * 파일명 : MGSTraversalComponent.cpp
 * 생성자 : 김동석
 * 생성일 : 2026-03-13
 * 수정자 : 김동석
 * 수정일 : 2026-03-13
 */

#include "Components/TraversalComponent/MGSTraversalComponent.h"

UMGSTraversalComponent::UMGSTraversalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UMGSTraversalComponent::CheckTraversal(struct FMGSTraversalChooserInputs& OutInputs)
{
	return false;
}


void UMGSTraversalComponent::BeginPlay()
{
	Super::BeginPlay();
}

