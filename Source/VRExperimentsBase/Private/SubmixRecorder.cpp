// Fill out your copyright notice in the Description page of Project Settings.


#include "SubmixRecorder.h"
#include "AudioDeviceManager.h"
#include "Sound/SoundSubmix.h"



// Sets default values for this component's properties
USubmixRecorder::USubmixRecorder(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}


void USubmixRecorder::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsActive())
	{
		//FScopeLock lock(&use_queue);
		if (!RecordQueue.IsEmpty())
		{
			if (OnRecorded)
			{
				Audio::TSampleBuffer<int16> buffer;
				RecordQueue.Dequeue(buffer);
				OnRecorded(buffer.GetData(), buffer.GetNumChannels(), buffer.GetNumSamples(), buffer.GetSampleRate());
			}
			else RecordQueue.Pop();
		}
	}

	if (RecordQueue.IsEmpty() && !bIsRecording && !bRecordFinished)
	{
		if (OnRecordFinished) OnRecordFinished();
		bRecordFinished = true;
	}
}

void USubmixRecorder::OnRegister()
{
	Super::OnRegister();
	if (!GEngine) return;
	if (NumChannelsToRecord == 1 || NumChannelsToRecord == 2) {
		if (FAudioDevice* AudioDevice = GetWorld()->GetAudioDeviceRaw())
		{
			AudioDevice->RegisterSubmixBufferListener(this, SubmixToRecord);
		}
	}
	else
		UE_LOG(LogAudio, Warning, TEXT("SubmixRecorder supports only 1 or 2 channels to record"));
}

void USubmixRecorder::OnUnregister()
{
	Super::OnUnregister();
	if (FAudioDevice* AudioDevice = GetWorld()->GetAudioDeviceRaw())
	{
		AudioDevice->UnregisterSubmixBufferListener(this, SubmixToRecord);
	}
}

void USubmixRecorder::StartRecording()
{
	bIsRecording = true;
	bRecordFinished = false;
}

void USubmixRecorder::StopRecording()
{
	bIsRecording = false;
}

void USubmixRecorder::OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock)
{
	if (bIsRecording)
	{
		Audio::TSampleBuffer<int16> buffer(AudioData, NumSamples, NumChannels, SampleRate);
		buffer.MixBufferToChannels(NumChannelsToRecord);
		RecordQueue.Enqueue(buffer);
	}
}