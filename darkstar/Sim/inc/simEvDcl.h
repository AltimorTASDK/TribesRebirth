//--------------------------------------------------------------------------- 
//
// Public definitions of event types, query types, and Id's
//
//--------------------------------------------------------------------------- 

#define SIM_ID_BLOCK_SHIFT    (10)       // block size of 1024 id's
#define SIM_RESERVED_ID_LIMIT (1 << SIM_ID_BLOCK_SHIFT)
#define SimRangeNum(x,y)	   ((x) + (y))

// SimObjectId 0 reserved for the manager:
#define SimIdRange            (1)

// event/query types run from 1024 to 2047
// 10 bits are sent across the wire

#define SimTypeRange          (1024)
#define SimObjTypeRange       (1)      // object types for ghosts
#define AppIdRange            (SIM_RESERVED_ID_LIMIT/2)    // halfway 

// if the number of SimEventTypes gets > 50, increase AppTypeRange
#define AppTypeRange          (SimTypeRange + 50)
#define AppObjTypeRange       (SimObjTypeRange + 100)

// manager Id range - used by netCSDelegate to assign ids to managers
#define ManagerIdRange        2
typedef unsigned int SimObjectId;
typedef unsigned int SimIdBlock;


//--------------------------------------------------------------------------- 
// ID's

#define ServerManagerId                   2048  // ManagerIdRange * range block size

#define SimActionHandlerId                SimRangeNum(SimIdRange,0)
#define SimResourceObjectId               SimRangeNum(SimIdRange,1)
#define SimCanvasSetId                    SimRangeNum(SimIdRange,2)   
#define SimRouterSetId                    SimRangeNum(SimIdRange,3)   
#define SimLookupSetId                    SimRangeNum(SimIdRange,4)
#define SimFrameEndNotifySetId            SimRangeNum(SimIdRange,5)
#define SimRootContainerId                SimRangeNum(SimIdRange,6)
#define SimTerrainId                      SimRangeNum(SimIdRange,7)

//--------------------------------------

#define SimCameraSetId                    SimRangeNum(SimIdRange,8)
#define SimConsoleId                      SimRangeNum(SimIdRange,9)
#define SimPingObjectId                   SimRangeNum(SimIdRange,10)
#define SimTimerSetId                     SimRangeNum(SimIdRange,11)
#define SimRenderSetId                    SimRangeNum(SimIdRange,12)
#define SimLightSetId                     SimRangeNum(SimIdRange,13)
#define SimPersistManSetId                SimRangeNum(SimIdRange,14)
#define SimCollisionManagerId             SimRangeNum(SimIdRange,15)
#define LSStampSetId                      SimRangeNum(SimIdRange,16)
#define SimDefaultRouterId                SimRangeNum(SimIdRange,17)
#define SimToolSetId                      SimRangeNum(SimIdRange,18)
#define SimTedSetId                       SimRangeNum(SimIdRange,19)
#define SimSoundFXObjectId                SimRangeNum(SimIdRange,20)
#define SimPreferenceSetId                SimRangeNum(SimIdRange,21)
#define SimCameraMountSetId               SimRangeNum(SimIdRange,22)
#define SimInputManagerId                 SimRangeNum(SimIdRange,23)
#define SimInputConsumerSetId             SimRangeNum(SimIdRange,24)
#define SimDefaultTagDictionaryId         SimRangeNum(SimIdRange,25)
#define SimEditSetId                      SimRangeNum(SimIdRange,26)
#define SimMissionGroupId                 SimRangeNum(SimIdRange,27)
#define SimDefaultDebrisTableId           SimRangeNum(SimIdRange,28)
#define SimDefaultExplosionTableId        SimRangeNum(SimIdRange,29)
#define SimMissionEditorId                SimRangeNum(SimIdRange,30)
#define SimTEDId                          SimRangeNum(SimIdRange,31)
#define SimITRTimerSetId                  SimRangeNum(SimIdRange,32)
#define SimGhostAlwaysSetId               SimRangeNum(SimIdRange,33)
#define SimCSDelegateId                   SimRangeNum(SimIdRange,34)
#define GhostManagerSetId                 SimRangeNum(SimIdRange,35)
#define GhostManagerId                    SimRangeNum(SimIdRange,36)
#define PacketStreamSetId                 SimRangeNum(SimIdRange,37)
#define SimSoundSequenceObjectId          SimRangeNum(SimIdRange,38)
#define SimCleanupSetId                   SimRangeNum(SimIdRange,39)
#define SimDefaultFireTableId             SimRangeNum(SimIdRange,40)
#define SimPathManagerId                  SimRangeNum(SimIdRange,41)
#define SimConsoleSchedulerId             SimRangeNum(SimIdRange,42)
#define NamedGuiSetId                     SimRangeNum(SimIdRange,43)
#define TaggedGuiSetId                    SimRangeNum(SimIdRange,44)
#define IRCClientObjectId                 SimRangeNum(SimIdRange,45)
#define SimRedbookSetId                   SimRangeNum(SimIdRange,46)
#define SimPreLoadManagerId               SimRangeNum(SimIdRange,47)


//------------------------------------------------------------------------------
// Event Type Id's

#define SimTimeEventType                  SimRangeNum(SimTypeRange,0)
#define SimMessageEventType               SimRangeNum(SimTypeRange,1)
#define SimLoseFocusEventType             SimRangeNum(SimTypeRange,2)
#define SimGainFocusEventType             SimRangeNum(SimTypeRange,3)
#define SimGroupObjectIdEventType         SimRangeNum(SimTypeRange,4)
#define SimObjectTransformEventType       SimRangeNum(SimTypeRange,5)
#define SimFrameEndNotifyEventType        SimRangeNum(SimTypeRange,6)
#define SimGhostUpdateEventType           SimRangeNum(SimTypeRange,7)
#define SimConsoleEventType               SimRangeNum(SimTypeRange,8)
#define SimPingEventType                  SimRangeNum(SimTypeRange,9)
#define SimTimerEventType                 SimRangeNum(SimTypeRange,10)
#define SimActionEventType                SimRangeNum(SimTypeRange,11)
#define SimCollisionEventType             SimRangeNum(SimTypeRange,12)
#define SimCollisionWakeupEventType       SimRangeNum(SimTypeRange,13)
#define SimTerrainApplyEventType          SimRangeNum(SimTypeRange,14)
#define SimSoundFXEventType               SimRangeNum(SimTypeRange,15)
#define SimPreferenceEventType            SimRangeNum(SimTypeRange,16)
#define SimGuiMessageEventType            SimRangeNum(SimTypeRange,17)
#define SimGuiEventType                   SimRangeNum(SimTypeRange,18)
#define SimCameraNetEventType             SimRangeNum(SimTypeRange,19)
#define SimCameraMountEventType           SimRangeNum(SimTypeRange,20)
#define SimCameraUnmountEventType         SimRangeNum(SimTypeRange,21)
#define SimInputEventType                 SimRangeNum(SimTypeRange,22)
#define SimInputActivateEventType         SimRangeNum(SimTypeRange,23)
#define Sim3DMouseEventType               SimRangeNum(SimTypeRange,24)
#define SimEditEventType                  SimRangeNum(SimTypeRange,25)
#define SimDebrisEventType                SimRangeNum(SimTypeRange,26)
#define SimITRTimerEventType              SimRangeNum(SimTypeRange,27)
#define SimDynamicLightUpdateEventType    SimRangeNum(SimTypeRange,28)
#define SimDynamicLightDecRefCntEventType SimRangeNum(SimTypeRange,29)
#define SimIgnorableEventType             SimRangeNum(SimTypeRange,30)
#define SimMovementCollisionEventType     SimRangeNum(SimTypeRange,31)
#define SimTriggerEventType               SimRangeNum(SimTypeRange,32)
#define RemoteCreateEventType             SimRangeNum(SimTypeRange,33)
#define SimPolledUpdateEventType          SimRangeNum(SimTypeRange,34)
#define SimSoundSequenceEventType         SimRangeNum(SimTypeRange,35)
#define NetGhostAlwaysDoneEventType       SimRangeNum(SimTypeRange,36)
#define SimPathEventType                  SimRangeNum(SimTypeRange,37)
#define SimCameraRemountEventType         SimRangeNum(SimTypeRange,38)
#define SimMCINotifyEventType             SimRangeNum(SimTypeRange,39)
#define SimMIXNotifyEventType             SimRangeNum(SimTypeRange,40)
#define SimRegisterTextureEventType       SimRangeNum(SimTypeRange,41)



// don't go past SimTypeRange, 50


//------------------------------------------------------------------------------
// Query Type Id's

#define SimCameraQueryType                SimRangeNum(SimTypeRange,0)
#define SimObjectTransformQueryType       SimRangeNum(SimTypeRange,1)
#define SimRenderQueryImageType           SimRangeNum(SimTypeRange,2)
#define SimLightQueryType                 SimRangeNum(SimTypeRange,3)
#define SimPersistAddQueryType            SimRangeNum(SimTypeRange,4)
#define SimPersistTimeoutQueryType        SimRangeNum(SimTypeRange,5)
#define SimImageTransformQueryType        SimRangeNum(SimTypeRange,6)
#define SimCollisionImageQueryType        SimRangeNum(SimTypeRange,7)
#define SimLookupQueryType                SimRangeNum(SimTypeRange,8)
#define SimInputPriorityQueryType         SimRangeNum(SimTypeRange,9)
#define SimWaypointQueryType              SimRangeNum(SimTypeRange,11)
#define SimPathQueryType                  SimRangeNum(SimTypeRange,12)
