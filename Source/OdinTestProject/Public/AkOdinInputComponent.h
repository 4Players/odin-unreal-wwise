/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "AkAudioInputComponent.h"
#include "OdinAudioControl.h"
#include "AkOdinInputComponent.generated.h"

class OdinMediaSoundGenerator;
class UOdinPlaybackMedia;

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UAkOdinInputComponent : public UAkAudioInputComponent, public IOdinAudioControl
{
	GENERATED_BODY()

public:
	/**
	 * Assigns a UOdinPlaybackMedia object to the component and initializes the associated sound generator.
	 *
	 * @param Media Reference to a pointer of a UOdinPlaybackMedia object, which will be assigned to the component.
	 *              Must not be null for successful assignment.
	 */
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound")
	void AssignOdinMedia(UPARAM(ref)
		UOdinPlaybackMedia*& Media);

	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;

	/**
	 * Retrieves the muted state of the Odin audio input.
	 *
	 * @return True if the audio is currently muted; otherwise, false.
	 */
	virtual bool GetIsMuted() const override;
	/**
	 * Sets the muted state for Odin audio input.
	 *
	 * @note This can be used to virtualize a voice in wwise. The requirement for this is to set a valid reference for
	 * the VoiceActivityRtpc property and set the rtpc up in a way, that it affects the Voice Volume of the Audio Input
	 * object in Wwise. Take a look at the guide for more information on the general setup.
	 *
	 * @param bNewIsMuted Specifies whether to mute (true) or unmute (false) the audio.
	 */
	virtual void SetIsMuted(bool bNewIsMuted) override;

protected:
	/**
	 * A pointer to an Odin playback media object used to retrieve audio from Odin.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinPlaybackMedia* PlaybackMedia = nullptr;
	/**
	 * A pointer to an Odin playback media object used to process or play back audio streams.
	 */
	UPROPERTY()
	TArray<float> Buffer;
	/**
	 * A reference to a Wwise RTPC object that manages the voice activity state in an Odin audio session.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Odin|Sound")
	UAkRtpc* VoiceActivityRtpc;


	TSharedPtr<OdinMediaSoundGenerator, ESPMode::ThreadSafe> SoundGenerator;
	FThreadSafeBool bIsMuted = false;
};
