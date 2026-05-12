/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "OdinAudio/OdinSoundGenerator.h"
#include "AkAudioInputComponent.h"
#include "AkOdinInputComponent.generated.h"

class UOdinDecoder;

/**
 * The UOdinAkInputComponent is designed for handling audio playback using the Wwise Audio Engine. Before calling
 * `UAkAudioInputComponent::PostAssociatedAudioInputEvent` you'll need to have set the Odin Decoder using
 * `UOdinAkInputComponent::AssignDecoder`. Otherwise it behaves as any other AkAudioInputComponent would.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UAkOdinInputComponent : public UAkAudioInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * Assigns a decoder to the input component, initializes the necessary sound generator, and configures internal properties.
	 *
	 * This method sets the provided UOdinDecoder as the decoder for this component. It initializes an OdinSoundGenerator and
	 * ties it to the decoder for audio sample generation.
	 *
	 * @param Decoder A reference to the decoder to be assigned to the component. If the provided decoder is null, the method logs an error and exits without performing any operations.
	 */
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound", meta=(Keywords="Connect,Decoder"))
	void AssignOdinDecoder(UOdinDecoder* Decoder);

	/**
	 * Unassigns a decoder from the input component and destroys the connected sound generator. After calling this, the Input Component will generate silence and can be reused by reassigning it to another Decoder. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound", meta=(Keywords="Disconnect,Clear"))
	void UnassignOdinDecoder();

	/**
	 * Sets up the Wwise Audio configuration for this component. Requires a connected Odin Decoder to be able to
	 * set up correctly, otherwise information on the used sample rate and channel count is missing.
	 *
	 * @param AudioFormat The output parameter that will hold the channel configuration details after retrieval.
	 */
	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	/**
	 * Fills the audio buffer with generated ODIN audio samples based on the specified number of channels and sample rate.
	 *
	 * @param NumChannels The number of audio channels requested.
	 * @param NumSamples The sample rate requested for audio generation.
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
	 * Buffer used to store audio samples temporarily during audio processing.
	 */
	UPROPERTY()
	TArray<float> Buffer;
	/**
	 * A reference to a Wwise RTPC object that manages the voice activity state in an Odin audio session.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Odin|Sound")
	UAkRtpc* VoiceActivityRtpc;
	/**
	 * Audio Sample Rate. Is set to the connected Decoders sample rate on assignment.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Odin|Sound")
	int32 SampleRate = 48000;
	/**
	 * Number of Audio Channels. Is set to the connected Decoders number of audio channels on assignment.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Odin|Sound")
	bool bIsStereo = false;

	/**
	 * OdinSoundGenerator instance, used to generate Odin-based audio samples.
	 */
	TUniquePtr<FOdinSoundGenerator> SoundGenerator;

	FThreadSafeBool bIsMuted = false;
};
