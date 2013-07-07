#if !defined __menu_cluster__
#define __menu_cluster__

#define CLUSTER_GEN_MENU   "%%CLUSTER_GEN%%"
#define CLUSTER_SPAWN_MENU "%%CLUSTER_SPAWN%%"
#define CLUSTER_LOAD_MENU  "%%CLUSTER_LOAD%%"
#define CLUSTER_DROP_MENU  "%%CLUSTER_DROP%%"
#define CLUSTER_DEFAULT_MENU "%%CLUSTER_DEFAULT%%"


Handle:CreateClusterMainMenu(client)
{
	if(!client)
		return INVALID_HANDLE;
	new Handle:hMenu = CreateMenu(Handler_ClusterMainMenu, RUNE_MENU_ACTIONS_DEFAULT|MenuAction_DisplayItem|MenuAction_DrawItem|MenuAction_Cancel);

	AddMenuItem(hMenu,CLUSTER_GEN_MENU,"default cluster");
	AddMenuItem(hMenu,CLUSTER_SPAWN_MENU,"test cluster spawn");
	AddMenuItem(hMenu,CLUSTER_LOAD_MENU,"load cluster into working set");
	AddMenuItem(hMenu,CLUSTER_DROP_MENU,"drop cluster from working set");

	SetMenuExitBackButton(hMenu,true);

	return hMenu;
}


public Handler_ClusterMainMenu(Handle:menu, MenuAction:m_action,param1,param2)
{
	new client = param1;
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
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectClusterMenu(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
		case MenuAction_DrawItem:
		{
			LOG_MENU_SELECT_CUSTOM("DRAW ClusterMain");
			if(!IsValidIter(g_Player[WorkingSet],0) && param2 == 3)
				return ITEMDRAW_DISABLED;
			return ITEMDRAW_DEFAULT;
		}
		case MenuAction_Cancel:
		{
			LOG_MENU_CANCEL
			if(param2 == MenuCancel_ExitBack)
			{
				new Handle:menu_back;
				if(!IsValidIter(g_iState[it_array],g_iState[it_idx]))
					menu_back = CreateMainMenu(client);
				else
					menu_back = CreateIterateMenu(client);
				DisplayMenu(menu_back,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}

	return 0;
}

Handle:ClientSelectClusterMenu(client, const String:item_str[])
{
	if(!strcmp(item_str,CLUSTER_GEN_MENU))
	{
		return CreateClusterDefaultMenu(client);
	} else if(!strcmp(item_str,CLUSTER_SPAWN_MENU)) {
		return CreateClusterSpawnGeneratorMenu(client);
	} else if(!strcmp(item_str,CLUSTER_LOAD_MENU)) {
		return CreateClusterLoadGeneratorMenu(client);
	} else if(!strcmp(item_str,CLUSTER_DROP_MENU)) {
		return CreateClusterDropGeneratorMenu(client);
	}

	PrintToConsole(client,"Unknown cluster menu %s", item_str);
	return CreateGeneratorMenu(client);
}


Handle:ClientSelectClusterGenerator(client, const String:item_str[])
{
  new id = GetArrayCell(g_iState[it_array],g_iState[it_idx]);
  if(AddToCluster(item_str,id) == -1)
  {
    if(RemoveFromCluster(item_str,id) == -1)
    {
      LogError("warning; invalid cluster %s or invalid generator %d",item_str,id);
      return INVALID_HANDLE;
    }
  }
  DisplayGenToClient(client,g_iState[it_array], g_iState[it_idx]);
  return CreateIterateMenu(client);
}

public Handler_ClusterGeneratorMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	new client = param1;
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_DrawItem:
		{
			LOG_MENU_SELECT_CUSTOM("CLUSTER_GEN DRAW")
		}
		case MenuAction_Select:
		{
			LOG_MENU_SELECT_CUSTOM("ClusterGenerator")
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectClusterGenerator(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
		case MenuAction_Cancel:
		{
			LOG_MENU_CANCEL
			if(param2 == MenuCancel_ExitBack)
			{
				new Handle:menu_back = CreateGeneratorMenu(client);
				DisplayMenu(menu_back,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
}


Handle:ClientSelectSpawnClusterGenerator(client, const String:item_str[])
{
	decl String:command[256];
	strcopy(command,sizeof(command),"sm_gen cluster spawn ");
	StrCat(command,sizeof(command),item_str);
	ClientCommand(client,command);
	
	if(IsValidIter(g_iState[it_array],g_iState[it_idx]))
		return CreateIterateMenu(client);
	return CreateClusterMainMenu(client);
}

public Handler_ClusterSpawnGeneratorMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	new client = param1;
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Select:
		{
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectSpawnClusterGenerator(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
		case MenuAction_Cancel:
		{
			if(param2 == MenuCancel_ExitBack)
			{

				new Handle:menu_back;
				if(IsValidIter(g_iState[it_array],g_iState[it_idx]))
					menu_back = CreateGeneratorMenu(client);
				else
					menu_back = CreateClusterMainMenu(client);
				DisplayMenu(menu_back,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
}

Handle:ClientSelectLoadClusterGenerator(client, const String:item_str[])
{
	decl String:command[256];
	strcopy(command,sizeof(command),"sm_gen cluster load ");
	StrCat(command,sizeof(command),item_str);
	if(Action:ClientCommand(client,command) == Plugin_Changed)
	{
		if(IsValidIter(g_iState[it_array],g_iState[it_idx]))
			return CreateIterateMenu(client);
		if(IsValidIter(g_iState[it_array],0))
		{
			g_iState[it_idx] = 0;
		}

	}
	return CreateClusterMainMenu(client);
}

Handle:ClientSelectDropClusterGenerator(client, const String:item_str[])
{
	decl String:command[256];
	strcopy(command,sizeof(command),"sm_gen cluster drop ");
	StrCat(command,sizeof(command),item_str);
	if(Action:ClientCommand(client,command) == Plugin_Changed)
	{
		if(!IsValidIter(g_iState[it_array],g_iState[it_idx]))
		{
			if(IsValidIter(g_iState[it_array],0))
				g_iState[it_idx] = 0;
			else
				return CreateClusterMainMenu(client);
		}
		return CreateIterateMenu(client);

	}
	return CreateMainMenu(client);
}

public Handler_ClusterLoadGeneratorMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	new client = param1;
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Select:
		{
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectLoadClusterGenerator(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
		case MenuAction_DrawItem:
		{
			new String:action[MAX_MENU_LEN];
			new default_style;
			GetMenuItem(menu,param2,action,sizeof(action),default_style);
			new Handle:array_ids = INVALID_HANDLE;
			if(!GetTrieValue(g_vGenCluster,action,array_ids))
				return ITEMDRAW_IGNORE;
			new id_size;
			if(array_ids == INVALID_HANDLE || ((id_size = GetArraySize(array_ids)) == 0))
				return ITEMDRAW_DISABLED;

			new w_size;
			if(g_Player[WorkingSet] == INVALID_HANDLE || ((w_size = GetArraySize(g_Player[WorkingSet])) == 0))
				return default_style;
	
			if(w_size < id_size)
				return default_style;

			for(new i; i < id_size;++i) 
			{
				new id = GetArrayCell(array_ids,i);
				if(FindIndexInArrayById(g_Player[WorkingSet],id) == -1)
					return default_style;
			}
			return ITEMDRAW_DISABLED;
		}
		
		case MenuAction_Cancel:
		{
			if(param2 == MenuCancel_ExitBack)
			{
				new Handle:menu_back;
				if(IsValidIter(g_iState[it_array],g_iState[it_idx])) 
					menu_back = CreateGeneratorMenu(client);
				else
					menu_back = CreateClusterMainMenu(client);
				DisplayMenu(menu_back,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
	return 0;
}


public Handler_ClusterDropGeneratorMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	new client = param1;
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Select:
		{
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectDropClusterGenerator(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
		case MenuAction_DrawItem:
		{
			new String:action[MAX_MENU_LEN];
			new default_style;
			GetMenuItem(menu,param2,action,sizeof(action),default_style);
			new Handle:array_ids = INVALID_HANDLE;
			//new w_size;
			if(g_Player[WorkingSet] == INVALID_HANDLE || GetArraySize(g_Player[WorkingSet]) == 0)
				return ITEMDRAW_DISABLED;
	
			if(!GetTrieValue(g_vGenCluster,action,array_ids))
				return ITEMDRAW_IGNORE;

			new id_size;
			if(array_ids == INVALID_HANDLE || ((id_size = GetArraySize(array_ids)) == 0))
				return ITEMDRAW_DISABLED;

			for(new i; i < id_size;++i) 
			{
				new id = GetArrayCell(array_ids,i);
				if(FindIndexInArrayById(g_Player[WorkingSet],id) != -1)
					return default_style;
			}
			return ITEMDRAW_DISABLED;
		}
		
		case MenuAction_Cancel:
		{
			if(param2 == MenuCancel_ExitBack)
			{
				new Handle:menu_back = CreateClusterMainMenu(client);
				DisplayMenu(menu_back,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
	return 0;
}

public Handler_ClusterDefaultMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	new client = param1;
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Select:
		{
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectDefaultCluster(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
	
		case MenuAction_Cancel:
		{
			if(param2 == MenuCancel_ExitBack)
			{
				new Handle:menu_back = CreateClusterMainMenu(client);
				DisplayMenu(menu_back,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
}

Handle:ClientSelectDefaultCluster(client, const String:item_str[])
{// Should really move this logic to rune_gen_iterate

	if(!strcmp(item_str,"%%CLEAR%%"))
	{
		ClearArray(g_Player[DefaultCluster]);
	} else {
		new idx;

		if(g_Player[DefaultCluster] == INVALID_HANDLE)
			g_Player[DefaultCluster] = CreateArray(MAX_CLUSTER_LEN);

		if((idx = FindStringInArray(g_Player[DefaultCluster],item_str)) != -1)
		{
			RemoveFromArray(g_Player[DefaultCluster], idx);
		} else {
			PushArrayString(g_Player[DefaultCluster],item_str);
		}
	}

	DisplayDefaultClusterToClient(client, g_Player[DefaultCluster]);

	return CreateClusterDefaultMenu(client);
}

#endif
