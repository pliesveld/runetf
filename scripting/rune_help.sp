#include <colors>
/*
	displays runetf player commands to new clients.
	todo: move to configurable file

*/
#define REQUIRE_PLUGIN
#include <runetf/runetf>
#include <runetf/spawn_gen>


#define PLUGIN_NAME "Rune Player Helper"
#define PLUGIN_DESCRIPTION "Greets player with runetf message.  Periodically warns players who have never used +use command."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


new String:g_sGreeting[][] = 
{
	{"Welcome to runeTF!"},
	{"Runes are powerups that spawn randomly throughout the map."},
	{"Powers granted by runes are active until death."},
	{"You can only have one rune active at a time."},
	{"The {red}!info_rune{default} command will display rune description."},
	{"Some runes require you to bind a key to the {blue}+use{default} command to activate the rune's ability."},
	{"You can give a rune to a teammate with the command {red}!drop{default}."},
	{"You can see what runes are on the ground {red}!inspect_rune{default}: see what rune a player is holding."},
	{"To turn off runeTF, you may vote with {red}!norunes{default}."}
}

new String:g_sGreeting2[][] = 
{
	{"runeTF is disabled.  You may vote with {blue}!runes{default} to turn it on."},
	{"runeTF is disabled.  You may vote with {blue}!runes{default} to turn it on."}
}

new Handle:g_HelpHudSync = INVALID_HANDLE;


enum HelpStateInfo
{
	Help_None = -1,
	Help_Loaded = 0,
	Help_MapStart = 1,
	Help_ConfigEnd = 2,
	Help_RuneEnabledInfo = 3,
	Help_RuneDisabledInfo = 4,
	Help_RuneUseAnnoy = 5,
	Help_RuneDone = 6,
};

enum MsgTypeInfo
{
	Help_MsgRuneOff = 0,
	Help_MsgRuneOn = 1,
};


enum MsgInfo
{
	MsgIdx,
	FirstSpawn,
	Handle:TimerMsg,
	bool:Annoy,
};

new g_Msg[MAXPLAYERS][MsgInfo];

new Handle:gAnnoyTimer = INVALID_HANDLE;

new bool:g_WaitingForPlayers = true;
new MsgTypeInfo:g_HelpState;

public OnMapStart()
{
	g_WaitingForPlayers = true;
	g_HelpState = Help_MapStart;
}

public TF2_OnWaitingForPlayersStart()
{
	g_WaitingForPlayers = true;
}

public TF2_OnWaitingForPlayersEnd()
{
	g_WaitingForPlayers = false;
}

public OnMapEnd()
{
	g_WaitingForPlayers = true;
}

public OnRuneToggle(bool:rune_enable)
{
	g_HelpState = rune_enable;

	new i;
	for(i = 1;i <= GetMaxClients();++i)
	{
		if(!IsValidEntity(i))
			continue;

		g_Msg[i][FirstSpawn] = true;
		g_Msg[i][MsgIdx] = 0;
		StartMessageTimer(i,g_Msg[i][TimerMsg]);
	}
}

public OnConfigsExecuted()
{
	new Handle:hRuneToggle = FindConVar("rune_enable");
	if( hRuneToggle != INVALID_HANDLE && GetConVarBool(hRuneToggle) == true)
		g_HelpState = MsgTypeInfo:Help_MsgRuneOn;
	else
		g_HelpState = MsgTypeInfo:Help_MsgRuneOff;
}

public OnPluginStart()
{
	g_HelpHudSync = CreateHudSynchronizer();
	g_WaitingForPlayers = true;
	HookEvent("player_spawn",MsgOnSpawn);

	gAnnoyTimer = CreateTimer(120.0,PeriodicAnnoyPlayer,0,TIMER_REPEAT);

//	CreateTimer(60.0*30.0,ChangeStartLevel,0,TIMER_FLAG_NO_MAPCHANGE)
}


//public Action:ChangeStartLevel(Handle:timer,any:data)
//{
//	ServerCommand("changelevel pl_badwater")
//	return Plugin_Stop
//}


public Action:PeriodicAnnoyPlayer(Handle:timer,any:data)
{
	new iClient;
	for(iClient = 1; iClient <= GetMaxClients();++iClient)
	{
		if(!IsValidEntity(iClient))
			continue;
		
		if(g_HelpState && IsPlayerAlive(iClient) && g_Msg[iClient][Annoy])
		{
			CPrintToChat(iClient, "{default}[{olive}runetf{default}]: You need to bind a key to {blue}+use{default} to activate a rune's ability.");
		}
	}
	return Plugin_Continue;
}
	

public OnPluginEnd()
{
	CloseHandle(g_HelpHudSync);
	CloseHandle(gAnnoyTimer);
}


//  This shouldn't be a global forward, it should be a local forward that we can remove
	// the players that have demonstrated that they have a key bound to the +USE
	// consider a helper plugin that gets paused if all players succeed?
public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	if( buttons & IN_USE )
		g_Msg[client][Annoy] = false;
	return Plugin_Continue;
}

stock StartMessageTimer(client,&Handle:timer)
{
	if(timer != INVALID_HANDLE)
		KillTimer(timer);
	timer = CreateTimer(2.75, Timer_HelpMsg,client,TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
}

public Action:MsgOnSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
	new userid = GetEventInt(event,"userid");
	new client = GetClientOfUserId(userid);
	if(g_WaitingForPlayers)
		return Plugin_Continue;

	if(g_Msg[client][FirstSpawn])
	{
		g_Msg[client][FirstSpawn] = false;
		g_Msg[client][MsgIdx] = 0;
		StartMessageTimer(client, g_Msg[client][TimerMsg] );
	}
	return Plugin_Continue;
}

public Action:Timer_HelpMsg(Handle:timer, any:client)
{
	if(!IsValidEntity(client))
	{
		g_Msg[client][TimerMsg] = INVALID_HANDLE;
		g_Msg[client][MsgIdx] = 0;
		g_Msg[client][FirstSpawn] = true;
		return Plugin_Stop;
	}

	new sizeMsg = (g_HelpState ? sizeof(g_sGreeting) : sizeof(g_sGreeting2));

	if(g_Msg[client][MsgIdx] >= sizeMsg)
	{
		g_Msg[client][TimerMsg] = INVALID_HANDLE;
		return Plugin_Stop;
	}

/*
	SetHudTextParams(0.2,0.8,15.0,
		255, 255, 255, 180,2);
	ShowSyncHudText(client,g_HelpHudSync, g_sGreeting[ g_Msg[client][MsgIdx]++ ]);
*/

	CPrintToChat(client, "{default}[{olive}runetf{default}]: %s ", 
				g_HelpState ? 
				( g_sGreeting[ g_Msg[client][MsgIdx]++ ] ) :
				( g_sGreeting2[ g_Msg[client][MsgIdx]++ ] )
		);
	return Plugin_Continue;
	
}

public OnClientPutInServer(client)
{
	g_Msg[client][FirstSpawn] = true;
	g_Msg[client][TimerMsg] = INVALID_HANDLE;
	g_Msg[client][MsgIdx] = 0;
	g_Msg[client][Annoy] = g_HelpState;

}

public OnClientDisconnect(client)
{
	if(g_Msg[client][TimerMsg])
	{
		KillTimer(g_Msg[client][TimerMsg]);
		g_Msg[client][TimerMsg] = INVALID_HANDLE;
	}
	g_Msg[client][MsgIdx] = 0;
	g_Msg[client][FirstSpawn] = false;
	g_Msg[client][Annoy] = false;

}



