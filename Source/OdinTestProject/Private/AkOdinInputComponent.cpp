// Fill out your copyright notice in the Description page of Project Settings.


#include "AkOdinInputComponent.h"

#include "OdinFunctionLibrary.h"
#include "OdinMediaSoundGenerator.h"
#include "OdinPlaybackMedia.h"
#include "OdinInitializationSubsystem.h"

UAkOdinInputComponent::UAkOdinInputComponent(const FObjectInitializer& ObjectInitializer) : UAkAudioInputComponent(
	ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAkOdinInputComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	if (nullptr != Buffer)
	{
		delete Buffer;
		Buffer = nullptr;
		BufferSize = 0;
	}
}

void UAkOdinInputComponent::AssignOdinMedia(UOdinPlaybackMedia*& Media)
{
	if (nullptr == Media)
		return;

	this->SoundGenerator = MakeShared<OdinMediaSoundGenerator, ESPMode::ThreadSafe>();
	this->PlaybackMedia = Media;

	SoundGenerator->SetOdinStream(Media->GetMediaHandle());
}

void UAkOdinInputComponent::GetChannelConfig(AkAudioFormat& AudioFormat)
{
	int NumChannels = 1;
	int SampleRate = 48000;

	if (GetWorld() && GetWorld()->GetGameInstance()) {
		const UOdinInitializationSubsystem* OdinInitSubsystem =
			GetWorld()->GetGameInstance()->GetSubsystem<UOdinInitializationSubsystem>();
		if (OdinInitSubsystem) {
			NumChannels = OdinInitSubsystem->GetChannelCount();
			SampleRate = OdinInitSubsystem->GetSampleRate();
		}
	}

	AkChannelConfig ChannelConfig;
	ChannelConfig.SetStandard(AK::ChannelMaskFromNumChannels(NumChannels));

	UE_LOG(LogTemp, Warning, TEXT("Initializing Ak Odin Input Component with %i channels and Sample Rate of %i"), NumChannels, SampleRate);

	// set audio format
	AudioFormat.SetAll(
		SampleRate, // Sample rate
		ChannelConfig, // \ref AkChannelConfig
		8 * sizeof(float), // Bits per samples
		sizeof(float), // Block Align = 4 Bytes? Shouldn't it be 2*4=8 Bytes, because of two channels?
		AK_FLOAT, // feeding floats
		AK_NONINTERLEAVED
	);
}

bool UAkOdinInputComponent::FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill)
{
	if (!SoundGenerator || !PlaybackMedia)
		return false;

	const int32 RequestedTotalSamples = NumChannels * NumSamples;
	if (BufferSize != RequestedTotalSamples)
	{
		if (nullptr != Buffer)
			delete Buffer;
		Buffer = new float[RequestedTotalSamples];
		BufferSize = RequestedTotalSamples;
	}

	const uint32 Result = SoundGenerator->OnGenerateAudio(Buffer, RequestedTotalSamples);
	if (odin_is_error(Result))
	{
		FString ErrorString = UOdinFunctionLibrary::FormatError(Result, true);
		UE_LOG(LogTemp, Error, TEXT("UAkOdinInputComponent: Error during FillSamplesBuffer: %s"), *ErrorString);
		return false;
	}

	for (uint32 s = 0; s < NumSamples; ++s)
	{
		for (uint32 c = 0; c < NumChannels; ++c)
		{
			BufferToFill[c][s] = Buffer[s * NumChannels + c];
		}
	}


	return true;
}