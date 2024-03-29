// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseInformant.h"
#include "Engine/TriggerBox.h"
#include "InteractableActor.generated.h"

UCLASS()
class VREXPERIMENTSBASE_API AInteractableActor : public AActor
{
	GENERATED_BODY()
	
public:
	AInteractableActor(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	class UBoxComponent* BoundingBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRecordLogs = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDraggable = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVisualizableInSciVi = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ATriggerBox* DragAndDropDestination = nullptr;

	virtual void OnExperimentStarted();
	virtual void OnExperimentFinished();
	//trigger interaction
	virtual void OnPressedByTrigger(const FHitResult& hitResult);
	virtual void OnReleasedByTrigger(const FHitResult& hitResult);
	//eye track interaction
	virtual void BeginOverlapByEyeTrack(const FGaze& gaze);
	virtual void ProcessEyeTrack(const FGaze& gaze);
	virtual void EndOverlapByEyeTrack();
	//controller interaction
	virtual void BeginOverlapByController(const FHitResult& hitResult);
	virtual void InFocusByController(const FHitResult& hitResult);
	virtual void EndOverlapByController();
	//distance
	virtual void HadCloseToPlayer();
	virtual void HadFarToPlayer();
	//Drag&Drop
	virtual void OnDrag();
	virtual void OnDrop();
	virtual void OnMove(const FTransform& new_transform);

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnExperimentStarted"))
	void OnExperimentStarted_BP();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnExperimentFinished"))
	void OnExperimentFinished_BP();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "HadEyeFocus"))
	void BeginOverlapByEyeTrack_BP(const FGaze& gaze);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EyeTrackTick"))
	void ProcessEyeTrack_BP(const FGaze& gaze);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "LostEyeFocus"))
	void EndOverlapByEyeTrack_BP();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRTriggerPressed"))
	void OnPressedByTrigger_BP(const FHitResult& hitResult);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnRTriggerReleased"))
	void OnReleasedByTrigger_BP(const FHitResult& hitResult);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "HadControllerFocus"))
	void BeginOverlapByController_BP(const FHitResult& hitResult);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ControllerFocusTick"))
	void InFocusByController_BP(const FHitResult& hitResult);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "LostControllerFocus"))
	void EndOverlapByController_BP();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "HadCloseToPlayer"))
	void HadCloseToPlayer_BP();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "HadFarToPlayer"))
	void HadFarToPlayer_BP();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDrag"))
	void OnDrag_BP();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDrop"))
	void OnDrop_BP(bool IsInDestination);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnMove"))
	void OnMove_BP(const FTransform& new_transform);

	UFUNCTION()
	void OnBeginOverlapWithDragAndDropDestination(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void OnEndOverlapWithDragAndDropDestination(AActor* OverlappedActor, AActor* OtherActor);
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	FTransform OldTransform;
	bool bIsDragged = false;
	bool bActorInDragAndDropDestination = false;

//	UFUNCTION(BlueprintCallable)
//	void WriteActionEverywhere(const FString& action);
//	virtual FString GazeToJSON(const FGaze& gaze, const FHitResult& hitResult) const;
//	virtual FString GazeToCSV(const FGaze& gaze, const FHitResult& hitResult) const;
//	virtual FString ActionToJSON(const FString& Action) const;
//	virtual FString ActionToCSV(const FString& Action) const;
//
//private:
//	inline void WriteGazeEverywhere(const FGaze& gaze, const FHitResult& hitResult);
//	inline void GetBBox2D(FVector2D& left_top, FVector2D& left_bottom, FVector2D& right_top, FVector2D& right_bottom) const;
//	inline void SendBBoxToSciVi() const;
};
