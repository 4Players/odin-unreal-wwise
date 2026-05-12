/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#include "AkOdinInputComponent.h"
#include "OdinAudio/OdinSoundGenerator.h"

void UAkOdinInputComponent::AssignOdinDecoder(UOdinDecoder* Decoder)
{
	if (nullptr == Decoder)
		return;

	SoundGenerator.Reset();
	this->SoundGenerator = MakeUnique<FOdinSoundGenerator>();
	
	this->PlaybackDecoder = Decoder;
	SoundGenerator->SetOdinDecoder(Decoder);
}

void UAkOdinInputComponent::UnassignOdinDecoder()
{
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
	if (GetIsMuted() && SoundGenerator.IsValid())
	{
		// We need to manually pop audio from the decoder, otherwise we won't receive audio events for the decoder,
		// which would prevent us from unmuting the component
		const int32 RequestedNumSamples = Buffer.Num() > 0 ? Buffer.Num() : 512;
		SoundGenerator->OnGenerateAudio(Buffer.GetData(), RequestedNumSamples);
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
	if (!SoundGenerator.IsValid() || !PlaybackDecoder)
	{
		return false;
	}

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
