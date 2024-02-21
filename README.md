
# ProcDunGen Unreal Plugin v0.1 Documentation
Procedural Dungeon Layout generation plugin for Unreal Engine ver. 4.27

----
## Table Of Contents
<!-- TOC start (generated with https://github.com/derlin/bitdowntoc) -->

   * [Importing the plugin](#importing-the-plugin)
   * [Step-By-Step Building your own DunGenRoom BP](#step-by-step-building-your-own-dungenroom-bp)
   * [The DunGenerator actor](#the-dungenerator-actor)
- [C++ Class Documentation](#c-class-documentation)
   * [DunGenerator](#dungenerator)
      + [Properties](#properties)
         - [bool RoomLimit](#bool-roomlimit)
         - [int MaxRoomCount](#int-maxroomcount)
         - [bool UsePreDefinedSeed](#bool-usepredefinedseed)
         - [int32 RandomSeedVal](#int32-randomseedval)
         - [TArray<ADunGenRoom*> EntryRooms](#tarray-entryrooms)
         - [TArray<ADunGenRoom*> ExitRooms](#tarray-exitrooms)
         - [TArray<ADunGenRoom*> DungeonRooms](#tarray-dungeonrooms)
         - [TArray<ADunGenRoom*> SpawnedRooms](#tarray-spawnedrooms)
         - [bool GenerateOnStart](#bool-generateonstart)
      + [Functions](#functions)
         - [CleanUp()](#cleanup)
         - [GenerateDungeon()](#generatedungeon)
         - [UDunGenDoor* GetValidExit(ADunGenRoom* room, int& absIdx)](#udungendoor-getvalidexitadungenroom-room-int-absidx)
         - [ADunGenRoom* SpawnValidRoom(UDunGenDoor* entrance)](#adungenroom-spawnvalidroomudungendoor-entrance)
         - [RotateAroundPoint(UDunGenDoor* prevRoomExit, UDunGenDoor* curRoomEntry, ADunGenRoom* curRoom)](#rotatearoundpointudungendoor-prevroomexit-udungendoor-curroomentry-adungenroom-curroom)
         - [RepopulateBranchingRooms()](#repopulatebranchingrooms)
   * [DunGenRoom](#dungenroom)
      + [Properties](#properties-1)
         - [UBoxComponent* RoomColl](#uboxcomponent-roomcoll)
      + [Functions](#functions-1)
         - [int GetOpenDoors() const](#int-getopendoors-const)
         - [TArray<class UDunGenDoor*> GetOpenPortals()](#tarray-getopenportals)
         - [SetPortalAsUtilized(int index)](#setportalasutilizedint-index)
         - [UDunGenDoor* GetPortalByName(UDunGenDoor* stalePortal)](#udungendoor-getportalbynameudungendoor-staleportal)
         - [bool CheckForRoomOverlaps(UBoxComponent* otherColl)](#bool-checkforroomoverlapsuboxcomponent-othercoll)
         - [UDunGenDoor* GetLastClosedPortal()](#udungendoor-getlastclosedportal)

<!-- TOC end -->

<!-- TOC --><a name="importing-the-plugin"></a>
## Importing the plugin
1. After downloading and extracting the plugin, navigate to the Unreal Project you would like to add it to.
2. Create a Plugins folder if your project does not already have one: 

![Example project directory](ProjectFolderExample.png)

3. Copy the ProceduralDunGen folder into the plugins folder.
4. If you would like to include ProcDunGen starter content, copy that folder into your project's content folder.

<!-- TOC --><a name="step-by-step-building-your-own-dungenroom-bp"></a>
## Step-By-Step Building your own DunGenRoom BP
1. Create a blueprint class using the DunGenRoom C++ class as a base:
 
![Screenshot of blueprint creation showing the proper class](DunGenRoomActorCreation.png)

2. Set up the geometry for your room:
   - If using static meshes, create a static mesh component and assign the desired models to it
   - Otherwise, you can use primitive shapes to define the geometry of your room in the prototype stage.
   
   ![Example Room](ExampleRoomGeometry.png)

3. Add a BoxComponent to define the area your room occupies:
    
    **At least one box component is required for each room**  
   - Use a convention that best suits your project either:
      - Size the component such that it completely encapsulates the geometry of the room (if you do not want walls to overlap)
      - OR size it so that it is only occupying the inner bounds of the room (if you dont care if walls overlap with each other)
    - As a best practice, keep the scale of the volume uniform and adjust the size using the shape field:
     
    ![Example Room with a BoxComponent](ExampleBoxComponentSetUp.png)

4. Add a DunGenDoor component to the room:
    
    **At least one DunGenDoor is required for each room**

    - Place the door at the location you would want to act as the "pivot" between rooms. Rooms will be placed such that their paired doors will have the same world position and face opposite directions. 
    - After placing the door at your desired position, rotate it so that the X axis points in the direction you want the door to point to:
    
    ![Example Room with doors added](ExampleDoorSetUp.png)

You can also refer to the StarterContent rooms to get ideas for how to set up your own rooms.

<!-- TOC --><a name="the-dungenerator-actor"></a>
## The DunGenerator actor
You have a few options when setting up and using your DunGenerator actor.
1. Specifying a set number of rooms for your dungeon:
    - Toggle the RoomLimit field on your DunGenerator actor.
    - This will cause the MaxRoomCount field to become visible and editable.
2. Specify the use of a random or pre determined seed:
    - Toggle the UsePredefinedSeed field for your DunGenerator actor.
    - This will cause the RandomSeedVal to become visible and editable.
    - RandomSeedVal has a min/max corresponding to the Int32 data type.
3. Specify if you want the dungeon to be generated on "Play":
    - toggle the GenerateOnStart field.
4. Add your rooms to their corresponding lists:
    - EntryRooms - all rooms you want to be the *first* room the generator will spawn.
    - ExitRooms - all rooms you want to be the *last* room the generator will spawn.
    - DungeonRooms - all other rooms that you want to make up the rest of the dungeon.
5. Generate dungeons!
    - Use the "GenerateDungeon" button to generate a dungeon given the constraints and rooms specified above.
    - NOTE: The entry point of the dungeon will *always* occupy the *same world location and rotation as the dungeon generator!*
    - To quickly remove a previously spawned dungeon, press the "CleanUp" button.

----
<!-- TOC --><a name="c-class-documentation"></a>
# C++ Class Documentation
<!-- TOC --><a name="dungenerator"></a>
## DunGenerator
<!-- TOC --><a name="properties"></a>
### Properties
<!-- TOC --><a name="bool-roomlimit"></a>
#### bool RoomLimit
- If set to true, will generate MaxRoomCount rooms unless that number is not possible without rooms overlapping. 

- Toggles visibility of MaxRoomCount.

- If set to false, will generate rooms randomly until an exit room is generated. Exit room is determined by if TArray ExitRooms contains the spawned room.  

<!-- TOC --><a name="int-maxroomcount"></a>
#### int MaxRoomCount
- Controls the number of rooms the dungeon generator will spawn if RoomLimit is set to 'true'

<!-- TOC --><a name="bool-usepredefinedseed"></a>
#### bool UsePreDefinedSeed
- If set to true, will allow the user to specify the in32 seed used to generate the rooms. 

- Toggles visibility of RandomSeedVal.

- If set to false, will generate a random seed on compile time.

<!-- TOC --><a name="int32-randomseedval"></a>
#### int32 RandomSeedVal
- Int32 value that defines the random number generator seed used by the Dungeon Generator.

<!-- TOC --><a name="tarray-entryrooms"></a>
#### TArray<ADunGenRoom*> EntryRooms
- List containing all rooms that you would like to use to start the dungeon.

- Only one of these rooms will be spawned per generation. 

- This is guaranteed to be the first room spawned by the generator.

<!-- TOC --><a name="tarray-exitrooms"></a>
#### TArray<ADunGenRoom*> ExitRooms
- List containing all rooms that you would like to use to end the dungeon.

- Only one of these rooms will be spawned per dungeon. 

- This is guaranteed to be the last room spawned by the generator.

<!-- TOC --><a name="tarray-dungeonrooms"></a>
#### TArray<ADunGenRoom*> DungeonRooms
- List of rooms you would like to use to fill the dungeon.

- If defining a room limit, N rooms will spawn where N = MaxRoomCount - 2.

<!-- TOC --><a name="tarray-spawnedrooms"></a>
#### TArray<ADunGenRoom*> SpawnedRooms
- List of rooms the dungeon generator spawned after generating a dungeon. 


<!-- TOC --><a name="bool-generateonstart"></a>
#### bool GenerateOnStart
- If true, when you BeginPlay on this level, it will generate a completely new dungeon.

<!-- TOC --><a name="functions"></a>
### Functions
<!-- TOC --><a name="cleanup"></a>
#### CleanUp()
- Iterates through the list of SpawnedRooms and marks each spawned DunGenRoom Actor for Destroy.

- Clears out list after destruction.

<!-- TOC --><a name="generatedungeon"></a>
#### GenerateDungeon()

- Responsible for generating a dungeon.
- If RoomLimit is set to true, the generator will generate rooms until that value is reached OR there are no valid rooms the generator can spawn. If RoomLimit is set to false, it will generate rooms until an ExitRoom is spawned.


<!-- TOC --><a name="udungendoor-getvalidexitadungenroom-room-int-absidx"></a>
#### UDunGenDoor* GetValidExit(ADunGenRoom* room, int& absIdx)
- Given a room, will return a valid Door for the given room. Additionally assigns to a passed in integer value for having access to the index of the valid Door in the OpenDoors() list on the given room.

<!-- TOC --><a name="adungenroom-spawnvalidroomudungendoor-entrance"></a>
#### ADunGenRoom* SpawnValidRoom(UDunGenDoor* entrance)
- Responsible for spawning a valid room. Behavior changes if there is a RoomLimit in place.

<!-- TOC --><a name="rotatearoundpointudungendoor-prevroomexit-udungendoor-curroomentry-adungenroom-curroom"></a>
#### RotateAroundPoint(UDunGenDoor* prevRoomExit, UDunGenDoor* curRoomEntry, ADunGenRoom* curRoom)
- Helper method used to rotate a room so that the previous exit and current Entry face opposite directions.

<!-- TOC --><a name="repopulatebranchingrooms"></a>
#### RepopulateBranchingRooms()
- Helper method used to repopulate the private list PossibleBranchRooms that is used if a room overlap is detected. Repopulated *only* when an overlap is detected. Valid rooms are those with any open doorways. 

<!-- TOC --><a name="dungenroom"></a>
## DunGenRoom
<!-- TOC --><a name="properties-1"></a>
### Properties
<!-- TOC --><a name="uboxcomponent-roomcoll"></a>
#### UBoxComponent* RoomColl
- Necessary BoxComponent used to determine if a room overlaps with another room.

<!-- TOC --><a name="functions-1"></a>
### Functions

<!-- TOC --><a name="int-getopendoors-const"></a>
#### int GetOpenDoors() const
- Returns the number of Doors contained in OpenDoors.

<!-- TOC --><a name="tarray-getopenportals"></a>
#### TArray<class UDunGenDoor*> GetOpenPortals()
- Returns the list OpenDoors list of this room.

<!-- TOC --><a name="setportalasutilizedint-index"></a>
#### SetPortalAsUtilized(int index)
- Given an index, mark the given room as utilized.
- Will move the marked room to the ClosedDoors list and remove it from the OpenDoors list. 

<!-- TOC --><a name="udungendoor-getportalbynameudungendoor-staleportal"></a>
#### UDunGenDoor* GetPortalByName(UDunGenDoor* stalePortal)
- Helper method for returning a door attached to this room given a scene component name. 

<!-- TOC --><a name="bool-checkforroomoverlapsuboxcomponent-othercoll"></a>
#### bool CheckForRoomOverlaps(UBoxComponent* otherColl)
- Helper method to determine if two rooms are overlapping.
- Uses the RoomColl BoxComponent.


<!-- TOC --><a name="udungendoor-getlastclosedportal"></a>
#### UDunGenDoor* GetLastClosedPortal()
- Helper method, used to return the last closed door of this room.
- Used if a valid room could not be spawned and we need to try the next possible room. 
