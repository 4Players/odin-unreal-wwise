/* Copyright (c) 2022-2024 4Players GmbH. All rights reserved. */

#pragma once

#include "CoreMinimal.h"
#include "OdinAudio/OdinSoundGenerator.h"
#include "AkAudioInputComponent.h"
#include "AkOdinInputComponent.generated.h"

class UOdinDecoder;

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UAkOdinInputComponent : public UAkAudioInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * Assigns a UOdinDecoder object to the component and initializes the associated sound generator.
	 *
	 * @param Decoder Reference to a pointer of a UOdinDecoder object, which will be assigned to the component.
	 *              Must not be null for successful assignment.
	 */
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound", meta=(Keywords="Connect,Decoder"))
	void AssignOdinDecoder(UOdinDecoder* Decoder);

	UFUNCTION(BlueprintCallable, Category = "Odin|Sound", meta=(Keywords="Disconnect,Clear"))
	void UnassignOdinDecoder();

	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
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

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/**
	 * A pointer to an Odin playback decoder object used to retrieve audio from Odin.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinDecoder* PlaybackDecoder;
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Odin|Sound")
	int32 SampleRate = 48000;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Odin|Sound")
	bool bIsStereo = false;
	

	TUniquePtr<FOdinSoundGenerator> SoundGenerator;
	FThreadSafeBool bIsMuted = false;
};


