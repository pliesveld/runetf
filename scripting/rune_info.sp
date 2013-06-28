/*
		rune_info.txt - load rune descriptions.

		Loads descriptions from (SorceMod)/gamedata/runetf.txt
		
		Maintains mapping of rune names to rune descriptions.

		Provides !inspect and !inspect_rune player commands to retrieve the type of rune on the ground or held by a player.

		@natives
			public RTF_RuneInfo( client, string:RuneName )
			-	displays rune description to client.  Used by rune_chooser.sp.
*/

#include <smlib>
#include <sdktools_functions>

#include <colors>
#include <runetf/runetf>


#define PLUGIN_NAME "Rune Info"
#define PLUGIN_DESCRIPTION "Adds commands to inspect a rune on the ground or held by player."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}


new Handle:g_RuneTrie = INVALID_HANDLE;
#define TRIE_DESC_LEN 256

new Handle:g_DescHudSync = INVALID_HANDLE;

public OnPluginStart()
{
	g_DescHudSync = CreateHudSynchronizer();
	RegConsoleCmd("inspect", OnPlayerInspect, "Inspects rune");
	RegConsoleCmd("inspect_rune", OnPlayerInspect, "Inspects rune");
}

public OnPluginEnd()
{
	CloseHandle(g_DescHudSync);
}

public Action:OnPlayerInspect(client,args)
{
	if(!client || !IsValidEntity(client) || !IsPlayerAlive(client))
		return Plugin_Continue;
	
	new ent = GetClientAimTarget(client, false);

	if(ent == -1)
		return Plugin_Continue;
	
	if(!IsValidEntity(ent))
		return Plugin_Continue;

	if(ent > 0 && ent <= GetMaxClients())
	{
		decl String:sRuneName[24]="";
		decl String:sPlayerName[24]="";
		decl String:sInspect[64]="";
		GetClientName(ent,sPlayerName,sizeof(sPlayerName));
		if(GetPlayerRuneName(ent,sRuneName,sizeof(sRuneName)))
		{
			Format(sInspect,sizeof(sInspect),"%s has Rune of %s",
				sPlayerName, sRuneName);
		} else {
			Format(sInspect,sizeof(sInspect),"%s has no rune.", sPlayerName);
		}
		
		DisplayRuneMsg(client, sInspect);
		return Plugin_Continue;
	}


	decl String:sName[32]="";
	Entity_GetName(ent,sName,sizeof(sName));
	if( strncmp(sName,"t_rune",6) == 0)
	{
		decl String:sInspect[64] = {"Rune of "};
		if(!IsCharNumeric(sName[6]))
		{
			StrCat(sInspect,sizeof(sInspect), sName[6]);
		} else {
			strcopy(sInspect,sizeof(sInspect),"Random Rune");
		}
		DisplayRuneMsg(client, sInspect);
	}
	
	return Plugin_Continue;
}

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
	RegPluginLibrary("rune_info");
	CreateNative("RuneTF_display_rune_info", RTF_RuneInfo);
}


public OnConfigsExecuted()
{
	ReadRuneFile();
	CheckDescriptions();
}

stock SMCError:ReadRuneFile(client = 0)
{
	new String:RuneDescFile[64];
	BuildPath(Path_SM, RuneDescFile, sizeof(RuneDescFile), "gamedata/runetf.txt");
	if(g_RuneTrie != INVALID_HANDLE)
		ClearTrie(g_RuneTrie);

	g_RuneTrie = CreateTrie();
	new Handle:hSMC = SMC_CreateParser();
	SMC_SetReaders(hSMC, SMCNewSection, SMCReadKeyValues, SMCEndSection);
	ProcessRuneFile(hSMC, RuneDescFile);
}


stock SMCError:ProcessRuneFile(&Handle:hSMC, const String:aRuneFile[])
{
  new iLine;
  new SMCError:ReturnedError = SMC_ParseFile(hSMC, aRuneFile, iLine);
  if (ReturnedError != SMCError_Okay)
  {
    decl String:sError[256];
    SMC_GetErrorString(ReturnedError, sError, sizeof(sError));
    if (iLine > 0)
    {
      LogError("[PP] Could not parse file (Line: %d, File \"%s\"): %s.", iLine, aRuneFile, sError);
      CloseHandle(hSMC);
      return ReturnedError;
    }

    LogError("[PP] Parser encountered error (File: \"%s\"): %s.", aRuneFile, sError);
  }
  CloseHandle(hSMC);
  return SMCError_Okay;
}


new String:g_temp_name[32];

public SMCResult:SMCNewSection(Handle:smc, const String:name[], bool:opt_quotes)
{
	//LogMessage("RuneDescFile: processing %s", name);
	strcopy(g_temp_name,sizeof(g_temp_name), name);
	return SMCParse_Continue;
}


public SMCResult:SMCReadKeyValues(Handle:smc, const String:key[], const String:value[], bool:key_quotes, bool:value_quotes)
{
	if(strcmp(key,"description"))
	{
		if(!SetTrieString(g_RuneTrie, g_temp_name, value))
		{
			return SMCParse_HaltFail;
		}
	}
	return SMCParse_Continue;
}


public SMCResult:SMCEndSection(Handle:smc)
{
	return SMCParse_Continue;
}

stock CheckDescriptions()
{
	new runeIds[64];
	new s = GetRuneIds(runeIds,sizeof(runeIds));

	new String:buffer[32];
	
	for(new i;i < s; ++i)
	{
		if(runeIds[i] == 0)
			continue; //extension sanity check
		if(!RuneNameById(runeIds[i], buffer, sizeof(buffer)))
		{
			LogMessage("Invalid Runeid %d", runeIds[i]);
			continue;
		}
		decl String:desc[TRIE_DESC_LEN];

		if(!GetTrieString(g_RuneTrie, buffer, desc, TRIE_DESC_LEN))
		{
			LogMessage("No rune description for %s", buffer);
		}
	}
}

DisplayRuneMsg(client, const String:sMsg[])
{
	new aColor[4] = {255,191,0,240}
	new bColor[4] = {165,194,118,125}
	SetHudTextParamsEx(
		0.015, 0.75, 17.0,
		aColor, bColor, 2, 3.0);
		
	ShowSyncHudText(client,g_DescHudSync, sMsg);
}


public RTF_RuneInfo(Handle:plugin,numParams)
{
	new String:r_name[32];
	new String:r_desc[TRIE_DESC_LEN];
	new client = GetNativeCell(1);

	if(client < 1 || client > MaxClients)
		return ThrowNativeError(SP_ERROR_NATIVE, "Invalid client index (%d)", client);


	GetNativeString(2,r_name,sizeof(r_name));

	if(!GetTrieString(g_RuneTrie,r_name, r_desc, TRIE_DESC_LEN))
	{
		new String:error_msg[48];
		Format(error_msg,sizeof(error_msg), "No rune description for %s", r_name);
		PrintToConsole(client,error_msg);
		return ThrowNativeError(SP_ERROR_NATIVE, error_msg);
	}
	
	DisplayRuneMsg(client,r_desc);
	return true;
}


