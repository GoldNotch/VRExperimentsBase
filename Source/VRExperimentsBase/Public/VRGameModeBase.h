// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SRanipal_Eyes_Enums.h"
#include "ExperimentStepBase.h"
#include "VRGameModeBase.generated.h"

UENUM()
enum ExperimentLogType
{
	EyeTrack = 0     UMETA(DisplayName = "EyeTrack"),
	Events      UMETA(DisplayName = "Events"),
	Total
};

USTRUCT(BlueprintType)
struct FExperimentLog
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString filename;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> header;

	void AppendRow(const FString& row);

	void Flush();

	void SetPath(const FString& new_path) { path = new_path; }

private:
	static const size_t RowsBufferCount = 1024;
	FString rows;
	size_t rows_to_write_count = 0;
	FString path;
};

/**
 * 
 */
UCLASS()
class VREXPERIMENTSBASE_API AVRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	AVRGameModeBase(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadonly, DisplayName = "Informant")
	class ABaseInformant* informant;
	UPROPERTY(EditAnywhere, BlueprintReadwrite)
	TArray<TSubclassOf<AExperimentStepBase>> ExperimentSteps;

	virtual void NotifyInformantSpawned(class ABaseInformant* _informant);
	UFUNCTION(BlueprintCallable)
	bool RayTrace(const AActor* ignoreActor, const FVector& origin, const FVector& end, FHitResult& hitResult);
	
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	void StartExperiment(bool recording = true, FString InformantName = TEXT(""));
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	FORCEINLINE bool IsExperimentStarted() { return bExperimentRunning; }
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	void FinishExperiment(int code, const FString& message);
	UFUNCTION()// Use only from other threads
	void StartExperimentByRemote(bool recording = true) 
	{
		bRecordLogs = recording;
		bExperimentStarting = true;
	}
	UFUNCTION()// Use only from other threads
	void FinishExperimentByRemote() { bExperimentFinishing = true; }
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	void NextExperimentStep();
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	void PrevExperimentStep();
	UFUNCTION(BlueprintCallable, Category = "Experiment")
	FORCEINLINE bool HasExperimentSteps() { return ExperimentSteps.Num() > 0; }

	// events
	virtual void OnExperimentStarted();
	virtual void OnExperimentFinished(int code, const FString& message);
	UFUNCTION(BlueprintImplementableEvent, Category = "Experiment", DisplayName = "OnExperimentStarted")
	void OnExperimentStarted_BP();
	UFUNCTION(BlueprintImplementableEvent, Category = "Experiment", DisplayName = "OnExperimentFinished")
	void OnExperimentFinished_BP(int code, const FString& message);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AExperimentStepBase* CurrentExperimentStep = nullptr;
	int32 CurrentExpeirmentStepIndex = -1;

	bool bRecordLogs = false;
	FThreadSafeBool bExperimentRunning = false;
	FThreadSafeBool bExperimentStarting = false;
	FThreadSafeBool bExperimentFinishing = false;
	TSharedPtr<class SWindow> ControlPanel;
	TSharedPtr<class SWindow> GameWindow;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	TSubclassOf<class UUserWidget> ControlPanelWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	FVector2D ControlPanelSize = FVector2D(200, 200);
	UPROPERTY()
	class UUserWidget* ControlPanelWidget = nullptr;

	FSimpleNoReplyPointerEventHandler OnGameWindowFocusLost;
	FNoReplyPointerEventHandler OnGameWindowFocus;

	FORCEINLINE int64 GetLogTimestamp() const 
	{
		auto t = FDateTime::Now();
		return t.ToUnixTimestamp() * 1000 + t.GetMillisecond();
	}
	//------------------- VR ----------------------
public:
	UFUNCTION(BlueprintCallable)
	void CalibrateVR();

	UPROPERTY(EditAnywhere)
	SupportedEyeVersion EyeVersion = SupportedEyeVersion::version1;
	//----------------- Experiment Logging -------------
public:
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	FString ExperimentLogsFolderPath = TEXT("ExperimentLogs");
	UFUNCTION(BlueprintCallable)
	void WriteToExperimentLog(ExperimentLogType log_type, const FString& row);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadonly)
	TArray<FExperimentLog> logs;
	TCHAR LogColumnSeparator = TEXT(';');
	
};
