/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#include "AkOdinInputComponent.h"
#include "Misc/ScopeLock.h"
#include "OdinAudio/OdinDecoder.h"
#include "OdinAudio/OdinSoundGenerator.h"

namespace
{
	// How far popping has to fall behind real time (e.g. while the voice is virtual) before TickComponent starts draining the decoder
	constexpr double CatchUpThresholdSeconds = 0.1;
	// How far behind real time TickComponent stops draining. Draining the decoder completely would make it report
	// silence although the remote peer might still be talking.
	constexpr double CatchUpTargetLagSeconds = 0.03;
	// Duration of one Odin audio frame, popping is done in whole frames.
	constexpr double OdinFrameSeconds = 0.02;
	// Most frames popped by a single tick.
	constexpr int32 MaxCatchUpFramesPerTick = 50;
}

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
	
	PoppedUntilTime = FPlatformTime::Seconds();
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
	if (!SoundGenerator.IsValid())
	{
		return;
	}

	// Wwise stops calling FillSamplesBuffer while the voice is virtual (for example when it's muted or due to distance).
	// Pop in its place, so the decoder keeps emitting events and doesn't build up latency.
	const double Lag = FPlatformTime::Seconds() - PoppedUntilTime;
	if (Lag > CatchUpThresholdSeconds)
	{
		const int32 NumFrames = FMath::Min(FMath::FloorToInt32((Lag - CatchUpTargetLagSeconds) / OdinFrameSeconds),
		                                   MaxCatchUpFramesPerTick);
		const int32 NumChannels = bIsStereo ? 2 : 1;
		const int32 NumSamplesToPop = NumFrames * FMath::RoundToInt32(OdinFrameSeconds * SampleRate) * NumChannels;
		TickDiscardBuffer.SetNumUninitialized(NumSamplesToPop, EAllowShrinking::No);
		SoundGenerator->OnGenerateAudio(TickDiscardBuffer.GetData(), NumSamplesToPop);
		PoppedUntilTime += NumFrames * OdinFrameSeconds;
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
	WwiseCallbackBuffer.SetNum(RequestedTotalSamples, EAllowShrinking::No);
	FMemory::Memzero(WwiseCallbackBuffer.GetData(), WwiseCallbackBuffer.Num() * sizeof(float));

	const uint32 Result = SoundGenerator->OnGenerateAudio(WwiseCallbackBuffer.GetData(), RequestedTotalSamples);
	// Wwise pops in real time
	PoppedUntilTime = FPlatformTime::Seconds();
	if (Result != RequestedTotalSamples)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAkOdinInputComponent: mismatch during FillSamplesBuffer in DspReadCallback"));
		return true;
	}

	for (uint32 s = 0; s < NumSamples; ++s)
	{
		for (uint32 c = 0; c < NumChannels; ++c)
		{
			BufferToFill[c][s] = WwiseCallbackBuffer[s * NumChannels + c];
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
		// Wwise uses the VoiceActivityRtpc asset's cooked Short ID, RTPC name is only a fallback for a null asset.
		SetRTPCValue(VoiceActivityRtpc, VoiceActivitySetting, 0, FString());
		UE_LOG(LogTemp, Verbose, TEXT("UAkOdinInputComponent: Set Voice Activity RTPC Value to %d"), VoiceActivitySetting);
	}
}
