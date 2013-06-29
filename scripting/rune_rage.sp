#include <sourcemod>
#include <sdktools>
#include <events>
#include <clients> 
#include <sdkhooks>
#include <smlib>

#include <tf2_stocks>
#define REQUIRE_EXTENSIONS
#define AUTOLOAD_EXTENSIONS
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

#define PLUGIN_NAME "Rune of Rage and Vanguard"
#define PLUGIN_DESCRIPTION "Increases rate of fire on hit.  Redirects damage from nearby weaker teammates."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}



enum RuneInfo
{
	Active,
	InRage,
};


// particles 
	// water_blood_impact_red_01
	// lowV_water_bubbles
	// miss_text


// env_sawblood 
		//death my vanguard

new g_Effect[MAXPLAYERS][RuneInfo];

new g_bPlayersHooked = false;

#define RUNE_RAGE 1
#define RUNE_VANGUARD 2 


new Handle:hVanguardRed = INVALID_HANDLE;
new Handle:hVanguardBlue = INVALID_HANDLE;

static offsNextPrimaryAttack 
static offsNextSecondaryAttack 
static offsWeaponPlaybackRate	
public OnPluginStart()
{
	offsNextPrimaryAttack 	= FindSendPropInfo("CTFWeaponBase", "m_flNextPrimaryAttack");
	offsNextSecondaryAttack = FindSendPropInfo("CTFWeaponBase", "m_flNextSecondaryAttack");
	offsWeaponPlaybackRate	= FindSendPropInfo("CTFWeaponBase",	"m_flPlaybackRate");



	AddRune("Rage", RageRunePick,RageRuneDrop, RUNE_RAGE);
	AddRune("Vanguard",VanguardRunePick, VanguardRuneDrop, RUNE_VANGUARD);
	g_bPlayersHooked = false;

	hVanguardRed = CreateArray();
	hVanguardBlue = CreateArray();
}

new HookRuneRef[RUNE_VANGUARD];

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public OnPluginEnd()
{
	//CloseHandle(g_RageRune);
}


FindVanguardIndex(client, &Handle:hVanguard)
{
	if(GetClientTeam(client) == 2)
		hVanguard = hVanguardRed;
	else if(GetClientTeam(client) == 3)
		hVanguard = hVanguardBlue;
	return FindValueInArray(hVanguard,client);
}

public VanguardRunePick(client, rune, ref)
{
	g_Effect[client][Active] = RUNE_VANGUARD;

	if(CheckHook(RUNE_RAGE,ref))
		HookPlayers();

	new Handle:hVanguard;
	if(FindVanguardIndex(client,hVanguard) == -1)
			PushArrayCell(hVanguard,client);
}


public VanguardRuneDrop(client, rune, ref)
{
	g_Effect[client][Active] = 0;

	if(CheckUnhook(RUNE_RAGE,ref))
		Unhookplayers();

	new Handle:hVanguard;
	new idx;
	if( (idx = FindVanguardIndex(client,hVanguard)) != -1)
		RemoveFromArray(hVanguard, idx);
}

public RageRunePick(client, rune, ref)
{
	g_Effect[client][Active] = RUNE_RAGE;
	g_Effect[client][InRage] = 0;
	if(CheckHook(RUNE_RAGE,ref))
		HookPlayers();
	return 1;
}

public RageRuneDrop(client, rune, ref)
{
	g_Effect[client][Active] = 0;
	g_Effect[client][InRage] = 0;

	if(CheckUnhook(RUNE_RAGE,ref))
		Unhookplayers();
	return 1;
}

stock CheckHook(rune, ref)
{
	
	HookRuneRef[rune - 1] = ref;
	if(ref == 1 && HookRuneRef[0] + HookRuneRef[1] == 1)
		return true;
	return false;
}

stock CheckUnhook(rune,ref)
{
	HookRuneRef[rune - 1] = ref;
	if(ref == 0 && HookRuneRef[0] + HookRuneRef[1] == 0)
		return true;
	return false;
}

HookPlayers()
{
	LogMessage("Hook");
	for(new i = 1; i <= GetMaxClients();++i)
	{
		if(!IsValidEntity(i))
			continue;
		SDKHook(i, SDKHook_OnTakeDamage, OnRageDmg);
	}
	g_bPlayersHooked = true;
}

Unhookplayers()
{
	LogMessage("UnHook");
	for(new i = 1; i <= GetMaxClients();++i)
	{
		if(!IsValidEntity(i))
			continue;
		SDKUnhook(i, SDKHook_OnTakeDamage, OnRageDmg);
	}
	g_bPlayersHooked = false;
}

public OnClientPutInServer(client)
{
	if(g_bPlayersHooked)
		SDKHook(client, SDKHook_OnTakeDamage, OnRageDmg);
}

public OnPlayerDisconnecting(client)
{
	if(g_bPlayersHooked)
		SDKUnhook(client, SDKHook_OnTakeDamage, OnRageDmg);
}

public Think_FastWeaponFire(client)
{
	new ent;
	//PrintToServer("Think");
	if(!IsValidEntity(client) || (ent = Client_GetActiveWeapon(client)) <= 0 || !IsValidEntity(ent) || !IsPlayerAlive(client) || g_Effect[client][Active] == 0)
	{
		g_Effect[client][InRage] = 0;
		SDKUnhook(client,SDKHook_PostThink,Think_FastWeaponFire);
		//PrintToServer("Invalid Weapon");
		return;
	}

	
	if(g_Effect[client][InRage] == 0)
		return;
		
	SDKUnhook(client,SDKHook_PostThink,Think_FastWeaponFire);
	new Float:enginetime = GetGameTime();

	new Float:flPriTime = (GetEntDataFloat(ent, offsNextPrimaryAttack) - enginetime);
	new Float:flSecTime = (GetEntDataFloat(ent, offsNextSecondaryAttack) - enginetime);

	SetEntDataFloat(ent,offsWeaponPlaybackRate,1.667);
	SetEntDataFloat(ent, offsNextPrimaryAttack, (flPriTime/1.667) + enginetime,true);
	SetEntDataFloat(ent, offsNextSecondaryAttack,(flSecTime/1.667) + enginetime,true);

	g_Effect[client][InRage] = 0;
}


public Action:OnRageDmg(victim, &attacker, &inflictor, &Float:damage, &damagetype, &weapon, Float:damageForce[3], Float:damagePosition[3])
{
	if(IsValidEntity(attacker) && attacker > 0 && attacker <= GetMaxClients() &&
		attacker != victim && IsPlayerAlive(attacker) && g_Effect[attacker][Active] == RUNE_RAGE)
	{
		if(g_Effect[attacker][InRage] == 0)
			{
				SDKHook(attacker,SDKHook_PostThink,Think_FastWeaponFire);
				g_Effect[attacker][InRage] = 1;
			}
	}

	new vTeam = GetClientTeam(victim);
	new Handle:hVanguard;
	
	if(vTeam == 2)
		hVanguard = hVanguardRed;
	else
		hVanguard = hVanguardBlue;

	new size;

	if((size = GetArraySize(hVanguard)) == 0)
	{
		return Plugin_Continue;
	}

	new vHealth = GetClientHealth(victim);
	new Float:vOri[3]
	GetClientAbsOrigin(victim, vOri);

	new best_i = -1;
	new best_h = -1;

	if(TF2_GetPlayerClass(victim) == TFClass_Medic)
	{
		best_i = TF2_GetHealingTarget(victim);
		if(best_i > 0 && g_Effect[best_i][Active] == RUNE_VANGUARD)
		{
			SDKHooks_TakeDamage(best_i, attacker, inflictor, damage, damagetype, weapon, damageForce, damagePosition);
			CreateParticleAtEntity(victim,"miss_text");
			damage = 0.0;
			return Plugin_Changed;
		}
	}


	for(new i; i < size;++i)
	{
		new Float:v_iOri[3];
		new v_h;
		new v_i = GetArrayCell(hVanguard,i);
		if( (v_h = GetClientHealth( v_i )) <= vHealth )
			continue;
		if(!IsPlayerAlive(v_i))
			continue;
		GetClientAbsOrigin(v_i,v_iOri);
		if(GetVectorDistance(vOri,v_iOri) > 535.0)
			continue;
	
		if(v_h > best_h)
		{
			best_h = v_h
			best_i = i;
		}
		
	}

	if(best_i == -1)
		return Plugin_Continue;

	CreateParticleAtEntity(victim,"miss_text");

	victim = GetArrayCell(hVanguard,best_i);
	if(float(best_h) < damage)
	{
		CreateParticleAtEntity(victim,"env_sawblood");
	} else {
		CreateParticleAtEntity(victim,"water_blood_impact_red_01");
	}
	SDKHooks_TakeDamage(victim, attacker, inflictor, damage, damagetype, weapon, damageForce, damagePosition);
	damage = 0.0;
	damageForce[2] = 0.0;

	return Plugin_Changed;

}


public RunePluginStart()
{
	PrintToServer("RageRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("RageRunePluginStop\n");
}


