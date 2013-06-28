/*
* Infinite-Jumping (Bunny Hop, Double Jump & Initial Jump)
* 
* Description:
* Lets user auto jump when holding down space. This plugin includes the DoubleJump plugin too. This plugin should work for all games.
* 
* Installation:
* Place infinite-jumping.smx into your '<moddir>/addons/sourcemod/plugins/' folder.
* Place plugin.infinite-jumping.cfg into your '<moddir>/cfg/sourcemod/' folder.
* 
* 
* For more information see: http://forums.alliedmods.net/showthread.php?p=1239361 OR http://www.mannisfunhouse.eu/
*/


// enforce semicolons after each code statement
#pragma semicolon 1

/*****************************************************************
P L U G I N   I N F O
*****************************************************************/
#define PLUGIN_NAME				"Rune of Pounce"
#define PLUGIN_DESCRIPTION		"Based on Chanz's rune of infinite-jumping."


#undef PLUGIN_AUTHOR
#undef PLUGIN_URL
#define PLUGIN_AUTHOR			"Chanz"
#define PLUGIN_URL				"http://forums.alliedmods.net/showthread.php?p=1239361 OR http://www.mannisfunhouse.eu/"

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}

#include <sourcemod>
#include <sdktools>
#include <smlib>
#include <sdkhooks>

#include <tf2_stocks>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>


#define TIMER_THINK 10.0


enum RuneProp
{
	Active,
	Float:boost_initial,
};

enum PlayerMoveInfo
{
	bool:bIsInAir,
	bool:bJumpPressed,
	Float:fMomentTouchedGround,
};

new g_Effect[MAXPLAYERS][RuneProp];
new g_Move[MAXPLAYERS][PlayerMoveInfo];
new Float:fOldVels[MAXPLAYERS][3];


new bool:g_bAllow_ForwardBoost[MAXPLAYERS+1];

new offset_scoutdash;

enum VelocityOverride {
	VelocityOvr_None = 0,
	VelocityOvr_Velocity,
	VelocityOvr_OnlyWhenNegative,
	VelocityOvr_InvertReuseVelocity
};


/*****************************************************************
F O R W A R D   P U B L I C S
*****************************************************************/
public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max){
	
	return APLRes_Success;
}

#define RUNE_POUNCE   1
#define RUNE_VAULT 2
#define RUNE_AIR 3

public OnPluginStart() 
{
	offset_scoutdash = FindSendPropInfo("CTFPlayer", "m_iAirDash");
	
	AddRune("Pounce", PounceRunePickup, JumpRuneDrop,RUNE_POUNCE);
	AddRune("Vault", VaultRunePickup, JumpRuneDrop,RUNE_VAULT);
	AddRune("AirBud", AirRunePickup, AirRuneDrop,RUNE_AIR);
}

public RunePluginStart()
{
}

public RunePluginStop()
{
}

public AirRunePickup(client,rune,ref)
{
	SDKHook(client,SDKHook_PostThink,OnPostThinkAir);
	SDKHook(client,SDKHook_OnTakeDamage,OnFallDamage);
	g_Effect[client][Active] = RUNE_AIR;
	g_bAllow_ForwardBoost[client] = false;
}

public AirRuneDrop(client,rune,ref)
{
	SDKUnhook(client,SDKHook_PostThink,OnPostThinkAir);
	SDKUnhook(client,SDKHook_OnTakeDamage,OnFallDamage);
	g_Effect[client][Active] = 0;
	g_bAllow_ForwardBoost[client] = false;
	g_Effect[client][boost_initial] = 0.0;
}


public VaultRunePickup(client, rune, ref)
{
	SDKHook(client,SDKHook_PostThink,OnPostThink);
	SDKHook(client,SDKHook_OnTakeDamage,OnFallDamage);
	g_bAllow_ForwardBoost[client] = false;
	g_Effect[client][Active] = RUNE_VAULT;
	g_Effect[client][boost_initial] = 535.0;
}

public PounceRunePickup(client, rune, ref)
{
	SDKHook(client,SDKHook_PostThink,OnPostThink);
	SDKHook(client,SDKHook_OnTakeDamage,OnFallDamage);
	g_Effect[client][Active] = RUNE_POUNCE;
	g_bAllow_ForwardBoost[client] = true;
	g_Effect[client][boost_initial] = 290.0;

}

public JumpRuneDrop(client,rune,ref)
{
	SDKUnhook(client,SDKHook_PostThink,OnPostThink);
	SDKUnhook(client,SDKHook_OnTakeDamage,OnFallDamage);
	g_Effect[client][Active] = 0;
	g_bAllow_ForwardBoost[client] = false;
	g_Effect[client][boost_initial] = 0.0;
}


public Action:OnFallDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype, &weapon, Float:damageForce[3], Float:damagePosition[3], damagecustom)                  
{
	if(!Client_IsValid(victim))
	{
		return Plugin_Continue;
	}
	
	if(damagetype & DMG_FALL)
	{
		return Plugin_Handled;
	}

	return Plugin_Continue;
}



public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	if(!IsClientInGame(client) || !IsPlayerAlive(client))
	{
		//PrintToChatAll("[%s] client: %d is not ingame, alive or a bot",PLUGIN_NAME);
		return Plugin_Continue;
	}


	if(Client_GetWaterLevel(client) > Water_Level:WATER_LEVEL_FEET_IN_WATER){
		//PrintToChatAll("[%s] Water level: %d",PLUGIN_NAME,Client_GetWaterLevel(client));
		return Plugin_Continue;
	}
	
	if(Client_IsOnLadder(client)){
		//PrintToChatAll("[%s] is on ladder",PLUGIN_NAME);
		return Plugin_Continue;
	}

	

	switch( g_Effect[client][Active] )
	{
		case RUNE_VAULT:
		{
			return Client_HandleVaultJumping(client,buttons);
		}
		case RUNE_POUNCE:
		{
			//PrintToServer("tick %d mouse x%d y%d", GetGameTickCount() - tickcount,mouse[0], mouse[1]);
			return Client_HandlePounceJumping(client,buttons);
		}
		case RUNE_AIR:
		{
			return Client_HandleAirJumping(client,vel,angles);
		}
	}
	return Plugin_Continue;
}

stock Action:Client_HandleAirJumping(client,Float:vel[3],Float:angles[3])
{
	if(g_Move[client][bIsInAir])
	{
		vel[0] *= 0.315;
		vel[1] *= 0.315;

	}
	return Plugin_Changed;
}

stock Action:Client_HandleVaultJumping(client, &buttons)
{

	static m_iDash[MAXPLAYERS] = {0};
	

	if(TF2_GetPlayerClass(client) == TFClass_Scout)
	{
		new iDash = GetEntData(client,offset_scoutdash);

		if(iDash != m_iDash[client])
		{
			//PrintToServer("Dash %d => %d", iDash, m_iDash[client]);
			//if(m_iDash[client] == 0)
			if(g_Move[client][bIsInAir])
				Client_ForceJump(client,g_Effect[client][boost_initial]);
		}

		m_iDash[client] = iDash;
		return Plugin_Continue;
	}


	if(!g_Move[client][bIsInAir])
	{
		if(buttons & IN_JUMP) 
		{
			g_Move[client][bJumpPressed] = true;
			Client_ForceJump(client,g_Effect[client][boost_initial]); // this should be in Think
		}
	}

	return Plugin_Continue;
}

stock Action:Client_HandlePounceJumping(client, &buttons)
{

	decl Float:clientEyeAngles[3];
	GetClientEyeAngles(client,clientEyeAngles);

	if(!g_Move[client][bIsInAir])
	{
		if(GetTickedTime() - g_Move[client][fMomentTouchedGround] <= 0.2)
		{
			if(buttons & IN_JUMP) 
			{
				g_Move[client][bJumpPressed] = true;
				Client_ForceJump(client,g_Effect[client][boost_initial]);
			}
		}
		//PrintToServer("eyeAngle %f %f %f", clientEyeAngles[0], clientEyeAngles[1], clientEyeAngles[2]);
	}


	
	return Plugin_Continue;
}



/****************************************************************
C A L L B A C K   F U N C T I O N S
****************************************************************/

//Thank you DarthNinja & javalia for this.
stock Client_Push(client, Float:clientEyeAngle[3], Float:power, VelocityOverride:override[3]=VelocityOvr_None)
{
	decl	Float:forwardVector[3],
	Float:newVel[3];
	
	GetAngleVectors(clientEyeAngle, forwardVector, NULL_VECTOR, NULL_VECTOR);
	NormalizeVector(forwardVector, forwardVector);
	ScaleVector(forwardVector, power);
	
	Entity_GetAbsVelocity(client,newVel);
	
	for(new i=0;i<3;i++){
		switch(override[i]){
			case VelocityOvr_Velocity:{
				newVel[i] = 0.0;
			}
			case VelocityOvr_OnlyWhenNegative:{				
				if(newVel[i] < 0.0){
					newVel[i] = 0.0;
				}
			}
			case VelocityOvr_InvertReuseVelocity:{				
				if(newVel[i] < 0.0){
					newVel[i] *= -1.0;
				}
			}
		}
		
		newVel[i] += forwardVector[i];
	}
	
	Entity_SetAbsVelocity(client,newVel);
}

Client_ForceJump(client,Float:power)
{
	Client_Push(client,Float:{-90.0,0.0,0.0},power,VelocityOverride:{VelocityOvr_None,VelocityOvr_None,VelocityOvr_None});
}


public OnPostThinkAir(client)
{
	if(!g_Effect[client][Active])
	{
		SDKUnhook(client,SDKHook_PostThink,OnPostThinkAir);
		return; 	
	} 
	new iGroundEntity = GetEntPropEnt(client, Prop_Send, "m_hGroundEntity");

	if (iGroundEntity == -1)
	{
    // Air  
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", fOldVels[client]);
		g_Move[client][bIsInAir] = true;
		//if( (GetTickedTime() - g_Move[client][fMomentTouchedGround]) <= 1.2)
		Client_ForceJump(client,5.5);

	}
	else
	{
    // Ground or entity
		if (g_Move[client][bIsInAir])
		{
			g_Move[client][fMomentTouchedGround] = GetTickedTime();
			g_Move[client][bIsInAir] = false;
		}
	}
}


public OnPostThink(client)
{
	if(!g_Effect[client][Active])
	{
		SDKUnhook(client,SDKHook_PostThink,OnPostThink);
		return; 	
	} 

	new iGroundEntity = GetEntPropEnt(client, Prop_Send, "m_hGroundEntity");

	if (iGroundEntity == -1)
	{
	// Air  
		GetEntPropVector(client, Prop_Data, "m_vecVelocity", fOldVels[client]);
		g_Move[client][bIsInAir] = true;
	} else {
    // Ground or entity
		if (g_Move[client][bIsInAir])
		{
			g_Move[client][fMomentTouchedGround] = GetTickedTime();
			g_Move[client][bIsInAir] = false;
		}
	}
}



#if 0
Float:GetVectorAngle(Float:x, Float:y)
{
  // set this to an arbitrary value, which we can use for error-checking
  new Float:theta=1337.00;

  // some math :)
  if (x>0)
  {
    theta = ArcTangent(y/x);
  }
  else if ((x<0) && (y>=0))
  {
    theta = ArcTangent(y/x) + Pi;
  }
  else if ((x<0) && (y<0))
  {
    theta = ArcTangent(y/x) - Pi;
  }
  else if ((x==0) && (y>0))
  {
    theta = 0.5 * Pi;
  }
  else if ((x==0) && (y<0))
  {
    theta = -0.5 * Pi;
  }

  // let's return the value
  return theta;
}
#endif


