#include <tf2>
#include <tf2_stocks>

#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>



enum RuneInfo
{
	Active,
}

new g_Effect[MAXPLAYERS][RuneInfo];

new g_AssaultRune = INVALID_HANDLE;


public RunePluginStart()
{
	PrintToServer("AssaultRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("AssaultRunePluginStop\n");
}


public OnPluginStart()
{

	g_AssaultRune = AddRune("Assault",AssaultRunePickup, AssaultRuneDrop,1);

#if defined DEBUG
	for(new i =1;i <= GetMaxClients();++i)
	{
		if(!IsValidEntity(i))
			continue;
	
		if(!IsPlayerAlive(i))
			continue;

		if(TF2_GetPlayerClass(i) != TFClass_Medic)
			continue;

		new entityIndex = GetPlayerWeaponSlot(i,1);

		SetEntPropFloat(entityIndex,Prop_Send,"m_flChargeLevel", 1.0);

	}
#endif
}

public AssaultRunePickup(client, rune, ref)
{
	g_Effect[client][Active] = 1;
	if(ref == 1)
	{
		HookEvent("player_chargedeployed", OnChargeDeployed);
		HookEvent("player_invulned", OnInvuln);
	}
}

public AssaultRuneDrop(client,rune,ref)
{
	g_Effect[client][Active] = 0;
	if(ref == 0)
	{
		UnhookEvent("player_chargedeployed", OnChargeDeployed);
		UnhookEvent("player_invulned", OnInvuln);
	}
}

public OnInvuln(Handle:event, const String:name[], bool:bDontbroadcast)
{

	new client = GetClientOfUserId(GetEventInt(event,"userid"));
	new medic = GetClientOfUserId(GetEventInt(event,"medic_userid"));


	if(g_Effect[client][Active] || g_Effect[medic][Active])
	{
		TF2_AddCondition(client,TFCond_SpeedBuffAlly,8.0);
		if(!g_Effect[medic][Active])
			TF2_AddCondition(medic,TFCond_SpeedBuffAlly,8.0);
	}

#if defined DEBUG
	decl String:sTarget[32];
	decl String:sMedic[32];
	GetClientName(client, sTarget, sizeof(sTarget));
	GetClientName(medic, sMedic, sizeof(sMedic));
	LogMessage("%s invuln %s", sMedic, sTarget);
#endif

	return Plugin_Continue;
}

public OnChargeDeployed(Handle:event, const String:name[], bool:bDontbroadcast)
{

	new client = GetClientOfUserId(GetEventInt(event,"userid"));
	new targetid = GetEventInt(event,"targetid")
	new target;

	if(g_Effect[client][Active])
	{
		TF2_AddCondition(client,TFCond_SpeedBuffAlly,8.0);
	}
	

#if defined DEBUG
	decl String:sTarget[32]="";
	decl String:sMedic[32];
	if(targetid > 0)
	{
		target = GetClientOfUserId(targetid);
		GetClientName(target, sTarget, sizeof(sTarget));
	}
	GetClientName(client, sMedic, sizeof(sMedic));
	LogMessage("%s deployed %s", sMedic, sTarget);
#endif
	return Plugin_Continue;
}

#if defined DEBUG
GetCondString(TFCond:cond, String:sText[], sLen)
{
	switch(cond)
	{
		case TFCond_Ubercharged:
		{
			StrCat(sText,sLen,"ubercharged");
		}
		case TFCond_Kritzkrieged:
		{
			StrCat(sText,sLen,"kritz");
		}
		case TFCond_MegaHeal:
		{
			StrCat(sText,sLen, "megaheal");
		}
		case TFCond_SpeedBuffAlly:
		{
			StrCat(sText,sLen,"speed buff");
		}
		default:
		{
			return 0;
		}
	}
	return 1;
}


#if defined DEBUG
public TF2_OnConditionAdded(client, TFCond:cond)
{
	decl String:sDbg[128] ="";
	decl String:sName[32]="";
	GetClientName(client,sName,sizeof(sName));

	if(GetCondString(cond,sDbg,sizeof(sDbg)))
	{
		StrCat(sDbg,sizeof(sDbg)," cond added");
		LogMessage("%s %s",sName,sDbg);
	}

	return Plugin_Continue;

}

public TF2_OnConditionRemoved(client, TFCond:cond)
{
	decl String:sDbg[128] ="";
	decl String:sName[32]="";
	GetClientName(client,sName,sizeof(sName));

	if(GetCondString(cond,sDbg,sizeof(sDbg)))
	{
		StrCat(sDbg,sizeof(sDbg)," cond removed");
		LogMessage("%s %s",sName,sDbg);
	}

}
#endif
#endif
