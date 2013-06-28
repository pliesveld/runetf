#if defined _rune_spawn_included
#endinput
#endif
#define _rune_spawn_included

#include <smlib>
#include <sdktools>

#include <sdkhooks>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>
#include <runetf/defines_debug>

#define RUNE_MODEL "models/props/rune_01.mdl"

#define PLUGIN_NAME "Rune of Repulsion"
#define PLUGIN_DESCRIPTION "Reflects reflectable projectiles.  Damage taken increased."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}



enum RuneProp
{
	active,
	ent_trig,
};

new g_Effect[MAXPLAYERS][RuneProp];


new g_FilterProjEnt = INVALID_HANDLE;
new g_FilterProjEntRed = INVALID_HANDLE;
new g_FilterProjEntBlu = INVALID_HANDLE;
//new g_FilterProjTeamRed = INVALID_HANDLE;
//new g_FilterProjTeamBlu = INVALID_HANDLE;


public OnPluginStart()
{
	HookEvent("teamplay_restart_round",OnRoundRestart,EventHookMode_PostNoCopy);
	AddRune("Repulsion", DeflectRunePickup,DeflectRuneDrop,1);
}

public DeflectRunePickup(client,ref)
{	
	new t_ent = SpawnTriggerPush(client);
	Entity_SetParent(t_ent, client);
	g_Effect[client][ent_trig] = EntIndexToEntRef(t_ent);
	g_Effect[client][active] = 1;
	SDKHook(client,SDKHook_OnTakeDamage,Jedi_OnTakeDamage);
}

public DeflectRuneDrop(client,ref)
{
	g_Effect[client][active] = 0;

	new t_ent = EntRefToEntIndex(g_Effect[client][ent_trig]);
	if(t_ent != INVALID_ENT_REFERENCE)
		RemoveEdict(t_ent);

	SDKUnhook(client,SDKHook_OnTakeDamage,Jedi_OnTakeDamage);
	g_Effect[client][ent_trig] = 0;
}

public Action:Jedi_OnTakeDamage(victim, &attacker, &inflictor, &Float:damage, &damagetype, &weapon, Float:damageForce[3], Float:damagePosition[3])
{
	if(DMG_CRIT & damagetype)
		damage = FloatMul(damage,1.333);
	else if(damagetype & DMG_BURN)
		damage = FloatMul(damage,1.250);
	else if(damagetype & DMG_BLAST)
		damage = FloatMul(damage,1.150);

	new Float:d = GetVectorLength(damageForce);
//	new Float:d2 = NormalizeVector(damageForce,damageForce);

	d = FloatDiv(d,2.000);
	ScaleVector(damageForce,d);
	return Plugin_Changed;

	
}

public OnMapStart()
{
	PrecacheModel(RUNE_MODEL)

	RemovePriorFilters()
	g_FilterProjEnt = 0;
}


public OnRoundRestart(Handle:event,const String:name[],bool:bDontbroadcast)
{
	RemovePriorFilters()
	g_FilterProjEnt = 0;
}



CreateFilter(String:cname[])
{
	decl String:tname[128]="";
	Format(tname,sizeof(tname),"filter_%s",cname);


	new ent = -1;
	while((ent = FindEntityByClassname(ent,"filter_activator_class")) != INVALID_ENT_REFERENCE)
	{
		decl String:e_name[128]="";
		Entity_GetName(ent,e_name,sizeof(e_name));
		//LogError("filter name %s", e_name);

		if(!strcmp(tname,e_name))
			return ent;
		
	}

	ent = Entity_Create("filter_activator_class")
	DispatchKeyValue(ent,"filterclass",cname);
	DispatchKeyValue(ent,"targetname",tname);
	DispatchSpawn(ent);
	return ent;
}

stock CreateFilterProjectile()
{	
	new fRedOnly = -1, fBluOnly = -1;	


	fRedOnly = Entity_Create("filter_activator_tfteam")
	DispatchKeyValue(fRedOnly,"TeamNum","2");
	DispatchKeyValue(fRedOnly,"targetname","filter_proj_red");
	SetEntProp(fRedOnly,Prop_Data,"m_iTeamNum",2);
	DispatchSpawn(fRedOnly);

	//DebugFilter(fRedOnly);

	fBluOnly = Entity_Create("filter_activator_tfteam")
	DispatchKeyValue(fBluOnly,"TeamNum","3");
	DispatchKeyValue(fBluOnly,"targetname","filter_proj_blu");
	SetEntProp(fBluOnly,Prop_Data,"m_iTeamNum",3);
	DispatchSpawn(fBluOnly);

	//DebugFilter(fBluOnly);



	new ent_r = CreateFilter("tf_projectile_rocket");
	new ent_sr = CreateFilter("tf_projectile_sentryrocket");
	new ent_p = CreateFilter("tf_projectile_arrow");
	new ent_pr = CreateFilter("tf_projectile_flare");

	new ent;
	ent = Entity_Create("filter_multi");
	SetEntPropEnt(ent,Prop_Data,"m_hFilter",ent_p,0);
	SetEntPropEnt(ent,Prop_Data,"m_hFilter",ent_sr,1);
	SetEntPropEnt(ent,Prop_Data,"m_hFilter",ent_r,2);
	SetEntPropEnt(ent,Prop_Data,"m_hFilter",ent_pr,3);
	SetEntPropEnt(ent,Prop_Data,"m_hFilter",-1,4);
	DispatchKeyValue(ent,"filtertype","1");
	DispatchKeyValue(ent,"targetname","filter_proj_multi");
	
	DispatchSpawn(ent)
	//DebugFilter(ent);

	new t_ent = Entity_Create("filter_multi");
	SetEntPropEnt(t_ent,Prop_Data,"m_hFilter",ent,0);
	SetEntPropEnt(t_ent,Prop_Data,"m_hFilter",fRedOnly,1);
	DispatchKeyValue(t_ent,"filtertype","0");
	DispatchKeyValue(t_ent,"targetname","filter_proj_multi_red");
	
	DispatchSpawn(t_ent)
	g_FilterProjEntRed = t_ent

	t_ent = Entity_Create("filter_multi");
	SetEntPropEnt(t_ent,Prop_Data,"m_hFilter",ent,0);
	SetEntPropEnt(t_ent,Prop_Data,"m_hFilter",fBluOnly,1);
	DispatchKeyValue(t_ent,"filtertype","0");
	DispatchKeyValue(t_ent,"targetname","filter_proj_multi_blu");
	DispatchSpawn(t_ent)


	g_FilterProjEntBlu = t_ent

	g_FilterProjEnt = ent;

//	g_FilterProjTeamRed = fRedOnly;
//	g_FilterProjTeamBlu = fBluOnly;

	return ent;
}


stock SpawnTriggerPush(client)
{
	new Float:ori[3];
	GetEntPropVector(client, Prop_Send, "m_vecOrigin", ori);


	if(!g_FilterProjEnt)
		g_FilterProjEnt = CreateFilterProjectile();
	new hFilter = g_FilterProjEnt;

	new iTeam;
	iTeam = GetClientTeam(client)

	if(iTeam  == 2)
		hFilter = g_FilterProjEntBlu;
	else if(iTeam == 3)
		hFilter = g_FilterProjEntRed;

	new trigger_ent = Entity_Create("trigger_push");
	DispatchKeyValue(trigger_ent, "pushdir", "-90 0 0");
	DispatchKeyValue(trigger_ent, "speed", "3000");
	DispatchKeyValue(trigger_ent, "spawnflags", "64");
	SetEntPropEnt(trigger_ent,Prop_Data,"m_hFilter",hFilter);

	DispatchSpawn(trigger_ent); 
	ActivateEntity(trigger_ent);
	TeleportEntity(trigger_ent,ori,NULL_VECTOR,NULL_VECTOR);
	SetEntityModel(trigger_ent,RUNE_MODEL);

	new Float:minbounds[3] = {-100.0, -100.0, 0.0}; 
	new Float:maxbounds[3] = {100.0, 100.0, 200.0};
	SetEntPropVector(trigger_ent, Prop_Send, "m_vecMins", minbounds);
	SetEntPropVector(trigger_ent, Prop_Send, "m_vecMaxs", maxbounds);

	SetEntProp(trigger_ent, Prop_Send, "m_nSolidType", 2);


	new enteffects = GetEntProp(trigger_ent, Prop_Send, "m_fEffects"); 
	enteffects |= 32; 
	SetEntProp(trigger_ent, Prop_Send, "m_fEffects", enteffects);
	SetVariantString("OnUser1 !self:Kill::120:-1");
	AcceptEntityInput(trigger_ent, "AddOutput");
	AcceptEntityInput(trigger_ent, "FireUser1");


	decl buf[28]="";
	GetEntPropString(trigger_ent,Prop_Data,"m_iFilterName",buf,sizeof(buf));
//	new ent2 = GetEntPropEnt(trigger_ent,Prop_Data,"m_hFilter");

//	LogMessage("ent1 %s ent2 %d", buf, EntRefToEntIndex(ent2));


	//SetVariantString("!activator");
	//AcceptEntityInput(trigger_ent, "SetParent", rune_ent);I

	/*SetVariantString("OnStartTouch !self:Disable::0:-1");
	AcceptEntityInput(trigger_ent, "AddOutput");
	SetVariantString("OnStartTouch !self:Enable::2.5:-1");
	AcceptEntityInput(trigger_ent, "AddOutput");
*/
	return _:trigger_ent;
}

RemovePriorFilters()
{

	new entity = INVALID_ENT_REFERENCE;
	while((entity = FindEntityByClassname(entity,"filter_activator_class")) != INVALID_ENT_REFERENCE)
	{
		decl String:name[128];
		Entity_GetName(entity,name,sizeof(name));
		if(!strncmp(name,"filter_tf_projectile",20))
			AcceptEntityInput(entity,"kill");
	}

	while((entity = FindEntityByClassname(entity,"filter_multi")) != INVALID_ENT_REFERENCE)
	{
		decl String:name[128];
		Entity_GetName(entity,name,sizeof(name));
		if(!strncmp(name,"filter_proj_multi",17))
			AcceptEntityInput(entity,"kill");
	}

	while((entity = FindEntityByClassname(entity,"filter_activator_tfteam")) != INVALID_ENT_REFERENCE)
	{
		decl String:name[128];
		Entity_GetName(entity,name,sizeof(name));
		if(!strncmp(name,"filter_proj",11))
			AcceptEntityInput(entity,"kill");
	}
	while((entity = FindEntityByClassname(entity,"trigger_push")) != INVALID_ENT_REFERENCE)
		AcceptEntityInput(entity,"kill");



}

#if defined DEBUG
public Handle_TestFilter(const String:output[], caller, activator, Float:delay)
{
	decl String:name[32]="";
	decl String:buf[32]="";

	Entity_GetClassName(activator,name,sizeof(name));
	Entity_GetName(caller,buf,sizeof(buf));
	LogMessage("%s => %s.%s()", name, buf, output);
}

stock DebugFilter(filter)
{
	if(filter == -1)
	{
		LogError("Invalid filter index");
		return;
	}
	HookSingleEntityOutput(filter,"OnPass",Handle_TestFilter);
	HookSingleEntityOutput(filter,"OnFail",Handle_TestFilter);
}

#else
#define DebugFilter() 
#endif

public RunePluginStart()
{
}

public RunePluginStop()
{
}




