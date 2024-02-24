// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenerator.h"
#include "DunGenRoom.h"
#include "DunGenDoor.h"
#include <Components/ArrowComponent.h>
#include "Components/BoxComponent.h"
#include <cassert>
#include <Misc/UObjectToken.h>

// Sets default values
ADunGenerator::ADunGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//	init random seed
	//srand(time(0));
	UsePredefinedSeed ? FGenericPlatformMath::SRandInit(RandomSeedVal) : FGenericPlatformMath::SRandInit(FPlatformTime::Cycles());

;
}

// Called when the game starts or when spawned
void ADunGenerator::BeginPlay()
{
	Super::BeginPlay();
	if (GenerateOnStart)
	{
		GenerateDungeon();
	}
}
//
//	DunGenerator In Engine validation
//
#if WITH_EDITOR
EDataValidationResult ADunGenerator::IsDataValid(TArray<FText>& ValidationErrors)
{
	//	cache result as parent validation
	EDataValidationResult result = Super::IsDataValid(ValidationErrors);

	bool ExitRoomInEntryList = false;
	bool ExitRoomInRoomList = false;

	for (auto& val : ExitRooms)
	{
		if (EntryRooms.Contains(val))
		{
			ExitRoomInEntryList = true;
		}

		if (DungeonRooms.Contains(val))
		{
			ExitRoomInRoomList = true;
		}
	}

	if (ExitRoomInEntryList)
	{
		ValidationErrors.Add(FText::FromString("Exit room found in EntryRooms list. Remove entries present in ExitRooms from EntryRooms " + this->GetName()));
		result = EDataValidationResult::Invalid;
	}

	if (ExitRoomInRoomList)
	{
		ValidationErrors.Add(FText::FromString("Exit room found in DungeonRooms list. Remove entries present in ExitRooms from DungeonRooms " + this->GetName()));
		result = EDataValidationResult::Invalid;
	}

	return result;
}

void ADunGenerator::CheckForErrors()
{
	Super::CheckForErrors();

	FMessageLog MapCheck("MapCheck");

	bool ExitRoomInEntryList = false;
	bool ExitRoomInRoomList = false;

	for (auto& val : ExitRooms)
	{
		if (EntryRooms.Contains(val))
		{
			ExitRoomInEntryList = true;
		}

		if (DungeonRooms.Contains(val))
		{
			ExitRoomInRoomList = true;
		}
	}

	if (ExitRoomInEntryList)
	{
		MapCheck.Warning()->AddToken(FUObjectToken::Create(this))->AddToken(FTextToken::Create((FText::FromString("Exit room found in EntryRooms list. Remove entries present in ExitRooms from EntryRooms " + this->GetName()))));
	}

	if (ExitRoomInRoomList)
	{
		MapCheck.Warning()->AddToken(FUObjectToken::Create(this))->AddToken(FTextToken::Create((FText::FromString("Exit room found in DungeonRooms list. Remove entries present in ExitRooms from DungeonRooms " + this->GetName()))));
	}
}
#endif

void ADunGenerator::GenerateDungeon()
{
	if (RoomLimit)
	{
		for (auto& val : DungeonRooms)
		{
			RoomPool.Add(val);
		}
	}
	else
	{
		for (auto& val : DungeonRooms)
		{
			RoomPool.Add(val);
		}
		for (auto& val : ExitRooms)
		{
			RoomPool.Add(val);
		}
	}

	CleanUp();

	//	if we have a room count limit-
	if (RoomLimit)
	{

		ADunGenRoom* curRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(
			GetRandomEntry(),
			&GetTransform()));
		curRoom->RoomColl = Cast<UBoxComponent>(curRoom->GetComponentByClass(UBoxComponent::StaticClass()));
		SpawnedRooms.Add(curRoom);

		curRoomCount++;

		while (curRoomCount < MaxRoomCount)
		{
			int _;

			if (curRoom == nullptr)
			{
				curRoom = SpawnedRooms.Last();
			}

			UDunGenDoor* exit = GetValidExit(curRoom, _);
			
			if (_ < 0)
			{
				curRoom = nullptr;
				exit = SpawnedRooms.Last()->GetLastClosedPortal();
			}
			else
			{
				curRoom = SpawnValidRoom(exit);
			}

			if (curRoom == nullptr)
			{	
				//	if our current room has returned a nullptr, we need to check previous rooms
				RepopulateBranchingRooms();
				int branchRoomCt = 0;
				bool roomFound = false;
				int branchRoomIdx = PossibleBranchRooms.Num() - 1;

				ADunGenRoom* branchTarget = nullptr;

				while (branchRoomCt < PossibleBranchRooms.Num() && !roomFound)
				{

					//	Start at end of possibleBranchRooms, work to beginning
					branchTarget = PossibleBranchRooms[branchRoomIdx];

					if (branchTarget->GetOpenDoors() > 0 && branchTarget != nullptr)
					{
						exit = GetValidExit(branchTarget, _);
						//	if valid room is found, set roomFound to true
						curRoom = SpawnValidRoom(exit);

						if (curRoom != nullptr)
						{
							roomFound = true;
						}
					}
					++branchRoomCt;
					--branchRoomIdx;
				}

				//	if a room hasn't been found, replace last spawned room with an exit
				if (!roomFound || curRoom == nullptr)
				{
					curRoom = SpawnedRooms.Last();
					SpawnedRooms.Remove(curRoom);
					FVector spawnLoc = exit->GetComponentLocation();
					curRoom->Destroy();

					//auto lastExit = SpawnedRooms.Last()->GetLastClosedPortal();

					auto newRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(GetRandomExit(), &spawnLoc));
					auto newEntry = newRoom->GetPortals()[0];
					RotateAroundPoint(exit, newEntry, newRoom);
					//	location of the new room
					auto roomLoc = curRoom->GetActorLocation();
					//	location of the exit from the prev room
					auto exitLoc = exit->GetComponentLocation();
					//	location of the entrance to the new room
					auto entranceLoc = newEntry->GetComponentLocation();
					//	cur - offset
					FVector displacement = roomLoc - entranceLoc;
					curRoom->SetActorLocation(curRoom->GetActorLocation() + displacement);


					newRoom->SetPortalAsUtilized(0);
					exit->PairedPortal = newEntry;
					newEntry->PairedPortal = exit;

					curRoomCount = MaxRoomCount;
					SpawnedRooms.Add(newRoom);
					curRoom = newRoom;
					//	curRoomCount++;
				}
				else
				{
					curRoomCount++;
					SpawnedRooms.Add(curRoom);
				}
			}
			else
			{
				curRoomCount++;
				SpawnedRooms.Add(curRoom);
			}
		}

	}
	//	otherwise, generate rooms randomly
	else
	{
		ADunGenRoom* curRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(
			GetRandomEntry(),
			&GetTransform()));
		curRoom->RoomColl = Cast<UBoxComponent>(curRoom->GetComponentByClass(UBoxComponent::StaticClass()));
		SpawnedRooms.Add(curRoom);

		while (!ExitRooms.Contains(curRoom->GetClass()))
		{
			int _;

			UDunGenDoor* exit = GetValidExit(curRoom, _);

			if (_ < 0)
			{
				curRoom = nullptr;
				exit = SpawnedRooms.Last()->GetLastClosedPortal();
			}
			else
			{
				curRoom = SpawnValidRoom(exit);
			}

			//	implement additional while loop checking for valid rooms using possible branch rooms
			if (curRoom == nullptr)
			{
				RepopulateBranchingRooms();
				//	if our current room has returned a nullptr, we need to check previous rooms
				int branchRoomCt = 0;
				bool roomFound = false;
				int branchRoomIdx = PossibleBranchRooms.Num() - 1;

				ADunGenRoom* branchTarget = nullptr;
				
				while (branchRoomCt < PossibleBranchRooms.Num() && !roomFound)
				{

					//	Start at end of possibleBranchRooms, work to beginning
					branchTarget = PossibleBranchRooms[branchRoomIdx];

					if (branchTarget->GetOpenDoors() > 0)
					{
						exit = GetValidExit(branchTarget, _);
						//	if valid room is found, set roomFound to true
						curRoom = SpawnValidRoom(exit);

						if (curRoom != nullptr)
						{
							roomFound = true;
						}
					}
					++branchRoomCt;
					--branchRoomIdx;
				}

				//	if a room hasn't been found, replace last spawned room with an exit
				if (!roomFound || curRoom == nullptr)
				{
					curRoom = SpawnedRooms.Last();
					SpawnedRooms.Remove(curRoom);
					FVector spawnLoc = exit->GetComponentLocation();
					curRoom->Destroy();

					//auto lastExit = SpawnedRooms.Last()->GetLastClosedPortal();
					auto newRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(GetRandomExit(), &spawnLoc));
					auto newEntry = newRoom->GetPortals()[0];
					RotateAroundPoint(exit, newEntry, newRoom);
					//	location of the new room
					auto roomLoc = curRoom->GetActorLocation();
					//	location of the exit from the prev room
					auto exitLoc = exit->GetComponentLocation();
					//	location of the entrance to the new room
					auto entranceLoc = newEntry->GetComponentLocation();
					//	cur - offset
					FVector displacement = roomLoc - entranceLoc;
					curRoom->SetActorLocation(curRoom->GetActorLocation() + displacement);


					newRoom->SetPortalAsUtilized(0);
					exit->PairedPortal = newEntry;
					newEntry->PairedPortal = exit;

					SpawnedRooms.Add(newRoom);

					curRoom = newRoom;
				}
				else 
				{
					SpawnedRooms.Add(curRoom);
				}
			}

			else
			{
				SpawnedRooms.Add(curRoom);
			}
		}

		//	TODO: update spawn weights (slowly decrease normal rooms, increase exit room)
	}
	OnDungeonGenerated.Broadcast();
}

UDunGenDoor* ADunGenerator::GetValidExit(ADunGenRoom* room, int & absIdx)
{
	TArray<UDunGenDoor*> validPortals;
	TArray<int> validPortalIdx;
	TArray<UDunGenDoor*> allPortals = room->GetPortals();

	for (int i = 0; i < allPortals.Num(); ++i)
	{
		//	only care about this portal if it ISN'T utilized
		if (!allPortals[i]->Utilized)
		{
			validPortals.Add(allPortals[i]);
			validPortalIdx.Add(i);
		}
	}

	UDunGenDoor* result = nullptr;

	//	if there are more than one VALID portal attached to this room
	if (validPortals.Num() > 1)
	{
		auto randRoomIdx = RandomIndexOfTArray(validPortals);
		absIdx = validPortalIdx[randRoomIdx];
		room->SetPortalAsUtilized(absIdx);
		return validPortals[randRoomIdx];
	}
	//	no VALID portals attached to room
	else if (validPortals.Num() < 1)
	{
		absIdx = -1;
		return result;
	}
	//	only one valid portal
	else
	{
		absIdx = validPortalIdx[0];
		room->SetPortalAsUtilized(absIdx);
		return validPortals[0];
	}
}

ADunGenRoom* ADunGenerator::SpawnValidRoom(UDunGenDoor* entrance)
{

	//	STEPS:

	//	1. get random idx to iterate through the list at-

	int startingIdx = rand() % DungeonRooms.Num();
	int curIdx = startingIdx;
	FVector spawnPos = entrance->GetComponentLocation();

	//	2.1. pick entrance portal for given room
	//	2.2. init temp room at given idx-
	//	2.3. offset room position to be room position + entrance offset
	ADunGenRoom* curRoom = nullptr;
	UDunGenDoor* curRoomEntrance = nullptr;
	int portalIdx;

	bool NoOverlap = false;

	TArray<int> invalidRoomIdxs;

	//	TODO: handle invalid portal with no possible rooms

	//	while we aren't overlapping-
	while (!NoOverlap)
	{
		//	if we have a room limit
		if (RoomLimit)
		{
			//	wrap index to 0 if it is currently greater than the number of possible rooms
			if (curIdx >= DungeonRooms.Num())
			{
				curIdx = 0;
			}


			//	if the room is NOT the last room-
			if (curRoomCount < MaxRoomCount - 1)
			{
				//	spawn a temp room-
				curRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(DungeonRooms[curIdx], &spawnPos));
			}

			else
			{
				curRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(GetRandomExit(), &spawnPos));
			}
		}

		else
		{
			if (curIdx >= DungeonRooms.Num())
			{
				curIdx = 0;
			}

			curRoom = Cast<ADunGenRoom>(GetWorld()->SpawnActor(DungeonRooms[curIdx], &spawnPos));
		}


		
		curRoomEntrance = GetValidExit(curRoom, portalIdx);

		//	rotate current room such that: fwd of the prev room exit and cur room entrance point in opposite directions
		RotateAroundPoint(entrance, curRoomEntrance, curRoom);


		//	location of the new room
		auto roomLoc = curRoom->GetActorLocation();


		//	location of the exit from the prev room
		auto exitLoc = entrance->GetComponentLocation();

		//	location of the entrance to the new room
		auto entranceLoc = curRoomEntrance->GetComponentLocation();

		//	cur - offset
		FVector displacement = roomLoc - entranceLoc;

		curRoom->SetActorLocation(curRoom->GetActorLocation() + displacement);

		//	CHECK FOR COLLISION BETWEEN ROOMS

		curRoom->RoomColl = Cast<UBoxComponent>(curRoom->GetComponentByClass(UBoxComponent::StaticClass()));

		bool Overlapping = false;

			for (auto& val : SpawnedRooms)
			{
				if (curRoom->CheckForRoomOverlaps(val->RoomColls))
				{
					Overlapping = true;
				}
			}
		

		if (!Overlapping) { NoOverlap = true; }


		if (!NoOverlap) 
		{ 
			curRoom->Destroy();
			invalidRoomIdxs.AddUnique(curIdx);

			if (invalidRoomIdxs.Num() == DungeonRooms.Num())
			{
				return nullptr;
			}

			curIdx++;
		}
	}

	entrance->PairedPortal = curRoomEntrance;
	curRoomEntrance->PairedPortal = entrance;

	return curRoom;
}

void ADunGenerator::RotateAroundPoint(UDunGenDoor* prevRoomExit, UDunGenDoor* curRoomEntry, ADunGenRoom* curRoom)
{
	auto exitFwd = prevRoomExit->GetForwardVector();
	auto entryFwd = curRoomEntry->GetForwardVector();

	auto desiredDir = -exitFwd;

	auto curRot = entryFwd.Rotation();
	auto desiredRot = desiredDir.Rotation();


	FRotator theta = desiredRot - curRot;
	curRoom->SetActorRotation(theta);
}

void ADunGenerator::RepopulateBranchingRooms()
{
	PossibleBranchRooms.Empty();

	for (auto& val : SpawnedRooms)
	{
		//	if the number of open doors in this room is NOT zero
		if (val->GetOpenDoors() > 0)
		{
			//	add the candidate
			PossibleBranchRooms.AddUnique(val);
		}
	}
}

void ADunGenerator::CleanUp()
{
	curRoomCount = 0;

	for (auto& val : SpawnedRooms)
	{
		if (val)
		{
			val->Destroy();
		}
	}

	SpawnedRooms.Empty();
}

TSubclassOf<class ADunGenRoom> ADunGenerator::GetRandomEntry() const
{
	return EntryRooms[RandomIndexOfTArray(EntryRooms)];
}

TSubclassOf<class ADunGenRoom> ADunGenerator::GetRandomExit() const
{
	return ExitRooms[RandomIndexOfTArray(ExitRooms)];
}

template<typename T>
int ADunGenerator::RandomIndexOfTArray(TArray<T> arr) const
{
	return FGenericPlatformMath::Rand() % arr.Num();
}


