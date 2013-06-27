#include <sourcemod>
#include <sdktools>
#include <events>
#include <clients> 
#include <sdkhooks>
#include <smlib>

#include <tf2>
#include <tf2_stocks>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

new g_Effect[MAXPLAYERS] = {0};

enum TrackKills
{
	KillCounter,
	Handle:KillTrackTimer
}

new g_TrackKills[MAXPLAYERS][TrackKills];

new g_PowerPlayRune = INVALID_HANDLE;

public OnPluginStart()
{
	ResetAllTrack();
	g_PowerPlayRune = AddRune("Powerplay", PowerPlayRunePick,PowerPlayRuneDrop,1);
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public OnPluginEnd()
{
	//CloseHandle(g_PowerPlayRune);
}

public PowerPlayRunePick(client, rune, ref)
{
	//PrintToServer("Pickup powerplay %d pickedup id %d",client,rune);
	g_Effect[client] = 1;
	g_TrackKills[client][KillCounter] = 0;
	if(ref == 1)
	{
		HookEvent("player_death",OnKill_Track);
	}

	return 1;
}

public PowerPlayRuneDrop(client, rune, ref)
{
	g_Effect[client] = 0;
	//PrintToServer("Drop powerplay %d dropped id %d ref %d\nKill count on drop: %d",client,rune,ref,g_TrackKills[client][KillCounter]);
	//PrintToConsole(client,"Drop powerplay %d dropped id %d ref %d\nKill count on drop: %d",client,rune,ref,g_TrackKills[client][KillCounter]);
	if(ref == 0)
	{
		UnhookEvent("player_death",OnKill_Track);
	}
	ResetTrack(client);
}

ResetTrack(client)
{
	if(g_TrackKills[client][KillTrackTimer] != INVALID_HANDLE)
	{
		KillTimer(g_TrackKills[client][KillTrackTimer])
		g_TrackKills[client][KillTrackTimer] = INVALID_HANDLE;
	}

	g_TrackKills[client][KillCounter] = 0;
}

ResetAllTrack()
{
	for(new i = 0; i <= GetMaxClients(); ++i)
	{
		ResetTrack(i);
	}
}

ForceSpawnTeam(client)
{
	new respawn_count = 0;
	new team = GetClientTeam(client);
	new String:s_name[24];

	GetClientName(client, s_name,sizeof(s_name));

	for(new i = 1; i <= GetMaxClients();++i)
	{
		if(!IsValidEntity(i) || !IsClientConnected(i) || IsPlayerAlive(i))
			continue;
	

		if(GetClientTeam(i) == team)
		{
			//new String:s_target[24];
			//GetClientName(i, s_target,sizeof(s_target));
			respawn_count++;
			TF2_RespawnPlayer(i);
			Client_PrintHintText(client,"You were respawned by %s's powerplay!", s_name);
			//Client_PrintHintText(i,"You respawned %s because of your powerplay!", s_target);
		}
	}
	return respawn_count;
}


stock update_kill_tracker(client,value)
{
	if(g_TrackKills[client][KillTrackTimer] != INVALID_HANDLE)
	{
		KillTimer(g_TrackKills[client][KillTrackTimer]);
	}
	g_TrackKills[client][KillTrackTimer] = CreateTimer(float(10), Timer_Reset_Kill_Count, client);

	g_TrackKills[client][KillCounter] += value;

	if(g_TrackKills[client][KillCounter] >= 30)
	{
		g_TrackKills[client][KillCounter] -= 30;
		new iTeammatesSpawned = ForceSpawnTeam(client);
		Client_PrintHintText(client,"powerplay respawned %d teammates!", iTeammatesSpawned);
		//LogMessage("Powerplay respawned %d teammates!", iTeammatesSpawned);
	}
}



public Action:OnKill_Track(Handle:event, const String:name[], bool:dontBroadcast)
{
	new client;
	new userid;

	client = (userid = GetEventInt(event,"attacker")) == -1 ? -1 : GetClientOfUserId(userid);
	if(client != -1 && g_Effect[client])
	{

		new dFlags;
		if((dFlags = GetEventInt(event,"death_flags")) & TF_DEATHFLAG_KILLERREVENGE)
			update_kill_tracker(client,35);
		else if(dFlags & TF_DEATHFLAG_ASSISTERREVENGE )
			update_kill_tracker(client,25);
		else
			update_kill_tracker(client,10);

	}

	client = (userid = GetEventInt(event,"assister")) == -1 ? -1 : GetClientOfUserId(userid);
	if(client != -1 && g_Effect[client])
	{

		new dFlags;
		if((dFlags = GetEventInt(event,"death_flags")) & TF_DEATHFLAG_KILLERREVENGE)
			update_kill_tracker(client,25);
		else if(dFlags & TF_DEATHFLAG_ASSISTERREVENGE )
			update_kill_tracker(client,15);
		else
			update_kill_tracker(client,10);
	}
	
	return Plugin_Continue;
}	

public Action:Timer_Reset_Kill_Count(Handle:timer, any:client)
{
//	LogMessage("Resetting killcount, was %d", g_TrackKills[client][KillCounter]);
	g_TrackKills[client][KillCounter] = 0;
	g_TrackKills[client][KillTrackTimer] = INVALID_HANDLE;
	return Plugin_Stop;
}


public RunePluginStart()
{
	PrintToServer("PowerPlayRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("PowerPlayRunePluginStop\n");
}
