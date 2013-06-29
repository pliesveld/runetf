#if !defined __menu_create__
#define __menu_create__
#include <menus.inc>

Handle:CreateClusterGeneratorMenu(client)
{
	if(!client)
		return INVALID_HANDLE;
	new size;
	if( (size = GetArraySize(g_ClusterNames)) == 0)
	{
		PrintToConsole(client,"no clusters found! use sm_gen cluster create <name>");
		return INVALID_HANDLE;
	}
	new Handle:hMenu = CreateMenu(Handler_ClusterGeneratorMenu,RUNE_MENU_ACTIONS_DEFAULT|MenuAction_DrawItem|MenuAction_Cancel);

	new cur_id = -1; 
	if(IsValidIter(g_iState[it_array],g_iState[it_idx]))
		cur_id = get_id_info(g_iState[it_array], g_iState[it_idx]);

	for(new i; i < size;++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetArrayString(g_ClusterNames,i,s_cluster,sizeof(s_cluster));
		if(cur_id == -1 || !DoesClusterHaveId(s_cluster,cur_id) || !InsertMenuItem(hMenu, 0, s_cluster, s_cluster))
			AddMenuItem(hMenu,s_cluster,s_cluster);
	}

	SetMenuExitBackButton(hMenu,true);
	return hMenu;	
}

Handle:CreateClusterDefaultMenu(client)
{
	new size;
	if(client <= 0)
		return INVALID_HANDLE;

	new Handle:hMenu = CreateMenu(Handler_ClusterDefaultMenu,RUNE_MENU_ACTIONS_DEFAULT|MenuAction_Cancel);

	if(g_Player[DefaultCluster] != INVALID_HANDLE
	&& (size = GetArraySize(g_Player[DefaultCluster])) != 0) {
		AddMenuItem(hMenu,"%%CLEAR%%","Clear");
	}

	for(new i;i < size;++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN]="";
		GetArrayString(g_Player[DefaultCluster],i, s_cluster,sizeof(s_cluster))
		AddMenuItem(hMenu,s_cluster,s_cluster);
	}

	size = GetArraySize(g_ClusterNames);
	for(new i; i < size;++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN]="";
		GetArrayString(g_ClusterNames,i,s_cluster,sizeof(s_cluster));
		if(g_Player[DefaultCluster] != INVALID_HANDLE && FindStringInArray(g_Player[DefaultCluster],s_cluster) != -1)
			continue;
		AddMenuItem(hMenu,s_cluster,s_cluster);
	}
	SetMenuExitBackButton(hMenu,true);

	return hMenu;
}

Handle:CreateClusterSpawnGeneratorMenu(client)
{
	new size;
	if( (size = GetArraySize(g_ClusterNames)) == 0)
	{
		PrintToConsole(client,"no clusters found! use sm_gen cluster create <name>");
		return INVALID_HANDLE;
	}
	new Handle:hMenu = CreateMenu(Handler_ClusterSpawnGeneratorMenu,RUNE_MENU_ACTIONS_DEFAULT|MenuAction_Cancel);

	for(new i; i < size;++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetArrayString(g_ClusterNames,i,s_cluster,sizeof(s_cluster));
		AddMenuItem(hMenu,s_cluster,s_cluster);
	}

	SetMenuExitBackButton(hMenu,true);
	return hMenu;	
}

stock BuildClusterNameMenu(&Handle:hMenu)
{
	new size;
	if( (size = GetArraySize(g_ClusterNames)) == 0)
	{
		SetMenuTitle(hMenu,"No clusters.  use sm_gen cluster create");
		return;
	}

	for(new i; i < size;++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetArrayString(g_ClusterNames,i,s_cluster,sizeof(s_cluster));
		AddMenuItem(hMenu,s_cluster,s_cluster);
	}

}

Handle:CreateClusterDropGeneratorMenu(client)
{
	if(!client)
		return INVALID_HANDLE;
	new Handle:hMenu = CreateMenu(Handler_ClusterDropGeneratorMenu,RUNE_MENU_ACTIONS_DEFAULT|MenuAction_DrawItem);
	BuildClusterNameMenu(hMenu);

	SetMenuExitBackButton(hMenu,true);
	return hMenu;	
}

Handle:CreateClusterLoadGeneratorMenu(client)
{
	if(!client)
		return INVALID_HANDLE;
	new Handle:hMenu = CreateMenu(Handler_ClusterLoadGeneratorMenu,RUNE_MENU_ACTIONS_DEFAULT|MenuAction_DrawItem);
	BuildClusterNameMenu(hMenu);

	SetMenuExitBackButton(hMenu,true);
	return hMenu;	
}

#endif
