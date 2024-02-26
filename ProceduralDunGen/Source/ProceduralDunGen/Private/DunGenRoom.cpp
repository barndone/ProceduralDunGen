// Fill out your copyright notice in the Description page of Project Settings.


#include "DunGenRoom.h"
#include "DunGenDoor.h"
#include "Components/BoxComponent.h"
#include "Engine/SCS_Node.h"


// Sets default values
ADunGenRoom::ADunGenRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CurrentSpawnWeight = InitialSpawnWeight;
	
}

void ADunGenRoom::GetRoomColliders()
{
	GetComponents(RoomColls);
}

// Called when the game starts or when spawned
void ADunGenRoom::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
EDataValidationResult ADunGenRoom::IsDataValid(TArray<FText>& ValidationErrors)
{
	//	cache result as parent validation
	EDataValidationResult result = Super::IsDataValid(ValidationErrors);
	
	//	get the class of this CDO and cast it to a blueprint generated class
	UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(this->GetClass());

	//	list of ActorBPNodes after constructing the BPGC
	const TArray<USCS_Node*>& ActorBPNodes= BPGC->SimpleConstructionScript->GetAllNodes();

	//	flags for driving validation
	bool hasBoxVolume = false;
	bool hasDoorComponent = false;

	//	iterate through all nodes
	for (const USCS_Node* val : ActorBPNodes)
	{
		//	check for early exit
		if (hasBoxVolume && hasDoorComponent)
		{
			break;
		}
		//	if we match with a box component, assign corresponding flag to true
		if (val->ComponentClass == UBoxComponent::StaticClass())
		{
			hasBoxVolume = true;
		}
		else
		{
			//	if we match with a UDunGenDoor component, assign corresponding flag to true
			if (val->ComponentClass == UDunGenDoor::StaticClass())
			{
				hasDoorComponent = true;
			}
		}
	}

	//	if the flag for the door component is still false, throw validation error and update result
	if (!hasDoorComponent)
	{
		ValidationErrors.Add(FText::FromString("No doors attached to " + this->GetName()));
		result = EDataValidationResult::Invalid;
	}

	//	if the flag for the box component is still false, throw validation error and update result
	if (!hasBoxVolume)
	{
		ValidationErrors.Add(FText::FromString("No collision volume attached to " + this->GetName()));
		result = EDataValidationResult::Invalid;
	}

	return result;
}
#endif

float ADunGenRoom::GetCurrentSpawnWeight() const
{
	return CurrentSpawnWeight;
}

void ADunGenRoom::UpdateCurrentSpawnWeight(float delta)
{
	CurrentSpawnWeight += delta;
}

bool ADunGenRoom::IsEndRoom() const
{
	return EndRoom;
}

int ADunGenRoom::GetOpenDoors() const
{
	return OpenPortals.Num();
}

TArray<class UDunGenDoor*> ADunGenRoom::GetPortals()
{
	if (OpenPortals.Num() == 0 && ClosedPortals.Num() == 0)
	{
		GetComponents(OpenPortals);
	}
	return OpenPortals;
}

void ADunGenRoom::SetPortalAsUtilized(int index)
{
	if (OpenPortals.Num() == 0 && ClosedPortals.Num() == 0)
	{
		GetComponents(OpenPortals);
	}
	ClosedPortals.AddUnique(OpenPortals[index]);
	OpenPortals[index]->Utilized = true;
	OpenPortals.RemoveAt(index);
}

UDunGenDoor* ADunGenRoom::GetPortalByName(UDunGenDoor* stalePortal)
{
	if (OpenPortals.Contains(stalePortal))
	{
		return stalePortal;
	}
	else
	{
		auto name = stalePortal->GetFName();

		if (OpenPortals.Num() == 0 && ClosedPortals.Num() == 0)
		{
			//	populate portals list
			GetPortals();
		}

		for (auto& val : OpenPortals)
		{
			if (val->GetFName() == name)
			{
				return val;
			}
		}
		return nullptr;
	}
}

bool ADunGenRoom::CheckForRoomOverlaps(UBoxComponent* otherColl)
{
	FMTDResult _;

	return RoomColl->ComputePenetration(_, otherColl->GetCollisionShape(), otherColl->GetComponentLocation(), FQuat(otherColl->GetComponentRotation()));
}

bool ADunGenRoom::CheckForRoomOverlaps(TArray<UShapeComponent*> otherColls)
{
	bool result = false;
	FMTDResult _;

	bool checkForBounds = false;
	while (!checkForBounds)
	{
		for (auto& thisCol : RoomColls)
		{
			for (auto& otherCol : otherColls)
			{
				result = thisCol->ComputePenetration(_, otherCol->GetCollisionShape(), 
													otherCol->GetComponentLocation(), 
													FQuat(otherCol->GetComponentRotation()));
				checkForBounds = result;
			}
		}
		checkForBounds = true;
	}

	return result;
}

UDunGenDoor* ADunGenRoom::GetLastClosedPortal()
{
	return ClosedPortals.Last();
}

