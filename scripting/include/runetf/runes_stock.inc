
#if defined __runes_stock__
 #endinput
#endif
#define __runes_stock__
/*
		Contains data common to all rune plugins
		
		public functions required:
		
			RunePluginStart
			RunePluginStop
	
			ActivateRune( client  )
			TerminateRune( client )
			
*/
#include <functions>
#include "runes_stock/eff_particles"
#include "runes_stock/rune_player_stock"

#define IsClientValidConnected(%1) (1<=%1<=MaxClients && IsConnected[%1])
#define IsClientValidAlive(%1) (1<=%1<=MaxClients && IsAlive[%1])

stock VerifyRunePlugins(bool:bDisable)
{
	new Handle:hIter;
	new Handle:hPlug;

	//LogMessage("VerifyRunePlugins == %d",bDisable);

	hIter = GetPluginIterator();
	do {
		decl String:s_plug[32]="";
		hPlug = ReadPlugin(hIter);

		if(hPlug == GetMyHandle())
			continue;

		GetPluginFilename(hPlug,s_plug,sizeof(s_plug));

		new bool:bRunning = (GetPluginStatus(hPlug) == PluginStatus:Plugin_Running);
		new Function:fPlugFunc = INVALID_FUNCTION;

		if(bDisable)
		{
			if((fPlugFunc = GetFunctionByName(hPlug,"RunePluginStop")) == INVALID_FUNCTION)
				continue;

			if(bRunning)
			{
				new plug_res;
				Call_StartFunction(hPlug, fPlugFunc);
				Call_Finish(plug_res);
				new ext_res;
				ext_res = RunesPause(hPlug);
				
#if defined DEBUG_RUNE_PAUSE
				LogMessage("Stopped %s: ret %d", s_plug,plug_res);
				LogMessage("rune pause %d", ext_res);
#endif
				continue;
			}

#if defined DEBUG_RUNE_PAUSE
			LogMessage("Checking %s; already stopped.", s_plug);
#endif
			
		} else {
			if((fPlugFunc = GetFunctionByName(hPlug,"RunePluginStart")) == INVALID_FUNCTION)
				continue;

			if(!bRunning)
			{
				new ext_res = RunesUnpause(hPlug);
				new plug_res;
				Call_StartFunction(hPlug, fPlugFunc);
				Call_Finish(plug_res);
#if defined DEBUG_RUNE_PAUSE
				LogMessage("Checking %s; restarting.", s_plug);
				LogMessage("rune unpause %d", ext_res);
#endif
			}
#if defined DEBUG_RUNE_PAUSE
			else
				LogMessage("Checking %s; already running.", s_plug);
#endif
						
		}

			

		
	} while(MorePlugins(hIter));
	

	CloseHandle(hPlug);
}



stock bool:VerifyRunesEnabled()
{
	new Handle:hEnabled = FindConVar("rune_enable");
	
	if(hEnabled == INVALID_HANDLE)
	{
		LogMessage("no cvar rune_enable found!");
		return false;
	}

	if(GetConVarBool(hEnabled) == false)
	{
		LogMessage("rune_enable == 0; not verify Failed.");
		return false;
	}

	return true;
}

enum RuneFlags
{
	Rune_Any = 0,
	Rune_NoDrop = (1<<0),
	Rune_Touch  = (1<<1),
	Rune_Death  = (1<<2),
	Rune_Use		= (1<<3),
	Rune_Drop		= (1<<4),
  Rune_HurtShield = (1<<5),
  Rune_HurtDmg    = (1<<6),
  Rune_Kill       = (1<<7),
  Rune_KillAssist = (1<<8)
};

native GetRuneFlags(Handle:hndl);
native SetRuneFlags(Handle:hndl, RuneFlags:flags);

public SharedPlugin:__p1_spawn_test =
{
	name = "spawn_test",
	file = "spawn_test.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};



