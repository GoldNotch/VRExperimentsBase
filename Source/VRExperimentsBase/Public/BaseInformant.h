// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "BaseInformant.generated.h"

USTRUCT(BlueprintType)
struct VREXPERIMENTSBASE_API FGaze
{
	GENERATED_BODY()
	UPROPERTY()
	FVector origin;
	UPROPERTY()
	FVector direction;
	UPROPERTY()
	float left_pupil_diameter_mm = 0.0f;
	UPROPERTY()
	float left_pupil_openness = 0.0f;
	UPROPERTY()
	float right_pupil_diameter_mm = 0.0f;
	UPROPERTY()
	float right_pupil_openness = 0.0f;
	UPROPERTY()
	float cf = 0.0f;
};

UCLASS()
class VREXPERIMENTSBASE_API ABaseInformant : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseInformant();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetVisibility_MC_Right(bool visibility);
	UFUNCTION(BlueprintCallable)
	void SetVisibility_MC_Left(bool visibility);
	void GetGaze(FGaze& gaze) const;
	UFUNCTION()
	void StartRecording();
	UFUNCTION()
	void StopRecording();
	bool IsRecording() const;

	UFUNCTION(BlueprintCallable)
	virtual void EnableInputEvents(bool enable);
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsInputEventsEnabled() const { return bIsInputEventsEnabled; }


	UFUNCTION(BlueprintImplementableEvent)
	void OnInputEnabled();
	UFUNCTION(BlueprintImplementableEvent)
	void OnInputDisabled();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsWalkingEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMouseControlling = false;
	UFUNCTION(BlueprintCallable)
	void Vibrate(float scale = 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UCameraComponent* CameraComponent;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UArrowComponent* EyeTrackingArrow;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UMotionControllerComponent* MC_Left;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UStaticMeshComponent* MC_Left_Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	class UWidgetInteractionComponent* MC_Left_Interaction_Lazer;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UMotionControllerComponent* MC_Right;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UStaticMeshComponent* MC_Right_Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	class UWidgetInteractionComponent* MC_Right_Interaction_Lazer;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	class UAudioCaptureComponent* AudioCapture;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	class USubmixRecorder* RecorderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadonly)
	UHapticFeedbackEffect_Base* VibrationEffect;

	virtual void OnExperimentStarted();
	virtual void OnExperimentFinished();

protected:
	bool bIsInputEventsEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	class USphereComponent* InteractionCollider;
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnExperimentStarted"))
	void OnExperimentStarted_BP();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnExperimentFinished"))
	void OnExperimentFinished_BP();

	UFUNCTION()
	void OnRTriggerPressed();
	UFUNCTION()
	void OnRTriggerReleased();
	UFUNCTION()
	void OnLTriggerPressed();
	UFUNCTION()
	void OnLTriggerReleased();
	UFUNCTION()
	void CameraMove_LeftRight(float value);
	UFUNCTION()
	void CameraMove_UpDown(float value);
	UFUNCTION()
	void DragActor_RHand();
	UFUNCTION()
	void DropActor_RHand();
	UFUNCTION()
	void DragActor_LHand();
	UFUNCTION()
	void DropActor_LHand();
	float Yaw;
	float CameraPitch;
	//------------ Walking -----------------
	UFUNCTION()
	void Walking_Trajectory();
	UFUNCTION()
	void Walking_Teleport();
	bool bIsWalking = false;
	FVector newPosition;
	float HeightDeviance = 5.0f;
	float FloorHeight = 0.0f;
	//------------ Interaction ----------------
public:
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	float InteractionDistance = 1000.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	float DragDistance = 50.0f;
protected:
	TSet<AActor*> close_actors;
	class AInteractableActor* eye_tracked_actor = nullptr;
	class AInteractableActor* actor_pointed_by_right_mc = nullptr;
	class AInteractableActor* actor_pointed_by_left_mc = nullptr;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	class AInteractableActor* DraggedActor_LHand = nullptr;
	class AInteractableActor* DraggedActor_RHand = nullptr;
	
private:
	void QuitGame();
};
