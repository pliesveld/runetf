#include <sourcemod>
#include <sdktools>
#include <events>
#include <clients> 
#include <sdkhooks>
#include <smlib>

#include <tf2>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>


#define PLUGIN_NAME "Rune of Sacrifice and DiamondSkin"
#define PLUGIN_DESCRIPTION "Damage reduction and damage sharing."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}



enum EffType
{
	Active,
}

//particles
		// ghost_pumpkin
		// blood_impact_red_01
		// env_sawblood_mist

// big redirect
// teleportedin_(red|blue)
// dispenser_heal(red|blue)
// medicgun_beal_red|blue
//healthgained_red

// heal_text


// miss?  

new g_Effect[MAXPLAYERS][EffType];

#define RUNE_SHARE_ID 1
#define RUNE_ARMOR_ID 2

public OnPluginStart()
{
	AddRune("Sacrifice", SacraficeRunePick,SacraficeRuneDrop,RUNE_SHARE_ID);
	AddRune("Diamondskin", DiamondskinRunePick,DiamondskinRuneDrop,RUNE_ARMOR_ID);
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public OnPluginEnd()
{
}

public SacraficeRunePick(client, rune, ref)
{
	g_Effect[client][Active] = RUNE_SHARE_ID;
	if(!SDKHookEx(client,SDKHook_OnTakeDamage,OnShareDmg))
	{
		ThrowError("SDKHookEx failed");
		return 0;
	}

	return 1;
}

public SacraficeRuneDrop(client, rune, ref)
{
	g_Effect[client][Active] = 0;
	SDKUnhook(client, SDKHook_OnTakeDamage,OnShareDmg);
	return 1;
}


public DiamondskinRunePick(client, rune, ref)
{
	g_Effect[client][Active] = RUNE_ARMOR_ID;
	if(!SDKHookEx(client,SDKHook_OnTakeDamage,OnNegateDmgType))
	{
		ThrowError("SDKHookEx failed");
		return 0;
	}

	return 1;
}

public DiamondskinRuneDrop(client, rune, ref)
{
	g_Effect[client][Active] = 0;
	SDKUnhook(client, SDKHook_OnTakeDamage,OnNegateDmgType);
	return 1;
}


public Action:OnShareDmg(victim, &attacker, &inflictor, &Float:damage, &damagetype, &weapon, Float:damageForce[3], Float:damagePosition[3])
{
	new ClientsNearby[MAXPLAYERS]; //= {{0}};
	new cnt = 0;

	new vTeam = GetClientTeam(victim);

	if(victim == attacker) // don't share self-damage
		return Plugin_Continue;

	if( g_Effect[victim][Active] == RUNE_SHARE_ID) 
	//Do I really need to check this? -- not unless I use custom forwards and register during RUNE_SHARE pickup
		// or rewrite event handling.
	{
		for(new i = 1; i <= GetMaxClients();++i)
		{
			if(!IsValidEntity(i) || !IsValidEntity(victim) || !IsPlayerAlive(i) || !IsPlayerAlive(victim))
				continue;
	
			if(vTeam != GetClientTeam(i))
				continue;

			if( i == attacker || i == victim)
				continue;

			if(Entity_InRange(victim, i, 475.00))
			{
				ClientsNearby[cnt] = i; 
				cnt++;
			}
		}
		decl Float:ori_vic[3]
		GetClientAbsOrigin(victim, ori_vic);
		SubtractVectors(damagePosition, ori_vic, ori_vic);

		if( cnt > 0 )
		{
			damage = Math_Min(FloatDiv(damage, float(cnt+1)), float(5));
			new new_vic;
			new new_attacker = attacker;
			new new_inflictor = inflictor;
			new Float:new_damage = damage;
			new new_damagetype = damagetype
			new new_weapon = weapon;
			new Float:new_damageForce[3];
			AddVectors(new_damageForce,damageForce,new_damageForce);
			for(new i = 0;  i < cnt; ++i)
			//for(new i = 0; (new_vic = ClientsNearby[i]) != 0; ++i)
			{
				new_vic = ClientsNearby[i];	
				decl Float:new_damagePosition[3];
				GetClientAbsOrigin(new_vic, new_damagePosition);
				AddVectors(new_damagePosition, ori_vic, new_damagePosition);

				//PrintToServer("dbg%i(%d): new vic %d dmg %f", i,cnt,ClientsNearby[i], new_damage);
				//Client_PrintHintTextToAll("dbg%i(%d): new vic %d dmg %f", i,cnt,ClientsNearby[i], new_damage);
 				SDKHooks_TakeDamage(new_vic, new_attacker, new_inflictor, new_damage, new_damagetype,new_weapon, new_damageForce, new_damagePosition);
				CreateParticleAtEntity(new_vic,"blood_impact_red_01");
			}

			if(vTeam == 2)
				CreateParticleAtEntity(victim,"healthgained_red",0.5,false,0.2);
			else
				CreateParticleAtEntity(victim,"healthgained_blue",0.5,false,0.2);
			
			//return Plugin_Continue;
			return Plugin_Changed;

		}
	}
	return Plugin_Continue;
}	


public TF2_OnConditionAdded(client,TFCond:condition)
{
	if(g_Effect[client][Active] == RUNE_ARMOR_ID)
	{
		if(
			 condition == TFCond:TFCond_Jarated ||
			 condition == TFCond:TFCond_Bleeding ||
			 condition == TFCond:TFCond_Milked ||
			 condition == TFCond:TFCond_DefenseBuffMmmph)
		{
			TF2_RemoveCondition(client,condition);
		}
	}
}


public Action:OnNegateDmgType(victim, &attacker, &inflictor, &Float:damage, &damagetype, &weapon, Float:damageForce[3], Float:damagePosition[3])
{
	if(g_Effect[victim][Active] != RUNE_ARMOR_ID)
		return Plugin_Continue;

	damagetype = 0;
	if(damage > 10.0)
		damage -= 10.0;
	return Plugin_Changed;
}

public RunePluginStart()
{
	PrintToServer("DmgRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("DmgRunePluginStop\n");
}

