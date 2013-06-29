Handle:CreateIterateMenu(client)
{
	if(!client)
		return INVALID_HANDLE;
	new Handle:menu = CreateMenu(Handler_IterateMenu,RUNE_MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_DisplayItem);
	AddMenuItem(menu,GEN_ITER_NEXT,	"next");
	AddMenuItem(menu,GEN_ITER_PREV,	"prev");
	AddMenuItem(menu,GEN_ITER_SPAWN,"spawn");
	AddMenuItem(menu,GEN_CLUSTER_MENU,"cluster");
	AddMenuItem(menu,GEN_MODIFY, 		"modify");
	AddMenuItem(menu,GEN_LOAD,   		"load");
	AddMenuItem(menu,GEN_MERGE,			"merge");
	AddMenuItem(menu,GEN_DROP,   		"delete");
	SetMenuExitBackButton(menu,true);
	return menu;
}

Handle:ClientSelectedGeneratorMenu(client,const String:item[])
{
	new Handle:hMenu = INVALID_HANDLE;
	new bool:bDisplay = false;

	if(!strcmp(item,GEN_ITER_NEXT)) {
		IterateTeleportPlayer(client, g_iState[it_array], g_iState[it_idx],true);
		bDisplay = true;
	} else if(!strcmp(item,GEN_ITER_PREV)) {
		IterateTeleportPlayer(client, g_iState[it_array], g_iState[it_idx],false);
		bDisplay = true;
	} else if(!strcmp(item,GEN_ITER_SPAWN)) {
		SpawnTestRune( g_iState[it_array], g_iState[it_idx]);
	} else if(!strcmp(item, GEN_MODIFY)) {
		hMenu = CreateGeneratorMenu(client);
	} else if(!strcmp(item,GEN_CLUSTER_MENU)) {
		hMenu = CreateClusterGeneratorMenu(client);
	} else if(!strcmp(item, GEN_DROP)) {
		new nId = get_id_info(g_iState[it_array], g_iState[it_idx]);
		RemoveFromAllClusters(nId);
		RemoveFromArray(g_iState[it_array],g_iState[it_idx]);
		if( GetArraySize(g_iState[it_array]) <= g_iState[it_idx])
			g_iState[it_idx]--;
		if(!IsValidIter(g_iState[it_array], g_iState[it_idx]))
			hMenu = CreateMainMenu(client);
		else {
			IterateTeleportPlayer(client, g_iState[it_array], g_iState[it_idx],false);
			bDisplay = true;
		}
	} else if(!strcmp(item, GEN_LOAD)) {
		if(g_iState[it_array] != g_Player[WorkingSet])
			ClientLoadGen(client, g_Player[WorkingSet],
				get_id_info(g_iState[it_array], g_iState[it_idx]));
	} else if(!strcmp(item, GEN_MERGE)) {
		new bool:bWasWorkingSet = (g_iState[it_array] == g_Player[WorkingSet]);
		if(g_Player[WorkingSet] != INVALID_HANDLE && GetArraySize(g_Player[WorkingSet]) > 0)
			if(((g_bChanged = (MergeWorkingSetCmd(client) > 0))) && bWasWorkingSet)
			{
				g_iState[it_array] = INVALID_HANDLE;
				g_iState[it_idx] = -1;
			}

		hMenu = CreateMainMenu(client);
	} else {
		PrintToConsole(client,"warning; unknown action %s",item);
	}

	if(bDisplay)
	{
		DisplayGenToClient(client,g_iState[it_array], g_iState[it_idx]);
	}

	if(hMenu == INVALID_HANDLE)
		hMenu = CreateIterateMenu(client);
	return hMenu;
}


public Handler_IterateMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_DisplayItem:
		{
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			if(!strcmp(action,GEN_DROP))
				if(g_iState[it_array] == g_Player[WorkingSet])
					return RedrawMenuItem("drop");
			if(!strcmp(action,GEN_CLUSTER_MENU))
				if(GetArraySize(g_ClusterNames) == 0)
					return RedrawMenuItem("need: sm_gen cluster create <cluster>");
			return 0;
		}

		case MenuAction_DrawItem:
		{
			new String:action[MAX_MENU_LEN];
			new default_style;
			GetMenuItem(menu,param2,action,sizeof(action),default_style);

			if(!strcmp(action,GEN_LOAD))
			{
				if(g_iState[it_array] == g_Player[WorkingSet] || g_iState[it_array] == INVALID_HANDLE)
					return ITEMDRAW_IGNORE;
				new nId = get_id_info(g_iState[it_array],g_iState[it_idx]);
				if(g_Player[WorkingSet] != INVALID_HANDLE 
					&& GetArraySize(g_Player[WorkingSet]) > 0
					&& FindIndexInArrayById(g_Player[WorkingSet],nId) != -1)
					return ITEMDRAW_DISABLED;
			} else if(!strcmp(action,GEN_MERGE)) {
				if(g_Player[WorkingSet] == INVALID_HANDLE || GetArraySize(g_Player[WorkingSet]) == 0)
					return ITEMDRAW_IGNORE;
			} else if(!strcmp(action,GEN_CLUSTER_MENU))
				if(GetArraySize(g_ClusterNames) == 0)
					return ITEMDRAW_DISABLED;
				// if working set && item == load
				// ==> disable
			// if !working set && item == merge
				// ==> disable
			return default_style;
		}
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Select:
		{
			new client=param1;
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectedGeneratorMenu(client,action);

			if(menu_next == INVALID_HANDLE || !DisplayMenu(menu_next, client, MENU_TIME_FOREVER))
			{
			  LogError("warning; failed to recreate menu from generator.");
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
		case MenuAction_Cancel:
		{
			new client = param1;
			if(MenuCancel_ExitBack == param2)
			{
				DisplayMainMenu(client);
			}
		}
	}
	//if(MenuAction_Cancel == m_action && param2 == MenuCancel_ExitBack)
	return 0
}

