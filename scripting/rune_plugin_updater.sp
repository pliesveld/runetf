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

new String:g_URL[256] = "";

new bool:g_bUpdateRegistered = false;

public OnPluginStart()
{
#if defined DEBUG
	return
	RegAdminCmd("rune_update_force", Command_Update, ADMFLAG_RCON, "Forces update check of plugin");
#endif

	decl String:sDesc[128]="";
	Format(sDesc,sizeof(sDesc),"Select a branch folder from %s to update from.", UPDATE_URL_BASE);
	hCvarBranch = CreateConVar("rune_update_branch", UPDATE_URL_BRANCH,
	sDesc, FCVAR_NOTIFY);

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
#if defined DEBUG
	return;
#endif

	
	if (LibraryExists("updater"))
	{
		Updater_AddPlugin(g_URL)
		g_bUpdateRegistered = true;
	} else {
#if defined DEBUG
		LogMessage("Updater not found.");
#endif
	}
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
#if defined DEBUG
	return;
#endif
	if (StrEqual(name, "updater"))
	{
		Updater_AddPlugin(g_URL)
	}
}
