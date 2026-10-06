/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#include "AkOdinInputComponent.h"
#include "Misc/ScopeLock.h"
#include "OdinAudio/OdinDecoder.h"
#include "OdinAudio/OdinSoundGenerator.h"

void UAkOdinInputComponent::AssignOdinDecoder(UOdinDecoder* Decoder)
{
	if (!IsValid(Decoder))
	{
		UE_LOG(LogTemp, Error, TEXT("UAkOdinInputComponent: cannot assign an invalid Odin decoder"));
		return;
	}

	FScopeLock Lock(&AudioCriticalSection);
	if (Decoder->SampleRate != SampleRate || Decoder->bStereo != bIsStereo)
	{
		UE_LOG(LogTemp, Error, TEXT("UAkOdinInputComponent: decoder sample rate and channel count must match the component"));
		return;
	}

	SoundGenerator.Reset();
	this->SoundGenerator = MakeUnique<FOdinSoundGenerator>();
	
	this->PlaybackDecoder = Decoder;
	SoundGenerator->SetOdinDecoder(Decoder);
}

void UAkOdinInputComponent::UnassignOdinDecoder()
{
	FScopeLock Lock(&AudioCriticalSection);
	if (SoundGenerator.IsValid())
	{
		SoundGenerator->Close();
	}
	SoundGenerator.Reset();
	PlaybackDecoder = nullptr;
}

void UAkOdinInputComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	FScopeLock Lock(&AudioCriticalSection);
	if (GetIsMuted() && SoundGenerator.IsValid())
	{
		const int32 RequestedNumSamples = Buffer.Num() > 0 ? Buffer.Num() : 512;
		TickBuffer.SetNum(RequestedNumSamples, EAllowShrinking::No);
		FMemory::Memzero(TickBuffer.GetData(), TickBuffer.Num() * sizeof(float));
		SoundGenerator->OnGenerateAudio(TickBuffer.GetData(), RequestedNumSamples);
	}
}

void UAkOdinInputComponent::GetChannelConfig(AkAudioFormat& AudioFormat)
{
	const int32 NumChannels = bIsStereo ? 2 : 1;

	AkChannelConfig ChannelConfig;
	ChannelConfig.SetStandard(AK::ChannelMaskFromNumChannels(NumChannels));

	UE_LOG(LogTemp, Log, TEXT("Initializing Ak Odin Input Component with %i channels and Sample Rate of %i"),
	       NumChannels, SampleRate);

	// set audio format
	AudioFormat.SetAll(
		SampleRate, // Sample rate
		ChannelConfig, // \ref AkChannelConfig
		8 * sizeof(float), // Bits per samples
		sizeof(float), // Block Align = 4 Bytes
		AK_FLOAT, // feeding floats
		AK_NONINTERLEAVED
	);
}

bool UAkOdinInputComponent::FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill)
{
	FScopeLock Lock(&AudioCriticalSection);
	if (!SoundGenerator.IsValid() || !PlaybackDecoder)
	{
		return false;
	}

	const int32 RequestedTotalSamples = NumChannels * NumSamples;
	Buffer.SetNum(RequestedTotalSamples, EAllowShrinking::No);
	FMemory::Memzero(Buffer.GetData(), Buffer.Num() * sizeof(float));

	const uint32 Result = SoundGenerator->OnGenerateAudio(Buffer.GetData(), RequestedTotalSamples);
	if (Result != RequestedTotalSamples)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAkOdinInputComponent: mismatch during FillSamplesBuffer in DspReadCallback"));
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
		UE_LOG(LogTemp, Verbose, TEXT("UAkOdinInputComponent: Set Voice Activity RTPC Value to %d"), VoiceActivitySetting);
	}
}
