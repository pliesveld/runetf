//https://forums.alliedmods.net/showthread.php?t=129597
#include <sourcemod>
#include <clients>
#include <tf2>
#include <sdktools>
#include <string>
#include <entity_prop_stocks>
#include <smlib>
#include <sdkhooks>

#include <vector>

#include <runetf/defines_debug>
#define REQUIRE_EXTENSIONS
#define AUTOLOAD_EXTENSIONS
#include <runetf/runetf>

#define REQUIRE_PLUGIN
#include <runetf/spawn_gen>

#include <runetf/spawn_rune>

public Action:Command_DelRune(client, args)
{
	new cnt = 0;
	new maxEntities = GetMaxEntities();
	for (new entity=MaxClients+1; entity < maxEntities; entity++)
	{
		if (!IsValidEntity(entity))
			continue;
		if(isEntaRune(entity))
		{
			//LogMessage("tname%d:%s",strlen(tname),tname);
			//RemoveEdict(entity);
    	AcceptEntityInput(entity, "KillHierarchy");
			++cnt;
		}
	}
	LogMessage("DropRune: %d entities removed", cnt);
}

	
public Action:Command_DumpRune(client, args)
{
	new maxEntities = GetMaxEntities();
	for (new entity=MaxClients+1; entity < maxEntities; entity++)
	{
		if (!IsValidEntity(entity))
			continue;
		if(isEntaRune(entity))
		{
			DbgPrint("dump",entity);
		}
	}

}

public Action:Command_PluginRune(client, args)
{
	decl String:cmdstr[64];
	decl String:arg[16];
	GetCmdArgString(cmdstr,sizeof(cmdstr));

	if(!strncmp(cmdstr,"stop",4))
	{
		RunePluginsStop();
	} else if(!strncmp(cmdstr,"start",5)) {
		RunePluginsStart();
	} 

}


new g_LogFileHandle = INVALID_HANDLE;


public OnPluginStart()
{
  RegConsoleCmd("sm_del", Command_DelRune, "drop rune");
  RegConsoleCmd("sm_dump", Command_DumpRune, "dump runes");
  RegConsoleCmd("drop", Command_DropRune,"drop rune");


	//RegConsoleCmd("sm_plugin", Command_PluginRune, "rune plugins");
  //RegConsoleCmd("sm_drop", Command_DropRune, "drop rune");
	//RegConsoleCmd("sm_give", Command_GiveRune, "gives a rune");
  RegAdminCmd("sm_take", Command_TakeRune,ADMFLAG_CHEATS, "take rune from a player");
	RegAdminCmd("sm_give", Command_GiveRune,ADMFLAG_CHEATS, "give a specific rune to a player.");


	new String:RuneLogDir[64];
	new String:RuneLogFile[64];
	BuildPath(Path_SM,RuneLogDir,sizeof(RuneLogDir), "logs/runetf/");
	if(!DirExists(RuneLogDir))
	{
		if(!CreateDirectory(RuneLogDir,493))
			return ThrowError("Could not create logs/runetf");
	}
	
	new String:RuneFileTimeStr[32];
	FormatTime(RuneFileTimeStr, sizeof(RuneFileTimeStr), "%b%d%H%M%S");
	Format(RuneLogFile, sizeof(RuneLogFile), "%s%s", RuneLogDir, RuneFileTimeStr);
	
	g_LogFileHandle	= OpenFile(RuneLogFile, "a");
	if(g_LogFileHandle == INVALID_HANDLE)
		return ThrowError("Could not open log file %s", RuneLogFile);

	LogToOpenFile(g_LogFileHandle, "log started");
	return Plugin_Continue;
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{

	RegPluginLibrary("spawn_test");

	return APLRes_Success;
}

new Handle:g_FileLog;

public OnMapStart()
{
	new i;
	AddRunesToDownloadTable();
	
/*
  if( PrecacheModel("models/quake/rune_01.vmt") == 0)
		LogError("MODEL ERROR: Failed to cache model %d", ++i);

  if(PrecacheGeneric("materials/models/quake/rune_01.vtf",true) == 0)
		LogError("Failed to cache model %d", ++i);

  if(PrecacheGeneric("materials/models/quake/rune_01n.vtf",true) == 0)
		LogError("Failed to cache model %d", ++i);
*/

  if(PrecacheModel(RUNE_MODEL,true) == 0)
		LogError("MODEL ERROR: Failed to cache model %d", ++i);

#if defined DEBUG
	PrintToServer("DBG DBG === Runes added to download table and rune model cached.")
#endif
	return Plugin_Continue;
}

public OnMapEnd()
{
}


public Action:Command_DropRune(client, args)
{
	new String:buffer[32];
	new Float:ori[3];
	new Float:ang[3];
	new Float:vec[3];
	new Float:loc[3];
	if(client == 0 || !IsPlayerAlive(client))
		return Plugin_Stop;
	new rId;
	if( (rId = GetPlayerRuneId(client) ) == 0)
	{
		PrintToConsole(client,"You do not have a rune to drop.");
		Client_PrintHintText(client, "You don't have a rune to drop.");
	} else {
		GetPlayerRuneName(client,buffer,sizeof(buffer));
		new ret;
		if( (ret = PlayerDropRune(client)) < 0 )
		{
			PrintToConsole(client,"Invalid rune\n")
			LogError("Player has invalid rune %s", buffer);
			PlayerDropRune(client,true); // force remove
			return Plugin_Handled;
		}
		if(ret == 0)
		{
			PrintToConsole(client, "You cannot remove the rune of %s", buffer);
			return Plugin_Handled;
		}
		PrintToConsole(client,"PlayerDropRune(%s) returned %d", buffer, ret);
    GetClientEyePosition(client, ori)
    GetClientEyeAngles(client, ang);

    loc[0] = (ori[0]+(100*((Cosine(DegToRad(ang[1]))) * (Cosine(DegToRad(ang[0]))))));
    loc[1] = (ori[1]+(100*((Sine(DegToRad(ang[1]))) * (Cosine(DegToRad(ang[0]))))));
    ang[0] -= (2*ang[0]);
    loc[2] = (ori[2]+(100*(Sine(DegToRad(ang[0])))));

		GetAngleVectors(ang,vec,NULL_VECTOR,NULL_VECTOR);
		ScaleVector(vec,425.0);


		new Float:fLifeTime = 35.0;
		new Handle:hCvarRuneLife;
		if( (hCvarRuneLife = FindConVar("rune_drop_lifetime")) != INVALID_HANDLE)
		{
			fLifeTime = GetConVarFloat(hCvarRuneLife);
		}
	
		new t_ent = SpawnNamedRune(buffer,loc, ang,vec, fLifeTime, false);

		CreateTimer(0.050, Timer_Rune_Enable_Trigger, t_ent)
	}
	return Plugin_Continue;
}

public Action:Timer_Rune_Enable_Trigger(Handle:timer, any:t_ent)
{
	AcceptEntityInput(t_ent,"Enable");
}



public Action:Command_TakeRune(client, args)
{
	return Command_CmdRune(true,client, args)
}

public Action:Command_GiveRune(client, args)
{
	return Command_CmdRune(false,client, args)
}
	

public Action:Command_CmdRune(bool:take,client, args)
{
	if(args < 1)
		return Plugin_Handled;

	decl String:Arguments[256];
	GetCmdArgString(Arguments, sizeof(Arguments));

	decl String:arg[65];
	new len = BreakString(Arguments, arg, sizeof(arg));
	
	if (len == -1)
	{
		/* Safely null terminate */
		len = 0;
		Arguments[0] = '\0';
	}

	decl String:target_name[MAX_TARGET_LENGTH];
	decl target_list[MAXPLAYERS], target_count, bool:tn_is_ml;
	new bool:anyRune = false;
	
	if ((target_count = ProcessTargetString(
			arg,
			client, 
			target_list, 
			MAXPLAYERS, 
			COMMAND_FILTER_CONNECTED,
			target_name,
			sizeof(target_name),
			tn_is_ml)) > 0)
	{
		decl String:s_rune[64];
		Format(s_rune, sizeof(s_rune), Arguments[len]);
		if(s_rune[0] == '\0' && take)
			anyRune = true;

		for(new i = 0; i < target_count;i++)
		{
			if(take)
			{
				if(anyRune)
				{
					GetPlayerRuneName(target_list[i],s_rune,sizeof(s_rune));
				}
				PlayerDropRune(target_list[i], true);
			} else {
				if(PlayerPickupRune(target_list[i], s_rune) == 0)
				{
					new String:buffer[32];
					GetClientName(target_list[i], buffer,sizeof(buffer));
					StrCat(buffer,sizeof(buffer) - strlen(buffer), "already has a rune.");
					PrintToConsole(client, buffer);
				}
			}
		}
	}
	return Plugin_Continue;
}

/*
public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3],&weapon)
{
	if( buttons & IN_USE )
	{
		PrintToServer("player +use");
		PrintToConsole(client, "held +use");
	}

	return Plugin_Continue;
}
*/


