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

// faster movement speed
// melee crits
// drain rate

//berserker rage is hard on the body




new g_Effect[MAXPLAYERS][RuneProp]

public MeleeRunePickup(client,rune)
{
  if(g_Effect[client][active])
    return LogError("client %d already active",client);

  new weapon = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee);
  if(Client_GetActiveWeapon(client) != weapon)
    Client_SetActiveWeapon(client, weapon);

  SDKHook(client,SDKHook_WeaponCanSwitchTo,BlockWeaponSwitch);
  g_Effect[client][active] = true;
  
	return Plugin_Continue;
}


public MeleeRuneDrop(client,rune)
{
  if(!g_Effect[client][active])
    return LogError("client %d not active",client);

  SDKUnhook(client,SDKHook_WeaponCanSwitchTo, BlockWeaponSwitch);
  g_Effect[client][active] = false;
	return Plugin_Continue;
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
	PrintToServer("MeleeRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("MeleeRunePluginStop\n");
}


