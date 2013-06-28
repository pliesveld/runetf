#include <sourcemod>

#include <runetf/runetf>
#undef REQUIRE_PLUGIN
#include <updater>

#define DEBUG


#define PLUGIN_NAME "Rune Auto-Updater"
#define PLUGIN_DESCRIPTION "Auto-update runetf plugin."

public Plugin:myinfo = {
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_URL
}

//#define UPDATE_URL_BASE   "https://raw.github.com/pliesveld/runetf"
#define UPDATE_URL_BASE   "http://localhost/runetf"
#define UPDATE_URL_BRANCH "master"
#define UPDATE_URL_FILE   "updateplugin.txt"
#define UPDATE_URL_CFG    "updatemapcfg.txt"

new Handle:hCvarBranch = INVALID_HANDLE;
new Handle:hCvarCfg = INVALID_HANDLE;

new String:g_URL[256] = "";
new String:g_URLMap[256] = "";

new bool:g_bUpdateRegistered = false;
new bool:g_bUpdateMapCfg = true;

public OnPluginStart()
{
#if defined DEBUG
	RegAdminCmd("rune_update_force", Command_Update, ADMFLAG_RCON, "Forces update check of plugin");
#endif

	decl sDesc[128]="";
	Format(sDesc,sizeof(sDesc),"Select a branch folder from %s to update from.", UPDATE_URL_BASE);
	hCvarBranch = CreateConVar("rune_update_branch", UPDATE_URL_BRANCH,
	sDesc, FCVAR_NOTIFY);
	hCvarCfg = CreateConVar("rune_update_mapcfg", "1", "Auto-update rune spawn generator map configuration files.", true, 0.0, true, 1.0);
	g_bUpdateMapCfg = GetConVarInt(hCvarCfg);

	decl String:branch[32]="";
	GetConVarString(hCvarBranch,branch,sizeof(branch));

	if(!VerifyBranch(branch,sizeof(branch)))
	{
		SetConVarString(hCvarBranch,UPDATE_URL_BRANCH);
#if defined DEBUG
		LogMessage("Resetting branch to %s", UPDATE_URL_BRANCH);
#endif
	}

	Format(g_URL,sizeof(g_URL),"%s/%s/%s",UPDATE_URL_BASE,branch,UPDATE_URL_FILE);
	Format(g_URLMap,sizeof(g_URLMap),"%s/%s/%s",UPDATE_URL_BASE,branch,UPDATE_URL_CFG);
	
	if (LibraryExists("updater"))
	{
		Updater_AddPlugin(g_URL)
		if(g_bUpdateMapCfg)
			Updater_AddPlugin(g_URLMap);

		g_bUpdateRegistered = true;
	} else {
#if defined DEBUG
		LogMessage("Updater not found.");
#endif
	}
	return Plugin_Continue;
}

stock bool:VerifyBranch(String:branch[],len)
{
	if(!strcmp(branch,"master"))
		return true;

	for(new idx; idx < len;++idx)
	{
		if(!IsCharAlpha(branch[idx]))
		{
			LogError("Invalid branch %s", branch);
			return false;
		}
	}
	return true;
}


#if defined DEBUG

public Action:Command_Update(client,args)
{
	if(!LibraryExists("updater")) {
		ReplyToCommand(client,"updater plugin not found.");
	} else if(!g_bUpdateRegistered) {
		ReplyToCommand(client,"Updater not registered.");
	} else {
		ReplyToCommand(client,"Force update returned %s", Updater_ForceUpdate() ? "true" : "false");
	
	}
	return Plugin_Handled;
}

public Action:Updater_OnPluginChecking()
{
	LogMessage("Checking for updates.");
	return Plugin_Continue;
}

public Action:Updater_OnPluginDownloading()
{
	LogMessage("Downloading update.");
	return Plugin_Continue;
}

public Updater_OnPluginUpdating()
{
	LogMessage("Updating plugin.");
}

public Updater_OnPluginUpdated()
{
	LogMessage("Update complete.");
}
#endif

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "updater"))
    {
        Updater_AddPlugin(g_URL)
				if(g_bUpdateMapCfg)
        	Updater_AddPlugin(g_URLMap)
					
    }
}
