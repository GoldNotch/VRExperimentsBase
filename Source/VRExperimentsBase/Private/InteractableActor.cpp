// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActor.h"
#include "VRGameModeWithSciViBase.h"
#include "Components/BoxComponent.h"
#include "UI_Blank.h"

AInteractableActor::AInteractableActor(const FObjectInitializer& ObjectInitializer)
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	BoundingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BBox"));
	BoundingBox->SetupAttachment(RootComponent);
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	bIsDraggable = bIsDraggable && IsRootComponentMovable();
	OldTransform = GetActorTransform();
}

void AInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//-------------------- Events -------------------

void AInteractableActor::BeginOverlapByEyeTrack(const FGaze& gaze)
{
	BeginOverlapByEyeTrack_BP(gaze);
}

void AInteractableActor::ProcessEyeTrack(const FGaze& gaze)
{
	ProcessEyeTrack_BP(gaze);
	//WriteGazeEverywhere(gaze, hitResult);
}

void AInteractableActor::EndOverlapByEyeTrack()
{
	EndOverlapByEyeTrack_BP();
}

void AInteractableActor::OnExperimentStarted()
{
	//SendBBoxToSciVi();
	OnExperimentStarted_BP();
}

void AInteractableActor::OnExperimentFinished()
{
	OnExperimentFinished_BP();
}

void AInteractableActor::OnPressedByTrigger(const FHitResult& hitResult)
{
	OnPressedByTrigger_BP(hitResult);
	//WriteActionEverywhere(TEXT("TriggerPressed"));

}

void AInteractableActor::OnReleasedByTrigger(const FHitResult& hitResult)
{
	OnReleasedByTrigger_BP(hitResult);
	//WriteActionEverywhere(TEXT("TriggerReleased"));
}

void AInteractableActor::BeginOverlapByController(const FHitResult& hitResult)
{
	BeginOverlapByController_BP(hitResult);
}

void AInteractableActor::InFocusByController(const FHitResult& hitResult)
{
	InFocusByController_BP(hitResult);
	//WriteActionEverywhere(TEXT("ControllerFocused"));
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
	if (IsValid(DragAndDropDestination)) {
		DragAndDropDestination->OnActorBeginOverlap.AddDynamic
		(this, &AInteractableActor::OnBeginOverlapWithDragAndDropDestination);
		DragAndDropDestination->OnActorEndOverlap.AddDynamic
		(this, &AInteractableActor::OnEndOverlapWithDragAndDropDestination);
	}
	bIsDragged = true;
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	//WriteActionEverywhere(TEXT("InformantDragItem"));
	OnDrag_BP();
}

void AInteractableActor::OnDrop()
{
	bool IsInDestination = false;
	if (IsValid(DragAndDropDestination) && bActorInDragAndDropDestination)
	{
		SetActorLocationAndRotation(DragAndDropDestination->GetActorLocation(),
			DragAndDropDestination->GetActorRotation());
		OldTransform = GetActorTransform();
		OnMove(OldTransform);
		IsInDestination = true;
	}
	else
	{
		SetActorLocationAndRotation(OldTransform.GetLocation(), OldTransform.GetRotation());
	}
	OnDrop_BP(IsInDestination);
	if (IsValid(DragAndDropDestination)) {
		DragAndDropDestination->OnActorBeginOverlap.Clear();
		DragAndDropDestination->OnActorEndOverlap.Clear();
	}
	bActorInDragAndDropDestination = false;
	bIsDragged = false;
}

void AInteractableActor::OnMove(const FTransform& new_transform)
{
	OnMove_BP(new_transform);
}

void AInteractableActor::OnBeginOverlapWithDragAndDropDestination(AActor* OverlappedActor, AActor* OtherActor)
{
	bActorInDragAndDropDestination = true;
}

void AInteractableActor::OnEndOverlapWithDragAndDropDestination(AActor* OverlappedActor, AActor* OtherActor)
{
	bActorInDragAndDropDestination = false;
}
#if WITH_EDITOR
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
#endif
