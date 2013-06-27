/// Temporary RuneCluster
#include <smlib/entities>
static t_Cluster[RuneCluster];
const CLUSTER_BLOCK_SIZE = sizeof(t_Cluster)

static t_Event[GenEvent];
const EVENT_BLOCK_SIZE = sizeof(t_Event)


public HandleGenEvent(const String:output[], caller, activator, Float:delay)
{
	LogMessage("HandleGenEvent");
}

ResetTempCluster(a_Cluster[RuneCluster])
{
	a_Cluster[ClusterName][0] = '\0';
	a_Cluster[EventArray] = INVALID_HANDLE;
}

ResetTempEvent(a_Event[GenEvent])
{
	a_Event[ClusterType] = ClusterGenType:0;
	a_Event[EventType] = EventHookType:0;
	a_Event[EventEntity][0] = '\0';
	a_Event[EventCName][0] = '\0';
	a_Event[EventOutput][0] = '\0';
	a_Event[EventTriggered] = EntityOutput:HandleGenEvent;
}

SetClusterName(a_Cluster[RuneCluster], const String:name[])
{
	strcopy( a_Cluster[ClusterName], sizeof(a_Cluster[ClusterName]), name);
}

SetEntityName(a_Event[GenEvent], const String:name[])
{
	strcopy( a_Event[EventEntity], sizeof(a_Event[EventEntity]),name);
}

SetEventCName(a_Event[GenEvent], const String:cname[])
{
	strcopy( a_Event[EventCName], sizeof(a_Event[EventCName]),cname);
}

SetEventOutput(a_Event[GenEvent], const String:output[])
{
	strcopy( a_Event[EventOutput], sizeof(a_Event[EventOutput]),output);
}


SetEventCallback(a_Event[GenEvent], EntityOutput:callback)
{
	a_Cluster[GenEvent] = callback;
}

stock bool:IsEventSame(lhs[GenEvent], rhs[GenEvent])
{
	new cType;
	if ((cType = lhs[ClusterType]) == rhs[ClusterType] && cType != ClusterGenType:GenEvent_None)
		if(!strcmp(lhs[EventCName],rhs[EventCName]) && lhs[EventCName][0] != '\0')
			if(!strcmp(lhs[EventOutput],rhs[EventOutput]) && lhs[EventOutput][0] != '\0')
				if( (lhs[EventType] == rhs[EventType] && lhs[EventType] != EventType:GenEvent_None))
				{
					if(lhs[EventType] == GenEvent_ByEnt)
						return (!strcmp(lhs[EventEntity],rhs[EventEntity]) && lhs[EventEntity][0] != '\0');
					return true;
				}
		
			
	return false;
}

stock CreateCluster(const String:name[], &Handle:a_cluster)
{
	new idx;

	if( (idx = FindStringInArray(g_ClusterNames, name)) == -1)
	{
		PushArrayString(g_ClusterNames, name);
#if defined DEBUG_GEN_CLUSTER
		LogMessage("Created cluster %s",name);
#endif
	}

	if(!GetTrieValue(g_vGenCluster, name, a_cluster))
	{
		a_cluster = CreateArray(1);
		SetTrieValue(g_vGenCluster, name, a_cluster);
	}
}

stock AddToCluster(const String:name[], id, bool:create = false)
{
	new Handle:c_array;
	new ret = -1;
	
	if(!GetTrieValue(g_vGenCluster, name, c_array))
	{
		if(!create)
			return ret;
			CreateCluster(name, c_array);
	}
	if(FindValueInArray(c_array,id) == -1)
		ret = PushArrayCell(c_array, id);

	return ret;
}

stock CopyClusterEvents( Handle:dest_array, &Handle:src_array, bool:erase = true)
{
	new ret;
	if(src_array == INVALID_HANDLE)
		return ret;
	for(new i; i < GetArraySize(src_array);++i)
	{
		new a_Event[GenEvent];
		GetArrayArray(src_array,i,a_Event);
		PushArrayArray(dest_array,a_Event);
		++ret;
	}
	if(erase)
	{
		ClearArray(src_array);
		CloseHandle(src_array);
		src_array = INVALID_HANDLE;
	}
	return ret;
}

stock RemoveFromArrayId(Handle:c_array, id)
{
	new ret = -1;
	if( (ret = FindValueInArray(c_array, id)) != -1)
		RemoveFromArray(c_array,ret);

	return ret;
}

stock bool:AcceptEntity(String:CName[], String:EventEnt[])
{
	if(!strcmp(CName,"trigger_capture_area"))
	{

#if defined DEBUG_GEN_EVENT
		LogMessage("AcceptEntity CName %s:%s",CName,EventEnt);
#endif
		return (Entity_FindByName(EventEnt) != INVALID_ENT_REFERENCE);
	}
	return false;
}


stock RemoveFromCluster(const String:name[], id)
{
	new Handle:c_array;
	new ret;
	if(GetTrieValue(g_vGenCluster, name, c_array))
		if( (ret = RemoveFromArrayId(c_array,id)) != -1)
		{
			if(GetArraySize(c_array) == 0)
			{
#if defined DEBUG_GEN_CLUSTER
				LogMessage("Cluster %s now has no entries.\n", name);
#endif
				//RemoveFromTrie(g_vGenCluster, name)
			}
		}

	return ret;
}

stock RemoveFromAllClusters(id)
{
	new ret;
	for(new i; i < GetArraySize(g_ClusterNames);++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetArrayString(g_ClusterNames,i,s_cluster,sizeof(s_cluster));
		if(RemoveFromCluster(s_cluster, id) != -1)
			++ret;
	}

	return ret;
}

stock bool:DoesClusterHaveId(const String:name[], id)
{
	new Handle:c_array;
	if(GetTrieValue(g_vGenCluster, name, c_array))
		return FindValueInArray(c_array, id) != -1;
}

add_event_info(&Handle:cluster, a_Event[GenEvent])
{
	if(a_Event[EventCName][0] == '\0')
	{
		LogMessage("warning; no cname entry for event");
		return -1;
	}
	new class_idx;

	if(!GetTrieValue(g_CNameTrie, a_Event[EventCName],class_idx))
	{
		LogMessage("warning; cname %s is not an acceptable class\n",a_Event[EventCName]);
		return -1;
	}

	if(CheckOutput(a_Event[EventCName], a_Event[EventOutput]) == -1)
	{
		LogMessage("warning; cname %s has invalid output %s", a_Event[EventCName], a_Event[EventOutput]);
		DisplayEntOutputs(a_Event[EventCName]);
		return -1;
	}

	new EntityOutput:event_handler = INVALID_FUNCTION;

	switch( a_Event[ClusterType] )
	{

		case GenEvent_Add,GenEvent_Remove,GenEvent_Toggle,GenEvent_Spawn:
		{
			event_handler = actionHandler[ a_Event[ClusterType] ];
		}
		default:
		{
			LogMessage("warning; event cluster has invalid action type");
			return -1;
		}
	}

	switch( a_Event[EventType] )
	{
		case GenEvent_ByClass:
		{
			if(FindEntityByClassname(-1, a_Event[EventCName]) == -1)
			{
				LogMessage("warning; no entity found with class name %s",a_Event[EventCName]);
				return -1;
			}
		}
		case GenEvent_ByEnt:
		{
			new ent;
			if(a_Event[EventEntity][0] == '\0')
			{

				LogMessage("warning; event type ByEnt, but no entity name specified");
				return -1;
			}
			if(AcceptEntity(a_Event[EventCName],a_Event[EventEntity]))
			{
			
			} else if( (ent = Entity_FindByName(a_Event[EventEntity])) == INVALID_ENT_REFERENCE)
				{
				LogMessage("warning; no entity found named %s", a_Event[EventEntity]);
				return -1;
			}
			if(ent && IsValidEntity(ent))
			{
				new String:cname[MAX_EVENTGEN_CNAME];
				if(!GetEntityClassname(ent,cname,sizeof(cname)))
				{
					LogMessage("warning; ent %s has invalid class", a_Event[EventEntity]);
					return -1;
				} else if(!StrEqual(class_type[class_idx],cname)) {
					LogMessage("warning; ent %s is not of class %s", a_Event[EventEntity], class_type[class_idx]);
					return -1;
				}
			}
		}
		case GenEvent_BySubClass:
		{
			LogMessage("warning; event entity type Event_BySubClass not supported.");
			return -1;
		}
		default:
		{
			LogMessage("warning; event entity has invalid type");
			return -1;
		}
	}
		
	a_Event[EventTriggered] = event_handler;

	if(cluster == INVALID_HANDLE)
		cluster = CreateArray(EVENT_BLOCK_SIZE);

	return PushArrayArray(cluster, a_Event, EVENT_BLOCK_SIZE);
}

add_cluster_info(&Handle:cluster, a_Cluster[RuneCluster])
{
	if(a_Cluster[ClusterName][0] == '\0')
	{
		LogMessage("warning; event cluster has invalid name");
		return -1;
	}

	new Handle:array_ids;
	CreateCluster( a_Cluster[ClusterName],array_ids);

	new Handle:event_arr = INVALID_HANDLE;
	new idx; 
	if( (idx = FindEventClusterInArray( a_Cluster[ClusterName], event_arr,cluster)) != -1 )
	{
		CopyClusterEvents( event_arr, a_Cluster[EventArray]);
		return idx;
	}
	

	return PushArrayArray(cluster, a_Cluster, CLUSTER_BLOCK_SIZE);

}
stock FreeCluster(const String:name[])
{
	new Handle:array;
	new idx;
	new bool:found = false;

	idx = FindStringInArray(g_ClusterNames,name);
	RemoveFromArray(g_ClusterNames, idx);

	if( idx == -1 )
		LogMessage("Warning; cluster %s invalid", name);

	if(!GetTrieValue(g_vGenCluster, name, array))
	{
		LogMessage("Warning; cluster %s found in named array but not trie", name);
	 	return;
	}

	CloseArray(array);
	RemoveFromTrie(g_vGenCluster, name);

/*
	Also iterate Event cluster_name
*/

}

stock CloseClusters()
{
	new String:c_name[MAX_CLUSTER_LEN];
	if(g_ClusterNames == INVALID_HANDLE)
		return;
	while( GetArraySize(g_ClusterNames) > 0)
	{
		GetArrayString(g_ClusterNames, 0,c_name,sizeof(c_name));
		FreeCluster(c_name);
	}
	
#if defined DEBUG_GEN_CLUSTER
	new s;
	if((s=GetTrieSize(g_vGenCluster)) != 0)
		LogMessage("CloseCluster:: g_vGenCluster should be 0 not %d",s);
#endif
	CloseHandle(g_ClusterNames);
	g_ClusterNames = INVALID_HANDLE;
	CloseHandle(g_vGenCluster);
	g_vGenCluster = INVALID_HANDLE;
}


stock RegCreateCmds()
{
	RegAdminCmd("sm_cluster", OnClusterCmd, ADMFLAG_CONFIG, "Cluster manipulation");
}

public Action:OnClusterCmd(client, args)
{
	new String:buffer[1024];
	PrintToConsole(client,"ClusterSize arr %d trie %d events %d",
		GetArraySize(g_ClusterNames), GetTrieSize(g_vGenCluster), g_vGenEvent == INVALID_HANDLE ? -1 : GetArraySize(g_vGenEvent));
	new bool:found = false;
	decl String:s_cluster[MAX_CLUSTER_LEN]="";
	if(GetCmdArgs() > 0)
	{
		GetCmdArg(1,s_cluster,sizeof(s_cluster));
	}
	
	for(new i; i < GetArraySize(g_ClusterNames);++i)
	{
		new String:cname[MAX_CLUSTER_LEN];
		GetArrayString(g_ClusterNames, i, cname, sizeof(cname));
		new len = Format(buffer, sizeof(buffer), "Cluster %-14s: ", cname);
		if(s_cluster[0] != '\0')
		{
			if(!strcmp(s_cluster,cname))
			{
				found = true;
			} else
				continue;
		}
		PrintToConsole(client,buffer);
		new Handle:array;
		if(GetTrieValue(g_vGenCluster,cname,array))
		{
			PrintArray(array, client);
		}
	}

	if(s_cluster[0] != '\0' && !found)
	{
		PrintToConsole(client,"No genrators for cluster %s found.",s_cluster);
	}

	found = false;
	for(new i; i < GetArraySize(g_vGenEvent) && !found;i++)
	{
		new a_Cluster[RuneCluster];
		GetArrayArray(g_vGenEvent,i,a_Cluster, CLUSTER_BLOCK_SIZE);
		Format(buffer,sizeof(buffer), "Events for %s",a_Cluster[ClusterName]);
		if(s_cluster[0] != '\0')
		{
			if(!strcmp(s_cluster, a_Cluster[ClusterName]))
				found = true;
			else
				continue;
		}
		PrintToConsole(client,buffer);
		PrintEvents(client, a_Cluster[EventArray]);
	}

	if(s_cluster[0] != '\0' && !found)
	{
		PrintToConsole(client,"No events for cluster %s found.",s_cluster);
	}

	return Plugin_Continue;
}

stock PrintEvents(client, Handle:event_arr)
{
	new size;
	if(event_arr == INVALID_HANDLE || (size = GetArraySize(event_arr)) < 1)
	{
		PrintToConsole(client, "none.");
		return;
	}

	for(new i; i < size;++i)
	{
		new a_Event[GenEvent];
		GetArrayArray(event_arr, i, a_Event, EVENT_BLOCK_SIZE);
		decl String:buffer[1024];
		Format(buffer,sizeof(buffer),
		"%16s %16s: %s %s %s",
		actionString[ a_Event[ClusterType] ],
		entityString[ a_Event[EventType] ],
		a_Event[ EventCName ],
		a_Event[ EventEntity ],
		a_Event[ EventOutput ]);
		PrintToConsole(client, buffer);
	}
}

GetClustersContainingId(id, String:buffer[], buf_len, &start = 0)
{
	new size;
	if( (size = GetArraySize(g_ClusterNames)) < 1)
		return 0;
	new cnt;
	for(new i = start; i < size;++i)
	{
		decl String:cluster_name[MAX_CLUSTER_LEN];
		new s_len = GetArrayString(g_ClusterNames,i,cluster_name, sizeof(cluster_name));
		new Handle:c_array;
		if(!GetTrieValue(g_vGenCluster, cluster_name, c_array))
			continue;
		if(FindValueInArray(c_array, id) == -1)
			continue;
		if(s_len + strlen(buffer) + 1> buf_len)
		{
			start = i;
			return cnt;
		}
		if(cnt)
			StrCat(buffer, buf_len, ",");
		StrCat(buffer, buf_len, cluster_name);
		cnt++;
	}
	start = -1;
	return cnt;
}

public SMCResult:SMCNewSection_Cluster(Handle:smc, const String:name[], bool:opt_quotes)
{
	new SMC_KeyValue:handler_kv;
	new SMC_EndSection:handler_end;
	//LogMessage("Section_Cluster %s ======== ", name);
	switch(name[0])
	{
		case 'e':
		{
			if(!strcmp(name,"event"))
			{
				ResetTempEvent(t_Event);
				handler_kv = SMCReadKeyValues_Event;
				handler_end = SMCEndSection_Event;
			} else
				return SMCParse_Halt;
		}
		case 'c':
		{
			if(!strcmp(name,"cluster"))
			{
				ResetTempCluster(t_Cluster);
				handler_kv = SMCReadKeyValues_Cluster;
				handler_end = SMCEndSection_Cluster;
			} else
				return SMCParse_Halt;
		}
		default:
		{
			return SMCParse_Halt;
		}
	}

	SMC_SetReaders(smc, SMCNewSection_Cluster, handler_kv, handler_end);
	return SMCParse_Continue;
}


public SMCResult:SMCReadKeyValues_Cluster(Handle:smc, const String:key[], const String:value[], bool:key_quotes, bool:value_quotes)
{
	//LogMessage("Section::Cluster %s => %s", key, value);
	if(!strcmp(key,"cluster_name"))
	{
		ResetTempCluster(t_Cluster);
		if( t_Cluster[EventArray] == INVALID_HANDLE )
		{
			t_Cluster[EventArray] = CreateArray(EVENT_BLOCK_SIZE);
		}
		strcopy(t_Cluster[ClusterName], MAX_CLUSTER_LEN, value);
		return SMCParse_Continue;
	}
	LogMessage("Unexpected key %s", key);
	return SMCParse_Halt;
}


public SMCResult:SMCEndSection_Cluster(Handle:smc)
{
	//LogMessage("EndSection_Cluster");
	add_cluster_info(g_vGenEvent,t_Cluster);
	SMC_SetReaders(smc, SMCNewSection, SMCReadKeyValues_Generator, SMCEndSection_Generator);
}



public SMCResult:SMCReadKeyValues_Event(Handle:smc, const String:key[], const String:value[], bool:key_quotes, bool:value_quotes)
{
	//LogMessage("Section::Event %s => %s", key, value);
	switch (key[0])
	{
		case 'a':
		{
			if(!strcmp(key,"action"))
			{
				new idx;
				if( (idx = CheckClusterType(value)) != -1)
				{
					t_Event[ClusterType] = idx;
					return SMCParse_Continue;
				}
			}
				
			LogMessage("Invalid event action %s: ", value);
			return SMCParse_Halt;
		}
		case 't':
		{
			if(!strcmp(key,"type"))
			{
				new idx;
				if( (idx = CheckEntityType(value)) != -1 )
				{
					t_Event[EventType] = idx;
					return SMCParse_Continue;
				}
			}
			LogMessage("Invalid event entity %s: ", value);
			return SMCParse_Halt;
		}
		case 'c':
		{
			if(!strcmp(key,"cname"))
			{
				SetEventCName(t_Event,value);
				return SMCParse_Continue;
			}
		}
		case 'e':
		{
			if(!strcmp(key,"entity"))
			{
				SetEntityName(t_Event,value);
				return SMCParse_Continue;
			}
		}
		case 'o':
		{
			if(!strcmp(key,"output"))
			{
				SetEventOutput(t_Event,value);
				return SMCParse_Continue;
			}
		}
	}
	LogError("Invalid key %s", key);
	return SMCParse_Halt;
}

public SMCResult:SMCEndSection_Event(Handle:smc)
{
	//LogMessage("EndSection_Event");
	add_event_info(t_Cluster[EventArray], t_Event);
	SMC_SetReaders(smc, SMCNewSection_Cluster, SMCReadKeyValues_Cluster, SMCEndSection_Cluster);
}


