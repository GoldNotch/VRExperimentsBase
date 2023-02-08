// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
//#define _USE_SCIVI_CONNECTION_

#ifdef _USE_SCIVI_CONNECTION_
#define DEPRECATED
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#define ASIO_STANDALONE 1
#include "ws/server_ws.hpp"
THIRD_PARTY_INCLUDES_END
#undef UI
#undef ERROR
#undef UpdateResource
#endif

#include "VRGameModeBase.h"
#include "BlueprintJsonLibrary.h"
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
#ifdef _USE_SCIVI_CONNECTION_
	void wsRun() { m_server.start(); }
	using WSServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
	WSServer m_server;
	TUniquePtr<std::thread> m_serverThread = nullptr;//you can't use std::thread in UE4, because ue4 can't destroy it then game is exiting
	TQueue<FString> message_queue;	
#else
	FString NoSciViConnectionLogMessage = TEXT("This project doesn't use SciVi Connection. " 
												"Please inherit game mode from VRGameModeBase or use #define _USE_SCIVI_CONNECTION_");
#endif
};
