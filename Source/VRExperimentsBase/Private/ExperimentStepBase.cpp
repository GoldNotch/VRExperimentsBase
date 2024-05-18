// Fill out your copyright notice in the Description page of Project Settings.


#include "ExperimentStepBase.h"
#include "VRGameModeWithSciViBase.h"

DEFINE_LOG_CATEGORY(LogExperiment);

void AExperimentStepBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogExperiment, Display, TEXT("Experiment step %s started"), *name.ToString());
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(rand(), 5.0f, FColor::Green, FString::Printf(TEXT("%s started"), *name.ToString()));
	if (auto GM_with_scivi = GetWorld()->GetAuthGameMode<AVRGameModeWithSciViBase>())
	{
		auto json = FString::Printf(TEXT("\"StepStarted\": \"%s\""), *name.ToString());
		GM_with_scivi->SendToSciVi(json);
	}
}

void AExperimentStepBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	UE_LOG(LogExperiment, Display, TEXT("Experiment step %s was finished"), *name.ToString());
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(rand(), 5.0f, FColor::Green, FString::Printf(TEXT("%s finished"), *name.ToString()));
	if (auto GM_with_scivi = GetWorld()->GetAuthGameMode<AVRGameModeWithSciViBase>())
	{
		auto json = FString::Printf(TEXT("\"StepFinished\": \"%s\""), *name.ToString());
		GM_with_scivi->SendToSciVi(json);
	}
}
