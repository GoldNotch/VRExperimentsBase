// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableActor.h"
#include "VRGameModeWithSciViBase.h"
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
		SendBBoxToSciVi();
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
	WriteGazeEverywhere(gaze, hitResult);
}

void AInteractableActor::EndOverlapByEyeTrack()
{
	EndOverlapByEyeTrack_BP();
}

void AInteractableActor::OnExperimentStarted()
{
	SendBBoxToSciVi();
	OnExperimentStarted_BP();
}

void AInteractableActor::OnExperimentFinished()
{
	OnExperimentFinished_BP();
}

void AInteractableActor::OnPressedByTrigger(const FHitResult& hitResult)
{
	OnPressedByTrigger_BP(hitResult);
	WriteActionEverywhere(TEXT("TriggerPressed"));

}

void AInteractableActor::OnReleasedByTrigger(const FHitResult& hitResult)
{
	OnReleasedByTrigger_BP(hitResult);
	WriteActionEverywhere(TEXT("TriggerReleased"));
}

void AInteractableActor::BeginOverlapByController(const FHitResult& hitResult)
{
	BeginOverlapByController_BP(hitResult);
}

void AInteractableActor::InFocusByController(const FHitResult& hitResult)
{
	InFocusByController_BP(hitResult);
	WriteActionEverywhere(TEXT("ControllerFocused"));
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
	WriteActionEverywhere(TEXT("InformantDragItem"));
	OnDrag_BP();
}

void AInteractableActor::OnDrop()
{
	bool IsInDestination = false;
	if (IsValid(DragAndDropDestination) && bActorInDragAndDropDestination)
	{
		WriteActionEverywhere(TEXT("InformantDropItemInCorrectPlace"));
		SetActorLocationAndRotation(DragAndDropDestination->GetActorLocation(),
			DragAndDropDestination->GetActorRotation());
		IsInDestination = true;
	}
	else
	{
		WriteActionEverywhere(TEXT("InformantDropItemInWrongPlace"));
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



//------------------------ service funcs -----------------------

void AInteractableActor::WriteActionEverywhere(const FString& action)
{
	if (auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>())
	{
		if (GM->IsExperimentStarted() && bRecordLogs)
		{
			GM->WriteToExperimentLog(ExperimentLogType::Events, ActionToCSV(action));
			//send to SciVi
			if (auto GM_with_scivi = Cast<AVRGameModeWithSciViBase>(GM))
				GM_with_scivi->SendToSciVi(ActionToJSON(action));
		}
	}
}

inline void AInteractableActor::WriteGazeEverywhere(const FGaze& gaze, const FHitResult& hitResult)
{
	if (auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>())
	{
		if (GM->IsExperimentStarted() && bRecordLogs)
		{
			GM->WriteToExperimentLog(ExperimentLogType::EyeTrack, GazeToCSV(gaze, hitResult));
			if (auto GM_with_scivi = Cast<AVRGameModeWithSciViBase>(GM))
				GM_with_scivi->SendToSciVi(GazeToJSON(gaze, hitResult));
		}
	}
}

inline void AInteractableActor::GetBBox2D(FVector2D& left_top, FVector2D& left_bottom, FVector2D& right_top, FVector2D& right_bottom) const
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

inline void AInteractableActor::SendBBoxToSciVi() const
{
	if (bRecordLogs && bIsVisualizableInSciVi)
		if (auto GM_with_scivi = GetWorld()->GetAuthGameMode<AVRGameModeWithSciViBase>())
			if (GM_with_scivi->IsExperimentStarted())
			{
				FVector2D lt, lb, rt, rb;
				GetBBox2D(lt, lb, rt, rb);
				auto json = FString::Printf(TEXT("\"NewAOIRect\": {"
					"\"AOI\": \"%s\","
					"\"BoundingRect\": [[%f, %f], [%f, %f], [%f, %f], [%f, %f]]"
					"}"),
					*GetName(), lt.X, lt.Y, lb.X, lb.Y, rb.X, rb.Y, rt.X, rt.Y);
				GM_with_scivi->SendToSciVi(json);
			}
}

inline FString AInteractableActor::GazeToJSON(const FGaze& gaze, const FHitResult& hitResult) const
{
	return FString::Printf(TEXT("\"GazeLog\": {"
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
}

inline FString AInteractableActor::GazeToCSV(const FGaze& gaze, const FHitResult& hitResult) const
{
	auto t = FDateTime::Now();
	return FString::Printf(TEXT("%lli;%f;%f;%f;%f;%f;%f;%f;%f;%s;%s\n"),
		t.ToUnixTimestamp() * 1000 + t.GetMillisecond(),
		gaze.origin.X, gaze.origin.Y, gaze.origin.Z,
		gaze.direction.X, gaze.direction.Y, gaze.direction.Z,
		gaze.left_pupil_diameter_mm, gaze.right_pupil_diameter_mm,
		*GetName(), *hitResult.Component->GetName());
}

inline FString AInteractableActor::ActionToJSON(const FString& Action) const
{
	const auto& loc = GetActorLocation();
	return FString::Printf(TEXT("\"ExperimentLog\": {"
		"\"Action\": \"%s\","
		"\"AOI\": \"%s\","
		"\"AOI_Location\": [%f, %f, %f]"
		"}"), *Action, *GetName(), loc.X, loc.Y, loc.Z);
}

inline FString AInteractableActor::ActionToCSV(const FString& Action) const
{
	const auto& loc = GetActorLocation();
	auto t = FDateTime::Now();
	return FString::Printf(TEXT("%lli;%s;%s;%f;%f;%f\n"), t.ToUnixTimestamp() * 1000 + t.GetMillisecond(), *Action, *GetName(), loc.X, loc.Y, loc.Z);
}
