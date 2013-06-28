//Upon death, if you were killed by an engineer's sentry gun;  assume ownership of gun.


// Upon object created, if object has netstr COSentryObject, record the player, and increase the count


//upon death, check count of sentry guns.  
// if other sentry guns, scan all entities for COSentryObject
// on hit, check m_builder -- and change if attacker was the owner of this sentry

#include <sdktools>
#include <sdkhooks>
#include <smlib>
#include <tf2>
#include <tf2_stocks>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

#define PLUGIN_NAME "Rune of Engineer"
#define PLUGIN_DESCRIPTION "Building related runes."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


//particles  tpdamage_4
		// spark_electric01
// crutgun_firstperson

//SetEntProp(entity, Prop_Send, "m_iHighestUpgradeLevel", 3);
enum RuneInfo
{
	Active,
}



new g_Effect[MAXPLAYERS][RuneInfo];


#define RUNE_ENGINEERING 1
#define RUNE_WRECK 2

public OnPluginStart()
{
	HookEvent("player_builtobject",Event_Built);
	AddRune("Engineering", EngineerRunePick, EngineerRuneDrop,RUNE_ENGINEERING);
	AddRune("Sabatoge",WreckRunePick, WreckRuneDrop, RUNE_WRECK);

	HookWrecker();
}

public OnPluginEnd()
{
	//CloseHandle(g_SentryRune);
}


/*
	player_builtobject
	player_upgradedobject
	player_carryobject
	player_dropobject

	player_sapped_object
		//ownerid - building owner
		//sapperid - index of sapper
	object_removed
	object_destroyed

//HookEvent("player_sapped_object", EventTestSapper);
public Action:EventTestSapper(Handle:event, const String:name[], bool:dontBroadcast)
{
  new spy = GetClientOfUserId(GetEventInt(event, "userid"));
  new engi = GetClientOfUserId(GetEventInt(event, "ownerid"));
  new sapper = GetEventInt(event, "sapperid");
	DbgPrint("sapper", sapper);
}



*/

public WreckRunePick(client,rune,ref)
{
	g_Effect[client][Active] = RUNE_WRECK;
	if(ref == 1)
		HookWrecker();
	SDKHook(client,SDKHook_TraceAttack,Wrecker_TraceAttack);
}

public WreckRuneDrop(client,rune,ref)
{
	g_Effect[client][Active] = 0;
	if(ref == 0)
		UnhookWrecker();
	SDKUnhook(client,SDKHook_TraceAttack,Wrecker_TraceAttack);
}

public Action:Wrecker_TraceAttack(victim, &attacker, &inflictor, &Float:damage, &damagetype, &ammotype, hitbox, hitgroup)
{
	if(!g_Effect[victim][Active] || !IsValidEntity(attacker) || !IsPlayerAlive(attacker))
		return Plugin_Continue;
	
	if(TF2_GetPlayerClass(attacker) != TFClass_Engineer)
		return Plugin_Continue;
	
	if( g_Effect[victim][Active] == RUNE_WRECK)
	{
		//LogMessage("TraceAttack attacker %d inflictor %d",attacker, inflictor);
		if(!TF2_IsPlayerInCondition(victim,TFCond_MarkedForDeath))
		{
			TF2_AddCondition(victim, TFCond_MarkedForDeath, 5.0);
		}
		damage += 13.0;
		return Plugin_Changed;
	}
	return Plugin_Continue;
}

public EngineerRunePick(client, rune, ref)
{
	g_Effect[client][Active] = RUNE_ENGINEERING;
	if(ref == 1)
	{
		HookEvent("player_carryobject", Event_Carry);
		HookEvent("player_dropobject", Event_Drop);
		HookEvent("player_death", Event_PlayerDeath, EventHookMode_Pre)
	}
}

public EngineerRuneDrop(client, rune, ref)
{
	g_Effect[client][Active] = 0;
	if(ref == 0)
	{
		UnhookEvent("player_carryobject", Event_Carry);
		UnhookEvent("player_dropobject", Event_Drop);
		UnhookEvent("player_death", Event_PlayerDeath, EventHookMode_Pre);
	}

}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}


public Action:Event_Built(Handle:event, const String:name[], bool:dontBroadcast)
{
	new client = GetClientOfUserId(GetEventInt(event,"userid"));
	decl String:sName[24]=""
	decl String:sBuf[128]="";
	GetClientName(client,sName,sizeof(sName));
	
	new TFObjectType:obj = TFObjectType:GetEventInt(event,"object");
	new ent = GetEventInt(event,"index");
	
	Format(sBuf,sizeof(sBuf),"%s built %s health %d",
		sName,
		(obj == TFObject_Dispenser ? "dispenser" :
		obj == TFObject_Teleporter ? "teleporter" :
		obj == TFObject_Sentry ? "sentry" :
		obj == TFObject_Sapper ? "sapper" : "unknown"),
		Entity_GetHealth(ent));
	//LogMessage(sBuf);
	return Plugin_Continue;
}

/*
bot -class Engineer
bot_mimic_yaw_offset 0
bot_mimic 1
bot_command bot01 "build 3"
bot_refill
*/
stock HookWrecker()
{

	HookEntityOutput("obj_attachment_sapper","OnDamaged",ObjSapped_Wrecker);
	HookEntityOutput("obj_dispenser","OnDamaged",ObjDamaged_Wrecker);
	HookEntityOutput("obj_sentrygun","OnDamaged",ObjDamaged_Wrecker);
	HookEntityOutput("obj_teleporter","OnDamaged",ObjDamaged_Wrecker);
	HookEvent("object_destroyed",Event_Destroy);
}
stock UnhookWrecker()
{
	UnhookEntityOutput("obj_attachment_sapper","OnDamaged",ObjSapped_Wrecker);
	UnhookEntityOutput("obj_dispenser","OnDamaged",ObjDamaged_Wrecker);
	UnhookEntityOutput("obj_sentrygun","OnDamaged",ObjDamaged_Wrecker);
	UnhookEntityOutput("obj_teleporter","OnDamaged",ObjDamaged_Wrecker);
	UnhookEvent("object_destroyed",Event_Destroy);
}

public ObjDamaged_Wrecker(const String:output[], caller, activator, Float:delay)
{
	//if(g_Effect[activator][Active] != RUNE_WRECK)
	//	return;

//	new client = GetEntDataEnt2(activator, FindSendPropOffs("CObjectSapper","m_hBuilder"));

	//LogMessage("damaged %d",Entity_GetHealth(caller));
	new n_dmg = - (15 + GetURandomInt()%50);
	SetVariantInt(n_dmg);
	AcceptEntityInput(caller,"AddHealth", -1,-1,0);
//	LogMessage("damaged %d",Entity_GetHealth(caller));
	
	//decl String:sName[24]="";
		//activator is not the client index of the spy that is sapping the building.
		//activator is probably a building, and I probably want the m_hBuilder of the activator ent
	//GetClientName(activator,sName,sizeof(sName))   
	//decl String:message[256]="";
	//Format(message,sizeof(message),"%s has sabotaged your gear!",sName);
	//Client_PrintKeyHintText(client,message);
}


public ObjSapped_Wrecker(const String:output[], caller, activator, Float:delay)
{
	decl String:cname[24]="";
	Entity_GetClassName(caller,cname,sizeof(cname));
	new client = GetEntDataEnt2(caller, FindSendPropOffs("CObjectSapper","m_hBuilder"))

	if(g_Effect[client][Active] != RUNE_WRECK)
		return;

	if(!strcmp(output,"OnDamaged"))
	{
		SetVariantInt(36);
		AcceptEntityInput(caller,"AddHealth", -1,-1,0);
	}

	//decl String:sName[24]="";
	//GetClientName(client,sName,sizeof(sName))
	//LogMessage("%s's %s::%s %d",sName,cname,output,Entity_GetHealth(caller));
	
	//decl String:message[256]="";
	//Format(message,sizeof(message),"%s has sabotaged your gear!",sName);
	//Client_PrintKeyHintText(activator,message);

}

public Action:Event_Drop(Handle:event, const String:name[], bool:dontBroadcast)
{
	new client = GetClientOfUserId( GetEventInt(event,"userid"));
//	new obj = GetEventInt(event,"object");
	new objIndex = GetEventInt(event,"index");
	if( g_Effect[client][Active] )
	{
		//LogMessage("%d dropped %d", client, objIndex);

		SetEntPropFloat(objIndex,Prop_Send,"m_flPercentageConstructed",0.75);

	}

}
public Action:Event_Carry(Handle:event, const String:name[], bool:dontBroadcast)
{
	new client = GetClientOfUserId( GetEventInt(event,"userid"));
	if( g_Effect[client][Active] )
	{
		TF2_AddCondition(client, TFCond_SpeedBuffAlly, 10.0);
		TF2_AddCondition(client, TFCond_MarkedForDeath, 2.0);
	}
	return Plugin_Continue;
}


public Action:Event_PlayerDeath(Handle:event, const String:name[], bool:dontBroadcast)
{
	new userid = GetEventInt(event, "userid")
	new iSentry = GetEventInt(event, "inflictor_entindex");
	new client = GetClientOfUserId(userid);
	
	if (!g_Effect[client][Active] || !IsValidEntity(iSentry))
	{
		return Plugin_Continue;
	}
	
	decl String:netclass[32];
	GetEntityNetClass(iSentry, netclass, sizeof(netclass));

	if (!strcmp(netclass, "CObjectSentrygun"))
	{
		new iTeam = GetClientTeam(client);

	// SetEntProp(iSentry, Prop_Send, "m_bHasSapper", 0, 2);
	// SetEntProp(iDispenser, Prop_Send, "m_bDisabled", 0, 2);
	//     SetEntPropFloat(iTeleporter, Prop_Send, "m_flPercentageConstructed",     1.0);


		if( GetURandomFloat() < 0.07 )
		{ // 10% of the time, the sentry will turncoat
			SetEntData(iSentry, FindSendPropOffs("CObjectSentrygun","m_nSkin"),  (iTeam-2), 1, true);
			SetEntDataEnt2(iSentry, FindSendPropOffs("CObjectSentrygun","m_hBuilder"),     client, true);

			SetVariantInt(iTeam);
			AcceptEntityInput(iSentry, "TeamNum", -1, -1, 0);

			SetVariantInt(iTeam);
			AcceptEntityInput(iSentry, "SetTeam", -1, -1, 0); 
		} /*else {
			SetEntData(iSentry,FindSendPropOffs("CObjectSentrygun","m_Disabled"), 1, 2, true);
			CreateTimer(7.0, TimerEnableSentry, EntIndexToEntRef(iSentry));
		}*/
	}
	return Plugin_Continue;
}

public Action:TimerEnableSentry(Handle:timer, any:data)
{ 
	decl String:netclass[32];  

	new iSentry = EntRefToEntIndex(data);

	if(iSentry == INVALID_ENT_REFERENCE || !IsValidEntity(iSentry))
		return Plugin_Stop;

	GetEntityNetClass(iSentry, netclass, sizeof(netclass));
	if (!strcmp(netclass, "CObjectSentrygun")) 
	{
		SetEntData( iSentry, FindSendPropOffs("CObjectSentrygun","m_bDisabled") , 0 , 2 , true );
	}

	return Plugin_Continue;
}       

public Action:Event_Destroy(Handle:event, const String:name[], bool:dontBroadcast)
{
//	new index = GetEventInt(event,"index");
	new userid = GetEventInt(event,"attacker");
	if(userid > 0)
	{
		new attacker = GetClientOfUserId(userid);
		if( g_Effect[attacker][Active] == RUNE_WRECK )
		{
			TF2_AddCondition(attacker,TFCond_MarkedForDeath,4.0);
			TF2_AddCondition(attacker,TFCond_Jarated,2.5);
		}
	}

	userid = GetEventInt(event,"assister");
	if(userid > 0)
	{
		new assister = GetClientOfUserId(userid);
		if( g_Effect[assister][Active] == RUNE_WRECK )
		{
			TF2_AddCondition(assister,TFCond_MarkedForDeath,2.0);
			TF2_AddCondition(assister,TFCond_Jarated,1.5);
		}
	}

}

public RunePluginStart()
{
	PrintToServer("EngineerRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("EngineerRunePluginStop\n");
}

/*
public Plugin:myinfo =
{
	name = "Engineer Rune",
	author = "happs",
	description = "Turncoats sentry on death",
	version = "1.0.0",
	url = "http://localhost/"
}

*/
