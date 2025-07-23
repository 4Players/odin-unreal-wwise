// Fill out your copyright notice in the Description page of Project Settings.

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
	// Sets default values for this component's properties
	UAkOdinInputComponent(const class FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Odin|Sound")
	void AssignOdinMedia(UPARAM(ref)
		UOdinPlaybackMedia*& Media);

	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;

	virtual bool GetIsMuted() const override;
	/**
	 * 
	 * @param bNewIsMuted 
	 */
	virtual void SetIsMuted(bool bNewIsMuted) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinPlaybackMedia* PlaybackMedia = nullptr;
	/**
	 * A pointer to an Odin playback media object used to process or play back audio streams.
	 */
	UPROPERTY()
	TArray<float> Buffer;
	/**
	 * A reference to a Wwise RTPC (Real-Time Parameter Control) object that manages the voice activity state in an Odin audio session.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Odin|Sound")
	UAkRtpc* VoiceActivityRtpc;


	TSharedPtr<OdinMediaSoundGenerator, ESPMode::ThreadSafe> SoundGenerator;
	FThreadSafeBool bIsMuted = false;
};
