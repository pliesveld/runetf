#if defined _rune_spawn_included
 #endinput
#endif
#define _rune_spawn_included


#include <sdktools>

#define RUNE_MODEL "models/props/rune_01.mdl"

stock AddRunesToDownloadTable()
{
  AddFileToDownloadsTable("materials/models/quake/rune_01.vmt");
  AddFileToDownloadsTable("materials/models/quake/rune_01.vtf");
  AddFileToDownloadsTable("materials/models/quake/rune_01n.vtf");
  AddFileToDownloadsTable("models/props/rune_01.dx80.vtx");
  AddFileToDownloadsTable("models/props/rune_01.dx90.vtx");
  AddFileToDownloadsTable("models/props/rune_01.mdl");
  AddFileToDownloadsTable("models/props/rune_01.phy");
  AddFileToDownloadsTable("models/props/rune_01.sw.vtx");
  AddFileToDownloadsTable("models/props/rune_01.vvd");
}




stock CreatePropRune()
{
	new ent = CreateEntityByName("prop_physics_multiplayer");

	SetEntityModel(ent, RUNE_MODEL)
	SetEntityMoveType(ent, MOVETYPE_VPHYSICS);

	SetEntProp(ent, Prop_Send, "m_CollisionGroup", COLLISION_GROUP_NONE);
	SetEntProp(ent, Prop_Send, "m_usSolidFlags", 6);
	SetEntProp(ent, Prop_Send, "m_nSolidType", SOLID_VPHYSICS);
	return ent;
}

stock GetEntityAbsOrigin(entity,Float:origin[3]) 
{ 
    decl String:class[32], offs 

    GetEntPropVector(entity,Prop_Send,"m_vecOrigin",origin) 

    if(!GetEntityNetClass(entity,class,sizeof class) || (offs = FindSendPropInfo(class,"m_vecMins")) == -1) { 
        return 
    } 

    decl Float:mins[3], Float:maxs[3] 

    GetEntDataVector(entity,offs,mins) 
    GetEntPropVector(entity,Prop_Send,"m_vecMaxs",maxs) 

    origin[0] += (mins[0] + maxs[0]) * 0.5 
    origin[1] += (mins[1] + maxs[1]) * 0.5 
    origin[2] += (mins[2] + maxs[2]) * 0.5 
}  

stock SpawnTrigger(rune_ent)
{
	new Float:ori[3];
	if(!IsValidEntity(rune_ent))
	{
		return -1;
	}

	GetEntityAbsOrigin(rune_ent, ori);
	decl String:rune_name[64];
	new trigger_ent = Entity_Create("trigger_multiple");
	DispatchKeyValue(trigger_ent, "spawnflags", "1");
	// DispatchKeyValue(trigger_ent, "wait", "2");
	DispatchKeyValue(trigger_ent, "StartDisabled","1");
	SetEntityModel(trigger_ent, RUNE_MODEL)

	new Float:minbounds[3] = {-25.0, -25.0, 0.0}; 
	new Float:maxbounds[3] = {25.0, 25.0, 70.0};

	/*
	new Float:minbounds[3] = {-17.5, -17.5, -35.0}; 
	new Float:maxbounds[3] = {17.5, 17.5, 35.0};
	*/
	Entity_GetTargetName(rune_ent,rune_name,sizeof(rune_name));
	Entity_SetTargetName(trigger_ent,"trig_%s", rune_name);
	Entity_GetName(rune_ent,rune_name,sizeof(rune_name));
	Entity_SetName(trigger_ent,"trig_%s", rune_name);

	DispatchSpawn(trigger_ent); 
	//  ActivateEntity(trigger_ent);

	Entity_SetMinSize(trigger_ent,minbounds);
	Entity_SetMaxSize(trigger_ent,maxbounds);
	//Entity_SetSolidType(trigger_ent,SOLID_BBOX);
	SetEntProp(trigger_ent, Prop_Send, "m_nSolidType", 2);

	Entity_SetCollisionGroup(trigger_ent, COLLISION_GROUP_NONE);

	TeleportEntity(trigger_ent,ori,NULL_VECTOR,NULL_VECTOR);

	// 	SetEntPropVector(trigger_ent, Prop_Send, "m_vecOrigin", ori); 
	new enteffects = GetEntProp(trigger_ent, Prop_Send, "m_fEffects"); 
	enteffects |= 32; 
	SetEntProp(trigger_ent, Prop_Send, "m_fEffects", enteffects);

	SetVariantString("!activator");
	AcceptEntityInput(trigger_ent, "SetParent", rune_ent);

	SetVariantString("OnStartTouch !self:Disable::0:-1");
	AcceptEntityInput(trigger_ent, "AddOutput");
	SetVariantString("OnStartTouch !self:Enable::2.5:-1");
	AcceptEntityInput(trigger_ent, "AddOutput");

	return _:trigger_ent;
}


stock SpawnPropRune(Float:ori[3], Float:ang[3] = NULL_VECTOR, Float:vel[3] = NULL_VECTOR, const String:name[] = "")
{
	static rune_counter = 0;
	new ent = CreatePropRune();
	if(name[0] == '\0')
		Entity_SetName(ent,"t_rune%d", ++rune_counter);
	else
		Entity_SetName(ent,"t_rune%s",name);
	DispatchSpawn(ent)
	ActivateEntity(ent);

	TeleportEntity(ent, ori, ang, vel)
	return _:ent;
}

stock SpawnRune(&rune_ent,&trigger_ent, EntityOutput:onPickup, Float:loc[3], Float:ang[3] = NULL_VECTOR, Float:vel[3] = NULL_VECTOR, Float:delay = 50.0, bool:enable = true, const String:name[] = "")
{
	rune_ent = SpawnPropRune(loc,ang,vel, name);
	trigger_ent = SpawnTrigger(rune_ent);

	decl String:variantString[60];
	Format(variantString,sizeof(variantString), "OnUser1 !self:KillHierarchy::%f:-1", delay);

	SetVariantString(variantString);
	AcceptEntityInput(rune_ent, "AddOutput");

	AcceptEntityInput(rune_ent, "FireUser1");
	

	HookSingleEntityOutput(trigger_ent, "OnStartTouch",onPickup);
	if(enable)
		AcceptEntityInput(trigger_ent, "Enable");


#if defined DEBUG_RUNE_SPAWN
	new Float:ori[3];
	GetEntityAbsOrigin(rune_ent,ori);
	LogMessage("model abs ori %f %f %f",ori[0], ori[1], ori[2]);
	GetEntPropVector(rune_ent,Prop_Send,"m_vecOrigin",ori);
	LogMessage("model --- ori %f %f %f",ori[0], ori[1], ori[2]);

	GetEntityAbsOrigin(trigger_ent,ori);
	LogMessage("rune  abs ori %f %f %f",ori[0], ori[1], ori[2]);
	GetEntPropVector(trigger_ent,Prop_Send,"m_vecOrigin",ori);
	LogMessage("rune --- ori %f %f %f",ori[0], ori[1], ori[2]);

#endif
}



bool:TestCanPlayerPickup(client)
{
	new RuneId = GetPlayerRuneId(client);
	//PrintToServer("TestCanPickup:: client %d has %d", client, RuneId);
	if(RuneId)
	{
		new String:buffer[16];
		GetPlayerRuneName(client,buffer,sizeof(buffer));
		Client_PrintHintText(client, "You already have rune of %s", buffer);
		return false;
	}
	return true;
}

public RandomRuneEntityTouchTest(const String:output[], caller, activator, Float:delay)
{

	decl String:playerName[16]="";

	new rune_ent = Entity_GetParent(caller);
	GetClientName(activator,playerName,sizeof(playerName));

	if(TestCanPlayerPickup(activator))
	{
		PlayerPickupRandomRune(activator);
		new String:rune_name[24];
		if(!GetPlayerRuneName(activator,rune_name,sizeof(rune_name)))
		{
			PrintToServer("ERROR SingleTest: invalid rune after Pickup!");
		}
		Client_PrintHintText(activator, "You found rune of %s!", rune_name);
		LogMessage("Player %s pickedup %s rune", playerName,rune_name);

		AcceptEntityInput(rune_ent,"KillHierarchy");
	}
}


public NamedRuneEntityTouchTest(const String:output[], caller, activator, Float:delay)
{
	if(!TestCanPlayerPickup(activator))
		return -1;

	new rune_ent = Entity_GetParent(caller);
	if(!IsValidEntity(rune_ent))
		return -1;

	new String:buffer[24];
	Entity_GetName(rune_ent,buffer,sizeof(buffer));

	if(strncmp(buffer,"t_rune",6))
		return -1;

	new String:rune_name[24];
	strcopy(rune_name, sizeof(rune_name), buffer[6]);

	Client_PrintHintText(activator, "Pickup rune of %s", rune_name);
	new String:s_dbg[48];
	new id;
	if((id = RuneIdByName(rune_name)) == 0)
	{
		Format(s_dbg,sizeof(s_dbg),"Named: Invalid rune name %s.", rune_name);
		PrintToConsole(activator,s_dbg);
		PrintToServer(s_dbg);
		return -1;
	}
	
//	LogMessage("Player %d Pickup rune of %s",activator, rune_name);
//	PrintToConsole(activator,"Pre-Pickup rune of %s(%d)", rune_name,id);
	decl String:playerName[16]="";
	GetClientName(activator,playerName,sizeof(playerName));

	new ret;
	if( (ret = PlayerPickupRune(activator, rune_name)) == 0)
	{
		Format(s_dbg,sizeof(s_dbg), "Player %s already has a rune.",rune_name);
	} else if(ret == -1) {
		Format(s_dbg,sizeof(s_dbg), "Player %s pickuped invalid rune %s(%d).", playerName, rune_name,id);
	} else {
		Format(s_dbg,sizeof(s_dbg), "Player %s pickuped up %s rune.",playerName,rune_name);
		LogMessage(s_dbg);
	}

	AcceptEntityInput(rune_ent,"KillHierarchy");
	PrintToConsole(activator,s_dbg);

	return ret;
}

stock SpawnNamedRune(const String:rune[], Float:loc[3], Float:ang[3] = NULL_VECTOR, Float:vel[3] = NULL_VECTOR, Float:delay = 90.0, bool:enable = true)
{
	new String:s_name[24];
	if(rune[0] == '\0' || RuneIdByName(rune) == 0)
		return ThrowError("SpawnNamedRune: invalid name %s", rune);

	new rune_ent, trig_ent;
	Format(s_name,sizeof(s_name),"t_rune%s",rune);
	SpawnRune(rune_ent, trig_ent, NamedRuneEntityTouchTest, loc, ang, vel, delay, enable, rune);
	Entity_SetName(trig_ent, s_name);
	return trig_ent;
}

stock SpawnRandomRune(Float:loc[3], Float:ang[3] = NULL_VECTOR, Float:vel[3] = NULL_VECTOR, Float:delay = 90.0, bool:enable = true) 
{

	new rune_ent, trig_ent;
	SpawnRune(rune_ent, trig_ent, RandomRuneEntityTouchTest, loc, ang, vel, delay, enable);
}

stock bool:isEntaRune(entity)
{
	decl String:tname[16];
	Entity_GetName(entity,tname,sizeof(tname));
	return (strncmp(tname,"t_rune",6) == 0);
}


stock KillAllRunes()
{
	new maxEntities = GetMaxEntities();
	for(new entity=GetMaxClients();entity < maxEntities;++entity)
	{
		if(!IsValidEntity(entity))
			continue;
		if(!isEntaRune(entity))
			continue;
		AcceptEntityInput(entity,"KillHierarchy");
	}
}

stock KillAllPlayerRunes()
{
	new i;
	for(i = 1;i <= GetMaxClients();++i)
	{
		if(!IsValidEntity(i))
			continue;
		if(!IsPlayerAlive(i))
			continue;
		if(!GetPlayerRuneId(i))
			continue;
		PlayerDropRune(i)
	}
}

