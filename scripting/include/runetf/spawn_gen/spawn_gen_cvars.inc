#if !defined __spawn_gen_cvars__
#define __spawn_gen_cvars__
new String:sRuneCvar[][] =
{
	"rune_spawn_interval",
	"rune_spawn_lifetime",
	"rune_spawn_droptime",
	"rune_round_start_clear",
	"rune_round_end_clear",
	"rune_enable"
};


#define INTERVAL "17.0"
#define LIFETIME "45.0"
#define DROPTIME "65.0"
#define CLEARSTART "1"
#define CLEAREND   "1"
#define RUNEENABLE "1"


new Handle:hCvarInterval  = INVALID_HANDLE;
new Handle:hCvarLifeTime  = INVALID_HANDLE;
new Handle:hCvarDropTime  = INVALID_HANDLE;
new Handle:hCvarClearStart= INVALID_HANDLE;
new Handle:hCvarClearEnd  = INVALID_HANDLE;
new Handle:hCvarRuneEnable= INVALID_HANDLE;


stock InitRuneConvars()
{
	hCvarInterval = CreateConVar("rune_spawn_interval",INTERVAL, 
	"Interval between rune spawns.",FCVAR_NOTIFY,true, 2.0, true, 600.0);

	hCvarLifeTime= CreateConVar("rune_spawn_lifetime",LIFETIME, 
	"Life time of runes spawned before disappearing.",FCVAR_NOTIFY,true, 2.0, true, 600.0);
	
	hCvarDropTime= CreateConVar("rune_spawn_droptime",DROPTIME, 
	"Life time of runes dropped by players before disappearing.",FCVAR_NOTIFY,true, 2.0, true, 600.0);

	hCvarClearStart= CreateConVar("rune_round_start_clear", CLEARSTART,
	"Clears Players' when a new round begins.", FCVAR_NOTIFY, true, 0.0, true, 1.0);

	hCvarClearEnd= CreateConVar("rune_round_end_clear", CLEARSTART,
	"Clears Players' when a mini round ends.", FCVAR_NOTIFY, true, 0.0, true, 1.0);

	hCvarRuneEnable= CreateConVar("rune_enable", RUNEENABLE,
	"Enables runetf.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
		
}


stock Handle:RegisterConVarChange(ConVarChanged:cvar_cb)
{
	HookConVarChange(hCvarInterval, cvar_cb);
	HookConVarChange(hCvarLifeTime, cvar_cb);
	HookConVarChange(hCvarDropTime, cvar_cb);
	HookConVarChange(hCvarClearStart, cvar_cb);
	HookConVarChange(hCvarClearEnd, cvar_cb);
	HookConVarChange(hCvarRuneEnable, cvar_cb);
}

stock UnregisterConCvarChange(ConVarChanged:cvar_cb)
{
	UnhookConVarChange(hCvarInterval, cvar_cb);
	UnhookConVarChange(hCvarLifeTime, cvar_cb);
	UnhookConVarChange(hCvarDropTime, cvar_cb);
	UnhookConVarChange(hCvarClearStart, cvar_cb);
	UnhookConVarChange(hCvarClearEnd, cvar_cb);
	UnhookConVarChange(hCvarRuneEnable, cvar_cb);
}




stock LoadConVarValues()
{
	g_RuneSpawn[fSpawnInterval] = GetConVarFloat(hCvarInterval);
	g_RuneSpawn[fRuneLifeTime] = GetConVarFloat(hCvarLifeTime);
	g_RuneSpawn[fRuneNamedLifeTime] = GetConVarFloat(hCvarDropTime);
	g_RuneSpawn[bRoundStartClear] = GetConVarInt(hCvarClearStart);
	g_RuneSpawn[bRoundEndClear] = GetConVarInt(hCvarClearEnd);
	g_RuneSpawn[bRuneEnable] = GetConVarInt(hCvarRuneEnable);
}

#endif
