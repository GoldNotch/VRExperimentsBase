// Fill out your copyright notice in the Description page of Project Settings.


#include "VRGameModeBase.h"
#include "BaseInformant.h"
#include "SRanipalEye_Framework.h"
#include "SRanipal_API_Eye.h"
#include "InteractableActor.h"
#include "EngineUtils.h"
#include "Widgets/SWindow.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"//for PredictProjectilePath

AVRGameModeBase::AVRGameModeBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;

	logs.AddDefaulted(ExperimentLogType::Total);
	logs[EyeTrack].filename = TEXT("eyetrack");
	logs[EyeTrack].header = {
							TEXT("timestamp"),
							TEXT("Origin_x"),TEXT("Origin_y"), TEXT("Origin_z"),
							TEXT("Direction_x"),TEXT("Direction_y"), TEXT("Direction_z"),
							TEXT("lpdmm"),TEXT("rpdmm"),
							TEXT("AOI"), TEXT("AOI_Component")
	};
	logs[Events].filename = TEXT("events");
	logs[Events].header = {
							TEXT("timestamp"), TEXT("Action"),
							TEXT("AOI"), TEXT("AOI_loc_x"), TEXT("AOI_loc_y"), TEXT("AOI_loc_z")
	};

	// if game window had focus - show controller
	OnGameWindowFocus.BindLambda([this](auto, auto) { informant->EnableInputEvents(true); });
	// if game window lost focus - hide controller
	OnGameWindowFocusLost.BindLambda([this](auto) { informant->EnableInputEvents(false); });
}

void AVRGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	// Init control panel
	if (IsValid(ControlPanelWidgetClass))
	{
		ControlPanel = SNew(SWindow)
			.Title(FText::FromString(TEXT("Control panel")))
			.SizingRule(ESizingRule::UserSized)
			.ClientSize(FVector2D(200, 200))
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.HasCloseButton(false)
			.CreateTitleBar(true)
			.ScreenPosition(FVector2D(0, 0))
			.AutoCenter(EAutoCenter::None);
		ControlPanelWidget = UUserWidget::CreateWidgetInstance(*GetGameInstance(), ControlPanelWidgetClass, FName(TEXT("ControlPanel")));
		ControlPanel->SetContent(ControlPanelWidget->TakeWidget());
		ControlPanel->Resize(ControlPanelSize);
		FSlateApplication& SlateApp = FSlateApplication::Get();
		GameWindow = SlateApp.GetActiveTopLevelWindow();
		SlateApp.AddWindow(ControlPanel.ToSharedRef(), true);
		GameWindow->SetOnMouseEnter(OnGameWindowFocus);
		GameWindow->SetOnMouseLeave(OnGameWindowFocusLost);
	}

	auto instance = SRanipalEye_Framework::Instance();
	if (instance)
		instance->StartFramework(EyeVersion);
}

void AVRGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsExperimentStarted() && bExperimentFinishing)
		FinishExperiment(0, TEXT("Experiment finished by remote host"));
	if (!IsExperimentStarted() && bExperimentStarting)
		StartExperiment(bRecordLogs);
}

void AVRGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	SRanipalEye_Framework::Instance()->StopFramework();
	if (ControlPanel.IsValid())
		ControlPanel->RequestDestroyWindow();
}

void AVRGameModeBase::NotifyInformantSpawned(ABaseInformant* _informant)
{
	informant = _informant;
}

bool AVRGameModeBase::RayTrace(const AActor* ignoreActor, const FVector& origin, const FVector& end, FHitResult& hitResult)
{
	const float ray_thickness = 1.0f;
	FCollisionQueryParams traceParam = FCollisionQueryParams(FName("traceParam"), true, ignoreActor);
	traceParam.bReturnPhysicalMaterial = false;
	const auto InformantTraceChannel = ECollisionChannel::ECC_Visibility;
	if (ray_thickness <= 0.0f)
	{
		return GetWorld()->LineTraceSingleByChannel(hitResult, origin, end,
			InformantTraceChannel, traceParam);
	}
	else
	{
		FCollisionShape sph = FCollisionShape();
		sph.SetSphere(ray_thickness);
		return GetWorld()->SweepSingleByChannel(hitResult, origin, end, FQuat(0.0f, 0.0f, 0.0f, 0.0f),
			InformantTraceChannel, sph, traceParam);
	}
}

void AVRGameModeBase::StartExperiment(bool recording/* = true*/, FString InformantName/* = FString()*/)
{
	bExperimentStarting = false;
	bExperimentRunning = true;
	if (HasExperimentSteps())
		NextExperimentStep();
	if (InformantName.IsEmpty()) 
	{
		InformantName = TEXT("Informant_");
		InformantName += FGuid::NewGuid().ToString();
	}
	// generate record files
	bRecordLogs = recording;
	if (bRecordLogs)
	{
		FString now = FDateTime::Now().ToString();
		FString full_dir_path = FPaths::ProjectDir() + TEXT("/") + ExperimentLogsFolderPath;
		auto& platformFile = FPlatformFileManager::Get().GetPlatformFile();
		if (!platformFile.DirectoryExists(*full_dir_path))
			if (!platformFile.CreateDirectory( * full_dir_path))
				UE_LOG(LogLoad, Error, TEXT("Directory wasn't created"));
		for (size_t i = 0; i < ExperimentLogType::Total; ++i)
		{
			logs[i].SetPath(FString::Printf(TEXT("%s/%s_%s_%s.csv"), *full_dir_path, *now, *logs[i].filename, *InformantName));
			// format header
			FString header_row = TEXT("");
			for (size_t j = 0, c = logs[i].header.Num(); j < c; ++j)
			{
				header_row += logs[i].header[j];
				if (j < c - 1) header_row += LogColumnSeparator;
			}
			header_row += TEXT("\n");
			logs[i].AppendRow(header_row);
		}
	}

	OnExperimentStarted();
	for (TActorIterator<AInteractableActor> It(GetWorld(), AInteractableActor::StaticClass()); It; ++It)
		It->OnExperimentStarted();
	for (TActorIterator<ABaseInformant> It(GetWorld(), ABaseInformant::StaticClass()); It; ++It)
		It->OnExperimentStarted();
}

void AVRGameModeBase::FinishExperiment(int code, const FString& message)
{
	bExperimentFinishing = false;
	bExperimentRunning = false;
	CurrentExpeirmentStepIndex = -1;
	bRecordLogs = false;
	for (auto&& log : logs)
		log.Flush();
	OnExperimentFinished(code, message);
	for (TActorIterator<AInteractableActor> It(GetWorld(), AInteractableActor::StaticClass()); It; ++It)
		It->OnExperimentFinished();
	for (TActorIterator<ABaseInformant> It(GetWorld(), ABaseInformant::StaticClass()); It; ++It)
		It->OnExperimentFinished();
}

void AVRGameModeBase::NextExperimentStep()
{
	if (CurrentExpeirmentStepIndex >= ExperimentSteps.Num() - 1)
		FinishExperiment(0, TEXT("Experiment is over due to all steps are passed"));
	else
	{
		if (IsValid(CurrentExperimentStep)) {
			GetWorld()->RemoveActor(CurrentExperimentStep, true);
			CurrentExperimentStep->Destroy();//it calls end play and quit step
		}
		CurrentExpeirmentStepIndex++;
		auto& step_class = ExperimentSteps[CurrentExpeirmentStepIndex];
		FActorSpawnParameters params;
		params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		params.Name = FName(FString::Printf(TEXT("ExperimentStep_%i"), CurrentExpeirmentStepIndex));
		CurrentExperimentStep = GetWorld()->SpawnActor<AExperimentStepBase>(step_class, params);//it calls begin play event and starts step
	}
}

void AVRGameModeBase::PrevExperimentStep()
{
	if (CurrentExpeirmentStepIndex > 0)
	{
		if (IsValid(CurrentExperimentStep)) {
			GetWorld()->RemoveActor(CurrentExperimentStep, true);
			CurrentExperimentStep->Destroy();//it calls end play and quit step
		}
		CurrentExpeirmentStepIndex--;
		auto& step_class = ExperimentSteps[CurrentExpeirmentStepIndex];
		FActorSpawnParameters params;
		params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
		params.Name = FName(FString::Printf(TEXT("ExperimentStep_%i"), CurrentExpeirmentStepIndex));
		CurrentExperimentStep = GetWorld()->SpawnActor<AExperimentStepBase>(step_class, params);//it calls begin play event and starts step
	}
}

void AVRGameModeBase::OnExperimentStarted()
{
	OnExperimentStarted_BP();
}

void AVRGameModeBase::OnExperimentFinished(int code, const FString& message)
{
	OnExperimentFinished_BP(code, message);
}

// ---------------------- VR ------------------------

void AVRGameModeBase::CalibrateVR()
{
	ViveSR::anipal::Eye::LaunchEyeCalibration(nullptr);
	UE_LOG(LogTemp, Display, TEXT("start calibration"));
}

void AVRGameModeBase::WriteToExperimentLog(ExperimentLogType log_type, const FString& row)
{
	if (bExperimentRunning && bRecordLogs)
	{
		logs[log_type].AppendRow(FString::Printf(TEXT("%lli;%s"), GetLogTimestamp(), *row));
	}
}

void FExperimentLog::AppendRow(const FString& row)
{
	rows.Append(row);
	rows_to_write_count++;
	if (rows_to_write_count == RowsBufferCount)
		Flush();
}

void FExperimentLog::Flush()
{
	FFileHelper::SaveStringToFile(rows, *path, FFileHelper::EEncodingOptions::ForceUTF8, & IFileManager::Get(), FILEWRITE_Append);
	rows_to_write_count = 0;
	rows.Reset();
}
