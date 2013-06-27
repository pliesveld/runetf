#include <sdktools>
#include <sdkhooks>
#include <tf2>
#include <tf2_stocks>
#include <smlib>


#define REQUIRE_EXTENSIONS
#define AUTOLOAD_EXTENSIONS
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>
#include <runetf/runes_stock/tf2_ammo>

#define AMMO_PICKUP_SND "items/ammo_pickup.wav"
#define AMMO_CLIP_RELOAD "items/battery_pickup.wav"

//particles
// teleported_flash


enum RuneProp
{
  g_active,
  Float:g_timestamp,
};

new cloakoff;
new g_Effect[MAXPLAYERS][RuneProp];

static Handle:clientPassiveAmmoTimer[MAXPLAYERS + 1];

static MaxPrimaryClip[MAXPLAYERS + 1];
static MaxSecondaryClip[MAXPLAYERS + 1];
static MaxPrimaryAmmo[MAXPLAYERS + 1];
static MaxSecondaryAmmo[MAXPLAYERS + 1];

new Handle:ClientWeaponName[MAXPLAYERS + 1] = {INVALID_HANDLE,...};
#define MAX_WEAPONNAME_LEN 32

new g_AmmoRune = INVALID_HANDLE;

public OnPluginStart() 
{
	new cloakoff = FindSendPropInfo("CTFPlayer","m_flCloakMeter");
	//PrecacheSound(AMMO_PICKUP_SND,true);
	//PrecacheSound(AMMO_CLIP_RELOAD,true);
	g_AmmoRune = AddRune("Ammo", AmmoRunePickup, AmmoRuneDrop, 1);
	HookEvent("player_spawn",event_PlayerSpawn);
	HookEvent("post_inventory_application",event_PlayerSpawn);
}

public OnPluginEnd()
{
	//CloseHandle(g_AmmoRune);
}

public OnMapStart()
{
	PrecacheSound(AMMO_PICKUP_SND,true);
	PrecacheSound(AMMO_CLIP_RELOAD,true);
	return Plugin_Continue;
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public AmmoRunePickup(client,rune)
{
	if( clientPassiveAmmoTimer[client] == INVALID_HANDLE)
		clientPassiveAmmoTimer[client] = CreateTimer(7.0, cb_regen_ammo, client, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);

	g_Effect[ client ][ g_active ] = 1;
	g_Effect[ client ][ g_timestamp ] = GetGameTime();

}

public AmmoRuneDrop(client,rune)
{
	if(clientPassiveAmmoTimer[client] != INVALID_HANDLE)
		KillTimer(clientPassiveAmmoTimer[client]);
	clientPassiveAmmoTimer[client] = INVALID_HANDLE;
	g_Effect[ client ][ g_active ] = 0;
}




GetAmmoCount(client, weapon)
{
	new offset_ammo = FindDataMapOffs(client, "m_iAmmo");
	new offset = offset_ammo + (Weapon_GetPrimaryAmmoType(weapon) * 4);
	return GetEntData(client,offset);
}

SetAmmoCount(client, weapon, ammo)
{
	new offset_ammo = FindDataMapOffs(client, "m_iAmmo");
	new offset = offset_ammo + (Weapon_GetPrimaryAmmoType(weapon) * 4);
	return SetEntData(client,offset,ammo,true);
}

public Action:getMaxClipAmmo(Handle:timer, any:client)
{
	new slot0, slot1;
  MaxPrimaryClip[client] =   ((slot0 = GetPlayerWeaponSlot(client,TFWeaponSlot_Primary)) == -1) ? 0 : Weapon_GetPrimaryClip(slot0);
  MaxSecondaryClip[client] = ((slot1 = GetPlayerWeaponSlot(client,TFWeaponSlot_Secondary)) == -1) ? 0 : Weapon_GetPrimaryClip(slot1);

  MaxPrimaryAmmo[client] =  (slot0 == -1 ? 0 : GetAmmoCount(client,slot0));
  MaxSecondaryAmmo[client] = (slot1 == -1 ? 0 : GetAmmoCount(client,slot1));

	SetWeaponNameArray( slot0, slot1,ClientWeaponName[client]);
	
#if 0
	new String:buffer[1024];
	Format(buffer,sizeof(buffer), "client %d: Class: %dMaxAmmo:\nprimary   clip %d\nsecondary clip %d\nprimary   ammo %d\nsecondary ammo%d",client,TF2_GetPlayerClass(client),
		MaxPrimaryClip[client], MaxSecondaryClip[client], MaxPrimaryAmmo[client], MaxSecondaryAmmo[client]);
	PrintToConsole(client,buffer);
	LogMessage(buffer);
#endif
}

SetWeaponNameArray( weap0, weap1, &Handle:array)
{
	new String:s_wep[MAX_WEAPONNAME_LEN];
	new const String:null_weapon[] = {"null_weapon"};

	if(array != INVALID_HANDLE)
		ClearArray(array)
	else
		array = CreateArray(MAX_WEAPONNAME_LEN);

	if(weap0 != INVALID_ENT_REFERENCE)
	{
		Entity_GetClassName(weap0,s_wep,sizeof(s_wep))
		PushArrayString(array,s_wep);
	} else {
		PushArrayString(array,null_weapon);
	}

	if(weap1 != INVALID_ENT_REFERENCE)
	{
		Entity_GetClassName(weap1,s_wep,sizeof(s_wep))
		PushArrayString(array,s_wep);
	} else {
		PushArrayString(array,null_weapon);
	}
}



public event_PlayerSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
  new client = GetClientOfUserId(GetEventInt(event, "userid"));
  if (client > 0  && IsPlayerAlive(client) && !IsClientObserver(client) )
  {
    CreateTimer(0.50, getMaxClipAmmo, client);
  }
}



public Action:cb_regen_ammo(Handle:timer, any:client)
{
	if (!IsClientInGame(client)
	|| !IsPlayerAlive(client))
	{
		return Plugin_Stop;
	}

	new changed = 0;
	new TFClassType:class = TF2_GetPlayerClass(client);


	decl String:weaponName[MAX_WEAPONNAME_LEN]="";

	for(new weapon_slot = TFWeaponSlot_Primary; weapon_slot == TFWeaponSlot_Primary || weapon_slot == TFWeaponSlot_Secondary; ++weapon_slot)
	{
		if( ClientWeaponName[client] )
			GetArrayString( ClientWeaponName[ client ], weapon_slot, weaponName, sizeof(weaponName));
		//Entity_GetClassName(
		new weapon = INVALID_ENT_REFERENCE;
		new prev_ammo;
		new ammo_inc = GetAmmoInc(client, class, weapon_slot, weaponName, weapon);
		if(weapon != INVALID_ENT_REFERENCE && ammo_inc > 0)
		{
			//prev_ammo = Weapon_GetPrimaryAmmoCount(weapon);
			prev_ammo = GetAmmoCount(client, weapon);
			new max_ammo;
			if(weapon_slot == TFWeaponSlot_Primary)
				max_ammo = MaxPrimaryAmmo[client];
			else if(weapon_slot == TFWeaponSlot_Secondary)
				max_ammo = MaxSecondaryAmmo[client];

			if(prev_ammo != -1 && prev_ammo < max_ammo)
			{
				prev_ammo += ammo_inc;
				if(prev_ammo > max_ammo)
					prev_ammo = max_ammo;
				++changed;
			//Client_SetWeaponPlayerAmmoEx(client, weapon, prev_ammo);
		//Weapon_SetPrimaryAmmoCount(weapon, prev_ammo);
			  SetAmmoCount(client,weapon,prev_ammo);
			}

		}
	}
	if(class ==  TFClassType:TFClass_Engineer)
	{
		new prev_metal = TF2_GetMetalAmount(client);
		if(prev_metal < 200 )
		{
			prev_metal = (prev_metal >= 185) ? 200 : (prev_metal+15);
			++changed;
			TF2_SetMetalAmount(client,prev_metal);
		}
	}

	if(changed)
		EmitSoundToClient(client,AMMO_PICKUP_SND);

	return Plugin_Continue;
}


new g_AmmoTable[TFClass_Engineer ][2] =
{
	//ammo
	// { slot prim, slot sec } 
	{3,2}, // scout
	{6,8}, // sniper
	{7,2}, // soldier
	{4,12}, // demoman
	{30,0}, // medic
	{40,0}, // heavy
	{60,4}, //pyro
	{5,0}, //spy
	{8,20} // engineer
}


GetAmmoInc(client,TFClassType:class, weapon_slot, String:weapon_name[], &weapon)
{
	new increment = 0;
	weapon = GetPlayerWeaponSlot(client, weapon_slot);
	increment = g_AmmoTable[class-1][weapon_slot];

/*
	if(StrEqual(weapon_name,"tf_weapon_sandvich"))
	{
		increment = 0;
	} else if(StrEqual(weapon_name,"tf_weapon_flaregun"))
	{
		increment = 5;
	}
*/
	return increment;
	
}


/* use forward ?*/
public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
  if( !g_Effect[ client ] [ g_active ] )
    return Plugin_Continue;
  if(! (buttons & IN_USE) )
    return Plugin_Continue;

  if( GetGameTime() < g_Effect[client][g_timestamp] )
    return Plugin_Continue;

  g_Effect[client][g_timestamp] = GetGameTime() + 4.0;
  //g_Effect[client][g_timestamp] = GetGameTime() + 30.0;

	new max_pri = MaxPrimaryClip[client];
	new max_sec = MaxSecondaryClip[client];

	new weapon_ent;
	new bool:changed = false;
	//PrintToConsole(client,"max_pri %d\nmax_sec %d", max_pri, max_sec);
	//LogMessage("max_pri %d\nmax_sec %d", max_pri, max_sec);

	if(max_pri > 0)
	{
		if((weapon_ent = GetPlayerWeaponSlot(client, TFWeaponSlot_Primary)) != INVALID_ENT_REFERENCE)
		{
			new current_clip = Weapon_GetPrimaryClip(weapon_ent);
			if(current_clip < max_pri)
			{
				Weapon_SetPrimaryClip(weapon_ent,max_pri);
				changed = true;
			}
		}
	}
	if(max_sec > 0)
	{
		if((weapon_ent = GetPlayerWeaponSlot(client, TFWeaponSlot_Secondary)) != INVALID_ENT_REFERENCE)
		{
			new current_clip = Weapon_GetPrimaryClip(weapon_ent);
			if(current_clip < max_sec)
			{
				Weapon_SetPrimaryClip(weapon_ent,max_sec);
				changed = true;
			}
		}
	}

	new iClass;
	if((iClass = TF2_GetPlayerClass(client)) == TFClass_Spy)
	{
		new Float:prev_cloak;
		if(cloakoff > 0 && (prev_cloak = GetEntDataFloat(client,cloakoff)) < 100.0)
		{
			SetEntDataFloat(client, cloakoff, 100.0);
			changed = true;
		}
	} else if(iClass == TFClass_Engineer) {
		new prev_ammo = TF2_GetMetalAmount(client);
		if(prev_ammo < 200)
		{
			prev_ammo += 75;
			if(prev_ammo > 200)
				prev_ammo = 200;
			TF2_SetMetalAmount(client,prev_ammo);
			changed = true;
		}
	}
	
	if(changed)
		EmitSoundToClient(client,AMMO_CLIP_RELOAD);

	return Plugin_Continue;
}


public RunePluginStart()
{
	PrintToServer("AmmoRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("AmmoRunePluginStop\n");
}

/*
public Plugin:myinfo = 
{
	name = "Ammo Rune",
	author = "Happs",
	description = "<- Description ->",
	version = "1.0",
	url = "<- URL ->"
}
*/
