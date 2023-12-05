# Odin to Wwise Adapter for Unreal Engine

This Unreal Engine sample project shows how to implement and use a class that passes the [Odin Voice Chat](https://www.4players.io/odin/)'s data to the [Wwise Audio Engine](https://www.audiokinetic.com/en/library/edge/?source=UE4&id=index.html).

## Pre-requisities

To be able to use the project you will need to have access to the Wwise Software Suite and their Unreal Plugin. You can get them [here](https://www.audiokinetic.com/en/library/edge/?source=UE4&id=index.html).
Also you will need to install the Odin Voice Chat Plugin for Unreal, available [here](https://github.com/4Players/odin-sdk-unreal/releases) or in the [Unreal Marketplace](https://www.unrealengine.com/marketplace/en-US/product/4players-odin-sdk).

In order to fully understand the sample and the code feel free to visit our [detailed integration guide](https://www.4players.io/odin/guides/unreal/odin-wwise/).

## Quick Start

The class is implemented in [this header](/Source/OdinTestProject/Public/AkOdinInputComponent.h) and [this source file](/Source/OdinTestProject/Private/AkOdinInputComponent.cpp). It can be assigned in your Player Character as a Component to use the Player Character's positional data for any attenuation. See the `Content/OdinContent/Blueprints/BP_OdinCharacter.uasset` in the sample project for reference. The `Content/OdinContent/Blueprints/C_OdinClient.uasset` blueprint showcases how to start passing incoming Voice Chat streams to Wwise using the provided adapter-class.
