// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActor.h"
#include "VRGameModeBase.h"
#include "Components/BoxComponent.h"
#include "UI_Blank.h"

AInteractableActor::AInteractableActor()
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
	if (IsValid(DragAndDropDestination)) {
		DragAndDropDestination->OnActorBeginOverlap.AddDynamic
		(this, &AInteractableActor::OnBeginOverlapWithDragAndDropDestination);
		DragAndDropDestination->OnActorEndOverlap.AddDynamic
		(this, &AInteractableActor::OnEndOverlapWithDragAndDropDestination);
	}
}

void AInteractableActor::Tick(float DeltaTime)
{
	auto& current_transform = GetActorTransform();
	if (!current_transform.Equals(OldTransform))//actor was replaced
	{
		OldTransform = current_transform;
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM->IsExperimentStarted() && bSendLogsToSciVi && bIsVisualizableInSciVi)
		{
			FVector2D lt, lb, rt, rb;
			GetBBox2D(lt, lb, rt, rb);
			auto json = FString::Printf(TEXT("\"NewAOIRect\": {"
				"\"AOI\": \"%s\","
				"\"BoundingRect\": [[%f, %f], [%f, %f], [%f, %f], [%f, %f]]"
				"}"),
				*GetName(), lt.X, lt.Y, lb.X, lb.Y, rb.X, rb.Y, rt.X, rt.Y);
			GM->SendToSciVi(json);
		}
	}
}

//-------------------- Events -------------------

void AInteractableActor::BeginOverlapByEyeTrack(const FGaze& gaze, const FHitResult& hitResult)
{
	BeginOverlapByEyeTrack_BP(gaze, hitResult);
}

void AInteractableActor::ProcessEyeTrack(const FGaze& gaze, const FHitResult& hitResult)
{
	ProcessEyeTrack_BP(gaze, hitResult);
	auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM->IsExperimentStarted() && bSendLogsToSciVi) {
		auto json = FString::Printf(TEXT("\"GazeLog\": {"
			"\"origin\": [%f, %f, %f],"
			"\"direction\": [%f, %f, %f],"
			"\"lpdmm\": %F, \"rpdmm\": %F,"
			"\"AOI\": \"%s\","
			"\"AOI_Component\": \"%s\""
			"}"),
			gaze.origin.X, gaze.origin.Y, gaze.origin.Z,
			gaze.direction.X, gaze.direction.Y, gaze.direction.Z,
			gaze.left_pupil_diameter_mm, gaze.right_pupil_diameter_mm,
			*GetName(), *hitResult.Component->GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::EndOverlapByEyeTrack()
{
	EndOverlapByEyeTrack_BP();
}

void AInteractableActor::OnExperimentStarted()
{
	if (bSendLogsToSciVi && bIsVisualizableInSciVi)
	{
		FVector2D lt, lb, rt, rb;
		GetBBox2D(lt, lb, rt, rb);
		auto json = FString::Printf(TEXT("\"NewAOIRect\": {"
			"\"AOI\": \"%s\","
			"\"BoundingRect\": [[%f, %f], [%f, %f], [%f, %f], [%f, %f]]"
			"}"),
			*GetName(), lt.X, lt.Y, lb.X, lb.Y, rb.X, rb.Y, rt.X, rt.Y);
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		GM->SendToSciVi(json);
	}
	OnExperimentStarted_BP();
}

void AInteractableActor::OnExperimentFinished()
{
	OnExperimentFinished_BP();
}

void AInteractableActor::OnPressedByTrigger(const FHitResult& hitResult)
{
	OnPressedByTrigger_BP(hitResult);
	auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM->IsExperimentStarted() && bSendLogsToSciVi) {
		auto json = FString::Printf(TEXT("\"ExperimentLog\": {"
			"\"Action\": \"TriggerPressed\","
			"\"AOI\": \"%s\""
			"}"),
			*GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::OnReleasedByTrigger(const FHitResult& hitResult)
{
	OnReleasedByTrigger_BP(hitResult);
	auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM->IsExperimentStarted() && bSendLogsToSciVi) {
		auto json = FString::Printf(TEXT("\"ExperimentLog\": {"
			"\"Action\": \"TriggerReleased\","
			"\"AOI\": \"%s\""
			"}"),
			*GetName());
		GM->SendToSciVi(json);
	}
}

void AInteractableActor::BeginOverlapByController(const FHitResult& hitResult)
{
	BeginOverlapByController_BP(hitResult);
}

void AInteractableActor::InFocusByController(const FHitResult& hitResult)
{
	InFocusByController_BP(hitResult);
	auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM->IsExperimentStarted() && bSendLogsToSciVi) {
		auto json = FString::Printf(TEXT("\"ExperimentLog\": {"
			"\"Action\": \"ControllerFocused\","
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
	bIsDragged = true;
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
	auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GM->IsExperimentStarted() && bSendLogsToSciVi) {
		auto json = FString::Printf(TEXT("\"ExperimentLog\": {"
			"\"Action\": \"InformantDragItem\","
			"\"AOI\": \"%s\""
			"}"),
			*GetName());
		GM->SendToSciVi(json);
	}
	OnDrag_BP();
}

void AInteractableActor::OnDrop()
{
	bool IsInDestination = false;
	if (IsValid(DragAndDropDestination) && bActorInDragAndDropDestination)
	{
		auto GM = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM->IsExperimentStarted() && bSendLogsToSciVi) {
			auto loc = GetActorLocation();
			auto json = FString::Printf(TEXT("\"ExperimentLog\": {"
				"\"Action\": \"InformantDropItemInCorrectPlace\","
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
			auto json = FString::Printf(TEXT("\"ExperimentLog\": {"
				"\"Action\": \"InformantDropItemInWrongPlace\","
				"\"AOI\": \"%s\","
				"\"DropPosition\": [%f, %f, %f]"
				"}"),
				*GetName(), loc.X, loc.Y, loc.Z);
			GM->SendToSciVi(json);
		}
		SetActorTransform(OldTransform);
	}
	OnDrop_BP(IsInDestination);
	bActorInDragAndDropDestination = false;
	bIsDragged = false;
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

void AInteractableActor::GetBBox2D(FVector2D& left_top, FVector2D& left_bottom, FVector2D& right_top, FVector2D& right_bottom)
{
	auto& transform = BoundingBox->GetComponentTransform();
	FVector extent = BoundingBox->GetScaledBoxExtent();
	FVector lt, lb, rt, rb;
	lt.X = lb.X = -extent.X;
	lt.Y = rt.Y = extent.Y;
	rt.X = rb.X = extent.X;
	lb.Y = rb.Y = -extent.Y;
	lt = transform.TransformPositionNoScale(lt);
	lb = transform.TransformPositionNoScale(lb);
	rt = transform.TransformPositionNoScale(rt);
	rb = transform.TransformPositionNoScale(rb);
	left_top.X = lt.X;
	left_top.Y = lt.Y;
	left_bottom.X = lb.X;
	left_bottom.Y = lb.Y;
	right_top.X = rt.X;
	right_top.Y = rt.Y;
	right_bottom.X = rb.X;
	right_bottom.Y = rb.Y;

}