// Fill out your copyright notice in the Description page of Project Settings.
#include "VRGameModeWithSciViBase.h"
#include "BaseInformant.h"
#include "SRanipalEye_Framework.h"
#include "SRanipal_API_Eye.h"
#include "InteractableActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"//for PredictProjectilePath

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


struct AVRGameModeWithSciViBase::Impl
{
	Impl(AVRGameModeWithSciViBase& GM) : owner(GM)
	{
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
			owner.OnSciViConnected();
		};

		ep.on_close = [this](std::shared_ptr<WSServer::Connection> connection, int status, const std::string&)
		{
			UE_LOG(LogTemp, Display, TEXT("WebSocket: Closed"));
			owner.OnSciViDisconnected();
		};

		ep.on_handshake = [](std::shared_ptr<WSServer::Connection>, SimpleWeb::CaseInsensitiveMultimap&)
		{
			return SimpleWeb::StatusCode::information_switching_protocols;
		};

		ep.on_error = [](std::shared_ptr<WSServer::Connection> connection, const SimpleWeb::error_code& ec)
		{
			UE_LOG(LogTemp, Warning, TEXT("WebSocket: Error"));
		};

		thread = std::make_unique<std::thread>([this] {m_server.start(); });
	}
	~Impl()
	{
		m_server.stop();
		thread->join();
	}

	void Tick(float deltaTime)
	{
		FString json_text;
		if (message_queue.Dequeue(json_text))
		{
			TSharedPtr<FJsonObject> jsonParsed;
			TSharedRef<TJsonReader<TCHAR>> jsonReader = TJsonReaderFactory<TCHAR>::Create(json_text);
			if (FJsonSerializer::Deserialize(jsonReader, jsonParsed))
			{
				if (jsonParsed->TryGetField("calibrate")) owner.CalibrateVR();
				else if (jsonParsed->TryGetField(TEXT("nextExperimentStep")))
					owner.NextExperimentStep();
				else if (jsonParsed->TryGetField(TEXT("prevExperimentStep")))
					owner.PrevExperimentStep();
				else if (jsonParsed->TryGetField(TEXT("startExperiment")))
					owner.StartExperiment(true);
				else if (jsonParsed->TryGetField(TEXT("finishExperiment")))
					owner.FinishExperiment(0, TEXT("Experiment finished by remote host"));
				else
					owner.OnSciViMessageReceived(jsonParsed);
			}
		}
	}

	void SendToSciVi(const FString& message)
	{
		for (auto& connection : m_server.get_connections())//broadcast to everyone
			connection->send(TCHAR_TO_UTF8(*message));
	}

private:
	AVRGameModeWithSciViBase& owner;
	using WSServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
	WSServer m_server;
	std::unique_ptr<std::thread> thread;
	TQueue<FString> message_queue;
};
#else
struct AVRGameModeWithSciViBase::Impl
{
	Impl(AVRGameModeWithSciViBase& owner) {};
	void Tick(float DeltaTime) {}
	void SendToSciVi(const FString& message) {}
};

#endif // _USE_SCIVI_CONNECTION_

void AVRGameModeWithSciViBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	impl->Tick(DeltaTime);
}

void AVRGameModeWithSciViBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete impl;
	Super::EndPlay(EndPlayReason);
}

void AVRGameModeWithSciViBase::NotifyInformantSpawned(ABaseInformant* _informant)
{
	Super::NotifyInformantSpawned(_informant);
	initWS();
}

//---------------------- SciVi communication ------------------

void AVRGameModeWithSciViBase::initWS()
{
	impl = new Impl(*this);
}

void AVRGameModeWithSciViBase::SendToSciVi(const FString& message)
{
	if (bExperimentRunning && bRecordLogs) 
	{
		auto msg = FString::Printf(TEXT("{\"Time\": %lli, %s}"), GetLogTimestamp(), *message);
		impl->SendToSciVi(msg);
	}
}

void AVRGameModeWithSciViBase::OnSciViMessageReceived(TSharedPtr<FJsonObject> msgJson)
{
#ifdef _USE_SCIVI_CONNECTION_
	FBlueprintJsonObject blueprint_json;
	blueprint_json.Object = msgJson;
	OnSciViMessageReceived_BP(blueprint_json);
#endif
}