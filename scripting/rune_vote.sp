/*
	rune_vote.sp
	
	Players may vote to change runetf related cvars;

	- disable/enable runetf mod
	- increment/decrement rune spawn rates
	- enable/disable specific runes

*/


#include <colors>
#include <runetf/defines_debug>
#include <runetf/runetf>

#define REQUIRE_PLUGIN
#include <runetf/runes_stock>


#undef REQUIRE_PLUGIN


#define PLUGIN_NAME "Rune Voter"
#define PLUGIN_DESCRIPTION "Allows for votes to be taken to disable/enable runetf."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}




new Handle:rune_clients = INVALID_HANDLE;

new Handle:hCvarVoteToggleThreshold = INVALID_HANDLE;
new Handle:hCvarVoteAllowEnable = INVALID_HANDLE;
new Handle:hCvarVoteAllowDisable = INVALID_HANDLE;


//global forward OnRuneToggle is called when rune_enable cvar has been changed
new Handle:g_hFwdRuneToggle  = INVALID_HANDLE;

// how many players are required to trigger rune_enable toggle

//Flags
new rune_toggle_threshold;
new Float:fToggleThreshold = 0.60;
new bool:bAllowEnable  = false;
new bool:bAllowDisable = false;
new bool:bRunesEnabled = false;
new bool:bLateLoad = false;

#define DELAY_START 45
new nPlayers = 0;


public OnPluginStart()
{

	rune_clients = CreateArray();

	g_hFwdRuneToggle  = CreateGlobalForward("OnRuneToggle",  ExecType:ET_Event,ParamType:Param_Cell);
	new Handle:hRuneCvar = CreateConVar("rune_enable", "1", "Enables runetf mod.", FCVAR_NOTIFY, true, 0, true, 0);
	HookConVarChange(hRuneCvar, Handle_RuneCvarToggle);
	

	RegConsoleCmd("runes",Command_Runes_Vote);
	RegConsoleCmd("norunes",Command_NoRunes_Vote);
	RegisterVoteConVars();
	nPlayers = 0;

	if(bLateLoad)
	{
		new iClient;
		for(iClient = 1; iClient <= GetMaxClients();++iClient)
			if(IsValidEntity(iClient))
				++nPlayers;
	}
	
	return Plugin_Continue;
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	bLateLoad = late;
	RegPluginLibrary("rune_vote");
	return APLRes_Success;
}

RegisterVoteConVars()
{
	hCvarVoteToggleThreshold = CreateConVar("rune_vote_threshold",     "0.6", "Percentage of players required to toggle runetf.",FCVAR_NOTIFY, true, 0.0, true, 1.0);
	hCvarVoteAllowEnable     = CreateConVar("rune_vote_allow_enable",  "1.0", "Allow players to vote to enable runetf.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	hCvarVoteAllowDisable    = CreateConVar("rune_vote_allow_disable", "1.0", "Allow players to vote to disable runetf.", FCVAR_NOTIFY, true, 0.0, true, 1.0);

	fToggleThreshold = GetConVarFloat(hCvarVoteToggleThreshold);
	bAllowEnable     = GetConVarInt(hCvarVoteAllowEnable);
	bAllowDisable    = GetConVarInt(hCvarVoteAllowDisable);

	HookConVarChange(hCvarVoteToggleThreshold,  Handle_RuneVoteCvar);
	HookConVarChange(hCvarVoteAllowEnable,      Handle_RuneVoteCvar);
	HookConVarChange(hCvarVoteAllowDisable,     Handle_RuneVoteCvar);
}

public Handle_RuneVoteCvar(Handle:convar, const String:oldValue[], const String:newValue[])
{
	if(convar == hCvarVoteToggleThreshold)
	{
		fToggleThreshold = StringToFloat(newValue);
		CheckRuneToggleThreshold();
	} else if(convar == hCvarVoteAllowEnable) {
		bAllowEnable = StringToInt(newValue);
	} else if(convar == hCvarVoteAllowDisable) {
		bAllowDisable = StringToInt(newValue);
	} else {
		LogMessage("Warning; Handle_RuneVoteCvar: invalid convar.");
	}

	if(!bAllowDisable && bRunesEnabled
		|| !bRunesEnabled && !bAllowEnable)
		ClearArray(rune_clients);
}


public Action:Command_Runes_Vote(client,args)
{
	return Toggle_Rune_Vote(client,true);
}

public Action:Command_NoRunes_Vote(client,args)
{
	return Toggle_Rune_Vote(client,false);
}

Action:Toggle_Rune_Vote(client,bool:bToEnable)
{
	if(!client)
		return Plugin_Stop;

	new idxClient = FindValueInArray(rune_clients,client);

	if(bToEnable && !bAllowEnable ||
		!bToEnable && !bAllowDisable)
	{
		CPrintToChat(client,"{default}[{olive}runetf{default}]: You may not vote to %s runes at this time.", (bToEnable ? "enable" : "disable"));
		return Plugin_Handled;
	}


	if(bRunesEnabled == bToEnable)
	{
		if(idxClient == -1)
		{
			CPrintToChat(client,"{default}[{olive}runetf{default}]: Cannot vote to %s because it already is.", (bToEnable ? "enable" : "disable"));
			return Plugin_Handled;
		} else {
			CPrintToChat(client,"{default}[{olive}runetf{default}]: You have removed your vote to %s runetf.", (bToEnable ? "disable" : "enable"));
			RemoveFromArray(rune_clients,idxClient);
			return Plugin_Handled;
		}
	}

	if(idxClient != -1)
	{
		CPrintToChat(client, "{default}[{olive}runetf{default}]: You have already voted to %s runetf.", (bToEnable ? "enable" : "disable"));
		return Plugin_Handled;
	}

	PushArrayCell(rune_clients,client);
	CheckRuneToggleThreshold();
	return Plugin_Continue;
}


public OnPluginEnd()
{
	
	CloseHandle(g_hFwdRuneToggle);
	CloseHandle(rune_clients);
}


public OnConfigsExecuted()
{
//	CheckCvarDefaults();
	new Handle:hRuneEnabled = FindConVar("rune_enable");
	bRunesEnabled = (hRuneEnabled != INVALID_HANDLE && GetConVarBool(hRuneEnabled) == true);
	VerifyRunePlugins(!bRunesEnabled);
}



/*
public LogConvarState(const String:cvar[])
{
	new Handle:hLogCvar = FindConVar(cvar);
	if(hLogCvar == INVALID_HANDLE)
	{
		LogError("LogConvarState: invalid cvar %s",cvar);
		return;
	}

	new fLowValue, fHighValue, fCurrentValue;
	GetConVarBounds(hLogCvar, ConVarBounds:ConVarBound_Lower, fLowValue);
	GetConVarBounds(hLogCvar, ConVarBounds:ConVarBound_Upper, fHighValue);
	fCurrentValue = GetConVarFloat(hLogCvar);

	
#if defined DEBUG_RUNE_CVAR
	LogMessage("cvar %s: %4.2f (low %4.2f high %4.2f)", cvar, fCurrentValue, fLowValue, fHighValue);
#endif
	
}
*/

public OnMapEnd()
{
	ClearArray(rune_clients);
}

public OnClientPutInServer(client)
{
	++nPlayers;
}

public OnClientDisconnect_Post(client)
{
	new idx;
	while((idx = FindValueInArray(rune_clients, client)) != -1)
		RemoveFromArray(rune_clients,idx);

	--nPlayers;

	CheckRuneToggleThreshold();
}

public Handle_RuneCvarToggle(Handle:convar, const String:oldValue[], const String:newValue[])
{
	decl String:cvar_str[32]="";
	GetConVarName(convar, cvar_str,sizeof(cvar_str));

#if defined DEBUG_RUNE_CVAR
	LogMessage("rune_vote: convar %s CHANGED => %s", cvar_str, newValue);
#endif

	if(StrEqual(cvar_str,"rune_enable"))
	{
		new bool:bEnabled = StringToInt(newValue);
		if(bEnabled && StringToInt(oldValue))
			return;  // still true

		VerifyRunePlugins(!bEnabled); // pause or unpause rune plugins
		bRunesEnabled = bEnabled;

		Call_StartForward(g_hFwdRuneToggle); // call global forward to signal
		Call_PushCell(bEnabled);
		Call_Finish();
		ClearArray(rune_clients);
	} else if(StrEqual(cvar_str,"rune_vote_allow_enable")) {
		bAllowEnable = StringToInt(newValue);
	} else if(StrEqual(cvar_str,"rune_vote_allow_disable")) {
		bAllowDisable = StringToInt(newValue);
	} else if(StrEqual(cvar_str,"rune_vote_threshold")) {
		fToggleThreshold = StringToFloat(newValue);
		CheckRuneToggleThreshold();
		return;
	}

	ClearArray(rune_clients);
	return;
}


CheckRuneToggleThreshold()
{
	new nClients = GetArraySize(rune_clients);
	rune_toggle_threshold = RoundToCeil(nPlayers*fToggleThreshold);

	CPrintToChatAll("{default}[{olive}runetf{default}] %d/%d votes to toggle runetf.",nClients, rune_toggle_threshold);
	if( nClients && nClients  >= rune_toggle_threshold )
	{
		ServerCommand("rune_enable %s", (bRunesEnabled ? "0" : "1"));
	}
}



