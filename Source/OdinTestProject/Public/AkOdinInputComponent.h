/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "OdinAudio/OdinSoundGenerator.h"
#include "AkAudioInputComponent.h"
#include "AkOdinInputComponent.generated.h"

class UOdinDecoder;

/**
 * The UAkOdinInputComponent handles audio playback using the Wwise Audio Engine. Before calling
 * `UAkAudioInputComponent::PostAssociatedAudioInputEvent`, assign a decoder (with the same sample rate and stereo value as the Component)
 *  using `UAkOdinInputComponent::AssignOdinDecoder`.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UAkOdinInputComponent : public UAkAudioInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * Assigns a decoder to the input component and initializes the sound generator.
	 *
	 * The decoder's sample rate and stereo value have to match the component's sample rate and stereo value.
	 *
	 * @param Decoder The decoder to assign.
	 */
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound", meta=(Keywords="Connect,Decoder"))
	void AssignOdinDecoder(UOdinDecoder* Decoder);

	/**
	 * Unassigns the decoder and destroys the Odin sound generator. The next Wwise audio callback stops the Wwise event.
	 * Assign a matching decoder and call PostAssociatedAudioInputEvent again to resume playback.
	 */
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound", meta=(Keywords="Disconnect,Clear"))
	void UnassignOdinDecoder();

	/**
	 * Sets up the Wwise audio configuration using this component's SampleRate and bIsStereo settings.
	 *
	 * @param AudioFormat The output parameter that will hold the channel configuration details after retrieval.
	 */
	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	/**
	 * Fills the audio buffer with generated ODIN audio samples based on the specified number of channels and sample rate.
	 *
	 * @param NumChannels The number of audio channels requested.
	 * @param NumSamples The number of samples requested per channel.
	 * @param BufferToFill A pointer to the buffer that will be filled with generated audio samples.
	 * @return Returns true if the buffer was successfully filled with audio samples.
	 */
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;

	/**
	 * Retrieves the muted state of the Odin audio input.
	 *
	 * @return True if the audio is currently muted; otherwise, false.
	 */
	UFUNCTION(BlueprintPure, Category="Odin|Sound")
	virtual bool GetIsMuted() const;
	/**
	 * Sets the muted state for Odin audio input.
	 *
	 * @note This can be used to virtualize a voice in wwise. The requirement for this is to set a valid reference for
	 * the VoiceActivityRtpc property and set the rtpc up in a way, that it affects the Voice Volume of the Audio Input
	 * object in Wwise. Take a look at the guide for more information on the general setup.
	 *
	 * @param bNewIsMuted Specifies whether to mute (true) or unmute (false) the audio.
	 */
	UFUNCTION(BlueprintCallable, Category="Odin|Sound")
	virtual void SetIsMuted(bool bNewIsMuted);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/**
	 * Used for processing audio input data  within the Odin audio system. Stores the decoder instance
	 * assigned to the component, which is responsible for retrieving sound data from ODIN. The decoder’s
	 * specific configuration settings (such as sample rate and channel count) determine how the audio data is
	 * processed and managed.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinDecoder* PlaybackDecoder;
	/**
	 * A reference to a Wwise RTPC object that manages the voice activity state in an Odin audio session.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Odin|Sound")
	UAkRtpc* VoiceActivityRtpc;
	/**
	 * Audio sample rate. The assigned decoder must use the same rate.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Odin|Sound")
	int32 SampleRate = 48000;
	/**
	 * Whether to use stereo audio. The assigned decoder must use the same channel count.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Odin|Sound")
	bool bIsStereo = false;

	/**
	 * OdinSoundGenerator instance, used to generate Odin-based audio samples.
	 */
	TUniquePtr<FOdinSoundGenerator> SoundGenerator;
	/**
	 * Interleaved samples popped in FillSamplesBuffer, before they're split into Wwise's per-channel buffers.
	 */
	TArray<float> WwiseCallbackBuffer;
	/**
	 * Receives the samples popped in TickComponent while Wwise isn't pulling audio, which are discarded.
	 */
	TArray<float> TickDiscardBuffer;
	FCriticalSection AudioCriticalSection;
	/**
	 * Point in real time (FPlatformTime::Seconds()) up to which audio has been popped from the decoder.
	 */
	double PoppedUntilTime = 0.0;

	FThreadSafeBool bIsMuted = false;
};
