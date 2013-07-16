#include "spawn_gen_types.inc"
#include "spawn_gen_util.inc"

//Globals
new Handle:g_vGen 				= INVALID_HANDLE;  //Array of RuneGens To spawn periodically
new Handle:g_vGenDisabled = INVALID_HANDLE;  //Array of RuneGens To spawn periodically
//CurrentMapName
new String:g_mapName[24];

new g_NextGenId = 1; // next Id to use when creating a runegen


//////Events
new Handle:g_EntOutTrie 	= INVALID_HANDLE;//keyed by 'class_type', output value is string array of acceptable outputs
new String:class_type[][32] = { 
  "team_control_point", "team_control_point_master", "team_control_point_round", 
	"item_teamflag", "func_capturezone", "trigger_capture_area", "tf_logic_cp_timer", 
	"tf_logic_arena" , "tf_gamerules", "game_round_win", "team_round_timer"
}

/*
  Classname entity reverse lookup.  Keyed by classname, the value
  is the index in class_type of the entity class.
*/
new Handle:g_CNameTrie 				= INVALID_HANDLE; 


/*
	Multi-block array of map entities found.  For each entity class:

	block 0 = count; number of entities found of the entity class
	block 1 = array handle contains entity references of the entity class
	block 2 = array handle contains entity names (if available) of the entity references of class type entity class

*/
new Handle:g_MapEventEntities = INVALID_HANDLE;


/*
	Cluster to Id Trie;  Indexed by cluster name, the value is a handle of array Ids.
*/
new Handle:g_vGenCluster 	= INVALID_HANDLE; 

//Array of cluster names
new Handle:g_ClusterNames = INVALID_HANDLE; //array of cluster names


/*
	Array of cluster-events.  Each entry is of type RuneClusters; each cluster
	has an event array.
*/
new Handle:g_vGenEvent 		= INVALID_HANDLE; // array of RuneClusters

new bool:g_bChanged = false;
new bool:g_bEventsHooked = false;

new String:actionString[][] =
{
	"GenEvent_None",
	"GenEvent_Add",
	"GenEvent_Remove",
	"GenEvent_Spawn",
	"GenEvent_Toggle"
};

new String:entityString[][] =
{
	"GenEvent_None",
	"GenEvent_ByClass",
	"GenEvent_ByEnt",
	"GenEvent_BySubClass"
};

stock CheckClusterType(const String:type[])
{
	for(new i = 1; i < sizeof(actionString);++i)
		if(StrEqual(type,actionString[i],false))
			return i;
	return -1;
}

stock CheckEntityType(const String:type[])
{
	for(new i = 1; i < sizeof(entityString);++i)
		if(StrEqual(type,entityString[i],false))
			return i;
	return -1;
}

new EntityOutput:actionHandler[] = 
{
	EntityOutput:HandleGenEvent,
	EntityOutput:EventHandler_EnableGen,
	EntityOutput:EventHandler_DisableGen,
	EntityOutput:EventHandler_SpawnGen,
	EntityOutput:EventHandler_ToggleGen
};

new g_RuneSpawn[GenRuneInfo];

stock ClearGlobalRuneArray()
{
	CloseArray(g_vGen);
	CloseArray(g_vGenDisabled);
	//FreeMapEventEntities(g_MapEventEntities);
	//CloseTrieOutputTypes(g_EntOutTrie);
	//CloseClusters();
	//CloseArray(g_ClusterNames);
}
