# Odin to Wwise Adapter for Unreal Engine

This Unreal Engine sample project shows how to implement and use a class that passes the [Odin Voice Chat](https://odin.4players.io/voice-chat/)'s data to the [Wwise Audio Engine](https://www.audiokinetic.com/en/library/edge/?source=UE4&id=index.html).

![Wwise and Odin](/Documentation/wwise_with_odin_header.jpg)

## Odin Version

The main branch of this repository uses the new ODIN Voice Plugin Version 2.x. If you're looking for the Wwise integration for ODIN Voice Version 1.x, please take a look at the  [Odin V1 Wwise Adapter Branch](https://github.com/4Players/odin-unreal-wwise/tree/v1).


## Pre-requisities

The project uses the following Versions:

- Unreal Engine Version 5.5.4
- Wwise Version v2025.1.6

## Installation

This repository does NOT contain the required Wwise Plugins due to their large file sizes. To start this project, you will need to manually install the Wwise plugins using the Wwise Launcher. Please take a look at [the Audiokinetic Wwise + Unreal Integration Guide](https://www.audiokinetic.com/en/public-library/launcher_2025.3.3.5754/?source=InstallGuide&id=integrating_wwise_into_an_unreal_project) for installation guidelines.

The ODIN Voice Plugin is preinstalled in the project and can be found under `Plugins/Odin`. 

## Testing the project

To test the project, please open up the `OddinWwiseMap` in `Content/_OdinContent/Maps`. Before starting PIE, go to the `Play Options` drop down menu und select the following settings:
1. Set `Number of Players` to 2
2. Set `Net Mode` to `Play As Listen Server` or `Play As Client`

After staring PIE, two Unreal game clients will connect both in multiplayer and in the ODIN Voice chat.

> [!IMPORTANT]
> When the clients are near each other, you will hear your own voice twice. This is due to Wwise playing audio from both clients and playing back audio for two audio listeners, so it is expected behaviour. To only hear the audio from the perspective of one client, you can do the following:
> 1. In the `Editor Preferences`, go to `Level Editor > Play > Multiplayer Options` and set `Run Under One Process` to false. This will make additional multiplayer clients start using their own processes, which will make Wwise playback audio as if a built version of your game was started.
> 2. Use the `M` button to mute or unmute one of the clients. This way you can only hear the audio from one of the clients.

## Quick Start

The `AkOdinIputComponent` replaces the `OdinSynthComponent`, which is ordinarily used for Voice Chat playback in your game. The workflow is as follows:
1. In the `On Room Peer Joined` event, create an `AkOdinInputComponent` or use an existing one.
2. Set the `Ak Audio Event` on the component to the `Play_VoiceChat` event created in Wwise.
3. Construct a Decoder based on the `Sample Rate` and `Is Stereo` value of the Input Component and register the Decoder to the Peer Id (default Odin Voice Chat workflow)
4. Call `Assign Odin Decoder` on the Input Component and supply the created Decoder.
5. Finally, call `Post Associated Audio Input Event` on the Input Component to start playing back voice chat using Wwise.

The [header file](/Source/OdinTestProject/Public/AkOdinInputComponent.h) and the [source file](/Source/OdinTestProject/Private/AkOdinInputComponent.cpp) of the component have been added in the project's source directory.

It can be added to actors in your world, for example on Player Character's to enable attenuation (= the player's voice will originate from their Character's positions). See the `Content/OdinContent/Blueprints/BP_OdinCharacter.uasset` in the sample project for reference. The `Content/OdinContent/Blueprints/C_OdinClient.uasset` blueprint showcases how to start passing incoming Voice Chat data to Wwise using the provided adapter-class.

### Remarks

In the project, the ODIN Voice Room ID is hardcoded (to tutorial), meaning all clients sharing the same Access Key connect to the same ODIN room. This works for small-scale testing with a few users on the same game server, but is unsuitable for production.
In production, generate a unique room name for each game session. For example, use the session ID from the Steam Online Subsystem as a prefix or suffix for the ODIN Voice room (e.g., session_12345_proximity_chat or global_radio_session_12345). This allows multiple voice chat types (proximity, radio, etc.) within the same game instance while ensuring players only connect to others in their specific game session.
