// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DunGenerator.generated.h"

UENUM()
enum class Flow : uint8
{
	NORTH,
	EAST,
	SOUTH,
	WEST,
	NONE
};

UCLASS()
class ADunGenerator : public AActor
{
	GENERATED_BODY()

private:
	/*
	* Controls wether the generated dungeon will be a certain length.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options", meta = (AllowPrivateAccess = true))
	bool RoomLimit = false;
	/*
	* The length at which a dungeon will generate to while RoomLimit is toggled.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options", meta = (EditCondition = "RoomLimit", EditConditionHides, AllowPrivateAccess = true))
	int MaxRoomCount = 10;

	int curRoomCount = 0;

	/*
	*	Controls wether or not the dungeon generator will use a pre-defined seed or a runtime generated seed
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options", meta = (AllowPrivateAccess = true))
	bool UsePredefinedSeed = false;

	/*
*	Random seed used in the dungeon generator
*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options", meta = (EditCondition = "UsePredefinedSeed", EditConditionHides, ClampMin = -2147483648, ClampMax = 2147483647, AllowPrivateAccess = true))
	int32 RandomSeedVal = 0;

	//	list containing rooms that have open doors and are an option for branching
	TArray<class ADunGenRoom*> PossibleBranchRooms;

	//	if true- limits branches to 3 or less-
	//	otherwise, determines wether a branch should become the main path on hitting 3 rooms
	bool HasMainPath = false;

	/*
	* List of rooms used as the first room of any generated dungeon.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options\|Rooms", meta = (AllowPrivateAccess = true))
	TArray<TSubclassOf<class ADunGenRoom>> EntryRooms;
	/*
	* List of rooms used as the first room of any generated dungeon.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options\|Rooms", meta = (AllowPrivateAccess = true))
	TArray<TSubclassOf<class ADunGenRoom>> ExitRooms;

	//	//	specialized Dungeon Rooms used to connect rooms
	//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options\|Rooms", meta = (AllowPrivateAccess = true))
	//	TArray<TSubclassOf<class ADunGenRoom>> Hallways;

	/*
	* List of rooms used as the filler for a generated dungeon.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options\|Rooms", meta = (AllowPrivateAccess = true))
	TArray<TSubclassOf<class ADunGenRoom>> DungeonRooms;

	//	list of possible spawnable rooms while not limiting the number of rooms the generator has to spawn 
	//	(combines end room list and dungeon rooms lists)
	TArray<TSubclassOf<class ADunGenRoom>> RoomPool;

	/*
* List of rooms that compose of the dungeon.
*/
	UPROPERTY(VisibleAnywhere)
	TArray<class ADunGenRoom*> SpawnedRooms;



	bool GenerateNavMesh = false;

	/*
* Controls wether a dungeon will be regenerated on beginning play
*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Parameters\|Generation Options", meta = (AllowPrivateAccess = true))
	bool GenerateOnStart = false;

	UFUNCTION()
	TSubclassOf<class ADunGenRoom> GetRandomEntry() const;

	UFUNCTION()
	TSubclassOf<class ADunGenRoom> GetRandomExit() const;
	
	template<typename T>
	int RandomIndexOfTArray(TArray<T> arr) const;

	
public:	
	// Sets default values for this actor's properties
	ADunGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	/*
* Generate a new dungeon.
*/
	UFUNCTION(CallInEditor)
	void GenerateDungeon();

	/*
* Delete the currently spawned dungeon.
*/
	UFUNCTION(CallInEditor)
	void CleanUp();


	class UDunGenDoor* GetValidExit(class ADunGenRoom* room, int & absIdx);
	class ADunGenRoom* SpawnValidRoom(class UDunGenDoor* entrance);
	void RotateAroundPoint(UDunGenDoor* prevRoomExit, UDunGenDoor* curRoomEntry, ADunGenRoom* curRoom);
	void RepopulateBranchingRooms();

};
