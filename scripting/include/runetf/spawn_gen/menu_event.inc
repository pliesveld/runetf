#if !defined __menu__events__
#define __menu__events__

/*
	

	EventType
		// name of cluster
			// ClusterGenType:action
			// EventHookType:hook
			// class name of entity
			// name of entity
			// output of entity
*/

new m_Event[GenEvent];
new m_EventClusterName[MAX_CLUSTER_LEN];


#define EVENT_ACTION_MENU "%%ACTION%%"
#define EVENT_CNAME_MENU  "%%CNAME%%"
#define EVENT_OUTPUT_MENU "%%OUTPUT%%"

stock ResetEventMenu(client)
{
	ClearEvent(m_Event);
}

stock ClearEvent(a_Event[GenEvent]) // Move to rune_gen_event
{
	a_Event[ClusterType] = ClusterGenType:GenEvent_None;
	a_Event[EventType]   = EventHookType:GenEvent_None;
	a_Event[EventCName][0]   = '\0';
	a_Event[EventEntity][0]  = '\0';
	a_Event[EventOutput][0] = '\0';
	m_EventClusterName[0] = '\0';
}


Handle:CreateEventMenu(client)
{
	new Handle:hMenu = CreateMenu(Handler_EventMenu,RUNE_MENU_ACTIONS_DEFAULT);
		// CloseHandle(hMenu) on fail??
	SetMenuExitBackButton(hMenu,true);
	if(m_Event[ClusterType] == ClusterType:GenEvent_None)
	{
		for(new i = 1; i < sizeof(actionString);++i)
		{
			AddMenuItem(hMenu,actionString[i],actionString[i]);
		}
		return hMenu;
	} else if(m_Event[EventCName][0] == '\0') {
		for(new i = 0;i < sizeof(class_type);++i)
		{
			new count = GetArrayCell(g_MapEventEntities,i);
			AddMenuItem(hMenu,class_type[i],class_type[i],(count ==  0 ? ITEMDRAW_DISABLED: ITEMDRAW_DEFAULT) );
		}
		return hMenu;
	} else if(m_Event[EventOutput][0] == '\0') {
		BuildEntityOutputMenu(hMenu, m_Event[EventCName]);
		return hMenu;
	} else if(m_Event[EventType] == 0) {
		for(new i = 1; i < sizeof(entityString);++i)
		{
			AddMenuItem(hMenu,entityString[i],entityString[i],(i > 2 ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT));
		}
		return hMenu;
	} else if(m_Event[EventType] == GenEvent_ByEnt && m_Event[EventEntity][0] == '\0') {
		new class_idx;
		if(!GetTrieValue(g_CNameTrie,m_Event[EventCName],class_idx))
			return INVALID_HANDLE;
		new count = GetArrayCell(g_MapEventEntities,class_idx);
		if(!count)
		{
			PrintToConsole(client,"cname %s has no named entities in this map.",class_type[class_idx]);
			m_Event[EventType] = EventHookType:GenEvent_None;
			return INVALID_HANDLE;
		}
		new Handle:hEntNames = GetArrayCell(g_MapEventEntities, class_idx, 2);
		for(new i; i < count;++i)
		{
			decl String:ent_name[MAX_EVENTGEN_CNAME];
			GetArrayString(hEntNames,i, ent_name,sizeof(ent_name));
			AddMenuItem(hMenu, ent_name,ent_name);
		}
		return hMenu;
	} else if( m_EventClusterName[0] == '\0') {
		new size;
		if(g_vGenEvent == INVALID_HANDLE ||
			(size = GetArraySize(g_vGenEvent)) == 0)
		{
			PrintToConsole(client,"No clusters founds.  use sm_gen cluster create.");
			return INVALID_HANDLE;
		}
		for(new i; i < size;++i)
		{
			new a_Cluster[RuneCluster];
			new String:s_cluster[MAX_CLUSTER_LEN];
			GetArrayArray(g_vGenEvent,i,a_Cluster,CLUSTER_BLOCK_SIZE);
			new Handle:hEvent= a_Cluster[EventArray];
			strcopy(s_cluster,sizeof(s_cluster), a_Cluster[ClusterName]);
			new bool:found = false;
			if(hEvent != INVALID_HANDLE && GetArraySize(hEvent) > 0)
				for(new j; j < GetArraySize(hEvent);++j)
				{
					new j_Event[GenEvent];
					GetArrayArray(hEvent,j, j_Event, EVENT_BLOCK_SIZE);
					if(IsEventSame(m_Event,j_Event))
					{
						found = true;
						break;
					}
				}

			AddMenuItem(hMenu, s_cluster,s_cluster, found ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
		}
		return hMenu;
	} else {
		PrintToConsole(client,"warning; clearing temporary event. %s %s %s",
				actionString[m_Event[ClusterType]], m_Event[EventCName], m_Event[EventOutput]);
		ClearEvent(m_Event);
	}
	
	return INVALID_HANDLE;
}

public Handler_EventMenu(Handle:menu, MenuAction:m_action, param1, param2)
{
	new client = param1; // note: not always
	LOG_MENU_HANDLER
	switch(m_action)
	{
		case MenuAction_Start:
		{
			LOG_MENU_START
		}
		case MenuAction_Display:
		{
		}
		case MenuAction_Select:
		{
			LOG_MENU_SELECT
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectEventMenu(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client,MENU_TIME_FOREVER))
				PrintToConsole(client,"Failed to recreate menu.\n");
		}
		case MenuAction_Cancel:
		{
			LOG_MENU_CANCELLED
			if(param2 == MenuCancel_ExitBack)
			{
				ClearEvent(m_Event);
				new Handle:menu_next = CreateEventMenu(param1);
				DisplayMenu(menu_next, client, MENU_TIME_FOREVER);
			}
		}
		case MenuAction_DrawItem:
		{
			decl String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			LOG_MENU_SELECT_CUSTOM("display => %s",action);
		}
		case MenuAction_End:
		{
			LOG_MENU_END
			CloseHandle(menu);
		}
		case MenuAction_DisplayItem:
		{
		}
	}
}

Handle:ClientSelectEventMenu(client,const String:item_str[])
{
	if(m_Event[ClusterType] == ClusterGenType:GenEvent_None)
	{
		new ClusterGenType:cType;
		if( (cType = CheckClusterType(item_str)) == -1)
			return INVALID_HANDLE;
			m_Event[ClusterType] = cType;
			return CreateEventMenu(client);
	} else if(m_Event[EventCName][0] == '\0') {
		new class_idx;
		if(!GetTrieValue(g_CNameTrie,item_str,class_idx))
			return INVALID_HANDLE;
		strcopy(m_Event[EventCName],sizeof(m_Event[EventCName]),class_type[class_idx]);
		return CreateEventMenu(client);
	} else if(m_Event[EventOutput][0] == '\0') {
		if(CheckOutput(m_Event[EventCName], item_str) == -1)
		{
			DisplayEntOutputs(m_Event[EventCName],client);
			return INVALID_HANDLE;
		}
		strcopy(m_Event[EventOutput], sizeof(m_Event[EventOutput]), item_str);
		return CreateEventMenu(client);
	} else if(m_Event[EventType] == EventType:GenEvent_None) {
		new eType;
		if( (eType = CheckEntityType(item_str)) == -1)
		{
			PrintToConsole(client,"Invalid EntityHookType %s", item_str);
			return INVALID_HANDLE;
		}
		m_Event[EventType] = eType;
		return CreateEventMenu(client);
	} else if(m_Event[EventType] == GenEvent_ByEnt && m_Event[EventEntity][0] == '\0') {
		new class_idx;
		if(!GetTrieValue(g_CNameTrie, m_Event[EventCName],class_idx))
			return INVALID_HANDLE;
  	new cnt = GetArrayCell(g_MapEventEntities,class_idx,0);
		if(cnt == 0)
			return INVALID_HANDLE;

		new Handle:ent_names_arr = GetArrayCell(g_MapEventEntities,class_idx,2);
		for(new i; i < cnt;++i)
		{
			decl String:t_ent_name[MAX_EVENTGEN_CNAME];
			GetArrayString(ent_names_arr,i,t_ent_name,sizeof(t_ent_name));
			if(t_ent_name[0] != '\0')
			{
				if(!strcmp(t_ent_name,item_str))
				{
					strcopy(m_Event[EventEntity],sizeof(m_Event[EventEntity]), item_str);
					return CreateEventMenu(client);
				}
			}
		}
		return INVALID_HANDLE;
	} else {
		new Handle:hEvent;
		new e_idx;
		if( (e_idx = FindEventCluster(item_str, hEvent)) != -1 )
		{
			if(add_event_info(hEvent, m_Event) == -1)
			{
				PrintToConsole(client,"warning; could not create event.");
				return CreateEventMenu(client);
			} else {	
				PrintToConsole(client,"Event added. Cluster #%d", e_idx);
				g_bChanged = true;
 				HookGenEvent(item_str, m_Event);
				ClearEvent(m_Event);
				return CreateMainMenu(client);
			}
		} else {
			PrintToConsole(client,"Cluster name %s not found.", item_str);
			return CreateMainMenu(client);
		}
	}

	PrintToConsole(client,"Unknown state.  Selected => %s", item_str);
	return INVALID_HANDLE;
}



// CreateEventMainMenu


// CreateEventSelectActionMenu
		//AddToMenu
				// ent_Add
				// ent_Remove
				// ent_Spawn
				// ent_Toggle

// CreateEventSelectClassnameMenu

// CreateEventSelectOutputMenu

// CreateEventSelectEntityMenu
				


// CreateEventSelectHookMenu
		//AddToMenu
			// ByClass
			// ByEnt




































#endif
