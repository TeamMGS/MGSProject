#include "AI/Core/AICoreAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

AAICoreAIController::AAICoreAIController()
{
	UAIPerceptionComponent* LocalPerceptionComponent = GetPerceptionComponent();
	if (!LocalPerceptionComponent)
	{
		LocalPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
		SetPerceptionComponent(*LocalPerceptionComponent);
	}
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));

	ConfigurePerception();
}

void AAICoreAIController::ConfigurePerception()
{
	UAIPerceptionComponent* LocalPerceptionComponent = GetPerceptionComponent();
	if (!LocalPerceptionComponent || !SightConfig || !HearingConfig)
	{
		return;
	}

	SightConfig->SightRadius = 2000.0f;
	SightConfig->LoseSightRadius = 2500.0f;
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	HearingConfig->HearingRange = 1500.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

	LocalPerceptionComponent->ConfigureSense(*SightConfig);
	LocalPerceptionComponent->ConfigureSense(*HearingConfig);
	LocalPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}
