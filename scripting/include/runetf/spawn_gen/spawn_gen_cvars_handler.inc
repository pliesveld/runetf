#if !defined __spawn_gen_cvars_handler__
#define __spawn_gen_cvars_handler__

//new Handle:g_hRuneCvars[sizeof(sRuneCvar)];

stock RegisterRuneSpawnCvars()
{
	InitRuneConvars();
	LoadConVarValues();
	RegisterConVarChange(RuneCvarChanged);
}

stock UnregisterCvars()
{
	UnregisterConCvarChange(RuneCvarChanged);
}

stock ResetConVars()
{
	ResetConVar(hCvarInterval);
	ResetConVar(hCvarLifeTime);
	ResetConVar(hCvarDropTime);
	ResetConVar(hCvarClearStart);
	ResetConVar(hCvarClearEnd);
}



public RuneCvarChanged(Handle:cvar, const String:oldValue[], const String:newValue[])
{
	decl String:s_cvar[32]="";
	GetConVarName(cvar,s_cvar,sizeof(s_cvar));

	if(StrEqual(s_cvar,"rune_spawn_interval"))
	{
		g_RuneSpawn[fSpawnInterval] = StringToFloat(newValue);
		if(g_RuneSpawn[bRuneEnable] && g_SpawnTimer != INVALID_HANDLE)
			RestartSpawnTimer(g_RuneSpawn[fSpawnInterval]);
	} else if(StrEqual(s_cvar,"rune_enable")) {
		if((g_RuneSpawn[bRuneEnable] = bool:StringToInt(newValue)) == true)
		{
			RestartSpawnTimer(g_RuneSpawn[fSpawnInterval])
		} else {
			RestartSpawnTimer(0.0);
			KillAllRunes();
			KillAllPlayerRunes();
		}	
	} else if (StrEqual(s_cvar,"rune_spawn_lifetime")) {
		g_RuneSpawn[fRuneLifeTime] = StringToFloat(newValue);
	} else if (StrEqual(s_cvar,"rune_spawn_droptime")) {
		g_RuneSpawn[fRuneNamedLifeTime] = StringToFloat(newValue);
	} else if (StrEqual(s_cvar,"rune_round_start_clear")) {
		g_RuneSpawn[bRoundStartClear] = bool:StringToInt(newValue);
	} else if (StrEqual(s_cvar,"rune_round_end_clear")) {
		g_RuneSpawn[bRoundEndClear] = bool:StringToInt(newValue);
	} else {
		LogMessage("warning; Unhandled cvar change: %s", s_cvar);
	}

}
	

#endif


#endif

