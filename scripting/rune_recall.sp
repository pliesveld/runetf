#include <sourcemod>
#include <sdktools>
#include <events>
#include <clients> 

#include <sdkhooks>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

#define SND_FILE_SET    "buttons/button14.wav"
#define SND_FILE_RESET	"common/wpn_hudoff.wav"
#define SND_FILE_TP 		"buttons/button19.wav"


enum RuneProp
{
	g_active,
	g_recall_set,
	Float:g_timestamp,
};

new g_Effect[MAXPLAYERS][RuneProp];
new Float:g_EffectOri[MAXPLAYERS][3];
new Float:g_EffectAng[MAXPLAYERS][3];
new Float:g_EffectVel[MAXPLAYERS][3];

new Float:g_EffectOri2[MAXPLAYERS][3];
new Float:g_EffectAng2[MAXPLAYERS][3];
new Float:g_EffectVel2[MAXPLAYERS][3];


new g_RecallRune = -1;
new g_BlinkRune = -1;

#define RUNE_RECALL 1
#define RUNE_BLINK  2

public OnPluginStart()
{
	g_RecallRune = AddRune("Recall", RecallRunePickup, RecallRuneDrop,RUNE_RECALL);
	g_BlinkRune = AddRune("Blink", RecallRunePickup, RecallRuneDrop,RUNE_BLINK);
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public OnMapStart()
{
	CacheSounds();
	return Plugin_Continue;
}

public OnPluginEnd()
{
}


public RecallRunePickup(client, rune,ref)
{

	if(rune == g_RecallRune)
	{
		g_Effect[client][ g_active ] = RUNE_RECALL;
		g_Effect[client][ g_recall_set ] = 0;
	}
	else if(rune == g_BlinkRune)
	{
		g_Effect[client][ g_active ] = RUNE_BLINK;
		g_Effect[client][ g_recall_set ] = 1;
		GetClientAbsOrigin(client, g_EffectOri[client]);
		GetClientAbsAngles(client, g_EffectAng[client]);
	}
	else 
	{
		LogMessage("Unknown rune %d; expected %d or %d", rune, g_RecallRune, g_BlinkRune);
		return -1;
	}

	g_Effect[client][g_timestamp] = GetGameTime();

	PrintToServer("Pickup RecallRune player %d pickedup id %d",client,rune);
}

public RecallRuneDrop(client, rune,ref)
{

	g_Effect[client][ g_active ] = 0;
	g_EffectOri[client] = NULL_VECTOR;
	g_EffectOri2[client] = NULL_VECTOR;
	g_Effect[client][g_recall_set ] = 0;
	PrintToServer("Drop RecallRune player %d dropped id %d",client,rune);
}


/* use forward ?*/
public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	new rune;
	if( !(rune = g_Effect[ client ] [ g_active ]) )
		return Plugin_Continue;
	if(! (buttons & IN_USE) )
		return Plugin_Continue;

	if( GetGameTime() < g_Effect[client][g_timestamp] )
		return Plugin_Continue;
		

	if(rune == RUNE_RECALL)
	{
		g_Effect[client][g_timestamp] = GetGameTime() + 7.0;

		if(!g_Effect[ client ][ g_recall_set ])
		{
			g_Effect[client][g_recall_set] = 1;
			//GetClientEyePosition(client, g_EffectOri[client]);
			GetClientAbsOrigin(client, g_EffectOri[client]);
			GetClientEyeAngles(client, g_EffectAng[client]);
			EmitSoundToClient(client,SND_FILE_SET);
		} else {
			new Float:_pos[3];
			GetClientAbsOrigin(client,_pos);
			if( GetVectorDistance(_pos, g_EffectOri[client]) < 125.0 )
			{
				g_Effect[client][g_recall_set] = 0;
				EmitSoundToClient(client,SND_FILE_RESET);
			} else { // check for telefrag
				TeleportRecall(client,g_EffectOri[client], g_EffectAng[client]);
				//EmitSoundToClient(client,SND_TP);
			}
		}
	} else { // rune == RUNE_BLINK
		g_Effect[client][g_timestamp] = GetGameTime() + 3.0;


		if(g_Effect[ client ][ g_recall_set ])
		{
			GetClientAbsOrigin(client, g_EffectOri2[client]);
			GetClientAbsAngles(client, g_EffectAng2[client]);
			g_Effect[ client ][ g_recall_set ] = 0;
			TeleportRecall(client,g_EffectOri[client], g_EffectAng[client]);
		} else {
			GetClientAbsOrigin(client, g_EffectOri[client]);
			GetClientAbsAngles(client, g_EffectAng[client]);
			g_Effect[ client ][ g_recall_set ] = 1;
			TeleportRecall(client,g_EffectOri2[client], g_EffectAng2[client]);
		}
	}


	//if( !FL_ONGROUND
	return Plugin_Continue;
}


stock TeleportRecall(client, Float:v_ori[3], Float:v_ang[3])
{
	new p_team = GetClientTeam(client);
	
	for(new i = 1; i <= GetMaxClients();i++)
	{
		if(!IsValidEntity(i) || !IsPlayerAlive(i))
			continue;

		if( GetClientTeam(i) == p_team )
			continue;

		new Float:i_pos[3];
		GetClientAbsOrigin(i,i_pos);

		if(GetVectorDistance(i_pos, g_EffectOri[client]) < 47.16)
		{
			SDKHooks_TakeDamage(i, 0, client, 1000.0, DMG_FALL);
		}
	}
	

	EmitSoundToAll(SND_FILE_TP, client);
	CreateParticleAtEntity(client,"eb_tp_escape_bits",3.0,false);
	CreateParticleAtEntity(client,"eyeboss_tp_player",3.0,true);
	TeleportEntity(client, v_ori, v_ang, NULL_VECTOR);
}


CacheSounds()
{
	if(!PrecacheSound(SND_FILE_SET,true))
		PrintToServer("Failed to cache %s", SND_FILE_SET);
	
	if(!PrecacheSound(SND_FILE_RESET,true))
		PrintToServer("Failed to cache %s", SND_FILE_RESET);

	if(!PrecacheSound(SND_FILE_TP,true))
		PrintToServer("Failed to cache %s", SND_FILE_TP);
}

public RunePluginStart()
{
	PrintToServer("RecallRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("RecallRunePluginStop\n");
}


// particles
  //stickybomb_pulse_red/blue
	//duel_red/blue_burst
	//god_rays
	//ghost_firepit_plate
	//eyeboss_tp_escape
//eb_tp_escape_bits
//eb_tp_escape_flash01
//eyeboss_tp_player
//eyeboss_tp_normal
//eyeboss_tp_rope


