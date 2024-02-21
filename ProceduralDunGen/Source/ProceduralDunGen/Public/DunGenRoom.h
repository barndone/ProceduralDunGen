// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DunGenRoom.generated.h"

UCLASS()
class ADunGenRoom : public AActor
{
	GENERATED_BODY()

private:

	/*
* List of entryways/exits that are "open" for this given room
*/
	UPROPERTY(VisibleAnywhere)
	TArray<class UDunGenDoor*> OpenPortals;

	/*
* List of entryways/exits that are "closed" for this given room
*/
	UPROPERTY(VisibleAnywhere)
	TArray<class UDunGenDoor*> ClosedPortals;

	//	TODO: make rooms keep track of adjacent rooms rather than paired doors
		/*
* List of rooms that have a connection to this room
*/
	UPROPERTY(VisibleAnywhere)
	TArray<class ADunGenRoom*> ConnectedRooms;



	//	TODO: uncomment UPROPERTY when implemented
	//	UPROPERTY(EditAnywhere, meta = (ClampMin=0.0f, ClampMax=1.0f, AllowPrivateAccess = true))
	float InitialSpawnWeight;
	float CurrentSpawnWeight;

	float BranchingChance;
	bool EndRoom = false;
	
public:	
	// Sets default values for this actor's properties
	ADunGenRoom();

	UPROPERTY()
		class UBoxComponent* RoomColl;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;

public:	

	float GetCurrentSpawnWeight() const;
	void UpdateCurrentSpawnWeight(float delta);
	bool IsEndRoom() const;
	int GetOpenDoors() const;
	TArray<class UDunGenDoor*> GetOpenPortals();
	void SetPortalAsUtilized(int index);
	UDunGenDoor* GetPortalByName(UDunGenDoor* stalePortal);
	bool CheckForRoomOverlaps(UBoxComponent* otherColl);
	UDunGenDoor* GetLastClosedPortal();

};
