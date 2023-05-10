// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "VRGameModeBase.h"
#include "BlueprintJsonLibrary.h"
//#define _USE_SCIVI_CONNECTION_
#include "VRGameModeWithSciViBase.generated.h"

/**
 * 
 */
UCLASS()
class VREXPERIMENTSBASE_API AVRGameModeWithSciViBase : public AVRGameModeBase
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyInformantSpawned(class ABaseInformant* _informant) override;

	// ----------------------- SciVi networking--------------
public:
	UFUNCTION(BlueprintCallable, Category = "SciVi",  DisplayName = "SendToSciVi")
	void SendToSciVi(const FString& message);
	UFUNCTION(BlueprintImplementableEvent, Category = "SciVi", DisplayName = "OnSciViMessageReceived")
	void OnSciViMessageReceived_BP(const FBlueprintJsonObject& msgJson);
	UFUNCTION(BlueprintImplementableEvent, Category = "SciVi")
	void OnSciViConnected();
	UFUNCTION(BlueprintImplementableEvent, Category = "SciVi")
	void OnSciViDisconnected();
protected:
	virtual void OnSciViMessageReceived(TSharedPtr<FJsonObject> msgJson);
	void initWS();
	
	struct Impl;
	Impl* impl;
};
