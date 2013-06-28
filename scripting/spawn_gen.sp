//https://forums.alliedmods.net/showthread.php?t=129597
#if !defined __spawn_gen__
#define __spawn_gen__
#include <sourcemod>
#include <clients>
#include <tf2>
#include <sdktools>
#include <string>
#include <entity_prop_stocks>
#include <smlib>
#include <sdkhooks>

#include <vector>

#define REQUIRE_EXTENSIONS
#define AUTOLOAD_EXTENSIONS
#include <runetf/defines_debug>

#include <runetf/runetf>

#include <runetf/spawn_rune>


#define PLUGIN_NAME "Rune Spawner"
#define PLUGIN_DESCRIPTION "Core spawning logic for runetf."


#include <runetf/spawn_gen/spawn_gen_vars>
#include <runetf/spawn_gen/spawn_gen_cvars>
new Handle:g_SpawnTimer = INVALID_HANDLE;
#include <runetf/spawn_gen/spawn_gen_cvars_handler>
#include <runetf/spawn_gen/spawn_gen_create>
#include <runetf/spawn_gen/spawn_gen_cluster>
#include <runetf/spawn_gen/spawn_gen_read>
#include <runetf/spawn_gen/rune_gen_events>
#include <runetf/spawn_gen/rune_gen_iterate>
#include <runetf/spawn_gen/spawn_gen_write>

#include <runetf/spawn_gen/rune_gen_menu>
#undef REQUIRE_EXTENSIONS
#include <steamtools>


public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


static bool:g_bDelayLoadConfig = true;
static bool:g_bConfigLoaded = false;

public OnPluginStart()
{

	HookMapEvents();

	RegCreateCmds();
	RegAdminCmd("sm_runetf_reload", OnReloadCmd, ADMFLAG_CONFIG, "Reload map rune gen config file");
	RegAdminCmd("sm_runetf_save", OnSaveCmd, ADMFLAG_CONFIG, "save map rune gen config file");
	RegAdminCmd("sm_gen_reload", OnReloadCmd, ADMFLAG_CONFIG, "Reload map rune gen config file");
	RegAdminCmd("sm_gen_save", OnSaveCmd, ADMFLAG_CONFIG, "save map rune gen config file");
	RegAdminCmd("sm_it", OnIterateCmd, ADMFLAG_CONFIG, "iterate rune spawn points");
	RegAdminCmd("sm_gen", OnGenCmd, ADMFLAG_CONFIG, "rune generator commands");
	RegAdminCmd("sm_gen_reset", OnGenResetCmd, ADMFLAG_CONFIG, "rune generator commands");

	RegisterMenus();
	InitRuneConvars();

	AddRunesToDownloadTable();
	BuildPath(Path_SM, mapListPath, sizeof(mapListPath), "data/runetf/");
	if(!DirExists(mapListPath))
	{
		if(!CreateDirectory(mapListPath,493))
			return ThrowError("Could not create SM_Path/data/runetf");
	}

	if(!g_bDelayLoadConfig)
	{
		g_bConfigLoaded = (ReadConfig() != -1);
	}
	return _:Plugin_Continue;
}

ReadConfig()
{
	GetCurrentMap(g_mapName, sizeof(g_mapName));
	ProcessMapEntities();	
	if(!ReadRuneFile())
		return -1;
		//return LogError("Could not read rune file.");
	if(g_SpawnTimer != INVALID_HANDLE)
	{
		//KillTimer(g_SpawnTimer);
		g_SpawnTimer = INVALID_HANDLE;
	}

	return 1;
}


public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	RegPluginLibrary("spawn_gen");
	LogMessage("AskLoad late = %d",late);

	if(late)
	{
		g_bDelayLoadConfig = false;
	}

	MarkNativeAsOptional("Steam_SetGameDescription");
	steamtools = LibraryExists("SteamTools")
	return APLRes_Success;
}

public OnLibraryAdded(const String:name[])
{
	if(!strcmp(name,"SteamTools", false))
		steamtools = true;
}

public OnLibraryRemoved(const String:name[])
{
	if(!strcmp(name,"SteamTools",false))
		steamtools = false;
}
public Action:OnIterateCmd(client, args)
{
	new String:cmd[16];
	new Handle:a_runegen = g_vGen;
	
	if(args > 1)
	{
		GetCmdArg(1,cmd,sizeof(cmd));
		if(StrEqual(cmd,"cluster",true))
		{
			if(args > 2)
			{
				GetCmdArg(2,cmd,sizeof(cmd));
				//find cluster array
			} else {
				return ThrowError("Expected cluster name to iterate");
			}
		} else if(!strcmp(cmd,"disabled"))
			a_runegen = g_vGenDisabled;
	}
	if(client > 0)
	{
		IterateRuneGen(client, a_runegen);
	}
	return Plugin_Continue;
}

public Action:OnReloadCmd(client, args)
{
	ReadRuneFile(client);
	return Plugin_Handled;
}

public OnConfigsExecuted()
{
	RegisterRuneSpawnCvars();
	g_bConfigLoaded = (ReadConfig() != -1);

	if(steamtools)
	{	
		new Handle:descriptionCvar = CreateConVar("st_gamedesc_override", "gamemode desc", "RuneTF v1");

		decl String:description[128];
		GetConVarString(descriptionCvar, description, sizeof(description));
		HookConVarChange(descriptionCvar, DescCvarChanged);
		Steam_SetGameDescription(description);
	}

#if defined DEBUG_GEN_LOAD
	LogMessage("SpawnGen CVars");
	LogMessage("spawn_interval %4.2f",g_RuneSpawn[fSpawnInterval]);
	LogMessage("rune_lifetime %4.2f",g_RuneSpawn[fRuneLifeTime]);
	LogMessage("rune_droptime %4.2f",g_RuneSpawn[fRuneNamedLifeTime]);
	LogMessage("clear on round start %s", g_RuneSpawn[bRoundStartClear] ? "yes" : "no");
	LogMessage("clear on round end %s", g_RuneSpawn[bRoundEndClear] ? "yes" : "no");
	new bool:bEnabled = false;
	LogMessage("rune_enable %s", (bEnabled = g_RuneSpawn[bRuneEnable]) ? "yes" : "no");

	if(!bEnabled && g_SpawnTimer != INVALID_HANDLE)
		LogMessage("warning; rune_enable 0 but g_SpawnTimer is valid");

#endif 
	if(!g_bConfigLoaded)
		LogMessage("warning; Config not loaded.")
}

public OnPluginEnd()
{
	UnregisterCvars();
	if(g_SpawnTimer != INVALID_HANDLE)
		KillTimer(g_SpawnTimer);
	UnhookMapEvents()
}

public DescCvarChanged(Handle:cvar, const String:oldVal[], const String:newVal[]) {
	decl String:description[128];
	GetConVarString(cvar, description, sizeof(description));
	Steam_SetGameDescription(description);
}




public OnMapStart()
{
	return Plugin_Continue;
}

public OnMapEnd()
{

	ShutdownGenEvents();
	ClearGlobalRuneArray();
	g_bDelayLoadConfig = true;
	g_bConfigLoaded = false;

	if(g_Player[DefaultCluster] != INVALID_HANDLE)
	{
		CloseHandle(g_Player[DefaultCluster]);
		g_Player[DefaultCluster] = INVALID_HANDLE;
	}

	if(g_Player[WorkingSet] != INVALID_HANDLE)
	{
		CloseHandle(g_Player[WorkingSet]);
		g_Player[WorkingSet] = INVALID_HANDLE;
	}

	UnregisterCvars();
	ResetConVars();

	if(g_SpawnTimer != INVALID_HANDLE)
		KillTimer(g_SpawnTimer);
	g_SpawnTimer = INVALID_HANDLE;
}

public Action:DelayedLoadConfig(Handle:timer)
{
	g_bConfigLoaded = (ReadConfig() != -1);
}

RestartSpawnTimer(Float:fInterval)
{
#if defined DEBUG_GEN_SPAWN
	LogMessage("spawn_gen::RestartSpawnTimer %f",fInterval);
#endif 

	if(g_SpawnTimer != INVALID_HANDLE)
	{
		KillTimer(g_SpawnTimer);
		g_SpawnTimer = INVALID_HANDLE;
	}

	if(!FloatCompare(fInterval,0.0))
		return;

	if(fInterval > 0.0)
		g_SpawnTimer = CreateTimer(fInterval, SpawnRuneTimer,INVALID_HANDLE,TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);

}

public Action:SpawnRuneTimer(Handle:timer)
{
	new a_size;
	//new t_rune[RuneGen]; should GetArrayArray

	if( g_vGen == INVALID_HANDLE || (a_size = GetArraySize(g_vGen)) == 0)
		return Plugin_Continue;

	new idx = (GetURandomInt()%a_size)

//	new _id;
	new Float:_ori[3];
	new Float:_ang[3];
	new Float:_force;


	new t_gen[RuneGen];
	GetArrayArray(g_vGen, idx, t_gen, RUNE_BLOCK_SIZE);

//	_id = t_gen[Id];
	GetTempGenVec(t_gen, _:g_ori, _ori);
	GetTempGenVec(t_gen, _:g_ang, _ang);
	_force = t_gen[g_force];

	if(_force < 250.0)
		_force = 250.0

	new Float:_vel[3];
	GetAngleVectors(_ang,_vel, NULL_VECTOR, NULL_VECTOR);
	ScaleVector(_vel, _force);
#if defined DEBUG_GEN_SPAWN
	PrintToServer("Spawned index %d: angle %f %f %f", idx, _ang[0], _ang[1], _ang[2]);
#endif
	SpawnRandomRune(_ori,_ang,_vel, g_RuneSpawn[fRuneLifeTime]);
	return Plugin_Continue;
}

public Action:SpawnRuneTimerBlock(Handle:timer, any:data)
{
	//new t_rune[RuneGen]; should GetArrayArray


	if(!g_RuneSpawn[bRuneEnable])
		return Plugin_Stop;

	if(!IsPackReadable(data,1))
		return Plugin_Stop;

	new r_id = ReadPackCell(data);

	new Handle:a_gen;
	new idx;

	if( (idx = FindIndexInGlobalArrayById(a_gen, r_id)) != -1)
	{
	
//		new _id;
		new Float:_ori[3];
		new Float:_ang[3];
		new Float:_force;

		new t_gen[RuneGen];
		GetArrayArray(a_gen, idx, t_gen, RUNE_BLOCK_SIZE);

//		_id = t_gen[Id];
		GetTempGenVec(t_gen, _:g_ori, _ori);
		GetTempGenVec(t_gen, _:g_ang, _ang);
		_force = t_gen[g_force];

		if(_force < 250.0)
			_force = 250.0

		new Float:_vel[3];
		GetAngleVectors(_ang,_vel, NULL_VECTOR, NULL_VECTOR);
		ScaleVector(_vel, _force);
#if defined DEBUG_GEN_SPAWN
		PrintToServer("Spawned index%s %d: origin %f %f %f", a_gen == g_vGenDisabled ? " of disabled" : "",idx, _ori[0], _ori[1], _ori[2]);
#endif
		SpawnRandomRune(_ori,_ang,_vel,g_RuneSpawn[fRuneLifeTime]);
		return Plugin_Continue;
	}
	
	return Plugin_Stop;
}


public Action:SpawnTestRuneTimerBlock(Handle:timer, any:data)
{
	//new t_rune[RuneGen]; should GetArrayArray


	if(!IsPackReadable(data,1))
		return Plugin_Stop;

	new r_id = ReadPackCell(data);

	new Handle:a_gen;
	new idx;

	if( (idx = FindIndexInGlobalArrayById(a_gen, r_id)) != -1)
	{

		SpawnTestRune(a_gen,idx);
	
		return Plugin_Continue;
	} else if( g_Player[WorkingSet] != INVALID_HANDLE &&  GetArraySize(g_Player[WorkingSet]) > 0)
	{
		if ((idx = FindIndexInArrayById(g_Player[WorkingSet],r_id)) != -1)
		{
			SpawnTestRune(g_Player[WorkingSet],idx);
		}
	}
	
	return Plugin_Stop;
}

public Action:OnGenResetCmd(client, args)
{
	PrintToConsole(client,"Reset %d rune generators from global and disabled arrays.",
		ResetRuneArrays());
	return Plugin_Continue;
}

public TF2_OnWaitingForPlayersStart()
{
	LogMessage("Start waiting for players");

// Delete runes that may have spawned.
// Stop Spawn timer

	if(g_SpawnTimer != INVALID_HANDLE)
	{
		KillTimer(g_SpawnTimer);
	}
	g_SpawnTimer = INVALID_HANDLE;
	KillAllRunes();
	KillAllPlayerRunes();

}

public TF2_OnWaitingForPlayersEnd()
{
	LogMessage("End waiting for players.");

	KillAllRunes();
	KillAllPlayerRunes();

	if(g_SpawnTimer != INVALID_HANDLE)
	{
		LogMessage("warning; SpawnTimer should be null");
	}

	if(g_RuneSpawn[bRuneEnable])
		g_SpawnTimer = CreateTimer(g_RuneSpawn[fSpawnInterval], SpawnRuneTimer,INVALID_HANDLE,TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);

	if(!g_bEventsHooked)
	{
		InitGenEvents();
		g_bEventsHooked = true;
	} else {
		ResetRuneArrays();
	}
}





#endif

