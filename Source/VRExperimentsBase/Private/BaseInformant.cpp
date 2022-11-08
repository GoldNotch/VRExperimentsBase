// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseInformant.h"
#include "VRGameModeBase.h"
#include "InteractableActor.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ForceFeedbackComponent.h"
#include <AudioCaptureComponent.h>
#include "SubmixRecorder.h"
#include "SRanipal_API_Eye.h"
#include "SRanipalEye_Core.h"
#include "SRanipalEye_FunctionLibrary.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "XRMotionControllerBase.h"
#include "Misc/Base64.h"
#include "Kismet/GameplayStatics.h"//for PredictProjectilePath

// Sets default values
ABaseInformant::ABaseInformant()
{
	static ConstructorHelpers::FObjectFinder<USoundSubmix> CaptureSubmixAsset(TEXT("SoundSubmix'/Game/CaptureSubmix.CaptureSubmix'"));
	//static ConstructorHelpers::FObjectFinder<UHapticFeedbackEffect_Base> VibrationEffectAsset(TEXT("HapticFeedbackCurve'/Game/VibrationEffect.VibrationEffect'"));

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.001f;//every millisecond
	// Take control of the default player
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;
	//create components
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(0, 0, 80));

	EyeTrackingArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EyeTracking"));
	EyeTrackingArrow->SetupAttachment(RootComponent);
	EyeTrackingArrow->SetHiddenInGame(true, true);//if you want to see eye tracking, then set first arg to false

	MC_Left = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MC_Left"));
	MC_Left->SetupAttachment(RootComponent);
	MC_Left->MotionSource = FXRMotionControllerBase::LeftHandSourceId;
	MC_Left_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MCLeft_Mesh"));
	MC_Left_Mesh->SetupAttachment(MC_Left);
	MC_Left_Interaction_Lazer = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("MCLeft_Lazer"));
	MC_Left_Interaction_Lazer->SetupAttachment(MC_Left);
	MC_Left_Interaction_Lazer->bShowDebug = true;
	MC_Left_Interaction_Lazer->DebugColor = FColor::Green;
	MC_Left_Interaction_Lazer->InteractionDistance = InteractionDistance;

	MC_Right = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MC_Right"));
	MC_Right->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	MC_Right->SetupAttachment(RootComponent);
	MC_Right_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MCRight_Mesh"));
	MC_Right_Mesh->SetupAttachment(MC_Right);
	MC_Right_Interaction_Lazer = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("MCRight_Lazer"));
	MC_Right_Interaction_Lazer->SetupAttachment(MC_Right);
	MC_Right_Interaction_Lazer->bShowDebug = true;
	MC_Right_Interaction_Lazer->DebugColor = FColor::Green;
	MC_Right_Interaction_Lazer->InteractionDistance = InteractionDistance;

	AudioCapture = CreateDefaultSubobject<UAudioCaptureComponent>(TEXT("AudioCapture"));
	AudioCapture->SetupAttachment(RootComponent);
	AudioCapture->SoundSubmix = CaptureSubmixAsset.Object;

	RecorderComponent = CreateDefaultSubobject<USubmixRecorder>(TEXT("Recorder"));
	RecorderComponent->SetupAttachment(RootComponent);
	RecorderComponent->NumChannelsToRecord = 1;
	RecorderComponent->SubmixToRecord = CaptureSubmixAsset.Object;

	InteractionCollider = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollider"));
	InteractionCollider->SetupAttachment(RootComponent);
	InteractionCollider->SetSphereRadius(InteractionDistance);
}

// Called when the game starts or when spawned
void ABaseInformant::BeginPlay()
{
	Super::BeginPlay();
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	Yaw = GetActorRotation().Yaw;
	if (!RecorderComponent->SubmixToRecord)
	{
		RecorderComponent->SubmixToRecord = dynamic_cast<USoundSubmix*>(AudioCapture->SoundSubmix);
	}

	//Create wav file for debug
	/*struct WAVHEADER
	{
		int chunkId = 0x52494646;
		int chunkSize = 0;
		int format = 0x57415645;
		int subchunk1Id = 0x666d7420;
		int subchunk1Size = 16;
		uint16 audioFormat = 1;
		uint16 numChannels = 1;
		int sampleRate = 48000;
		int byteRate = 96000;
		uint16 blockAlign = 2;
		uint16 bitsPerSample = 16;
		int subchunk2Id = 0x64617461;
		int subchunk2Size = 0;
	};
	WAVHEADER header;
	TArray64<uint8_t> arr((uint8_t*)&header, sizeof(header));
	FFileHelper::SaveArrayToFile(arr, TEXT("aud.wav"));*/

	RecorderComponent->OnRecorded = [this](const int16* AudioData, int NumChannels, int NumSamples, int SampleRate)
	{
		auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
		/*TArray64<uint8_t> arr((uint8_t*)AudioData, NumSamples * sizeof(int16));
		FFileHelper::SaveArrayToFile(arr, TEXT("aud.wav"));*/
		auto b64pcm = FBase64::Encode((uint8_t*)AudioData, NumSamples * sizeof(int16));
		auto json = FString::Printf(TEXT("\"WAV\": {\"SampleRate\": %i,"
			"\"PCM\": \"data:audio/wav;base64,%s\"}"), SampleRate, *b64pcm);
		GM->SendToSciVi(json);
	};
	RecorderComponent->OnRecordFinished = [this]()
	{
		auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
		auto json = FString::Printf(TEXT("\"WAV\": \"End\""));
		GM->SendToSciVi(json);
	};
	AudioCapture->Activate();
	FloorHeight = GetActorLocation().Z - (RootComponent->CalcLocalBounds().BoxExtent.Z);

	auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
	if (GM)
		GM->NotifyInformantSpawned(this);
}

// Called every frame
void ABaseInformant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
	//check gaze overlaps
	{
		FGaze gaze;
		FHitResult hitPoint(ForceInit);
		GetGaze(gaze);
		EyeTrackingArrow->SetWorldLocationAndRotation(gaze.origin, gaze.direction.Rotation());
		auto end = gaze.origin + gaze.direction * InteractionDistance;
		if (GM->RayTrace(this, gaze.origin, end, hitPoint))
		{
			if (hitPoint.Actor != eye_tracked_actor)
			{
				if (IsValid(eye_tracked_actor))
					eye_tracked_actor->EndOverlapByEyeTrack();
				eye_tracked_actor = Cast<AInteractableActor>(hitPoint.Actor);
				if (IsValid(eye_tracked_actor))
					eye_tracked_actor->BeginOverlapByEyeTrack(gaze, hitPoint);
			}
			if (IsValid(eye_tracked_actor))
				eye_tracked_actor->ProcessEyeTrack(gaze, hitPoint);
		}
		else
		{
			if (IsValid(eye_tracked_actor))
				eye_tracked_actor->EndOverlapByEyeTrack();
			eye_tracked_actor = nullptr;
		}
	}

	//process right hand
	if (!MC_Right->bHiddenInGame) {
		//check right motion controller overlaps
		{
			FHitResult hitResult;
			auto end = MC_Right->GetComponentLocation() + MC_Right->GetForwardVector() * InteractionDistance;
			if (GM->RayTrace(this, MC_Right->GetComponentLocation(), end, hitResult))
			{
				if (hitResult.Actor != actor_pointed_by_right_mc)
				{
					if (IsValid(actor_pointed_by_right_mc) &&
						actor_pointed_by_right_mc != actor_pointed_by_left_mc)
						actor_pointed_by_right_mc->EndOverlapByController();
					actor_pointed_by_right_mc = Cast<AInteractableActor>(hitResult.Actor);
					if (IsValid(actor_pointed_by_right_mc) &&
						actor_pointed_by_right_mc != actor_pointed_by_left_mc)
						actor_pointed_by_right_mc->BeginOverlapByController(hitResult);
				}
				if (IsValid(actor_pointed_by_right_mc))
					actor_pointed_by_right_mc->InFocusByController(hitResult);
			}
			else
			{
				if (IsValid(actor_pointed_by_right_mc) &&
					actor_pointed_by_right_mc != actor_pointed_by_left_mc)
					actor_pointed_by_right_mc->EndOverlapByController();
				actor_pointed_by_right_mc = nullptr;
			}
		}

		//create trajectory for walking_teleport
		if (bIsWalking)
		{
			FPredictProjectilePathParams params(HeightDeviance,
				MC_Right->GetComponentLocation(),
				MC_Right->GetForwardVector() * 1000.0f, 2.0f,
				ECollisionChannel::ECC_WorldStatic, { this });
			params.DrawDebugType = EDrawDebugTrace::ForOneFrame;
			FPredictProjectilePathResult result;
			if (UGameplayStatics::PredictProjectilePath(GetWorld(), params, result)
				&& result.HitResult.Location.Z < FloorHeight + HeightDeviance &&
				result.HitResult.Location.Z >= FloorHeight - HeightDeviance)
				newPosition = result.HitResult.Location;
			else
				newPosition = GetActorLocation();
		}

		//Process Drag&Drop with right hand
		if (IsValid(DraggedActor_RHand))
		{
			const auto new_loc = MC_Right->GetComponentLocation() + MC_Right->GetForwardVector() * DragDistance;
			DraggedActor_RHand->SetActorLocation(new_loc);
		}
	}

	//process left hand
	if (!MC_Left->bHiddenInGame)
	{
		//check left motion controller overlaps
		{
			FHitResult hitResult;
			auto end = MC_Left->GetComponentLocation() + MC_Left->GetForwardVector() * InteractionDistance;
			if (GM->RayTrace(this, MC_Left->GetComponentLocation(), end, hitResult))
			{
				if (hitResult.Actor != actor_pointed_by_left_mc)
				{
					if (IsValid(actor_pointed_by_left_mc) &&
						actor_pointed_by_left_mc != actor_pointed_by_right_mc)
						actor_pointed_by_left_mc->EndOverlapByController();
					actor_pointed_by_left_mc = Cast<AInteractableActor>(hitResult.Actor);
					if (IsValid(actor_pointed_by_left_mc) &&
						actor_pointed_by_left_mc != actor_pointed_by_right_mc)
						actor_pointed_by_left_mc->BeginOverlapByController(hitResult);
				}
				if (IsValid(actor_pointed_by_left_mc) &&
					actor_pointed_by_left_mc != actor_pointed_by_right_mc)
					actor_pointed_by_left_mc->InFocusByController(hitResult);
			}
			else
			{
				if (IsValid(actor_pointed_by_left_mc) &&
					actor_pointed_by_left_mc != actor_pointed_by_right_mc)
					actor_pointed_by_left_mc->EndOverlapByController();
				actor_pointed_by_left_mc = nullptr;
			}
		}

		//Process Drag&Drop with right hand
		if (IsValid(DraggedActor_LHand))
		{
			const auto new_loc = MC_Left->GetComponentLocation() + MC_Left->GetForwardVector() * DragDistance;
			DraggedActor_LHand->SetActorLocation(new_loc);
		}
	}


}

// Called to bind functionality to input
void ABaseInformant::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAction("RController_Trigger", IE_Pressed, this, &ABaseInformant::OnRTriggerPressed);
	InputComponent->BindAction("RController_Trigger", IE_Released, this, &ABaseInformant::OnRTriggerReleased);
	InputComponent->BindAction("LController_Trigger", IE_Pressed, this, &ABaseInformant::OnLTriggerPressed);
	InputComponent->BindAction("LController_Trigger", IE_Released, this, &ABaseInformant::OnLTriggerReleased);
	InputComponent->BindAction("Walking", IE_Pressed, this, &ABaseInformant::Walking_Trajectory);
	InputComponent->BindAction("Walking", IE_Released, this, &ABaseInformant::Walking_Teleport);
	InputComponent->BindAction("RController_Grip", IE_Pressed, this, &ABaseInformant::DragActor_RHand);
	InputComponent->BindAction("RController_Grip", IE_Released, this, &ABaseInformant::DropActor_RHand);
	InputComponent->BindAction("LController_Grip", IE_Pressed, this, &ABaseInformant::DragActor_LHand);
	InputComponent->BindAction("LController_Grip", IE_Released, this, &ABaseInformant::DropActor_LHand);
	InputComponent->BindAction("Quit", IE_Pressed, this, &ABaseInformant::QuitGame);
	if (bUseMouseControlling) {
		InputComponent->BindAxis("CameraMove_RightLeft", this, &ABaseInformant::CameraMove_LeftRight);
		InputComponent->BindAxis("CameraMove_UpDown", this, &ABaseInformant::CameraMove_UpDown);
	}
}

//------------------- Input Events ----------------------

void ABaseInformant::OnExperimentStarted()
{
	OnExperimentStarted_BP();
}

void ABaseInformant::OnExperimentFinished()
{
	OnExperimentFinished_BP();
}

void ABaseInformant::OnRTriggerPressed()
{
	if (!MC_Right->bHiddenInGame)
	{
		FGaze gaze;
		GetGaze(gaze);
		auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
		FVector trigger_ray = MC_Right->GetComponentLocation() +
			MC_Right->GetForwardVector() * InteractionDistance;
		FHitResult hitPoint(ForceInit);
		if (GM->RayTrace(this, MC_Right->GetComponentLocation(), trigger_ray, hitPoint))
		{
			auto actor = Cast<AInteractableActor>(hitPoint.Actor);
			if (IsValid(actor))
				actor->OnPressedByTrigger(hitPoint);
		}
	}

	//start event of clicking
	FKey LMB(TEXT("LeftMouseButton"));
	MC_Right_Interaction_Lazer->PressPointerKey(LMB);
}

void ABaseInformant::OnRTriggerReleased()
{
	if (!MC_Right->bHiddenInGame)
	{
		FGaze gaze;
		GetGaze(gaze);
		auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
		FVector trigger_ray = MC_Right->GetComponentLocation() +
			MC_Right->GetForwardVector() * InteractionDistance;
		FHitResult hitPoint(ForceInit);
		if (GM->RayTrace(this, MC_Right->GetComponentLocation(), trigger_ray, hitPoint))
		{
			auto actor = Cast<AInteractableActor>(hitPoint.Actor);
			if (IsValid(actor))
				actor->OnReleasedByTrigger(hitPoint);
		}
	}
	//start event of clicking
	FKey LMB(TEXT("LeftMouseButton"));
	MC_Right_Interaction_Lazer->ReleasePointerKey(LMB);
}

void ABaseInformant::OnLTriggerPressed() {}

void ABaseInformant::OnLTriggerReleased() {}

void ABaseInformant::CameraMove_LeftRight(float value)
{
	Yaw += value;
	GetController()->SetControlRotation(FRotator(0.0f, Yaw, 0.0f));
}

void ABaseInformant::CameraMove_UpDown(float value)
{
	CameraPitch += value;
	if (CameraPitch > 89.0f)
		CameraPitch = 89.0f;
	if (CameraPitch < -89.0f)
		CameraPitch = -89.0f;
	CameraComponent->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));
}

void ABaseInformant::DragActor_RHand()
{
	if (!MC_Right->bHiddenInGame)
	{
		FGaze gaze;
		GetGaze(gaze);
		auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
		FVector trigger_ray = MC_Right->GetComponentLocation() +
			MC_Right->GetForwardVector() * DragDistance;
		FHitResult hitPoint(ForceInit);
		if (GM->RayTrace(this, MC_Right->GetComponentLocation(), trigger_ray, hitPoint))
		{
			auto actor = Cast<AInteractableActor>(hitPoint.Actor);
			if (IsValid(actor) && actor->bIsDraggable) {
				DraggedActor_RHand = actor;
				DraggedActor_RHand->OnDrag();
			}
		}
	}
}

void ABaseInformant::DropActor_RHand()
{
	if (IsValid(DraggedActor_RHand))
		DraggedActor_RHand->OnDrop();
	DraggedActor_RHand = nullptr;
}

void ABaseInformant::DragActor_LHand()
{
	if (!MC_Left->bHiddenInGame)
	{
		FGaze gaze;
		GetGaze(gaze);
		auto GM = GetWorld()->GetAuthGameMode<AVRGameModeBase>();
		FVector trigger_ray = MC_Left->GetComponentLocation() +
			MC_Left->GetForwardVector() * DragDistance;
		FHitResult hitPoint(ForceInit);
		if (GM->RayTrace(this, MC_Left->GetComponentLocation(), trigger_ray, hitPoint))
		{
			auto actor = Cast<AInteractableActor>(hitPoint.Actor);
			if (IsValid(actor) && actor->bIsDraggable) {
				DraggedActor_LHand = actor;
				DraggedActor_LHand->OnDrag();
			}
		}
	}
}

void ABaseInformant::DropActor_LHand()
{
	if (IsValid(DraggedActor_LHand))
		DraggedActor_LHand->OnDrop();
	DraggedActor_LHand = nullptr;
}

void ABaseInformant::Walking_Trajectory()
{
	if (!MC_Right->bHiddenInGame && bIsWalkingEnabled && !bIsWalking)
	{
		bIsWalking = true;
	}
}

void ABaseInformant::Walking_Teleport()
{
	if (bIsWalking) {
		bIsWalking = false;
		newPosition.Z = GetActorLocation().Z;
		if ((newPosition - GetActorLocation()).Size() >= 10.0f)
		{
			SetActorLocation(newPosition);
			//check actors to interact
			TSet<AActor*> new_close_actors;
			InteractionCollider->GetOverlappingActors(new_close_actors, AInteractableActor::StaticClass());
			for (auto actor : close_actors)
			{
				if (!new_close_actors.Contains(actor))
					Cast<AInteractableActor>(actor)->HadFarToPlayer();
			}
			for (auto actor : new_close_actors)
			{
				if (!close_actors.Contains(actor))
					Cast<AInteractableActor>(actor)->HadCloseToPlayer();
			}
		}
	}
}

#if WITH_EDITOR
void ABaseInformant::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseInformant, InteractionDistance))
	{
		InteractionCollider->SetSphereRadius(InteractionDistance);
		MC_Left_Interaction_Lazer->InteractionDistance = InteractionDistance;
		MC_Right_Interaction_Lazer->InteractionDistance = InteractionDistance;
	}
}
#endif
// ---------------------- Public API ---------------------

void ABaseInformant::SetVisibility_MC_Right(bool visibility)
{
	if (IsValid(MC_Right) && IsValid(MC_Right_Mesh) && IsValid(MC_Right_Interaction_Lazer))
	{
		MC_Right->SetHiddenInGame(!visibility, true);
		MC_Right_Interaction_Lazer->bShowDebug = visibility;
		if (visibility)
			MC_Right_Interaction_Lazer->Activate();
		else
			MC_Right_Interaction_Lazer->Deactivate();
	}
	else
		UE_LOG(LogPlayerController, Warning, TEXT("Something in MCRight is ot valid %i, %i, %i"),
			IsValid(MC_Right), IsValid(MC_Right_Mesh), IsValid(MC_Right_Interaction_Lazer));
}

void ABaseInformant::SetVisibility_MC_Left(bool visibility)
{
	if (IsValid(MC_Left) && IsValid(MC_Left_Mesh) && IsValid(MC_Left_Interaction_Lazer))
	{
		MC_Left->SetHiddenInGame(!visibility, true);
		MC_Left_Interaction_Lazer->bShowDebug = visibility;
		if (visibility)
			MC_Left_Interaction_Lazer->Activate();
		else
			MC_Left_Interaction_Lazer->Deactivate();
	}
	else
		UE_LOG(LogPlayerController, Warning, TEXT("Something in MCLeft is ot valid %i, %i, %i"),
			IsValid(MC_Left), IsValid(MC_Left_Mesh), IsValid(MC_Left_Interaction_Lazer));
}

void ABaseInformant::GetGaze(FGaze& gaze) const
{
	auto instance = SRanipalEye_Core::Instance();
	ViveSR::anipal::Eye::VerboseData vd;
	instance->GetVerboseData(vd);
	instance->GetGazeRay(GazeIndex::COMBINE, gaze.origin, gaze.direction);
	FRotator HMD_orient;
	FVector HMD_pos;
	UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMD_orient, HMD_pos);

	gaze.origin = GetTransform().TransformPosition(HMD_pos);
	gaze.direction = GetTransform().TransformVector(HMD_orient.RotateVector(gaze.direction));
	gaze.left_pupil_diameter_mm = vd.left.pupil_diameter_mm;
	gaze.left_pupil_openness = vd.left.eye_openness;
	gaze.right_pupil_diameter_mm = vd.right.pupil_diameter_mm;
	gaze.right_pupil_openness = vd.right.eye_openness;
	//here you can insert custom calibration
}

void ABaseInformant::StartRecording()
{
	RecorderComponent->StartRecording();
}

void ABaseInformant::StopRecording()
{
	RecorderComponent->StopRecording();
}

bool ABaseInformant::IsRecording() const
{
	return RecorderComponent->IsRecording();
}

void ABaseInformant::Vibrate(float scale)
{
	if (GetWorld()->GetFirstPlayerController() == GetController())
	{
		if (!MC_Left->bHiddenInGame)
			GetController<APlayerController>()->PlayHapticEffect(VibrationEffect, EControllerHand::Left, scale);
		if (!MC_Right->bHiddenInGame)
			GetController<APlayerController>()->PlayHapticEffect(VibrationEffect, EControllerHand::Right, scale);
	}
}

void ABaseInformant::QuitGame()
{
	GIsRequestingExit = true;
}