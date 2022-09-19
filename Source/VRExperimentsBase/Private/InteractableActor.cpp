// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActor.h"
#include "VRGameModeBase.h"
#include "UI_Blank.h"

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(DragAndDropDestination)) {
		DragAndDropDestination->OnActorBeginOverlap.AddDynamic
		(this, &AInteractableActor::OnBeginOverlapWithDragAndDropDestination);
		DragAndDropDestination->OnActorEndOverlap.AddDynamic
		(this, &AInteractableActor::OnEndOverlapWithDragAndDropDestination);
	}
}

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

//---------------------- Drag & Drop -----------------------------
void AInteractableActor::OnDrag()
{
	TransformBeforeDrag = GetActorTransform();
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	if (bSendLogsToSciVi) {
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		auto json = FString::Printf(TEXT("\"ControllerLog\": {"
			"\"Action\": \"DragItem\","
			"\"AOI\": \"%s\""
			"}"),
			*GetName());
		GM->SendToSciVi(json);
	}
	OnDrag_BP();
}

void AInteractableActor::OnDrop()
{
	bool IsInDestination = false;;
	if (IsValid(DragAndDropDestination) && bActorInDragAndDropDestination)
	{
		if (bSendLogsToSciVi) {
			auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
			auto loc = GetActorLocation();
			auto json = FString::Printf(TEXT("\"ControllerLog\": {"
				"\"Action\": \"DropItemInCorrectPlace\","
				"\"AOI\": \"%s\","
				"\"DropPosition\": [%f, %f, %f]"
				"}"),
				*GetName(), loc.X, loc.Y, loc.Z);
			GM->SendToSciVi(json);
		}
		SetActorLocationAndRotation(DragAndDropDestination->GetActorLocation(),
									DragAndDropDestination->GetActorRotation());
		IsInDestination = true;
	}
	else
	{
		if (bSendLogsToSciVi) {
			auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
			auto loc = GetActorLocation();
			auto json = FString::Printf(TEXT("\"ControllerLog\": {"
				"\"Action\": \"DropItemInWrongPlace\","
				"\"AOI\": \"%s\","
				"\"DropPosition\": [%f, %f, %f]"
				"}"),
				*GetName(), loc.X, loc.Y, loc.Z);
			GM->SendToSciVi(json);
		}
		SetActorTransform(TransformBeforeDrag);
	}
	OnDrop_BP(IsInDestination);
	bActorInDragAndDropDestination = false;
}

void AInteractableActor::OnBeginOverlapWithDragAndDropDestination(AActor* OverlappedActor, AActor* OtherActor)
{
	bActorInDragAndDropDestination = true;
}

void AInteractableActor::OnEndOverlapWithDragAndDropDestination(AActor* OverlappedActor, AActor* OtherActor)
{
	bActorInDragAndDropDestination = false;
}

void AInteractableActor::PreEditChange(FProperty* PropertyAboutToChange)
{
	if (PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(AInteractableActor, DragAndDropDestination))
	{
		if (IsValid(DragAndDropDestination)) 
		{
			DragAndDropDestination->OnActorBeginOverlap.Clear();
			DragAndDropDestination->OnActorEndOverlap.Clear();
		}
		bActorInDragAndDropDestination = false;
	}
}

void AInteractableActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AInteractableActor, DragAndDropDestination)) 
	{
		if (IsValid(DragAndDropDestination)) {
			DragAndDropDestination->OnActorBeginOverlap.AddDynamic
				(this, &AInteractableActor::OnBeginOverlapWithDragAndDropDestination);
			DragAndDropDestination->OnActorEndOverlap.AddDynamic
				(this, &AInteractableActor::OnEndOverlapWithDragAndDropDestination);
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
