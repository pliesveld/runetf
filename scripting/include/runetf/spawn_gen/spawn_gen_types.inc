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
	Types of cluster events.
*/
enum _:ClusterGenType
{
	GenEvent_None = 0,
	GenEvent_Add = 1, 	//add cluster into global spawn timer
	GenEvent_Remove, // remove cluster from global spawn timer
	GenEvent_Spawn,   // instantly spawn the cluster 
	GenEvent_Toggle   // add to active spawner if disabled, disable if actively spawning
};

#define MAX_CLUSTER_LEN 16

/*
	Types of entities used in events
*/
enum EventHookType
{
	GenEvent_None = 0,
	GenEvent_ByClass, //Event is registered for all entities of classname
	GenEvent_ByEnt,		//Event is registered for a specific named entity
	GenEvent_BySubClass,   //not implemented -- Event is registered on all entities sharing a common root class
	GenEvent_BySubEnt      //not implemented -- Event is registered on all entities sharing a common root entity name
};


/*
	Event structure  Contains an entity name or classname of entities and an output to hook.
*/
enum GenEvent
{
	ClusterGenType:ClusterType,	//Action to perform
	EventHookType:EventType,		//Type of entity performing action
	String:EventCName[MAX_EVENTGEN_CNAME], //classname of entity 
	String:EventEntity[MAX_EVENTGEN_CNAME], //valid if EventType == ByEnt
	String:EventOutput[MAX_EVENT_OUTPUTNAME], //Output to to register
	EntityOutput:EventTriggered					//HookEntityOutput handler registered for this event
};



/*
	List of events for a cluster.
*/
enum RuneCluster
{
	String:ClusterName[MAX_CLUSTER_LEN],
	Handle:EventArray,		//Array of GenEvent
	EntityOutput:EventTriggered  //I'm not sure.
};


#define LAST_RUNE_CVAR_FLOAT 2

/*
	Global spawn_gen parameters.  Configurable by associated cvars.
*/
enum GenRuneInfo
{
	Float:fSpawnInterval,
	Float:fRuneLifeTime,
	Float:fRuneNamedLifeTime,
	bool:bRoundStartClear,
	bool:bRoundEndClear,
	bool:bRuneEnable
};


new bool:steamtools = false;
