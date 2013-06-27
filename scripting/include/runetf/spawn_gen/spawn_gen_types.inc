enum RuneGen
{
	Id,
	Float:g_ori[3],
	Float:g_ang[3],
	Float:g_force,
	bool:g_StartDisabled,
};

#define RUNE_INDEX_SIZE 4

#define MAX_EVENTGEN_NAME   16
#define MAX_EVENTGEN_CNAME 32
#define MAX_EVENT_OUTPUTNAME 32
#define MAX_EVENT_ENTITY_OUTPUT 64

/*
	When an output triggered, the action to peform on the rune cluster
*/
enum ClusterGenType
{
	GenEvent_None = 0,
	GenEvent_Add = 1, 	//add cluster into global spawn timer
	GenEvent_Remove, // remove cluster from global spawn timer
	GenEvent_Spawn,   // instantly spawn the cluster 
	GenEvent_Toggle,   // add to active spawner if disabled, disable if actively spawning
	GenEvent_Last				//place holder, not used
};

#define MAX_CLUSTER_LEN 16

enum EventHookType
{
	GenEvent_None = 0,
	GenEvent_ByClass = 1, //Event is registered with HookEntityOutput
  GenEvent_ByEnt,				//Event is registered with HookSingleEntityOutput
	GenEvent_BySubClass,   //Event is registered on all entities sharing a common root class
	GenEvent_BySubEnt      //Event is registered on all entities sharing a common root entity name
};

enum RuneCluster
{
	String:ClusterName[MAX_CLUSTER_LEN],
	Handle:EventArray,		//Array of GenEvent
	EntityOutput:EventTriggered,
};


enum GenEvent
{
	ClusterGenType:ClusterType,
	EventHookType:EventType,
	String:EventCName[MAX_EVENTGEN_CNAME],
	String:EventEntity[MAX_EVENTGEN_CNAME],
	String:EventOutput[MAX_EVENT_OUTPUTNAME],
	EntityOutput:EventTriggered,
};


#define LAST_RUNE_CVAR_FLOAT 2
enum GenRuneInfo
{
	Float:fSpawnInterval,
	Float:fRuneLifeTime,
	Float:fRuneNamedLifeTime,
	bool:bRoundStartClear,
	bool:bRoundEndClear,
	bool:bRuneEnable,
};


new bool:steamtools = false;
