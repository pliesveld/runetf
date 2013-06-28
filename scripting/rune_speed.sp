
#include <sourcemod>
#include <sdkhooks>

#include <tf2>
#include <tf2_stocks>
#include <colors>

#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>


#define PLUGIN_NAME "Rune of Haste"
#define PLUGIN_DESCRIPTION "Passively increases movement speed based on player's class."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}



static String:FlMaxSpeed[] = "m_flMaxspeed"


enum RuneProp
{
  bool:active,
	Float:fSpeed,
	Float:fCustomSpeed,
  Float:g_timestamp,
	bool:bSpeedPenality,
	Float:fPenalitySpeed,
};

new g_Effect[MAXPLAYERS][RuneProp];

enum ClassSpeedProp
{
	Float:fSpeedMax,
	Float:fSpeedPenality,
}

new g_Speed[TFClass_Engineer][ClassSpeedProp] =
{
	// { max speed, condition penality } 
	{435.0,0.666}, // scout
	{335.0,0.8}, // sniper
	{320.0,0.75}, // soldier
	{372.4,0.75}, // demoman
	{415.0,0.9}, // medic
	{225.0,0.5}, // heavy
	{340.5,0.8}, //pyro
	{400.0,1.2}, //spy
	{345.0,1.25} // engineer
}

enum CondSpeedPenality
{
	bool:bJarate,
	bool:bBleed,
	bool:bMilk,
	bool:bDaze,
}


new g_Cond[MAXPLAYERS][CondSpeedPenality];

SetSpeedPenalityConditions(client)
{
	new bool:bPenality = false;

	if(TF2_IsPlayerInCondition(client,TFCond_Jarated) 
			&& (bPenality = true))
		g_Cond[client][bJarate] = true;
	else
		g_Cond[client][bJarate] = false;

	if(TF2_IsPlayerInCondition(client,TFCond_Bleeding) 
			&& (bPenality = true))
		g_Cond[client][bBleed] = true;
	else
		g_Cond[client][bBleed] = false;

	if(TF2_IsPlayerInCondition(client,TFCond_Milked) 
			&& (bPenality = true))
		g_Cond[client][bMilk] = true;
	else
		g_Cond[client][bMilk] = false;

	if(TF2_IsPlayerInCondition(client,TFCond_Dazed) 
			&& (bPenality = true))
		g_Cond[client][bDaze] = true;
	else
		g_Cond[client][bDaze] = false;
	
#if defined DEBUG_RUNE
	DebugSpeedCond(client);
#endif
	g_Effect[client][bSpeedPenality] = bPenality;

}

stock SetCustomSpeedVars(client)
{
	new TFClass:iClass = TFClass:TF2_GetPlayerClass(client);
	g_Effect[client][fCustomSpeed] = g_Speed[iClass-TFClass:1][fSpeedMax];
	g_Effect[client][fPenalitySpeed] = FloatMul( g_Effect[client][fSpeed], g_Speed[iClass-TFClass:1][fSpeedPenality]);
#if defined DEBUG_RUNE
	LogMessage("ori speed %f; new speed %f; penality speed %f", g_Effect[client][fSpeed], g_Effect[client][fCustomSpeed],g_Effect[client][fPenalitySpeed]);
#endif
}

public SpeedRunePickup(client,ref)
{
	SDKHook(client,SDKHook_PreThinkPost,OnPreThinkPost);
	SetSpeedPenalityConditions(client);
	SetCustomSpeedVars(client);
	g_Effect[client][active] = true;
}

public SpeedRuneDrop(client,ref)
{
	g_Effect[client][active] = false;
	SDKUnhook(client,SDKHook_PreThinkPost,OnPreThinkPost);
	//LogMessage("reset speed %f", g_Effect[client][fSpeed]);
	if (client > 0  && IsValidEntity(client) && IsPlayerAlive(client) && !IsClientObserver(client) )
	{
		SetEntPropFloat(client,Prop_Data,FlMaxSpeed, g_Effect[client][fSpeed]);
	}
}

public event_PlayerClassChange_Pre(Handle:event, const String:name[], bool:dontBroadcast)
{
	new client = GetClientOfUserId(GetEventInt(event, "userid"));
	if (client > 0  && IsPlayerAlive(client) && !IsClientObserver(client) )
	{
		if(g_Effect[client][active])
		{
			SDKUnhook(client,SDKHook_PreThinkPost,OnPreThinkPost);
			//g_Effect[client][fSpeed] = GetEntPropFloat(client,Prop_Data,FlMaxSpeed);
		}
		CreateTimer(0.50, Timer_SetDefaultSpeed, client);
	}
}

public event_PlayerClassChange(Handle:event, const String:name[], bool:dontBroadcast)
{
  new client = GetClientOfUserId(GetEventInt(event, "userid"));
  if (client > 0  && IsValidEntity(client) && IsPlayerAlive(client) && !IsClientObserver(client) )
  {
		if(g_Effect[client][active])
		{
			CreateTimer(0.55, Timer_SetHasteSpeed, client);
		}
  }
}

public event_PlayerSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
  new client = GetClientOfUserId(GetEventInt(event, "userid"));
  if (client > 0  && IsPlayerAlive(client) && !IsClientObserver(client) )
  {
    CreateTimer(0.50, Timer_SetDefaultSpeed, client);
  }
}

public Action:Timer_SetHasteSpeed(Handle:timer, any:client)
{

  if (client > 0  && IsValidEntity(client) && IsPlayerAlive(client) && !IsClientObserver(client) )
	{
		SetCustomSpeedVars(client);
		SDKHook(client,SDKHook_PreThinkPost,OnPreThinkPost);
	}
}

public Action:Timer_SetDefaultSpeed(Handle:timer, any:client)
{
	g_Effect[client][fSpeed] = GetEntPropFloat(client,Prop_Data,FlMaxSpeed);
}


#if defined DEBUG_RUNE
stock DebugSpeedCond(client, add = 1)
{
	decl String:sBuffer[128]="";
	GetClientName(client,sBuffer,sizeof(sBuffer));
	Format(sBuffer,sizeof(sBuffer),
		"%s Cond %s: %d %d %d %d", (add ? "ADD" : "REM"),
		sBuffer, g_Cond[client][bJarate], g_Cond[client][bBleed], g_Cond[client][bMilk], g_Cond[client][bDaze]);

	CPrintToChat(client,sBuffer);
	LogMessage(sBuffer);
	
}
#endif

public TF2_OnConditionAdded(client, TFCond:cond)
{
	switch(cond)
	{
		case TFCond_Jarated:
		{
			g_Cond[client][bJarate] = true;
		}
		case TFCond_Bleeding:
		{
			g_Cond[client][bBleed] = true;
		}
		case TFCond_Milked:
		{
			g_Cond[client][bMilk] = true;
		}
		case TFCond_Dazed:
		{
			g_Cond[client][bDaze] = true;
		}
		default:
		{
			return;
		}
	}

#if defined DEBUG_RUNE
	DebugSpeedCond(client);
#endif

	if( g_Effect[client][bSpeedPenality] != true)
 		g_Effect[client][bSpeedPenality] = true;
}

public TF2_OnConditionRemoved(client,TFCond:cond)
{
	switch(cond)
	{
		case TFCond_Jarated:
		{
			g_Cond[client][bJarate] = false;
		}
		case TFCond_Bleeding:
		{
			g_Cond[client][bBleed] = false;
		}
		case TFCond_Milked:
		{
			g_Cond[client][bMilk] = false;
		}
		case TFCond_Dazed:
		{
			g_Cond[client][bDaze] = false;
		}
		default:
		{
			return;
		}
	}

#if defined DEBUG
	DebugSpeedCond(client,0);
#endif

	g_Effect[client][bSpeedPenality] = g_Cond[client][bJarate]
			|| g_Cond[client][bBleed]
			|| g_Cond[client][bDaze]
			|| g_Cond[client][bMilk];

}


public OnPluginStart()
{
	AddRune("Haste",SpeedRunePickup,SpeedRuneDrop,1);
	HookEvent("player_spawn",event_PlayerSpawn);
	HookEvent("player_changeclass", event_PlayerClassChange_Pre,EventHookMode_Pre);
	HookEvent("player_changeclass", event_PlayerClassChange);

}
// FindSendPropOffs("CBasePlayer","m_flLaggedMov ementValue")

public OnPreThinkPost(x)
{
	new Float:CustomSpeed;
	if(g_Effect[x][bSpeedPenality])
		CustomSpeed = g_Effect[x][fPenalitySpeed];
	else
		CustomSpeed = g_Effect[x][fCustomSpeed];

	SetEntPropFloat(x, Prop_Data, FlMaxSpeed, CustomSpeed);
}  


public RunePluginStart()
{
	PrintToServer("SpeedRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("SpeedRunePluginStop\n");
}

