// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActor.h"
#include "VRGameModeBase.h"
#include "UI_Blank.h"

//-------------------- Events -------------------

void AInteractableActor::BeginOverlapByEyeTrack()
{
	BeginOverlapByEyeTrack_BP();
}

void AInteractableActor::ProcessEyeTrack(const FGaze& gaze, const FHitResult& hitResult)
{
	ProcessEyeTrack_BP(gaze, hitResult);
	if (bSendLogsToSciVi) {
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		auto json = FString::Printf(TEXT("\"GazeLog\": {"
			"\"origin\": [%f, %f, %f],"
			"\"direction\": [%f, %f, %f],"
			"\"lpdmm\": %F, \"rpdmm\": %F,"
			"\"AOI\": \"%s\""
			"}"),
			gaze.origin.X, gaze.origin.Y, gaze.origin.Z,
			gaze.direction.X, gaze.direction.Y, gaze.direction.Z,
			gaze.left_pupil_diameter_mm, gaze.right_pupil_diameter_mm,
			*GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::EndOverlapByEyeTrack()
{
	EndOverlapByEyeTrack_BP();
}

void AInteractableActor::OnPressedByTrigger(const FHitResult& hitResult)
{
	OnPressedByTrigger_BP(hitResult);
	if (bSendLogsToSciVi) {
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		auto json = FString::Printf(TEXT("\"ControllerLog\": {"
										"\"Action\": \"Press\","
										"\"AOI\": \"%s\""
										"}"),
										*GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::OnReleasedByTrigger(const FHitResult& hitResult)
{
	OnReleasedByTrigger_BP(hitResult);
	if (bSendLogsToSciVi) {
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		auto json = FString::Printf(TEXT("\"ControllerLog\": {"
			"\"Action\": \"Release\","
			"\"AOI\": \"%s\""
			"}"),
			*GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::BeginOverlapByController()
{
	BeginOverlapByController_BP();
}

void AInteractableActor::InFocusByController(const FHitResult& hitResult)
{
	InFocusByController_BP(hitResult);
	if (bSendLogsToSciVi) {
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		auto json = FString::Printf(TEXT("\"ControllerLog\": {"
			"\"Action\": \"Focus\","
			"\"AOI\": \"%s\""
			"}"),
			*GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::EndOverlapByController()
{
	EndOverlapByController_BP();
}

void AInteractableActor::HadCloseToPlayer()
{
	HadCloseToPlayer_BP();
}

void AInteractableActor::HadFarToPlayer()
{
	HadFarToPlayer_BP();
}
