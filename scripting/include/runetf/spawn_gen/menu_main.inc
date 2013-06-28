#define MAIN_MENU_ITER "%%ITERATE%%"
#define MAIN_MENU_LOAD "%%LOAD%%"
#define MAIN_MENU_MERGE "%%MERGE%%"
#define MAIN_MENU_TEST "%%TEST%%"
#define MAIN_MENU_CREATE "%%CREATE%%"
#define MAIN_MENU_EVENTS "%%EVENTS%%"
#define MAIN_MENU_CLUSTER "%%CLUSTER%%"
#define MAIN_MENU_SAVE "%%SAVE%%"

static String:menu_action[][MAX_MENU_LEN] =
{
	MAIN_MENU_ITER,
	MAIN_MENU_CREATE,
	MAIN_MENU_CLUSTER,
	MAIN_MENU_EVENTS,
	MAIN_MENU_MERGE,
	MAIN_MENU_SAVE
};

static String:menu_text[][32] =
{
	"iterate spawn points",
	"create rune generator",
	"clusters",
	"events",
	"merge changes",
	"save changes"
}
stock DisplayMainMenu(client)
{
	new Handle:hMenu = CreateMainMenu(client)
	DisplayMenu(hMenu, client, MENU_TIME_FOREVER);
}

stock CreateMainMenu(client)
{
	new Handle:hMenu;
	hMenu = CreateMenu(Handler_MainMenu, RUNE_MENU_ACTIONS_DEFAULT|MenuAction_DrawItem);
	SetMenuExitButton(hMenu, true);
	for(new i; i < sizeof(menu_action);++i)
		AddMenuItem(hMenu, menu_action[i], menu_text[i]);
	//DisplayMenu(hMenu, client, MENU_TIME_FOREVER);
	return hMenu;
}

Handle:ClientSelectedMenu(client,String:action[])
{
	new Handle:menu_next = INVALID_HANDLE;
	if(!strcmp(MAIN_MENU_ITER, action))
	{
		menu_next = CreateIteratorMenu(client);
	} else if(!strcmp(MAIN_MENU_CLUSTER,action))  {
		menu_next = CreateClusterMainMenu(client);
	} else if(!strcmp(MAIN_MENU_MERGE,action)) {
		menu_next = ClientSelectedGeneratorMenu(client,MAIN_MENU_MERGE);
		// MergeWorkingSetCmd(client)
	} else if(!strcmp(MAIN_MENU_CREATE,action)) {
		new idx = CreateGenClientCmd(client);
		DisplayGenToClient(client, g_Player[WorkingSet], idx);

		if(g_iState[it_array] != INVALID_HANDLE && g_iState[it_array] == g_Player[WorkingSet]) 
		{
			g_iState[it_idx] = idx;
			menu_next = CreateGeneratorMenu(client);
		} else {
			menu_next = CreateMainMenu(client);
		}

	} else if(!strcmp(MAIN_MENU_EVENTS,action)) {
		ResetEventMenu(client);
		menu_next = CreateEventMenu(client);
	} else if(!strcmp(MAIN_MENU_SAVE,action)) {
		if( OnSaveCmd(client,0) == Plugin_Changed)
			g_bChanged = false;
	} else {
		PrintToConsole(client, "Unknown action %s", action);
		menu_next = CreateMainMenu(client);
	}
	return menu_next;
}

public Handler_MainMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Select:
		{
			LOG_MENU_SELECT
			new client = param1;
			decl String:action[MAX_MENU_LEN];
			GetMenuItem(menu, param2, action,sizeof(action));
			new Handle:menu_next = ClientSelectedMenu(client, action);

			if(menu_next == INVALID_HANDLE || !DisplayMenu(menu_next, client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to create next menu.");
		}
		case MenuAction_DrawItem:
		{
			new style;
			decl String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action),style);
			if(!strcmp(MAIN_MENU_MERGE,action))
			{
				if(g_Player[WorkingSet] == INVALID_HANDLE || GetArraySize(g_Player[WorkingSet]) == 0)
				return ITEMDRAW_IGNORE;
			}
			else if(!strcmp(MAIN_MENU_CLUSTER,action) 
				&& GetArraySize(g_ClusterNames) == 0)
			{
				PrintToConsole(param1,"warning; no clusters.  Use sm_gen cluster create");
				return ITEMDRAW_DISABLED;
			} else if(!strcmp(MAIN_MENU_SAVE,action))
				if(g_bChanged == false)
					return ITEMDRAW_IGNORE;

			return style;
		}
		case MenuAction_Cancel:
		{
			LOG_MENU_CANCELLED
		}
		case MenuAction_End:
		{
			LOG_MENU_END
			CloseHandle(menu);
		}
	}
	return 0;
}





