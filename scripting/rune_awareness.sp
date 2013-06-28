#include <sourcemod>
#include <sdktools>
#include <events>
#include <clients> 
#include <tf2>
#include <tf2_stocks>

#define AUTOLOAD_EXTENSIONS
#define REQUIRE_EXTENSIONS
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

#define SND_BLIP "buttons/button17.wav"
#define SND_PING "sonarping.mp3"
#define SND_PONG "sonarpong.mp3"

#define COLOR_RED     0
#define COLOR_BLUE  	1 



#define PLUGIN_NAME "Rune of Awareness"
#define PLUGIN_DESCRIPTION "Scans health of nearby teammates."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


/*

	medic hud like icon for health bars for enemies damaged.
	display nearby medic charge levels


*/

new g_Effect[MAXPLAYERS];
new Handle:g_EffTimer[MAXPLAYERS];

new Float:g_lastPing[MAXPLAYERS];

new Handle:g_hHudSyncRed = INVALID_HANDLE;
new Handle:g_hHudSyncBlue = INVALID_HANDLE;

enum e_ClientInfo
{
  iRune,
  TFTeam:iTeam,
	TFClassType:iClass,
	bool:bUbered,
	iChargeEffect,
  Float:iChargeLevel,
  iMaxHealth,
  iHealth,
};


new g_aClientInfo[MAXPLAYERS+1][e_ClientInfo];
#define MAX_HUD_LEN 512

stock BuildTeamInfo(client, String:sRedTeam[], String:sBlueTeam[], iClient)
{
	decl String:sClient[80]="";
	if(client == iClient)
		return;
	if(!IsValidEntity(iClient) || 
		!IsClientInGame(iClient) ||
		!IsPlayerAlive(iClient))
		return;

	new pClass = g_aClientInfo[iClient][iClass];
	Format(sClient,sizeof(sClient),"%s%s %d / %d\n",
	( g_aClientInfo[iClient][bUbered] ? "UBER" : ""),
	( pClass == TFClass_Scout ? "Scout" : 
	( pClass == TFClass_Sniper ? "Sniper" :
	( pClass == TFClass_Soldier ? "Soldier" :
	( pClass == TFClass_DemoMan ? "Demoman" :
	( pClass == TFClass_Medic ? "Medic" :
	( pClass == TFClass_Heavy ? "Heavy" :
	( pClass == TFClass_Pyro ? "Pyro" :
	( pClass == TFClass_Spy ? "Spy" :
	( pClass == TFClass_Engineer ? "Engineer" : "Unknown"))))))))),
	g_aClientInfo[iClient][iHealth],
	g_aClientInfo[iClient][iMaxHealth]);
		

	if(g_aClientInfo[iClient][iTeam] == TFTeam_Red)
		StrCat(sRedTeam,MAX_HUD_LEN,sClient);
	else if(g_aClientInfo[iClient][iTeam] == TFTeam_Blue)
		StrCat(sBlueTeam,MAX_HUD_LEN,sClient);
	

}


stock DisplayInfoToClient(client,String:sRedTeam[], String:sBlueTeam[])
{
	if(sRedTeam[0] != '\0')
	{
		SetHudTextParams(0.01, 0.55, 7.0, 
			155, 30 , 30,  //red
			210);

		ShowSyncHudText(client,g_hHudSyncRed, sRedTeam);
	}
		
	if(sBlueTeam[0] != '\0')
	{
		SetHudTextParams(0.85, 0.55, 7.0, 
			80, 50, 255, //blue
			21);

		ShowSyncHudText(client,g_hHudSyncBlue, sBlueTeam);
	}
}






public OnPluginStart()
{
	AddCustomSounds();
	PrecacheSound("buttons/button17.wav");
	PrecacheSound(SND_PING);
	PrecacheSound(SND_PONG);
	g_hHudSyncRed = CreateHudSynchronizer();
	g_hHudSyncBlue = CreateHudSynchronizer();
	AddRune("Aware", AwarenessRunePickup, AwarenessRuneDrop,1);
}

public OnMapStart()
{
	AddCustomSounds();
 	PrecacheSound(SND_PING);
	PrecacheSound(SND_PONG);
	PrecacheSound("buttons/button17.wav");
	return Plugin_Continue;
}

public OnPluginEnd()
{
	CloseHandle(g_hHudSyncBlue);
	CloseHandle(g_hHudSyncRed);
	//CloseHandle(g_AwareRune);
}


public AwarenessRunePickup(client, rune)
{
	g_Effect[client] = 1;
	if( g_EffTimer[client] != INVALID_HANDLE )
		KillTimer(g_EffTimer[client]);

	g_EffTimer[client] = CreateTimer(7.8, TimerAwarenessTick, client, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
	g_lastPing[client] = GetGameTime();
	PrintToServer("Pickup AwarenessRune player %d pickedup id %d",client,rune);
}

public AwarenessRuneDrop(client, rune)
{
	g_Effect[client] = 0;
	KillTimer(g_EffTimer[client]);
	g_EffTimer[client] = INVALID_HANDLE;
	PrintToServer("Drop AwarenessRune player %d dropped id %d",client,rune);
}

AddCustomSounds()
{
	new String:buffer[64];
	new len = strcopy(buffer,sizeof(buffer),"sound/");
	StrCat(buffer,sizeof(buffer) - len, SND_PING);
	AddFileToDownloadsTable(buffer);
	len = strcopy(buffer,sizeof(buffer),"sound/");
	StrCat(buffer,sizeof(buffer) - len, SND_PONG);
	AddFileToDownloadsTable(buffer);
}


public Action:TimerAwarenessTick(Handle:timer, any:client)
{
	decl String:sRedTeam[MAX_HUD_LEN]="";
	decl String:sBlueTeam[MAX_HUD_LEN]="";
	if(!IsValidEntity(client) || !IsPlayerAlive(client))
		return Plugin_Stop;

	new Float:pos[3];
	GetClientAbsOrigin(client,pos);
	new pitch = 100;
	new Handle:h_sndping = INVALID_HANDLE;

	UpdateAllClientInfo();
	

	for(new i = 1; i <= MaxClients; ++i)
	{
		if(!IsValidEntity(i) || !IsPlayerAlive(i) || i == client)
			continue;
//		new level = 165; // - 100
		new Float:dist;
		new Float:vol = 1.0;
		new Float:i_pos[3];
		GetClientAbsOrigin(i,i_pos);

		if( (dist = GetVectorDistance(pos,i_pos)) > 875.0 )
			continue;

		BuildTeamInfo(client, sRedTeam, sBlueTeam,i);

		if(h_sndping == INVALID_HANDLE)
			h_sndping = CreateDataPack();

		pitch += 15;

		new distance = RoundToFloor(dist/150.0);

		//PrintToServer("d: %d", distance);
		vol = 1.0 - (distance*0.1);
	

	//	level -= RoundToFloor(dist/15.0);
		WritePackCell(h_sndping, client);
		WritePackFloat(h_sndping, vol);
		//WritePackCell(h_sndping, level);
		WritePackCell(h_sndping, pitch);
		WritePackCell(h_sndping, i);
		//WritePackFloat(h_sndping, i_pos[0]);
		//WritePackFloat(h_sndping, i_pos[1]);
		//WritePackFloat(h_sndping, i_pos[2]);

	}

	DisplayInfoToClient(client,sRedTeam,sBlueTeam);

	if(h_sndping != INVALID_HANDLE)
	{
		EmitSoundToAll(SND_PING,client);
		WritePackCell(h_sndping, -1);
		ResetPack(h_sndping);
		CreateTimer(0.600, TimerSoundPingStart, h_sndping, TIMER_FLAG_NO_MAPCHANGE);
	}
	return Plugin_Continue;
}

public Action:TimerSoundPingStart(Handle:timer, any:data)
{
		CreateTimer(0.050, TimerSoundPing, data, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE | TIMER_DATA_HNDL_CLOSE);
}

public Action:TimerSoundPing(Handle:timer, any:data)
{

	new client = ReadPackCell(data);
	if(client == -1 || !IsValidEntity(client) || !IsPlayerAlive(client) || data == INVALID_HANDLE)
		return Plugin_Stop;
	new Float:vol = ReadPackFloat(data);
	new pitch = ReadPackCell(data);
/*
	new Float:pos[3];
	pos[0] = ReadPackFloat(data);
	pos[1] = ReadPackFloat(data);
	pos[2] = ReadPackFloat(data);
*/
	new other = ReadPackCell(data);

	if(!IsValidEntity(other) || !IsPlayerAlive(other))
		return Plugin_Continue;

	//PrintToServer("client %d, vol %f, pitch %d from %d", client, vol, pitch, other);
	//PrintToConsole(client,"client %d, vol %f, pitch %d from %d", client, vol, pitch, other);

	//EmitSoundToClient(client, "buttons/button17.wav", _, _, level, _, 1.0, pitch,_,pos);
	EmitSoundToClient(client, "buttons/button17.wav", other, vol, 80, _, 1.0, pitch);
	//EmitSoundToClient(client, SND_PONG, other, _, 120, _, 1.0, pitch);

	return Plugin_Continue;
}


public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	if( !g_Effect[ client ] )
		return Plugin_Continue;
	if(! (buttons & IN_USE) )
		return Plugin_Continue;

	if( GetGameTime() < g_lastPing[client] )
		return Plugin_Continue;

	g_lastPing[client] = GetGameTime() + 15.0;
	
	if( g_EffTimer[client] != INVALID_HANDLE)
	{
		TriggerTimer( g_EffTimer[client], true );
	}
	EmitSoundToAll(SND_PING,client);
	return Plugin_Continue;
}

public RunePluginStart()
{
	PrintToServer("AwareRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("AwareRunePluginStop\n");
}


stock UpdateAllClientInfo()
{
	for(new iClient = 1;iClient <= GetMaxClients();++iClient)
		TF2_UpdateClientInfo(iClient);
	for(new iClient = 1;iClient <= GetMaxClients();++iClient)
		TF2_UpdateMedicInfo(iClient);
}

stock bool:TF2_UpdateClientInfo(client)
{
	if (!IsClientInGame(client) || !IsPlayerAlive(client))
		return false;

	g_aClientInfo[client][iRune] = GetPlayerRuneId(client);
	g_aClientInfo[client][iChargeLevel] = 0.0;
	g_aClientInfo[client][bUbered] = false;
	g_aClientInfo[client][iChargeEffect] = 0;

	g_aClientInfo[client][iClass] = TF2_GetPlayerClass(client);
	g_aClientInfo[client][iTeam] = GetClientTeam(client);
	g_aClientInfo[client][iHealth] = GetClientHealth(client);
	g_aClientInfo[client][iMaxHealth] = TF2_GetPlayerResourceData(client, TFResource_MaxHealth);
	return true;
}

stock bool:TF2_UpdateMedicInfo(client)
{
  if (!IsClientInGame(client) || !IsPlayerAlive(client))
    return false;
  if (TF2_GetPlayerClass(client) == TFClass_Medic)
  {
		new entityIndex = GetPlayerWeaponSlot(client, 1);
		g_aClientInfo[client][iChargeLevel] = FloatMul(GetEntPropFloat(entityIndex, Prop_Send, "m_flChargeLevel"),100.0);
		new ubered_ent;
		if( (g_aClientInfo[client][bUbered] = GetEntProp(entityIndex,Prop_Send,"m_bChargeRelease")) == true 
				&& (ubered_ent = TF2_GetHealingTarget(client)) != -1)
		{
			g_aClientInfo[ubered_ent][bUbered] = true;
		}
	}
  return true;
}


