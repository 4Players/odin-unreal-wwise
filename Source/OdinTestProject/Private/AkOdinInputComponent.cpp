/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#include "AkOdinInputComponent.h"

#include "OdinFunctionLibrary.h"
#include "OdinMediaSoundGenerator.h"
#include "OdinPlaybackMedia.h"
#include "OdinSubsystem.h"


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
	int NumChannels = ODIN_DEFAULT_CHANNEL_COUNT;
	int SampleRate = ODIN_DEFAULT_SAMPLE_RATE;

	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		if (const UOdinSubsystem* OdinInitSubsystem =
			GetWorld()->GetGameInstance()->GetSubsystem<UOdinSubsystem>())
		{
			NumChannels = OdinInitSubsystem->GetChannelCount();
			SampleRate = OdinInitSubsystem->GetSampleRate();
		}
	}

	AkChannelConfig ChannelConfig;
	ChannelConfig.SetStandard(AK::ChannelMaskFromNumChannels(NumChannels));

	UE_LOG(LogTemp, Warning, TEXT("Initializing Ak Odin Input Component with %i channels and Sample Rate of %i"),
	       NumChannels, SampleRate);

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

	if (GetIsMuted())
	{
		Buffer.SetNumZeroed(RequestedTotalSamples, EAllowShrinking::No);
	}
	else
	{
		if (Buffer.Num() != RequestedTotalSamples)
		{
			Buffer.SetNum(RequestedTotalSamples, EAllowShrinking::No);
		}
	}


	const uint32 Result = SoundGenerator->OnGenerateAudio(Buffer.GetData(), RequestedTotalSamples);
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

bool UAkOdinInputComponent::GetIsMuted() const
{
	return bIsMuted;
}

void UAkOdinInputComponent::SetIsMuted(bool bNewIsMuted)
{
	bIsMuted = bNewIsMuted;
	if (VoiceActivityRtpc)
	{
		const int32 VoiceActivitySetting = bIsMuted ? 0 : 1;
		SetRTPCValue(VoiceActivityRtpc, VoiceActivitySetting, 0,
		             VoiceActivityRtpc->GetWwiseName().ToString());
	}
}
