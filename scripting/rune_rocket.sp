#include <tf2>
#include <sourcemod>
#include <sdktools>
#include <sdkhooks>

#include <smlib>
#include <tf2>
#include <tf2_stocks>
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>
#include <runetf/runes_stock/runegen_tempent>

#define SND_TICK "buttons/button17.wav"


#define SND_BOOM_SMALL  "ambient/explosions/explode_9.wav"
#define SND_BOOM_NORMAL "ambient/explosions/explode_8.wav"
#define SND_BOOM_LARGE	"misc/doomsday_missile_explosion.wav"

#undef PLUGIN_AUTHOR
#undef PLUGIN_URL

#define THIS_PLUGIN_AUTHOR "javalia + happs"
#define THIS_PLUGIN_URL "http://forums.alliedmods.net/showthread.php?t=162888"


#define PLUGIN_NAME "Rune of IronDome and DeadMansTrigger"
#define PLUGIN_DESCRIPTION "Based on sm_rocket, spawns rockets that track and knockback enemies.  Creates massive explosion on death."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = THIS_PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = THIS_PLUGIN_URL
}


// particles
// iron dome
//asplode_hoodoo_shockwave


		// on intercept
			//achieved
			//mini_fireworks



  // deadman:  	smoke_whitebillow, smoke_blackbillowflame
					//		smoke_pipeline_001, smoke_train

		// FireSmokeExplosion4
		// FireSmokeExplosion_trackb
	// fireSmoke_collumn

			//asplode_hoodoo_burning_debris
			//asplode_hoodoo_smoke
				//strong expl
				//	cinefx_goldrush_hugedustup
		//bombinomicon_burningdebris
//bomibomicon_ring
		//bomibomicon_ring
		//bombinomicon_flash
		//bombinomicon_flash_small

///cauldron_smoke_lit
					// ExplosionCore_Buildings
				// Explosion_CoreFlash

					//weak expl
						//Explosion_Dustup_2
  

enum RuneProp
{
  g_active,
  Float:g_timestamp,
	Handle:g_timer,
	bool:g_armed,
	g_counter,
	bool:g_intercepted,
};

new g_Effect[MAXPLAYERS][RuneProp];
new g_iTarget[2048];

#define RUNE_DOME_ID 1
#define RUNE_DEAD_ID 2

new Handle:g_hHudSyncMsg = INVALID_HANDLE;

public OnPluginStart()
{
	AddRune("IronDome", RocketRunePickup, RocketRuneDrop,RUNE_DOME_ID);
	AddRune("DeadMansTrigger", DeadManRunePickup, RocketRuneDrop,RUNE_DEAD_ID);

	g_hHudSyncMsg = CreateHudSynchronizer();

	//SetRuneFlags(g_IronDomeRune, RuneFlags:Rune_NoDrop);
	HookEvent("player_death",LaunchOnDeath,EventHookMode_Pre);
}

public OnMapStart()
{
	PrecacheSound(SND_TICK);
	PrecacheSound(SND_BOOM_SMALL);
	PrecacheSound(SND_BOOM_NORMAL);
	if(!PrecacheSound(SND_BOOM_LARGE))
		LogError("failed to cache sound");
}

public Action:LaunchOnDeath(Handle:event, const String:name[], bool:dontBroadcast)
{
	new userid = GetEventInt(event,"userid");
	new client = GetClientOfUserId(userid);
	new eff;
	if(( eff = g_Effect[client][g_active]) == RUNE_DOME_ID)
	{
		new victims[MAXPLAYERS];
		new cnt = EnemiesNearPlayer(client, victims, 3075.0);
		for(new i; i < cnt; ++i)
		{
			SpawnRocket(client,victims[i]);
		}
	} else if(eff == RUNE_DEAD_ID) {
		decl Float:fPos[3];
		GetClientAbsOrigin(client,fPos);
		CreateExplosion(client,fPos);
		new victims[MAXPLAYERS];

		new Float:fStun_radi = 275.0;	
		new Float:fStun_dur  = 3.5;
		new Float:fStun_eff = 0.10;

		if(g_Effect[client][g_armed])
		{
			if(g_Effect[client][g_intercepted])
			{
				fStun_radi /= 25.0;
				fStun_dur = 0.5;
			} else {
				fStun_radi += 175.0 * ( 3.0 - float(g_Effect[client][g_counter]));
				fStun_dur += (1.5 * ((2.0 - float(g_Effect[client][g_counter]))));
				fStun_eff += 0.10 * (4.0 - float(g_Effect[client][g_counter]));

				if(g_Effect[client][g_counter] <= 0)
				{
					fStun_dur += 2.0;
					fStun_eff += 0.20;
				}
			}

		}
		//LogMessage("Stun radi %f, dur %f, slow-down %f",
	//		fStun_radi, fStun_dur, fStun_eff);

		new cnt = EnemiesNearPlayer(client,victims,fStun_radi);
		for(new i;i < cnt;++i)
		{
			TF2_StunPlayer(victims[i],fStun_dur,fStun_eff,TF_STUNFLAGS_SMALLBONK,client);
		}
	}
	return Plugin_Continue;
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	return APLRes_Success;
}

public OnPluginEnd()
{
	CloseHandle(g_hHudSyncMsg);
	UnhookEvent("player_death",LaunchOnDeath,EventHookMode_Pre);
	//CloseHandle(g_IronDomeRune);
}


public RocketRunePickup(client, rune)
{
	g_Effect[ client ][ g_active ] = RUNE_DOME_ID;
	g_Effect[ client ][ g_timestamp ] = GetGameTime();
	g_Effect[ client ][ g_armed ] = false;
	g_Effect[ client ][ g_intercepted ] = false;
	g_Effect[ client ][ g_counter ] = 0;
	g_Effect[ client ][ g_timer ] = INVALID_HANDLE;
}

public DeadManRunePickup(client, rune)
{
	g_Effect[ client ][ g_active ] = RUNE_DEAD_ID;
	g_Effect[ client ][ g_timestamp ] = GetGameTime();
	g_Effect[ client ][ g_armed ] = false;
	g_Effect[ client ][ g_intercepted ] = false;
	g_Effect[ client ][ g_counter ] = 0;
	g_Effect[ client ][ g_timer ] = INVALID_HANDLE;

}

public RocketRuneDrop(client, rune)
{
	g_Effect[ client ][ g_active ] = 0;
	g_Effect[ client ][ g_armed ] = false;
	g_Effect[ client ][ g_counter ] = 0;
	g_Effect[ client ][ g_intercepted ] = false;
	if(g_Effect[client][g_timer] != INVALID_HANDLE)
	{
		KillTimer(g_Effect[client][g_timer]);
		g_Effect[ client ][ g_timer ] = INVALID_HANDLE;
	}

}


public SpawnRocket(client,target)
{
	new ent_rocket
	new Float:ori[3]
	new Float:ang[3]
	new Float:vec[3]

	GetClientEyeAngles(client, ang) 
	//GetClientEyePosition(client, ori)
	OriginNearPlayer(client,ori,100);

	ent_rocket = CreateEntityByName("tf_projectile_rocket")

	SetEntDataEnt2(ent_rocket, FindSendPropInfo("CTFProjectile_Rocket", "m_hOwnerEntity"), client, true)
	//SetEntData(ent_rocket, FindSendPropInfo("CTFProjectile_Rocket", "m_bCritical"), 0, 1, true)
	SetEntData(ent_rocket, FindSendPropInfo("CTFProjectile_Rocket", "m_iTeamNum"), GetClientTeam(client), true)
	SetEntDataVector(ent_rocket, FindSendPropInfo("CTFProjectile_Rocket", "m_angRotation"), ang, true) 
	SetEntProp(ent_rocket,Prop_Send,"m_CollisionGroup", COLLISION_GROUP_NONE);
	SetEntProp(ent_rocket,Prop_Send,"m_usSolidFlags",28);
	SetEntProp(ent_rocket,Prop_Send,"m_nSolidType",SOLID_VPHYSICS);
	DispatchSpawn(ent_rocket)

	GetAngleVectors(ang, vec, NULL_VECTOR, NULL_VECTOR)
	ScaleVector(vec, 733.3)
	TeleportEntity(ent_rocket, ori, NULL_VECTOR, vec)

	CreateParticle(ent_rocket, "sparks_metal", true);
	CreateParticle(ent_rocket, "sparks_metal_2", true);
	if(ent_rocket > 2048)
		return;
	g_iTarget[ent_rocket] = EntIndexToEntRef(target);
	SDKHook(ent_rocket, SDKHook_StartTouch, Event_StartTouch);
	SDKHook(ent_rocket, SDKHook_Think, RocketThinkHook);
}

public Event_StartTouch(entity, other)
{
	decl String:netclass[32];
	decl String:netclass2[32];

	GetEntityNetClass(entity, netclass2, sizeof(netclass2));
	GetEntityNetClass(other, netclass, sizeof(netclass));


/*
	if(!strcmp(netclass,netclass2))
	{
		return Plugin_Handled;
	} else {
  	SDKUnhook(entity, SDKHook_StartTouch, Event_StartTouch)
	}
	new client = GetEntDataEnt2(entity, FindSendPropInfo("CPhysicsProp", "m_hOwnerEntity"));
  */
	new Float:pos[3]
	GetEntDataVector(entity, FindSendPropInfo("CPhysicsProp", "m_vecOrigin"), pos);


	if(other > 0 && other <= GetMaxClients())
	{ 
		new Float:r_angle[3];
		decl Float:forwardVector[3];
		//GetEntDataVector(other, FindSendPropInfo("CPhysicsProp", "m_vecOrigin"), player_pos);
		GetEntPropVector(entity, Prop_Data, "m_angRotation", r_angle);
		//GetEntDataVector(entity, FindSendPropInfo("CTFProjectile_Rocket", "m_angRotation"), r_angle);

		GetAngleVectors(r_angle, forwardVector, NULL_VECTOR, NULL_VECTOR);
		NormalizeVector(forwardVector, forwardVector);
		ScaleVector(forwardVector, 375.0);
		new owner = GetEntPropEnt(entity, Prop_Data, "m_hOwnerEntity");
		CreateParticleAtEntity(other,"asplode_hoodoo_shockwave",0.5,false,0.01);
		SDKHooks_TakeDamage(other,0,owner,0.01);


		
		Entity_GetAbsVelocity(other,pos);
		for(new i;i < 3;++i)
			pos[i] += forwardVector[i];
		pos[2] += 375.0;
		Entity_SetAbsVelocity(other,pos);

	} else {

		if (!strcmp(netclass, "CObjectSentrygun"))
		{
	//		SetEntProp(other, Prop_Send, "m_bHasSapper", 1, 2);
			SetEntData( other, FindSendPropOffs("CObjectSentrygun","m_bDisabled") , 1 , 2 , true );
			CreateTimer(7.0, TimerEnableSentry, other,TIMER_FLAG_NO_MAPCHANGE);
		}
	}


  //CreateExplosion(client, pos)
	AcceptEntityInput(entity, "Explode", -1, -1, 0);
}


public RocketThinkHook(entity)
{
	new target = EntRefToEntIndex(g_iTarget[entity]);
	decl Float:rocketposition[3], Float:targetpos[3], Float:vecangle[3], Float:angle[3];
	GetEntPropVector(entity, Prop_Send, "m_vecOrigin", rocketposition);
	GetClientEyePosition(target, targetpos);

	MakeVectorFromPoints(rocketposition, targetpos, vecangle);
	NormalizeVector(vecangle, vecangle);
	GetVectorAngles(vecangle, angle);
	decl Float:speed[3];
	GetEntPropVector(entity, Prop_Data, "m_vecVelocity", speed);
	ScaleVector(vecangle, GetVectorLength(speed));
	TeleportEntity(entity, NULL_VECTOR, angle, vecangle);
}


public Action:TimerEnableSentry(Handle:timer, any:data)
{
	decl String:netclass[32];
	if(!IsValidEntity(data)) // probably want to use EntRefToEntIndex just in case sentry was destroyed and entity index was recycled.
		return Plugin_Continue;

	GetEntityNetClass(data, netclass, sizeof(netclass));
	if (!strcmp(netclass, "CObjectSentrygun"))
	{
		SetEntData( data , FindSendPropOffs("CObjectSentrygun","m_bDisabled") , 0 , 2 , true );
	}

	return Plugin_Continue;
}

/*
explosions/explode_4.wav // for each countdown
sound/weapons/explode1.wav // big
explosions/explode_8.wav // normal
explosions/explode_9.wav // intercepted
*/

public CreateExplosion(client, Float:pos[3])
{
	new ent = CreateEntityByName("env_explosion");
  
	new Float:exp_radi = 375.0;
	new Float:exp_dmg = 35.0;

	if(g_Effect[client][g_intercepted])
	{
		EmitSoundToAll(SND_BOOM_SMALL,client);
	} else if(g_Effect[client][g_armed]) {
		exp_radi *= 3.0;
		exp_dmg *= 2.65;
		EmitSoundToAll(SND_BOOM_LARGE,client);
	} else {
		EmitSoundToAll(SND_BOOM_NORMAL,client);
		exp_radi *= 1.5;
	}
	CreateParticleAtEntity(client,"FireSmokeExplosion_trackb",3.0,true);

	DispatchKeyValueFloat(ent,"Magnitude",exp_dmg);
	DispatchKeyValueFloat(ent,"Radius Override", exp_radi);
	DispatchKeyValue(ent,"spawnflags","64");

	SetEntPropEnt(ent, Prop_Data, "m_hOwnerEntity", client) //Set the owner of the explosion

	DispatchSpawn(ent)
  
	TeleportEntity(ent, pos, NULL_VECTOR, NULL_VECTOR)
	AcceptEntityInput(ent, "Explode", -1, -1, 0)
}

public Action:Timer_Explosion(Handle:timer, any:client)
{
	if(!IsValidEntity(client) || !IsPlayerAlive(client))
	{
		g_Effect[client][g_timer] = INVALID_HANDLE;
		return Plugin_Stop;
	}

	new cnt;

	if( ( cnt = (g_Effect[client][g_counter] -= 1)) <= 0)
	{
		if(!g_Effect[client][g_intercepted])
		{
			new victims[MAXPLAYERS];
			cnt = EnemiesNearPlayer(client,victims,875.0);
			for(new i;i < cnt;++i)
			{
				TF2_MakeBleed(victims[i],client,6.0);
			}
			CreateParticleAtEntity(client,"smoke_train",4.0,false,1.0);
		}

		ForcePlayerSuicide(client);
		return Plugin_Stop;
	} else {
		decl String:sTickTimer[48]="";
		for(new i = 5;i > cnt;--i)
		{
			decl String:sTickNum[6];
			Format(sTickNum,sizeof(sTickNum),"%d...",i-1);
			StrCat(sTickTimer,sizeof(sTickTimer),sTickNum);
		}

		SetHudTextParams(-1.0,-1.0,0.8,
				255, 255, 0, 175);
		ShowSyncHudText(client, g_hHudSyncMsg, sTickTimer);
		EmitSoundToAll(SND_TICK,client);
		CreateParticleAtEntity(client,"bomibomicon_ring");
	
	}
	return Plugin_Continue;
}

public Action:OnPlayerRunCmd(client, &buttons, &impulse, Float:vel[3], Float:angles[3], &weapon)
{
	if( !g_Effect[ client ] [ g_active ])
		return Plugin_Continue;
	if(! (buttons & IN_USE) )
		return Plugin_Continue;

	if( g_Effect[ client ] [ g_active ] == RUNE_DOME_ID )
	{
		if( GetGameTime() < g_Effect[client][g_timestamp] )
			return Plugin_Continue;

		g_Effect[client][g_timestamp] = GetGameTime() + 2.7;
	 

		new victims[MAXPLAYERS];
		new cnt = EnemiesNearPlayer(client, victims, 1025.0);
		for(new i; i < cnt; ++i)
		{
			if(TF2_IsPlayerInCondition(victims[i], TFCond_Cloaked) 
					&& !TF2_IsPlayerInCondition(victims[i], TFCond_CloakFlicker))
				continue;
			if(!TF2_IsPlayerInCondition(victims[i],TFCond_Disguising)
					&& TF2_IsPlayerInCondition(victims[i], TFCond_Disguised))
				continue;
			SpawnRocket(client,victims[i]);
		}

		if(cnt)
			SDKHooks_TakeDamage(client,0,client,30.0);

	} else if( g_Effect[client][g_active] == RUNE_DEAD_ID) {
		if(g_Effect[client][g_armed])
			return Plugin_Continue;
		
		//EmitSoundToAll(SND_TICK,client);

		g_Effect[client][g_armed] = true;
		g_Effect[client][g_counter] = 5;
		g_Effect[client][g_timer] = CreateTimer(1.0,Timer_Explosion,client,TIMER_FLAG_NO_MAPCHANGE | TIMER_REPEAT);
		TriggerTimer(g_Effect[client][g_timer],true);
	}
	return Plugin_Continue;
}

public RunePluginStart()
{
}

public RunePluginStop()
{
}
