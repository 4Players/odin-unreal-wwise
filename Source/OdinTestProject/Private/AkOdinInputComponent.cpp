/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#include "AkOdinInputComponent.h"

#include "OdinFunctionLibrary.h"
#include "OdinAudio/OdinSoundGenerator.h"
#include "OdinAudio/OdinSynthComponent.h"
#include "OdinSubsystem.h"


void UAkOdinInputComponent::AssignOdinMedia(UOdinSynthComponent*& Media)
{
	if (nullptr == Media)
		return;

	this->SoundGenerator = MakeShared<FOdinSoundGenerator, ESPMode::ThreadSafe>();
	this->PlaybackMedia = Media;
	
	SoundGenerator->SetOdinDecoder(Media->GetDecoder());
}

void UAkOdinInputComponent::GetChannelConfig(AkAudioFormat& AudioFormat)
{
	int NumChannels = 2;
	int SampleRate = 48000;

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
	if (Result != RequestedTotalSamples)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAkOdinInputComponent: missmatch during FillSamplesBuffer in dspreadcallback"));
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
