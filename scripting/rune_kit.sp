#include <sourcemod>
#include <sdktools>
#include <events>
#include <clients> 

#include <smlib>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>

#include <runetf/runes_stock/runegen_tempent>


#define PLUGIN_NAME "Rune of Sharing"
#define PLUGIN_DESCRIPTION "Share health/ammo kits with nearby teammates."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


enum RuneProp
{
	g_user,
	g_item,
};

new g_Effect[MAXPLAYERS][RuneProp];


new Handle:g_PickupMsgTrie = INVALID_HANDLE;

public OnPluginStart()
{
	HookEntities();
	CacheModels();
	AddRune("Sharing", KitRunePickup, KitRuneDrop,1);
	g_PickupMsgTrie = CreateTrie();
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public OnMapStart()
{
	CacheModels();
	HookEvent("teamplay_round_active",Event_Round);
}

public OnMapEnd()
{
	ClearTrie(g_PickupMsgTrie);
	UnhookEvent("teamplay_round_active",Event_Round);
	UnhookEntities();
}

public Action:Event_Round(Handle:event, const String:strName[], bool:bDontBroadcast)
{
	HookEntities();
}

public OnPluginEnd()
{
	CloseHandle(g_PickupMsgTrie);
	//CloseHandle(g_KitRune);
}

public KitRunePickup(client, rune,ref)
{
	g_Effect[client][g_user] = 1;
	g_Effect[client][g_item] = 0;

	PrintToServer("Pickup KitRune player %d pickedup id %d ref %d",client,rune, ref);
}

public KitRuneDrop(client, rune,ref)
{
	g_Effect[client][g_user] = 0;
	g_Effect[client][g_item] = 0;
	PrintToServer("Drop KitRune player %d dropped id %d ref %d",client,rune,ref);
}

stock CreateTempKit(String:cname[])
{

	
}

public OnPackPickup(const String:output[], caller, activator, Float:delay)
{
	new String:item_class[32];
	new clients[MaxClients];
	new len;

	if(!g_Effect[activator][g_user] )
	{
		return;
	}

	g_Effect[activator][g_item] = caller;

	Entity_GetClassName(caller, item_class, sizeof(item_class));
	len = TeammatesNearPlayer(activator, clients);
	//LogMessage("Pickedup  player %d, caller %d;  %d players nearby",activator, caller,len);
	//PrintToConsole(activator,"Pickedup ent %d, %d players nearby", caller,len);
	decl String:sPlayer[28];
	GetClientName(activator,sPlayer,sizeof(sPlayer));

	if(len == 0)
		return;

	new Handle:datapack = CreateDataPack();

	new String:kit_key[60];
	Format(kit_key,sizeof(kit_key),"%s.%s",sPlayer,item_class);
	SetTrieValue(g_PickupMsgTrie,kit_key, datapack);
	WritePackCell(datapack,activator);
	WritePackString(datapack,kit_key);

	for(new i; i < len;++i)
	{
		new Float:vecOri_mate[3];
		Entity_GetAbsOrigin(clients[i], vecOri_mate);
		new ent = CreateEntityByName(item_class);
		if(!IsValidEntity(ent))
			continue;
		SetEntProp(ent, Prop_Send, "m_iTeamNum", 0, 4);
		Entity_SetOwner(ent, activator);
		DispatchSpawn(ent);
		HookSingleEntityOutput(ent,"OnPlayerTouch",OnTempPackPickup,true);
		TeleportEntity(ent, vecOri_mate, NULL_VECTOR, NULL_VECTOR);
		SetVariantString("OnUser1 !self:Kill::0.04:1");
		AcceptEntityInput(ent, "AddOutput");
		AcceptEntityInput(ent,"FireUser1");
		AcceptEntityInput(ent, "Enable");
	}

	CreateTimer(0.5,TimerPickupMsg,datapack,TIMER_DATA_HNDL_CLOSE|TIMER_FLAG_NO_MAPCHANGE);
	TE_PlayerToTeammates(activator, clients, len);
}


public Action:TimerPickupMsg(Handle:timer, Handle:data)
{
	ResetPack(data);
	new client = ReadPackCell(data);
	decl String:kit_key[60];

	ReadPackString(data,kit_key,sizeof(kit_key));
	RemoveFromTrie(g_PickupMsgTrie,kit_key);

	decl String:message[256];
	message[0] = '\0';

	new idx = FindCharInString(kit_key,'.');

	new bool:bFound = false;
	Format(message,sizeof(message),"You shared your %s with ",kit_key[idx+6]);
	
	while(IsPackReadable(data,1))
	{
		decl String:sTeammate[28];
		new teammate = ReadPackCell(data);
		if(!IsValidEntity(teammate))
			continue;
		if(!GetClientName(teammate,sTeammate,sizeof(sTeammate)))
			continue;
		if(bFound)
			StrCat(message,sizeof(message),",");
		bFound = true;
		StrCat(message,sizeof(message), sTeammate);
	}

	if(bFound)
	{
		Client_PrintKeyHintText(client,message);
	}
}

public OnTempPackPickup(const String:output[], caller, activator, Float:delay)
{
	new String:item_class[32];
	new String:player_name[28];
	Entity_GetClassName(caller, item_class, sizeof(item_class));

	new player = Entity_GetOwner(caller);
	if(!IsValidEntity(player) || !GetClientName(player,player_name,sizeof(player_name)))
		return;

	if(g_Effect[activator][g_user])
		g_Effect[activator][g_item] = caller;

	new Handle:datapack = INVALID_HANDLE;
	decl String:kit_key[60];
	Format(kit_key,sizeof(kit_key),"%s.%s",player_name,item_class);
	if(!GetTrieValue(g_PickupMsgTrie,kit_key, datapack))
		return;

	WritePackCell(datapack,activator);
	
	decl String:message[256];
	message[0] = '\0';
	Format(message,sizeof(message),"%s shared their %s with you!",
		player_name, item_class[5]);
	Client_PrintKeyHintText(activator,message);
}


/*
public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	new rune;
	new ent;
	if( !(rune = g_Effect[ client ] [ g_user ]) )
		return Plugin_Continue;
	if(! (buttons & IN_USE) )
		return Plugin_Continue;
	if(! (ent = g_Effect[client][g_item]))
		return Plugin_Continue;

	if(!IsValidEntity(ent) || 
		(!Entity_ClassNameMatches(ent,"item_healthkit",true) &&
		!Entity_ClassNameMatches(ent,"item_ammopack",true)))
			g_Effect[client][g_item] = 0;
	else
	{

		static i = 0;
		if( i >= sizeof(TestString))
			i = 0;
		AcceptEntityInput(ent,TestString[i]);
		decl String:netclass[32];
		GetEntityNetClass(ent,netclass,sizeof(netclass));
		PrintToConsole(client,"class %s",netclass);
		PrintToConsole(client,"tst %s",TestString[i]);
		i++;
	}

	return Plugin_Continue;
}


*/
CreateItemEntList(ent_list[], len)
{
	new j = 0;
	for(new i= GetMaxClients()+1; i < GetMaxEntities(); ++i)
	{
		if(!IsValidEntity(i))
			continue;
		if(!Entity_ClassNameMatches(i, "item_health", true) && !Entity_ClassNameMatches(i, "item_ammo",true))
			continue;
	
		ent_list[j++] = i;
		if(j >= len)
			break;
	}
	return j;
}

HookEntities()
{
	new ent_list[65];
	new len = CreateItemEntList(ent_list,sizeof(ent_list));
	for(new i; i < len; ++i)
	{
		//DbgPrint("ent ", ent_list[i]);
		UnhookSingleEntityOutput( ent_list[i], "OnPlayerTouch", OnPackPickup);
		HookSingleEntityOutput( ent_list[i], "OnPlayerTouch", OnPackPickup);
	}
	
}

UnhookEntities()
{
	new ent_list[65];
	new len = CreateItemEntList(ent_list,sizeof(ent_list));
	for(new i; i < len; ++i)
		UnhookSingleEntityOutput( ent_list[i], "OnPlayerTouch", OnPackPickup);
}


public RunePluginStart()
{
	PrintToServer("KitRunePluginStart\n");
}

public RunePluginStop()
{
	PrintToServer("KitRunePluginStop\n");
}

