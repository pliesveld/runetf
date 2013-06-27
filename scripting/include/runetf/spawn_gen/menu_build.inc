	

stock BuildClusterMenu(&Handle:menu)
{
	new size,ret;
	if(g_ClusterNames == INVALID_HANDLE)
		return 0;
	if((size = GetArraySize(g_ClusterNames)) == 0)
		return 0;

	SetMenuTitle(menu,"Select a cluster");

	for(new i; i < size;++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetArrayString(g_ClusterNames, i, s_cluster, sizeof(s_cluster));
		AddMenuItem(menu, s_cluster, s_cluster);
		++ret;
	}
	return ret;
}

stock BuildEventClusterMenu(&Handle:menu)
{
	new size, ret;
	if(g_vGenEvent == INVALID_HANDLE || (size = GetArraySize(g_vGenEvent)) == 0)
		return 0;

	for(new i; i < size;++i)
	{
		new a_cluster[RuneCluster];
		GetArrayArray(g_vGenEvent, i, _:a_cluster, CLUSTER_BLOCK_SIZE);
		AddMenuItem(menu, a_cluster[ClusterName], a_cluster[ClusterName]);
		++ret;
	}
	return ret;
}

stock BuildEventMenu(&Handle:menu,const String:s_cluster[])
{
	new size;
	if(g_vGenEvent == INVALID_HANDLE || (size = GetArraySize(g_vGenEvent)) == 0)
		return 0;
	for(new i; i < size;++i)
	{
		new a_cluster[RuneCluster];
		GetArrayArray(g_vGenEvent, i, a_cluster, CLUSTER_BLOCK_SIZE);
		if(!strcmp(s_cluster, a_cluster[ClusterName]))
			return AddEventMenu(menu, a_cluster[EventArray]);
			
	}
	return 0;
}



stock AddEventMenu(&Handle:menu, Handle:event_arr)
{
	new size,ret;
	if(event_arr == INVALID_HANDLE || (size = GetArraySize(event_arr)) == 0)
		return 0;

	for(new i; i < size;++i)
	{
		new a_event[GenEvent];
		GetArrayArray(event_arr,i,a_event,EVENT_BLOCK_SIZE);
		new String:event[MAX_EVENT_ENTITY_OUTPUT];
		if( a_event[Event_Type] == GenEvent_ByEnt )
			StrCopy(event, sizeof(event), a_event[EventEntity]);
		else
			StrCopy(event, sizeof(event), a_event[EventCName]);

		StrCat(event,sizeof(event),".");
		StrCat(event,sizeof(event),a_event[EventOutput]);
		StrCat(event,sizeof(event),"() ");
		StrCat(event,sizeof(event), actionString[ a_event[ClusterType] ][9]);
		AddMenuItem(menu, event, event);

	}
}





stock BuildGenIdFromClusterMenu(&Handle:menu,const String:s_cluster[])
{
	new ret;
	new Handle:array_ids;
	if(!GetTrieValue(g_vGenCluster,s_cluster,array_ids))
		return 0;

	new size;
	if(array_ids == INVALID_HANDLE || (size = GetArraySize(array_ids)) == 0)
		return 0;

	for(new i; i < size;++i)
	{
		decl String:s_id[MAX_EVENT_GEN_NAME];
		Format(s_id,sizeof(s_id),"%d", GetArrayCell(array_ids, i));
		AddMenuItem(menu, s_id, s_id);
	}
	return 1;
}

stock BuildCNameMenu(&Handle:menu)
{
	new ret;
	if(g_MapEventEntities == INVALID_HANDLE)
		return 0;
	for(new i; i < sizeof(class_type);++i)
	{
		new count = GetArrayCell(g_MapEventEntities,i,0);
		if(!count)
			continue;
	
		AddMenuItem(menu, class_type[i], class_type[i]);
		++ret;
	}
	return ret;
}

stock BuildEntityOutputMenu(&Handle:menu, const String:name[], bool:classtype = true)
{
	new ret;
	new class_idx;
	new String:cname[MAX_EVENTGEN_CNAME];
	if(classtype)
		strcopy(cname, sizeof(cname), name);
	else
	{
		new ent;
		if ( (ent = Entity_FindByName(name)) == INVALID_ENT_REFERENCE)
			return 0;
		if(!Entity_GetClassName(ent,cname,sizeof(cname)))
			return 0;
	}	

	if(!GetTrieValue(g_CNameTrie, cname, class_idx))
		return 0;

	if(g_EntOutTrie == INVALID_HANDLE)
		return 0;

	new Handle:out_arr;

	if(!GetTrieValue(g_EntOutTrie, cname, out_arr))
		return 0;

	if(out_arr == INVALID_HANDLE || GetArraySize(out_arr) == 0)
		return 0;

	for(new i; i < GetArraySize(out_arr);++i)
	{
		decl String:s_output[MAX_EVENT_OUTPUTNAME];
		GetArrayString(out_arr, i, s_output, sizeof(s_output));
		AddMenuItem(menu, s_output, s_output);
		++ret;
	}

	return ret;
}




