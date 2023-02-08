// Fill out your copyright notice in the Description page of Project Settings.
#include "VRGameModeWithSciViBase.h"
#include "BaseInformant.h"
#include "SRanipalEye_Framework.h"
#include "SRanipal_API_Eye.h"
#include "InteractableActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"//for PredictProjectilePath

void AVRGameModeWithSciViBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
#ifdef _USE_SCIVI_CONNECTION_
	FString json_text;
	if (message_queue.Dequeue(json_text))
	{
		TSharedPtr<FJsonObject> jsonParsed;
		TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<TCHAR>::Create(json_text);
		if (FJsonSerializer::Deserialize(jsonReader, jsonParsed))
		{
			if (jsonParsed->TryGetField("calibrate")) CalibrateVR();
			else if (jsonParsed->TryGetField(TEXT("nextExperimentStep")))
				NextExperimentStep();
			else if (jsonParsed->TryGetField(TEXT("prevExperimentStep")))
				PrevExperimentStep();
			else if (jsonParsed->TryGetField(TEXT("startExperiment")))
				StartExperiment(true);
			else if (jsonParsed->TryGetField(TEXT("finishExperiment")))
				FinishExperiment(0, TEXT("Experiment finished by remote host"));
			else
				OnSciViMessageReceived(jsonParsed);
		}
	}
#else
	UE_LOG(LogNet, Error, TEXT("%s"), *NoSciViConnectionLogMessage);
#endif
}

void AVRGameModeWithSciViBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#ifdef _USE_SCIVI_CONNECTION_
	m_server.stop();
	m_serverThread->join();
	m_serverThread.Reset();
#else
	UE_LOG(LogNet, Error, TEXT("%s"), *NoSciViConnectionLogMessage);
#endif
	Super::EndPlay(EndPlayReason);
}

void AVRGameModeWithSciViBase::NotifyInformantSpawned(ABaseInformant* _informant)
{
	Super::NotifyInformantSpawned(_informant);
#ifdef _USE_SCIVI_CONNECTION_
	initWS();
#else 
	UE_LOG(LogNet, Error, TEXT("%s"), *NoSciViConnectionLogMessage);
#endif
}

//---------------------- SciVi communication ------------------

void AVRGameModeWithSciViBase::initWS()
{
#ifdef _USE_SCIVI_CONNECTION_
	m_server.config.port = 81;

	auto& ep = m_server.endpoint["^/ue4/?$"];

	ep.on_message = [this](std::shared_ptr<WSServer::Connection> connection, std::shared_ptr<WSServer::InMessage> msg)
	{
		auto str = FString(UTF8_TO_TCHAR(msg->string().c_str()));
		message_queue.Enqueue(str);
	};

	ep.on_open = [this](std::shared_ptr<WSServer::Connection> connection)
	{
		UE_LOG(LogTemp, Display, TEXT("WebSocket: Opened"));
		OnSciViConnected();
	};

	ep.on_close = [this](std::shared_ptr<WSServer::Connection> connection, int status, const std::string&)
	{
		UE_LOG(LogTemp, Display, TEXT("WebSocket: Closed"));
		OnSciViDisconnected();
	};

	ep.on_handshake = [](std::shared_ptr<WSServer::Connection>, SimpleWeb::CaseInsensitiveMultimap&)
	{
		return SimpleWeb::StatusCode::information_switching_protocols;
	};

	ep.on_error = [](std::shared_ptr<WSServer::Connection> connection, const SimpleWeb::error_code& ec)
	{
		UE_LOG(LogTemp, Warning, TEXT("WebSocket: Error"));
	};

	m_serverThread = MakeUnique<std::thread>(&AVRGameModeBase::wsRun, this);
#endif
}

void AVRGameModeWithSciViBase::SendToSciVi(const FString& message)
{
#ifdef _USE_SCIVI_CONNECTION_
	auto t = FDateTime::Now();
	int64 time = t.ToUnixTimestamp() * 1000 + t.GetMillisecond();
	auto msg = FString::Printf(TEXT("{\"Time\": %llu, %s}"), time, *message);
	for (auto& connection : m_server.get_connections())//broadcast to everyone
		connection->send(TCHAR_TO_UTF8(*msg));
#endif
}

void AVRGameModeWithSciViBase::OnSciViMessageReceived(TSharedPtr<FJsonObject> msgJson)
{
#ifdef _USE_SCIVI_CONNECTION_
	FBlueprintJsonObject blueprint_json;
	blueprint_json.Object = msgJson;
	OnSciViMessageReceived_BP(blueprint_json);
#endif
}