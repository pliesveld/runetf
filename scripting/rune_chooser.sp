#include <smlib>
#define REQUIRE_EXTENSIONS
#define AUTOLOAD_EXTENSIONS
#include <runetf/runetf>
#define REQUIRE_PLUGIN
#include <runetf/runes_stock>
#include <runetf/spawn_rune>
#include <runetf/rune_info>


new Handle:g_RuneMenu = INVALID_HANDLE;

new bool:bSpawnRuneAll = false

public OnPluginStart()
// TODO::cvar rune_debug to allow non-admins to spawn runes
// TODO::cvar rune_allow_spawn_setup to allow rune spawning during setup time.
{
  RegAdminCmd("info_rune", Command_DisplayRunes,0, "display rune description");

  RegAdminCmd("spawn_rune", Command_SpawnRune,0, "spawns a rune of a specific type");
 // RegAdminCmd("spawn_rune", Command_SpawnRune,0, "spawns a rune of a specific type");

	RegAdminCmd("toggle_spawn_rune", Command_ToggleSpawnRune,ADMFLAG_CHEATS,"toggles !spawn_rune for everyone.")
}

public Action:Command_ToggleSpawnRune(client,args)
{
	PrintToConsole(client,"spawn_rune is now %s",( bSpawnRuneAll = !bSpawnRuneAll) ? "on" : "off")
}


public Action:Command_SpawnRune(client, args)
{
  new Float:ori[3]
  new Float:ang[3]
  new Float:loc[3] = { -1100.0, -1900.0, 420.0 } ;

  decl String:runestr[24];
	decl String:arg[16];
  	decl i;

	if(!bSpawnRuneAll)
		return Plugin_Handled;
	

	if(client == 0)
	{
		if(args > 2)
		{
			for(i = 1; i <= args; ++i)
			{
				GetCmdArg(i,arg,sizeof(arg));
				loc[i-1] = StringToFloat(arg);
				if(i > 3)
					break;
			}
		}

		if(args >= 4)
		{
			GetCmdArg(4,runestr,sizeof(runestr));
			SpawnNamedRune(runestr,loc);
			return Plugin_Handled;
		}

		SpawnNamedRune("aware",loc);
	//	SpawnRandomRune(loc);
		return Plugin_Handled;
	}

	new Handle:hMenu; 
	CreateRuneMenu(hMenu,Handler_SpawnRuneMenu);
	DisplayMenu(hMenu, client, MENU_TIME_FOREVER);

  return Plugin_Continue;
}

stock CreateRuneMenu(&Handle:hMenu,MenuHandler:handler)
{
	hMenu = CreateMenu(handler, MenuAction:MenuAction_Start | MENU_ACTIONS_DEFAULT);
  BuildRuneMenu(hMenu);
  SetMenuExitButton(hMenu, true);
}

public Action:Command_DisplayRunes(client, args)
{
	new Handle:hMenu; 
	CreateRuneMenu(hMenu,Handler_DisplRuneMenu);
	DisplayMenu(hMenu, client, MENU_TIME_FOREVER);
	return Plugin_Continue;
}
	
public Handler_DisplRuneMenu(Handle:menu, MenuAction:action, param1, param2)
{
	switch(action)
	{
		case MenuAction_Start:
		{
			PrintToServer("MenuStart param1 %d param2 %d", param1, param2);
		}
		case MenuAction_Select:
		{
			decl String:rune[24];
			new client = param1;
			new item = param2;
			GetMenuItem(menu, param2, rune,sizeof(rune));
			PrintToConsole(client,"You selected item %d: %s", item, rune);
			DisplayRune(client, rune);
		}
		case MenuAction_Cancel:
		{
			new client = param1;
			new reason = param2;
			PrintToServer("Menu: Client %d cancelled", client);
			PrintToConsole(client,"You cancelled %d.", reason);
		}
		case MenuAction_End:
		{
			g_RuneMenu = INVALID_HANDLE;
			CloseHandle(menu);
		}
	}
}

public Handler_SpawnRuneMenu(Handle:menu, MenuAction:action, param1, param2)
{
	switch(action)
	{
		case MenuAction_Select:
		{
			decl String:rune[24];
			new client = param1;
			new item = param2;
			GetMenuItem(menu, param2, rune,sizeof(rune));
			new Float:loc[3];
			OriginNearPlayer(client,loc);
			SpawnNamedRune(rune,loc);
		}	
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
}

BuildRuneMenu(&Handle:menu)
{
	new RuneArray[32];
	new size = GetRuneIds(RuneArray,sizeof(RuneArray));

	PrintToServer("Menu Build: found %d runes", size);

	for(new i; i < size;++i)
		AddRuneToMenu(menu,RuneArray[i]);
}
	

AddRuneToMenu(&Handle:menu, runeid)
{
	new String:buffer[32];
	if(!RuneNameById(runeid, buffer, sizeof(buffer)))
	{
		PrintToServer("Couldn't find rune %d!", runeid);
		return;
	}
	PrintToServer("Menu Build: id %d name %s", runeid, buffer);
	AddMenuItem(menu, buffer, buffer);
}

	
	
stock DisplayRune(client, String:rune[])
{
	RuneTF_display_rune_info(client,rune);
}
