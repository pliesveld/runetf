
#include "generator_stock.inc"

#if 0 
static String:iter_action[][MAX_MENU_LEN] =
{
	ITER_TYPE_GLOBAL,
	ITER_TYPE_DISABLED,
	ITER_TYPE_WORKING
}
static String:iter_text[][32] =
{
	"active",
	"disabled",
	"workingset"
};
#endif

stock CreateIteratorMenu(client)
{
	new Handle:hMenu;
	hMenu = CreateMenu(Handler_SelectIterType, RUNE_MENU_ACTIONS_DEFAULT);
	//SetMenuExitButton(hMenu, true);
	SetMenuTitle(hMenu,"Select iterator type.");

	AddMenuItem(hMenu,ITER_TYPE_GLOBAL,"active", 		  !IsValidIter(g_vGen,0) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(hMenu,ITER_TYPE_DISABLED,"disabled",  !IsValidIter(g_vGenDisabled,0) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
	AddMenuItem(hMenu,ITER_TYPE_WORKING, "workingset",!IsValidIter(g_Player[WorkingSet],0) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);

	return hMenu;
}

public Handler_SelectIterType(Handle:menu, MenuAction:m_action, param1, param2)
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
			decl String:action[24];
			GetMenuItem(menu, param2, action, sizeof(action));
			
			new Handle:menu_next = ClientSelectedIteratorType(client,action);
			if(menu_next && DisplayMenu(menu_next,client, MENU_TIME_FOREVER))
				SetEntityMoveType(client,MOVETYPE_NOCLIP);
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
}

Handle:ClientSelectedIteratorType(client, const String:action[])
{
	new Handle:menu_next;
	if(!strcmp(action,ITER_TYPE_GLOBAL))
	{
		g_iState[it_array] = g_vGen;
	} else if(!strcmp(action,ITER_TYPE_DISABLED)) {
		g_iState[it_array] = g_vGenDisabled;
	} else if(!strcmp(action,ITER_TYPE_WORKING)) {
		g_iState[it_array] = g_Player[WorkingSet];
	} else {
		PrintToConsole(client,"Unknown action %s", action);
		return CreateMainMenu(client);
	}
	menu_next = CreateIterateMenu(client);


//	if(IterateTeleportPlayer(client, g_iState[it_array], g_iState[it_idx]) == -1)	return menu_next;
   //DisplayGenToClient(client,g_iState[it_array], g_iState[it_idx]);         

	return menu_next;

}


