// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "AkAudioInputComponent.h"
#include "AkOdinInputComponent.generated.h"


class OdinMediaSoundGenerator;
class UOdinPlaybackMedia;

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UAkOdinInputComponent : public UAkAudioInputComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAkOdinInputComponent(const class FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "Odin|Sound")
	void AssignOdinMedia(UPARAM(ref) UOdinPlaybackMedia*& Media);

	virtual void GetChannelConfig(AkAudioFormat& AudioFormat) override;
	virtual bool FillSamplesBuffer(uint32 NumChannels, uint32 NumSamples, float** BufferToFill) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinPlaybackMedia* PlaybackMedia = nullptr;
	TSharedPtr<OdinMediaSoundGenerator, ESPMode::ThreadSafe> SoundGenerator;
	UPROPERTY()
	TArray<float> Buffer;
};