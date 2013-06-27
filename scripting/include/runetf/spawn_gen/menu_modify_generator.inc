#if !defined __menu_modify_generator__
#define  __menu_modify_generator__


#define MIN_FORCE 100.00
#define MAX_FORCE	30000.00
 
Handle:CreateGeneratorMenu(client)
{
	new Handle:hMenu = CreateMenu(Handler_GeneratorMenu,RUNE_MENU_ACTIONS_DEFAULT);
	AddMenuItem(hMenu,GEN_SET_ORIGIN,			"origin");
	AddMenuItem(hMenu,GEN_SET_ANGLE,			"angle");
	AddMenuItem(hMenu,GEN_FORCE_MENU,			"force");
	AddMenuItem(hMenu,GEN_TOGGLE_DISABLE,	"start_disabled");
	AddMenuItem(hMenu,GEN_CLUSTER_MENU,		"cluster");
	SetMenuExitBackButton(hMenu,true);
	return hMenu;
}

#define FORCE_SET "FSET_"
#define FORCE_INC "FINC_"
#define FORCE_DEC "FDEC_"

Handle:CreateForceMenu(client)
{
	new Handle:hMenu = CreateMenu(Handler_ModifyForceMenu,RUNE_MENU_ACTIONS_DEFAULT);
	AddMenuItem(hMenu,"FINC_5", "inc 5");
	AddMenuItem(hMenu,"FINC_30", "inc 30");
	AddMenuItem(hMenu,"FINC_240", "inc 240");
	AddMenuItem(hMenu,"FINC_600", "inc 600");
	AddMenuItem(hMenu,"FDEC_5",		"dec 5");
	AddMenuItem(hMenu,"FDEC_30", "dec 30");
	AddMenuItem(hMenu,"FDEC_240", "dec 240");
	AddMenuItem(hMenu,"FDEC_600", "dec 600");
	SetMenuExitBackButton(hMenu,true);
	return hMenu;
}

Handle:ClientSelectedForceMenu(client,const String:item_str[])
{
	new idx;
	if(strlen(item_str) < 6)
		return INVALID_HANDLE;
	if(!IsValidIter(g_iState[it_array], g_iState[it_idx]))
		return INVALID_HANDLE;

	new Float:vForce = StringToFloat(item_str[5]);
	new Float:nForce = get_force_info(g_iState[it_array],g_iState[it_idx]);

	if(!strncmp(item_str, FORCE_SET,5)) {
		nForce = vForce;
	} else if(!strncmp(item_str, FORCE_INC,5)) {
		nForce = nForce + vForce;
	} else if(!strncmp(item_str, FORCE_DEC,5)) {
		nForce = nForce - vForce;
	} else {
		PrintToConsole(client,"Invalid ModifyForce %s", item_str);
		return INVALID_HANDLE;
	}


	if(nForce > MAX_FORCE)
		nForce = MAX_FORCE;
	else if(nForce < MIN_FORCE)
		nForce = MIN_FORCE;

	set_force_info(g_iState[it_array], g_iState[it_idx], nForce);
	if(g_iState[it_array] != g_Player[WorkingSet])
		g_bChanged = true;
  DisplayGenToClient(client,g_iState[it_array], g_iState[it_idx]);
  SpawnTestRune( g_iState[it_array], g_iState[it_idx]);

	return CreateForceMenu(client);	
}

Handle:ClientSelectGeneratorMenu(client,const String:item_str[])
{
	new Handle:hMenu = INVALID_HANDLE;

	if(!IsValidIter(g_iState[it_array], g_iState[it_idx]))
		return INVALID_HANDLE;

	new bool:bDisplay = false;
	if(!strcmp(item_str,GEN_SET_ORIGIN)) {
		new Float:origin[3];
		GetClientEyePosition(client,origin);
		set_origin_info(g_iState[it_array],g_iState[it_idx],origin);
  	SpawnTestRune( g_iState[it_array], g_iState[it_idx]);
		bDisplay = true;
	} else if(!strcmp(item_str,GEN_SET_ANGLE)) {
		new Float:angle[3];
		GetClientEyeAngles(client,angle);
		set_angle_info(g_iState[it_array],g_iState[it_idx],angle);
  	SpawnTestRune( g_iState[it_array], g_iState[it_idx]);
		bDisplay = true;
	} else if(!strcmp(item_str,GEN_FORCE_MENU)) {
		bDisplay = true;
		hMenu = CreateForceMenu(client);
	} else if(!strcmp(item_str,GEN_TOGGLE_DISABLE)) {
		bDisplay = true;
		set_disable_info(g_iState[it_array],g_iState[it_idx],
			!(get_disable_info(g_iState[it_array],g_iState[it_idx])));
	} else if(!strcmp(item_str,GEN_CLUSTER_MENU)) {
		hMenu = CreateClusterGeneratorMenu(client);
	}

	if(g_iState[it_array] != g_Player[WorkingSet])
		g_bChanged = true;
 
	if(hMenu == INVALID_HANDLE)
		hMenu = CreateGeneratorMenu(client);

	if(bDisplay)
  	DisplayGenToClient(client,g_iState[it_array], g_iState[it_idx]);
  return hMenu;
}



public Handler_GeneratorMenu(Handle:menu, MenuAction:m_action, param1, param2)
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
			new Handle:menu_next = ClientSelectGeneratorMenu(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; failed to recreate next menu.");
		}
		case MenuAction_Cancel:
		{
			LOG_MENU_CANCEL
			if(param2 == MenuCancel_ExitBack)
			{
				new Handle:menu_next = CreateIterateMenu(client);
				DisplayMenu(menu_next,client,MENU_TIME_FOREVER);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
}

public Handler_ModifyForceMenu(Handle:menu, MenuAction:m_action, param1, param2)
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
			LOG_MENU_SELECT_CUSTOM(" Modify Force")
			new String:action[MAX_MENU_LEN];
			GetMenuItem(menu,param2,action,sizeof(action));
			new Handle:menu_next = ClientSelectedForceMenu(client,action);
			if(!menu_next || !DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				PrintToConsole(client,"warning; error modifying force");
		}
		case MenuAction_Cancel:
		{
			if(MenuCancel_ExitBack)
			{
				new Handle:hMenu = CreateGeneratorMenu(client);
				if(hMenu)
					DisplayMenu(hMenu,client,MENU_TIME_FOREVER);
				else
					DisplayMainMenu(client);
			}
		}
		case MenuAction_End:
		{
			CloseHandle(menu);
		}
	}
}


#endif
