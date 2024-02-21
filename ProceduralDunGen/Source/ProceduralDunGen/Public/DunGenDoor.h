// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DunGenDoor.generated.h"

/*
* This component is used as the connection between rooms for the dungeon generator. Rooms will be spawned on, and then positioned such that "paired" doors are at the same position but face opposite directions.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UDunGenDoor : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDunGenDoor();

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UDunGenDoor* PairedPortal;

	UPROPERTY()
	bool Utilized = false;

	UPROPERTY()
	bool MainPathRoom = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};
