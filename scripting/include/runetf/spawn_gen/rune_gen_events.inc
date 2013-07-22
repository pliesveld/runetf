#include <runetf/runes_stock/mergesort>

static Handle:g_HookedEvents[ClusterGenType];// = {INVALID_HANDLE,...} // <--- why is this a tag match
static Handle:g_EventHandleArray = INVALID_HANDLE;

stock MapEntityFound(ent, String:class[], idx, Handle:DEPRECATED_array_ent, &Handle:array_names)
{ //ent is of type class #i of max
	//new ent_ref;
	decl String:s_name[MAX_EVENTGEN_CNAME]="";
	if(!strcmp(class,"trigger_capture_area"))
	{
		GetEntPropString(ent,Prop_Data,"m_iszCapPointName",s_name,sizeof(s_name));
		Format(s_name,sizeof(s_name),"%s",s_name);
	} else {
		Entity_GetName(ent, s_name,sizeof(s_name));
	}
#if defined DEBUG_GEN_EVENT
	LogMessage("item found: %s %d", s_name,ent);
#endif
	SetArrayString(array_names, idx, s_name);
}

stock ProcessMapEntities()
{
	if(g_EntOutTrie != INVALID_HANDLE)
	{
		CloseTrieOutputTypes(g_EntOutTrie);
	}
	g_EntOutTrie = InitEntityOutputs();

	FreeHookedEvents();


	new ent;
	if(g_MapEventEntities != INVALID_HANDLE)
		FreeMapEventEntities(g_MapEventEntities);

	if(g_CNameTrie == INVALID_HANDLE)
		g_CNameTrie = CreateTrie();
		
	g_MapEventEntities = CreateArray(3,sizeof(class_type));

	for(new i; i < sizeof(class_type);++i)
	{
		new count = 0;
		SetTrieValue(g_CNameTrie, class_type[i], i);
		ent = -1;
		while((ent = FindEntityByClassname(ent,class_type[i])) != -1)
			++count;
		SetArrayCell(g_MapEventEntities,i, count);
		new Handle:action_ref_array = CreateArray(2,ClusterGenType);
		for(new j;j < GetArraySize(action_ref_array);++j)
		{
			SetArrayCell(action_ref_array,j,CreateTrie(),0); //cname.output
			SetArrayCell(action_ref_array,j,CreateTrie(),1); //entity.output
		}
		SetArrayCell(g_MapEventEntities,i, action_ref_array,1);
		SetArrayCell(g_MapEventEntities,i, INVALID_HANDLE,2);
	}

	for(new i; i < sizeof(class_type);++i)
	{
		new count = GetArrayCell(g_MapEventEntities,i);
		if(count)
		{
#if defined DEBUG_GEN_EVENT
			LogMessage("%32s: %d", class_type[i], count);
#endif
			new j = 0;
			//new Handle:mapEntities = CreateArray(1,count);
			new Handle:mapEntNames = CreateArray(MAX_EVENTGEN_CNAME,count);
			ent = -1;
			while((ent = FindEntityByClassname(ent,class_type[i])) != -1)
				MapEntityFound(ent, class_type[i], j++, INVALID_HANDLE, mapEntNames);
			//SetArrayCell(g_MapEventEntities, i,mapEntities, 1);
			SetArrayCell(g_MapEventEntities, i,mapEntNames, 2);
		} else {
			new Handle:t_unused_array;
#if defined DEBUG_GEN_EVENT
			LogMessage("%32s: no entity found", class_type[i]);
#endif
			if(GetTrieValue(g_EntOutTrie, class_type[i], t_unused_array))
			{
				if(t_unused_array != INVALID_HANDLE)
				{
					CloseHandle(t_unused_array);
					SetTrieValue(g_EntOutTrie, class_type[i], INVALID_HANDLE);
				}
			}
		}
	}

	//CloseHandle(g_MapEventEntities);
}


stock FreeMapEventEntities(&Handle:array)
{
	new Handle:array1 = INVALID_HANDLE, Handle:array2 = INVALID_HANDLE;
	if(array == INVALID_HANDLE)
	{
		LogError("Warning; FreeMapEventEntities invalid array");
		return;
	}
	while(GetArraySize(array) > 0)
	{
		array1 = GetArrayCell(array, 0, 1);
		for(new j; j < GetArraySize(array1);++j)
		{
			new Handle:named_ent_trie;
			named_ent_trie = GetArrayCell(array1,j,1);
			if(named_ent_trie != INVALID_HANDLE)
				CloseHandle(named_ent_trie);
			named_ent_trie = GetArrayCell(array1,j,0);
			if(named_ent_trie != INVALID_HANDLE)
				CloseHandle(named_ent_trie);
		}
		array2 = GetArrayCell(array, 0, 2);
		CloseArray(array1);
		CloseArray(array2);
		RemoveFromArray(array,0);
	}
}


stock FreeHookedEvents()
{
	if(g_EventHandleArray != INVALID_HANDLE)
	{
		for(new i; i < GetArraySize(g_EventHandleArray);++i)
		{
			new Handle:clusters = GetArrayCell(g_EventHandleArray,i);
			CloseHandle(clusters);
		}
		ClearArray(g_EventHandleArray);
	}
	else
		g_EventHandleArray = CreateArray();

	for(new i = 1;i < ClusterGenType;++i)
	{
		if( g_HookedEvents[i] != INVALID_HANDLE)
		{
			ClearTrie(g_HookedEvents[i]);
			CloseHandle(g_HookedEvents[i]);
			g_HookedEvents[i] = INVALID_HANDLE;
		}
	}
}

stock RemoveEventHook(const String:event[], action = -1)
{
	new String:cname[MAX_EVENTGEN_CNAME];
	new String:output[MAX_EVENTGEN_CNAME];
	new idx;

	if( (idx = FindCharInString(event,'.')) == -1)
	{
		LogError("warning; malformed event");
		return -1;
	}
	
	strcopy(cname,idx,event);
	strcopy(output,sizeof(output),event[idx+1]);


	if(action == -1)
		return UnhookEntityOutput(cname, output, actionHandler[action]);

	new cnt;
	for(new i = 1; i < ClusterGenType;++i)
		if(UnhookEntityOutput(cname, output, actionHandler[i]))
			++cnt;
	return cnt;
}

	
stock FindMapEntity(const String:entity[], &class_idx = 0)
{
	new idx;
	for(idx = 0; idx < sizeof(class_type);++idx)
	{
		new Handle:ent_names_arr = GetArrayCell(g_MapEventEntities,idx,2);
		new cnt = GetArrayCell(g_MapEventEntities,idx,0);
		for(new j; j < cnt;++j)
		{
			decl String:t_ent_name[MAX_EVENTGEN_CNAME];
			GetArrayString(ent_names_arr,j,t_ent_name,sizeof(t_ent_name));
			if(!strcmp(t_ent_name,entity))
			{
				class_idx = idx;
				return j;
			}
		}
	}
	return -1;
}


	

stock HookMapEvents()
{
	HookEvent("teamplay_round_start", Event_Round);
	HookEvent("teamplay_round_win",Event_Round);
	HookEvent("teamplay_round_active",Event_Round);
	HookEvent("teamplay_round_selected",Event_Round);
	HookEvent("teamplay_waiting_begins",Event_Round);
	HookEvent("teamplay_waiting_abouttoend",Event_Round);
	HookEvent("teamplay_waiting_ends",Event_Round);
	HookEvent("teamplay_restart_round",Event_Round);

	HookEvent("teamplay_point_captured", Event_Point);
	HookEvent("teamplay_point_locked", Event_Point);
	HookEvent("teamplay_point_unlocked", Event_Point);


	HookEvent("teamplay_flag_event", Event_Flag);
	HookEvent("ctf_flag_captured", Event_Flag_Captured);
}

stock UnhookMapEvents()
{
#if 0
	UnhookEvent("teamplay_round_start", Event_Round);
	UnhookEvent("teamplay_round_win",Event_Round);
	UnhookEvent("teamplay_round_active",Event_Round);
	UnhookEvent("teamplay_round_selected",Event_Round);
	UnhookEvent("teamplay_waiting_begins",Event_Round);
	UnhookEvent("teamplay_waiting_abouttoend",Event_Round);
	UnhookEvent("teamplay_waiting_ends",Event_Round);
	UnhookEvent("teamplay_restart_round",Event_Round);

	UnhookEvent("teamplay_point_captured", Event_Point);
	UnhookEvent("teamplay_point_locked", Event_Point);
	UnhookEvent("teamplay_point_unlocked", Event_Point);


	UnhookEvent("teamplay_flag_event", Event_Flag);
	UnhookEvent("ctf_flag_captured", Event_Flag_Captured);
#endif
}


public Action:Event_Round(Handle:event, const String:strName[], bool:bDontBroadcast)
{
	decl String:eName[80]="";
	GetEventName(event,eName,sizeof(eName));
#if defined DEBUG_GEN_EVENT
	LogMessage("Event_Round %s: %s", strName,eName);
#endif
	new startover = -1;
	new miniRound = -1;
	if(StrEqual(strName,"teamplay_round_start",false))
	{
		startover = GetEventInt(event,"fullreset");
		if(startover) 
		{ 
			if(g_RuneSpawn[bRoundStartClear])
				KillAllPlayerRunes();

#if defined DEBUG_GEN_EVENT
			LogMessage("round start: fullreset");
#endif
		} else {
#if defined DEBUG_GEN_EVENT
			LogMessage("round start: not full reset");
#endif
		}

	} else if(StrEqual(strName,"teamlpay_round_win",false)) {
		miniRound = GetEventInt(event, "full_round"); // 1 -> mini round, 0 -> full round

		if(miniRound && g_RuneSpawn[bRoundEndClear])
		{
			KillAllPlayerRunes();
		}
		
		if(miniRound) 
		{


#if defined DEBUG_GEN_EVENT
			LogMessage("round win: mini round");
#endif
		} else {
#if defined DEBUG_GEN_EVENT
			LogMessage("round win: full round");
#endif
		}

	} else if(StrEqual(strName, "teamplay_round_active",false)) {
		//players active, can move.
	} else if(StrEqual(strName, "teamplay_round_selected",false)) {
		decl String:round_name[80]="";
		GetEventString(event,"round",round_name,sizeof(round_name));
#if defined DEBUG_GEN_EVENT
		LogMessage("round select: %s ", round_name);
#endif
	}
	return Plugin_Continue;
}


public Action:Event_Point(Handle:event, const String:strName[], bool:bDontBroadcast)
{
	decl String:capStr[64]="";
	decl String:capPlayerStr[64]="";

	GetEventString(event,"cpname",capStr,sizeof(capStr)); // name of point

#if defined DEBUG_GEN_EVENT

	new iTeam = -1;
	new cpIdx = GetEventInt(event,"cp"); // index of point that was captured
	iTeam = GetEventInt(event,"team");

	LogMessage("CP Event %s: %s %d %s",strName,
		capStr, cpIdx, (iTeam == 1 ? "by Red" : iTeam == 2 ? "by Blue" : ""));
#endif
	


	GetEventString(event,"cappers", capPlayerStr,sizeof(capPlayerStr));
	if(strlen(capPlayerStr)) { }

	return Plugin_Continue;
}

public Action:Event_Flag_Captured(Handle:event, const String:strName[], bool:bDontBroadcast)
{
	new iTeam 	= GetEventInt(event,"capping_team");
	new iScore  = GetEventInt(event, "capping_team_score");
	if(iTeam) { }
	if(iScore) { }
}

public Action:Event_Flag(Handle:event, const String:strName[], bool:bDontBroadcast)
{
	new iUser    = GetEventInt(event,"player");
	new iCarrier = GetEventInt(event, "carrier");
	new eType    = GetEventInt(event,"eventtype"); 
		// 0 = pickup
	  // 1 = capture
		// 2 = defend
    // 3 = drop
	if( iUser && iCarrier && eType ) { }
}



stock Handle:InitEntityOutputs()
{
	new Handle:t_trie;
	t_trie = CreateTrie();
	LoadTrieOutputTypes(t_trie);
	return t_trie;
}

stock LoadTrieOutputTypes(&Handle:class_trie)
{
	new Handle:cl_team_control_point_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_team_control_point_outputs, "OnOwnerChangedToTeam1");
	PushArrayString(cl_team_control_point_outputs, "OnOwnerChangedToTeam2");
	PushArrayString(cl_team_control_point_outputs, "OnCapReset");
	PushArrayString(cl_team_control_point_outputs, "OnRoundStartOwnedByTeam1");
	PushArrayString(cl_team_control_point_outputs, "OnRoundStartOwnedByTeam2");
	PushArrayString(cl_team_control_point_outputs, "OnCapTeam1");
	PushArrayString(cl_team_control_point_outputs, "OnCapTeam2");
	PushArrayString(cl_team_control_point_outputs, "OnUnlocked");
	new Handle:cl_trigger_capture_area_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_trigger_capture_area_outputs, "OnStartTeam1");
	PushArrayString(cl_trigger_capture_area_outputs, "OnBreakTeam1");
	PushArrayString(cl_trigger_capture_area_outputs, "OnCapTeam1");
	PushArrayString(cl_trigger_capture_area_outputs, "OnStartTeam2");
	PushArrayString(cl_trigger_capture_area_outputs, "OnBreakTeam2");
	PushArrayString(cl_trigger_capture_area_outputs, "OnCapTeam2");
	PushArrayString(cl_trigger_capture_area_outputs, "OnStartCap");
	PushArrayString(cl_trigger_capture_area_outputs, "OnBreakCap");
	PushArrayString(cl_trigger_capture_area_outputs, "OnEndCap");
	new Handle:cl_team_control_point_master_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_team_control_point_master_outputs, "OnWonByTeam1");
	PushArrayString(cl_team_control_point_master_outputs, "OnWonByTeam2");
	new Handle:cl_team_control_point_round_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_team_control_point_round_outputs, "OnStart");
	PushArrayString(cl_team_control_point_round_outputs, "OnEnd");
	PushArrayString(cl_team_control_point_round_outputs, "OnWonByTeam1");
	PushArrayString(cl_team_control_point_round_outputs, "OnWonByTeam2");
	new Handle:cl_teamflag_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_teamflag_outputs, "OnReturn");
	PushArrayString(cl_teamflag_outputs, "OnPickup");
	PushArrayString(cl_teamflag_outputs, "OnPickupTeam1");
	PushArrayString(cl_teamflag_outputs, "OnPickupTeam2");
	PushArrayString(cl_teamflag_outputs, "OnDrop");
	PushArrayString(cl_teamflag_outputs, "OnCapture");
	PushArrayString(cl_teamflag_outputs, "OnCapTeam1");
	PushArrayString(cl_teamflag_outputs, "OnCapTeam2");
	new Handle:cl_func_capturezone_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_func_capturezone_outputs, "OnCapture")
	PushArrayString(cl_func_capturezone_outputs, "OnCapTeam1")
	PushArrayString(cl_func_capturezone_outputs, "OnCapTeam2")
	new Handle:cl_tf_logic_cp_timer_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_tf_logic_cp_timer_outputs, "OnCountdownStart");
	PushArrayString(cl_tf_logic_cp_timer_outputs, "OnCountdownStop");
	new Handle:cl_tf_logic_arena_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_tf_logic_arena_outputs, "OnArenaRoundStart")
	PushArrayString(cl_tf_logic_arena_outputs, "OnCapEnabled")
	new Handle:cl_tf_gamerules_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_tf_gamerules_outputs, "OnWonByTeam1");
	PushArrayString(cl_tf_gamerules_outputs, "OnWonByTeam2");
	new Handle:cl_game_round_win_outputs = CreateArray(MAX_EVENT_OUTPUTNAME)
	PushArrayString(cl_game_round_win_outputs, "OnRoundWin");
	new Handle:cl_team_round_timer_outputs = CreateArray(MAX_EVENT_OUTPUTNAME);
	PushArrayString(cl_team_round_timer_outputs, "On5MinRemain");
	PushArrayString(cl_team_round_timer_outputs, "On4MinRemain");
	PushArrayString(cl_team_round_timer_outputs, "On3MinRemain");
	PushArrayString(cl_team_round_timer_outputs, "On2MinRemain");
	PushArrayString(cl_team_round_timer_outputs, "On1MinRemain");
	PushArrayString(cl_team_round_timer_outputs, "On30SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "On10SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "On5SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "On4SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "On3SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "On2SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "On1SecRemain");
	PushArrayString(cl_team_round_timer_outputs, "OnRoundStart");
	PushArrayString(cl_team_round_timer_outputs, "OnFinished");
	PushArrayString(cl_team_round_timer_outputs, "OnSetupStart");
	PushArrayString(cl_team_round_timer_outputs, "OnSetupFinished");


	SetTrieValue(class_trie, "team_control_point", cl_team_control_point_outputs);
	SetTrieValue(class_trie, "trigger_capture_area", cl_trigger_capture_area_outputs);
	SetTrieValue(class_trie, "team_control_point_master", cl_team_control_point_master_outputs);
	SetTrieValue(class_trie, "team_control_point_round", cl_team_control_point_round_outputs);
	SetTrieValue(class_trie, "item_teamflag", cl_teamflag_outputs);
	SetTrieValue(class_trie, "func_capturezone", cl_func_capturezone_outputs);
	SetTrieValue(class_trie, "tf_logic_cp_timer", cl_tf_logic_cp_timer_outputs);
	SetTrieValue(class_trie, "tf_logic_arena", cl_tf_logic_arena_outputs);
	SetTrieValue(class_trie, "tf_gamerules", cl_tf_gamerules_outputs);
	SetTrieValue(class_trie, "game_round_win", cl_game_round_win_outputs);
	SetTrieValue(class_trie, "team_round_timer", cl_team_round_timer_outputs);
}

DisplayMapEntityByType(class_idx, client = 0)
{
	if(class_idx < 0 || class_idx > sizeof(class_type))
		return;
	decl String:buffer[1024];
	new cnt = GetArrayCell(g_MapEventEntities,class_idx,0); 
	Format(buffer,sizeof(buffer),"Class %s found %d entities: ", class_type[class_idx],cnt);

	new Handle:ent_names_arr = GetArrayCell(g_MapEventEntities,class_idx,2);
	for(new j; j < cnt;++j)  
	{
		decl String:t_ent_name[MAX_EVENTGEN_CNAME];
		GetArrayString(ent_names_arr,j,t_ent_name,sizeof(t_ent_name));
		if(t_ent_name[0] != '\0')
		{
			StrCat(t_ent_name,sizeof(t_ent_name), " ");
			StrCat(buffer,sizeof(buffer), t_ent_name);
		}
	}

	PrintToConsole(client, buffer);
}

DisplayMapEntities(String:name[] = "",client = 0)
{
	new bool:found = false;

	for(new i; i < sizeof(class_type);++i)
	{
		if(!StrContains(name,class_type[i],false))
		{
			found = true;
			DisplayMapEntityByType(i, client);
			DisplayEntOutputs(class_type[i],client);
		}
	}

	if(found)
		return;

	for(new i; i < sizeof(class_type);++i)
	{
		DisplayMapEntityByType(i, client);
		DisplayEntOutputs(class_type[i],client);
	}


}

stock DisplayEntOutputs(const String:cname[], client = 0)
{
	new Handle:a_entout;
	decl String:buffer[1024];
	buffer[0] = '\0'


	if(GetTrieValue(g_EntOutTrie, cname, a_entout))
	{
		if(a_entout == INVALID_HANDLE)
			return;
		Format(buffer,sizeof(buffer), "Valid entity outputs for %s: ", cname);
		for(new i; i < GetArraySize(a_entout);++i)
		{
			decl String:s_out[MAX_EVENT_OUTPUTNAME];	
			GetArrayString(a_entout,i, s_out,sizeof(s_out));
			StrCat(s_out,sizeof(s_out), " ");
			StrCat(buffer,sizeof(buffer), s_out);
		}
		PrintToConsole(client,buffer);
	} 
}

stock CheckOutput(const String:cname[], const String:output[])
{
	new Handle:a_entout;
	GetTrieValue(g_EntOutTrie, cname, a_entout);

	if(a_entout == INVALID_HANDLE)
		return -1;
	for(new i; i < GetArraySize(a_entout);++i)
	{
		decl String:s_out[MAX_EVENT_OUTPUTNAME];	
		GetArrayString(a_entout,i, s_out,sizeof(s_out));
		if(!strcmp(s_out,output))
			return 1;
	}
	return -1;
}

stock CloseTrieOutputTypes(&Handle:class_trie)
{
	new Handle:t_array = INVALID_HANDLE;
	if(class_trie == INVALID_HANDLE)
		return;
	for(new i; i < sizeof(class_type);++i)
	{
		if(!GetTrieValue(class_trie, class_type[i], t_array))
			continue;
		CloseArray(t_array);
	}
	CloseHandle(class_trie);
	class_trie = INVALID_HANDLE;
}

stock InitGenEvents()
{
	if(g_vGenEvent == INVALID_HANDLE)
		return;

#if defined DEBUG_GEN_EVENT
	LogMessage("spawn_gen::InitGenEvents()");
#endif

	for(new i; i < GetArraySize(g_vGenEvent);++i)
	{
		new a_Cluster[RuneCluster];
		GetArrayArray(g_vGenEvent,i,a_Cluster[0],CLUSTER_BLOCK_SIZE);
		HookClusterEvents(a_Cluster);
	}
}

stock ShutdownGenEvents()
{
	if(g_vGenEvent == INVALID_HANDLE)
		return;

#if defined DEBUG_GEN_EVENT
	LogMessage("spawn_gen::ShutdownGenEvents()");
#endif

	for(new i; i < GetArraySize(g_vGenEvent);++i)
	{
		new a_Cluster[RuneCluster];
		GetArrayArray(g_vGenEvent,i,a_Cluster[0],CLUSTER_BLOCK_SIZE);
		HookClusterEvents(a_Cluster,true);
	}

	
// sanity check
	for(new class_idx; class_idx < sizeof(class_type);++class_idx)
	{
		new count  = GetArrayCell(g_MapEventEntities,class_idx,0);
		if(!count)
			continue;
		
		new Handle:hActionArray = GetArrayCell(g_MapEventEntities,class_idx,1);
		for(new action = 1; action < ClusterGenType;++action)
		{
			new Handle:hClassActionTrie = GetArrayCell(hActionArray,action,0);
			if(hClassActionTrie == INVALID_HANDLE)
				continue;

			if(GetTrieSize(hClassActionTrie) != 0)
			{
				LogError("warning; Hooked %s Trie for type %s  was not empty on map end!",
					actionString[action], class_type[class_idx]);
			}
		}

	}
}



stock HookClusterEvents(a_Cluster[RuneCluster], bool:remove = false)
{
	new size;
	if( a_Cluster[EventArray] != INVALID_HANDLE && (size = GetArraySize(a_Cluster[EventArray])) > 0 )
	{
#if defined DEBUG_GEN_EVENT
		LogMessage("%sooking #%d events.", remove ? "Unh" : "H", size);
#endif
		for(new j;j < size;++j)
		{
			new a_Event[GenEvent];
			GetArrayArray(a_Cluster[EventArray],j,a_Event[0],EVENT_BLOCK_SIZE);
			HookGenEvent( a_Cluster[ClusterName], a_Event, remove);
		}
	}
}

stock UnhookClusterEvents(a_Cluster[RuneCluster])
{
	HookClusterEvents(a_Cluster,true);
}

stock HookGenEvent(const String:cluster_name[], a_Event[GenEvent], bool:remove = false)
{
	new String:entity[MAX_EVENTGEN_CNAME];
	new String:cname[MAX_EVENTGEN_CNAME];
	new String:output[MAX_EVENTGEN_CNAME];

	strcopy(entity,sizeof(entity), a_Event[EventEntity]);
	strcopy(cname,sizeof(cname), a_Event[EventCName]);
	strcopy(output, sizeof(output), a_Event[EventOutput]);

	new EntityOutput:handler = a_Event[EventTriggered];	
	new action = _:a_Event[ClusterType];

	handler = actionHandler[ a_Event[ClusterType] ];
	new String:event[MAX_EVENT_ENTITY_OUTPUT];

	new class_idx;
	if(!GetTrieValue(g_CNameTrie,cname,class_idx))
	{
		LogMessage("HookGen class %s not supported", cname);
		return;
	}

	new hCount;
	//new bool:bHookedByNamedEnt = false;

	new Handle:hActionArray = GetArrayCell(g_MapEventEntities,class_idx,1);
	new Handle:hClassActionTrie = GetArrayCell(hActionArray,action,0);
	new Handle:hNamedActionTrie = GetArrayCell(hActionArray,action,1);

	if(a_Event[EventType] != GenEvent_ByEnt)
	{
		Format(event,sizeof(event),"%s.%s",cname,output);
		if((hCount = RegisterEvent(event, cluster_name,ClusterGenType:action,remove))==-1)
			return;
		if(remove && hCount == 0)
		{
			RemoveFromTrie(hClassActionTrie,event);
#if defined DEBUG_GEN_EVENT
			LogMessage("HookGenEvent %s removed.",event);
#endif
		} else {
			SetTrieValue(hClassActionTrie,event,hCount);
#if defined DEBUG_GEN_EVENT
			LogMessage("HookGenEvent %s ref %d",event,hCount);
#endif

		}
	} else {
		if(entity[0] == '\0')
		{
			LogMessage("warning; event type ByEnt, but entity name is null.");
			return;
		} else  {
			new Handle:ent_names_arr = GetArrayCell(g_MapEventEntities,class_idx,2);
			for(new j; j < GetArraySize(ent_names_arr);++j)
			{
				decl String:t_ent_name[MAX_EVENTGEN_CNAME];
				GetArrayString(ent_names_arr,j,t_ent_name,sizeof(t_ent_name));
				if(!strcmp(t_ent_name,entity))
				{
					Format(event,sizeof(event),"%s.%s",entity,output);
					if((hCount = RegisterEvent(event, cluster_name,ClusterGenType:action,remove))==-1)
						return;
					if(remove && hCount == 0)
					{
						RemoveFromTrie(hNamedActionTrie,event);
#if defined DEBUG_GEN_EVENT
						LogMessage("HookGenEvent %s removed.",event);
#endif
					} else {
						SetTrieValue(hNamedActionTrie,event,hCount);
#if defined DEBUG_GEN_EVENT
						LogMessage("HookGenEvent %s ref %d",event,hCount);
#endif
					}
					//bHookedByNamedEnt = true;
				}
			}
		}
	}
	//hCount is the number of clusters in the string array pointed to by Trie g_HookedEvent[action] keyed with event.

	//new ClassOutRef = GetTrieSize(hClassActionTrie);
	//new NamedEntRef = GetTrieSize(hNamedActionTrie);
	new bool:bHook = false;
	new bool:bUnhook = false;


	new value;
	if(hCount == 1 && !remove)
	{
		decl String:baseEvent[MAX_EVENT_ENTITY_OUTPUT];
		Format(baseEvent,sizeof(baseEvent),"%s.%s",cname,output);
		if(!GetTrieValue(hClassActionTrie,baseEvent,value))
		{
			value = 1;
			SetTrieValue(hClassActionTrie,baseEvent,value);
			bHook = true;
		} else {
			value++;
			SetTrieValue(hClassActionTrie,baseEvent,value);
		}
	} else if (hCount == 0 && remove) {
		decl String:baseEvent[MAX_EVENT_ENTITY_OUTPUT];
		Format(baseEvent,sizeof(baseEvent),"%s.%s",cname,output);
		if(!GetTrieValue(hClassActionTrie,baseEvent,value))
		{
			LogError("warning; during event remove, invalid reference count %d. cname.event %s",value, baseEvent);
			LogMessage("improper unhook. action %s event %s", actionString[action], baseEvent);
		} else {
			if(value == 1)
			{ 
				RemoveFromTrie(hClassActionTrie,baseEvent);
				bUnhook = true;
			} else {
				value--;
				SetTrieValue(hClassActionTrie,baseEvent,value);
			}
		}
	}


	if(!remove)
	{
		if(bHook)
		{
			HookEntityOutput(cname,output, handler);
#if defined DEBUG_GEN_EVENT
			LogMessage("HookOutput[%s] %s.%s",actionString[action][5], cname,output);
#endif
		}
	}
	else
	{
		if(bUnhook)
		{
			UnhookEntityOutput(cname,output, handler);
#if defined DEBUG_GEN_EVENT
			LogMessage("UnhookOutput[%s] %s.%s",actionString[action][5], cname,output);
#endif
		}
	}
}

stock RegisterEvent(const String:event[], const String:cluster[], ClusterGenType:action, bool:remove = false)
{
	if(g_HookedEvents[action] == INVALID_HANDLE)
		g_HookedEvents[action] = CreateTrie();


	new n_clusters = -1;
	new Handle:array_clusters;
	if(GetTrieValue(g_HookedEvents[action], event, array_clusters))
	{
		new idx;
		if( (idx = FindStringInArray(array_clusters, cluster)) != -1)
		{
			if(remove)
			{
				RemoveFromArray(array_clusters,idx);
				if((n_clusters = GetArraySize(array_clusters)) == 0)
				{
					new a_idx;
					if( (a_idx = FindValueInArray(g_EventHandleArray,array_clusters)) != -1)
						RemoveFromArray(g_EventHandleArray,a_idx);
					CloseHandle(array_clusters);
					RemoveFromTrie(g_HookedEvents[action], event);
					array_clusters = INVALID_HANDLE;
				}
			} else {
				new String:buffer[MAX_EVENTGEN_NAME];
				GetArrayString(array_clusters,idx, buffer, sizeof(buffer));
				LogMessage("Warning; event %s already registered cluster %s prior %s", event, cluster, buffer);
				return -1;
			}
		} else if(!remove) {
			PushArrayString(array_clusters, cluster);
			n_clusters = GetArraySize(array_clusters);
		} else {
			LogMessage("Warning; could not remove cluster %s because it was not found in cluster array for event %s", cluster,event);
			return -1;
		}
	} else if(!remove) {
		array_clusters = CreateArray(MAX_EVENTGEN_NAME);
		PushArrayString(array_clusters, cluster);
		SetTrieValue(g_HookedEvents[action],event,array_clusters);
		PushArrayCell(g_EventHandleArray, array_clusters);
		n_clusters = 1;
	} else {
		LogMessage("Warning; could not remove cluster %s because event %s was not hooked.", cluster, event);
		return -1;
	}

#if defined DEBUG_GEN_EVENT
	if(!remove)
		LogMessage("Register Event [%s] %s => %s",actionString[action], event, cluster);
	else
		LogMessage("Unregister Event [%s] %s => %s",actionString[action], event, cluster);
#endif
	return n_clusters;
}

stock UnregisterEvent(const String:event[], const String:cluster[], ClusterGenType:action)
{
	return RegisterEvent(event,cluster,true);
}


Handle:GetEntityEvent(ClusterGenType:action,caller,const String:output[],String:event[],len)
{
	new String:cname[MAX_EVENTGEN_CNAME];
	new String:ename[MAX_EVENTGEN_CNAME];
	new Handle:ret_array = INVALID_HANDLE;

	Entity_GetName(caller,ename,sizeof(ename));
	Entity_GetClassName(caller,cname,sizeof(cname));

	new Handle:clusters;
	Format(event,len,"%s.%s", cname, output);
	if(g_HookedEvents[action] == INVALID_HANDLE)
	{
		LogError("INVALID g_HookedEvents[%s] for event %s",
			actionString[action], event);
		return INVALID_HANDLE;
	}

	if(GetTrieValue(g_HookedEvents[action], event, clusters))
	{
		if(clusters == INVALID_HANDLE)
		{
			LogMessage("string array in event trie was invalid.\n");
			return INVALID_HANDLE;
		}

		GetIdsFromClusters(clusters, ret_array);
	}

	if(!strcmp(cname,"trigger_capture_area"))
	{
		GetEntPropString(caller,Prop_Data,"m_iszCapPointName",ename,sizeof(ename));
	}
		


	if(ename[0] != '\0')
	{
		decl String:named_event[MAX_EVENT_ENTITY_OUTPUT]="";
		Format(named_event,sizeof(named_event),"%s.%s", ename, output);

		if(GetTrieValue(g_HookedEvents[action], named_event, clusters))
		{
			if(clusters == INVALID_HANDLE)
			{
				LogMessage("string array in event trie was invalid.\n");
				return INVALID_HANDLE;
			}

			GetIdsFromClusters(clusters, ret_array);
		}
		Format(event,len,"%s.%s",ename,event);
	}


	return ret_array;
}


stock GetIdsFromClusters(Handle:clusters, &Handle:ret_array)
{
	for(new i; i < GetArraySize(clusters);++i)
	{
		new String:buffer[MAX_EVENTGEN_NAME];
		GetArrayString(clusters,i,buffer,sizeof(buffer));
#if defined DEBUG_GEN_EVENT
		LogMessage("TRIGGER cluster -> %s", buffer);
#endif

		new Handle:id_array;
		if(GetTrieValue(g_vGenCluster, buffer, id_array))
		{
			if(ret_array == INVALID_HANDLE)
				ret_array = CreateArray(1);
			for(new j; j < GetArraySize(id_array);++j)
			{
	
				new id = GetArrayCell(id_array,j);
				if((FindValueInArray(ret_array, id) == -1))
				{
					PushArrayCell(ret_array, id);
				}
			}
		}
		
	}
}


public EventHandler_EnableGen(const String:output[], caller, activator, Float:delay)
{
	new String:event[MAX_EVENT_ENTITY_OUTPUT];
	decl String:sEventDebug[1024]="";
	sEventDebug[0] = '\0';
	new Handle:array_ids = GetEntityEvent(ClusterGenType:GenEvent_Add,caller,output,event,sizeof(event));
#if defined DEBUG_GEN_EVENT
	LogMessage("enable %s", event);
#endif
	if(array_ids== INVALID_HANDLE)
	{
		return;
	}

	while(GetArraySize(array_ids) > 0)
	{
		new id = GetArrayCell(array_ids, 0);
		new Handle:g_array;
	
		new g_index = FindIndexInGlobalArrayById(g_array, id);
		if(g_index == -1)
		{
			LogMessage("warning; enable handler could not find generator %d", id);
		} else if( g_array == g_vGen) {
#if defined DEBUG_GEN_EVENT_WARNING
			LogMessage("warning; enable handler generator %d already in global array", id);
#endif
		} else {
			decl String:sId[8]="";
			new a_Gen[RuneGen];
			GetArrayArray(g_array, g_index, a_Gen[0], RUNE_BLOCK_SIZE);
			RemoveFromArray(g_array, g_index);
			PushArrayArray(g_vGen, a_Gen[0], RUNE_BLOCK_SIZE);
			Format(sId,sizeof(sId),"%d ",a_Gen[Id]);
			StrCat(sEventDebug,sizeof(sEventDebug),sId);
		}
		
		RemoveFromArray(array_ids,0);
	}

#if defined DEBUG_GEN_EVENT
	if(sEventDebug[0] != '\0')
		Client_PrintToChatAll(true,"enable: %s %s",event,sEventDebug);
#endif

	CloseHandle(array_ids);
}


public EventHandler_DisableGen(const String:output[], caller, activator, Float:delay)
{
	//new String:testName[MAX_EVENT_ENTITY_OUTPUT];
	decl String:sEventDebug[1024]="";
	sEventDebug[0] = '\0';
	
	new String:event[MAX_EVENT_ENTITY_OUTPUT];
	new Handle:array_ids = GetEntityEvent(ClusterGenType:GenEvent_Remove,caller,output,event,sizeof(event));
#if defined DEBUG_GEN_EVENT
	LogMessage("disable %s", event);
#endif
	if(array_ids== INVALID_HANDLE)
	{
		return;
	}

	while(GetArraySize(array_ids) > 0)
	{
		new id = GetArrayCell(array_ids, 0);
		new Handle:g_array;
	
		new g_index = FindIndexInGlobalArrayById(g_array, id);
		if(g_index == -1)
		{
			LogMessage("warning; disable handler could not find generator %d", id);
		} else if( g_array == g_vGenDisabled) {
#if defined DEBUG_GEN_EVENT_WARNING
			LogMessage("warning; disable handler generator %d already in global array", id);
#endif
		} else {
			decl String:sId[8]="";
			new a_Gen[RuneGen];
			GetArrayArray(g_array, g_index, a_Gen[0], RUNE_BLOCK_SIZE);
			RemoveFromArray(g_array, g_index);
			PushArrayArray(g_vGenDisabled, a_Gen[0], RUNE_BLOCK_SIZE);
			Format(sId,sizeof(sId),"%d ",a_Gen[Id]);
			StrCat(sEventDebug,sizeof(sEventDebug),sId);
		}
		
		RemoveFromArray(array_ids,0);
	}
#if defined DEBUG_GEN_EVENT
	if(sEventDebug[0] != '\0')
		Client_PrintToChatAll(true,"disable: %s %s",event,sEventDebug);
#endif

	CloseHandle(array_ids);
}

public EventHandler_ToggleGen(const String:output[], caller, activator, Float:delay)
{
	new String:event[MAX_EVENT_ENTITY_OUTPUT];
	new Handle:array_ids = GetEntityEvent(ClusterGenType:GenEvent_Toggle,caller,output,event,sizeof(event));

	decl String:sEventDebug[1024]="";
	sEventDebug[0] = '\0';
#if defined DEBUG_GEN_EVENT
	LogMessage("toggle %s", event);
#endif
	if(array_ids== INVALID_HANDLE)
	{
		return;
	}


	while(GetArraySize(array_ids) > 0)
	{
		new id = GetArrayCell(array_ids, 0);
		new Handle:g_array;
		new Handle:g_array_dest;
	
		new g_index = FindIndexInGlobalArrayById(g_array, id);
		if(g_index == -1)
		{
			LogMessage("warning; toggle handler could not find generator %d", id);
		} else {
			decl String:sId[8]="";
			new a_Gen[RuneGen];
			GetArrayArray(g_array, g_index, a_Gen[0], RUNE_BLOCK_SIZE);
			RemoveFromArray(g_array, g_index);
			g_array_dest = (g_array == g_vGen) ? g_vGenDisabled : g_vGen;
			PushArrayArray(g_array_dest, a_Gen[0], RUNE_BLOCK_SIZE);
			Format(sId,sizeof(sId),"%d ",a_Gen[Id]);
			StrCat(sEventDebug,sizeof(sEventDebug),sId);
		}
		
		RemoveFromArray(array_ids,0);
	}

#if defined DEBUG_GEN_EVENT
	if(sEventDebug[0] != '\0')
		Client_PrintToChatAll(true,"toggle: %s %s",event,sEventDebug);
#endif

	CloseHandle(array_ids);
}

public EventHandler_SpawnGen(const String:output[], caller, activator, Float:delay)
{
	new String:event[MAX_EVENT_ENTITY_OUTPUT];
	decl String:sEventDebug[1024]="";
	sEventDebug[0] = '\0';
	new Handle:array_ids = GetEntityEvent(ClusterGenType:GenEvent_Spawn,caller,output,event,sizeof(event));
#if defined DEBUG_GEN_EVENT
	LogMessage("spawn %s", event);
#endif

	if(array_ids== INVALID_HANDLE)
	{
		return;
	}


	new Handle:datapack = CreateDataPack();

	for(new k; k <GetArraySize(array_ids);++k)
	{
		decl String:sId[8]="";
		new _id = GetArrayCell(array_ids,k);
		WritePackCell(datapack, _id);
		Format(sId,sizeof(sId),"%d ",_id);
		StrCat(sEventDebug,sizeof(sEventDebug),sId);
}

#if defined DEBUG_GEN_EVENT
	if(sEventDebug[0] != '\0')
		Client_PrintToChatAll(true,"spawn: %s %s",event,sEventDebug);

#endif
	ResetPack(datapack);
	CreateTimer(0.115,SpawnRuneTimerBlock,datapack,TIMER_DATA_HNDL_CLOSE | TIMER_FLAG_NO_MAPCHANGE|TIMER_REPEAT);
	CloseHandle(array_ids);
}


