#include <sdktools_entoutput>
#include <sdkhooks>
#include <smlib>
#include <tf2>
#include <tf2_stocks>

#define REQUIRE_EXTENSIONS
#define AUTOLOAD_EXTENSIONS
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

enum RuneProp
{
  bool:active,
}


#define PLUGIN_NAME "Rune of Berserker"
#define PLUGIN_DESCRIPTION "Restricts weapon to melee, and grants crits on hit."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


// faster movement speed
// melee crits
// drain rate

//berserker rage is hard on the body




new g_Effect[MAXPLAYERS][RuneProp]

public MeleeRunePickup(client,rune)
{
	new weapon = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee);
	if(Client_GetActiveWeapon(client) != weapon)
		Client_SetActiveWeapon(client, weapon);

	SDKHook(client,SDKHook_WeaponCanSwitchTo,BlockWeaponSwitch);
	g_Effect[client][active] = true;
  
}


public MeleeRuneDrop(client,rune)
{
	SDKUnhook(client,SDKHook_WeaponCanSwitchTo, BlockWeaponSwitch);
	g_Effect[client][active] = false;
}

public Action:BlockWeaponSwitch(client,weapon)
{
    return Plugin_Handled;
}  

public OnPluginStart()
{
	AddRune("Melee", MeleeRunePickup, MeleeRuneDrop,1);
}

public Action:TF2_CalcIsAttackCritical(client, weapon, String:weaponname[], &bool:result)
{
	if( g_Effect[client][active] 
			&& GetPlayerWeaponSlot(client, TFWeaponSlot_Melee) == weapon)
	{
 		result = true;
		return Plugin_Changed;
	}
	return Plugin_Continue;
}


public RunePluginStart()
{
}

public RunePluginStop()
{
}


