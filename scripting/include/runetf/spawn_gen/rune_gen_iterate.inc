
#include <adt_stack>
#include <colors>


enum GenPlayerInfo
{
	clientId,
	CurrentGeneratorIndex,
	Handle:WorkingSet,
	Handle:DefaultCluster,
	//String:DefaultCluster[MAX_CLUSTER_LEN],
};

stock FindIndexInArrayById(&Handle:array, ID)
{
	new i;
	for(i = 0; i < GetArraySize(array);++i)
		if( get_id_info(array,i) == ID)
			return i;

	return -1;
}


stock FindIndexInGlobalArrayById(&Handle:array, ID)
{
	array = g_vGen;
	new i;
	for(i = 0; i < GetArraySize(array);++i)
		if( get_id_info(array,i) == ID)
			return i;
	array = g_vGenDisabled;
	for(i = 0; i < GetArraySize(array);++i)
		if( get_id_info(array,i) == ID)
			return i;
	array = INVALID_HANDLE;
	return -1;
}

new g_Player[GenPlayerInfo];

stock FindEventCluster(const String:name[], &Handle:eventsHandle)
{
	return FindEventClusterInArray(name,eventsHandle,g_vGenEvent);
}
stock FindEventClusterInArray(const String:name[], &Handle:eventsHandle, Handle:g_event_array)
{
	new idx;
	for(idx; idx < GetArraySize(g_event_array);++idx)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN];
		new a_Cluster[RuneCluster];
		GetArrayArray(g_event_array,idx,a_Cluster, CLUSTER_BLOCK_SIZE);
		if(!strcmp(name, a_Cluster[ClusterName]))
		{
			eventsHandle = a_Cluster[EventArray];
			return idx;
		}
	}
	return -1;
}
	
	


stock IsValidGenId(ID)
{
	if( g_Player[WorkingSet] )
		if(FindIndexInArrayById( g_Player[WorkingSet], ID) != -1)
			return 1;

	new Handle:t_arr;
	if(FindIndexInGlobalArrayById(t_arr,ID) != -1)
		return 1;

	return 0;
}


// load [id, ... ] //copy from global array into working set
ClientLoadGen(client, &Handle:global, ID, bool:remove = false)
{
	LogMessage("%d Loading ID %d remove %s", client, ID, (remove == true ? "true" : "false" ));
	new Handle:a_gen = INVALID_HANDLE;

  if(global != INVALID_HANDLE && GetArraySize(global) > 0 && FindIndexInArrayById(global, ID) != -1)
  {
    PrintToConsole(client,"warning; gen %d is already in the workingset.", ID);
    return -1;
  }


	new g_index = FindIndexInGlobalArrayById(a_gen, ID);

	if(g_index == -1)
		return -1;

	new pack[RuneGen];
	GetArrayArray(a_gen, g_index, pack, RUNE_BLOCK_SIZE);

	if(global == INVALID_HANDLE)
		global = CreateArray(RUNE_BLOCK_SIZE);

	g_Player[CurrentGeneratorIndex] = PushArrayArray(global, pack, RUNE_BLOCK_SIZE);

	if(remove)
		RemoveFromArray(a_gen, g_index);

	return g_Player[CurrentGeneratorIndex];
}


stock LoadGenCmd(client, args)
{
	//decl String:cmdstr[512];
	//GetCmdArgString(cmdstr,sizeof(cmdstr));

	for(new i = 2;i <= GetCmdArgs();++i)
	{
		new String:buf[12];
		new ID;
		GetCmdArg(i,buf,sizeof(buf));
		if(ClientLoadGen(client, g_Player[ WorkingSet ], (ID = StringToInt( buf )),true) < 0)
		{
			PrintToConsole(client,"global array does not contain %s.", buf);
		} else {
			PrintToConsole(client, "Loaded %d from global array", ID);
		}
	}
}


static g_CreateCnt = 0;
// create // create new entity in workingset
CreateGenClientCmd(client)
{
	++g_CreateCnt;
	//new newID = (client + 1)*1000 + g_CreateCnt;
	new newID = g_NextGenId++;
	new Float:_ori[3];
	new Float:_ang[3];
	new Float:_force  = 500.0;

	new pack[RuneGen];
	ResetTempGen(pack);

	GetClientEyePosition(client, _ori);
	GetClientEyeAngles(client, _ang);
	
	pack[ Id] = newID;
	SetTempGenVec( pack, _:g_ori, _ori);
	SetTempGenVec( pack, _:g_ang, _ang);
	pack[ g_force ] = _force;
	pack[ g_StartDisabled ] = false;

	if(g_Player[WorkingSet] == INVALID_HANDLE)
		g_Player[WorkingSet] = CreateArray(RUNE_BLOCK_SIZE);
	
	if(g_Player[DefaultCluster] != INVALID_HANDLE)
	{
		new size = GetArraySize(g_Player[DefaultCluster]);
	
		for(new i;i < size;++i)
		{
			decl t_sCluster[MAX_CLUSTER_LEN]="";	
			GetArrayString(g_Player[DefaultCluster], i, t_sCluster, sizeof(t_sCluster));
			AddToCluster(t_sCluster, newID);

		}
	} else {
		g_Player[DefaultCluster] = CreateArray(MAX_CLUSTER_LEN);
	}

	return (g_Player[CurrentGeneratorIndex] = add_rune_info( g_Player[WorkingSet] , pack ));

}

stock DisplayDefaultClusterToClient(client, Handle:hDefaultClusters)
{

	decl String:sMessage[256]="";

	new len = StrCopy(sMessage,sizeof(sMessage),"Default clusters: ");
	new size;
	
	if((size = GetArraySize(hDefaultClusters)) == 0)
	{
		StrCat(sMessage,sizeof(sMessage),"none.");
		CPrintToChat(client,sMessage);
		return;
	}

	decl String:s_cluster[MAX_CLUSTER_LEN]="";
	len += GetArrayString(hDefaultClusters,0,s_cluster,sizeof(s_cluster));
	StrCat(sMessage,sizeof(sMessage),s_cluster);


	for(new i = 1; i < size; ++i)
	{
		decl String:s_cluster[MAX_CLUSTER_LEN]="";
		new slen = GetArrayString(hDefaultClusters,i,s_cluster,sizeof(s_cluster));
		if(slen + len > sizeof(sMessage) - 2)
		{
			len = 0;
			CPrintToChat(client,sMessage);
			len = StrCopy(sMessage,sizeof(sMessage),s_cluster);
		} else {
			len += StrCat(sMessage,sizeof(sMessage),", ");
			len += StrCat(sMessage,sizeof(sMessage), s_cluster);
		}
	}
	CPrintToChat(client,sMessage);

}

stock DisplayGenToClient(client, &Handle:working_set, index)
{
	new _id;
	new Float:_ori[3];
	new Float:_ang[3];
	new Float:_force;
	new bool:_disabled;

	new pack[RuneGen];
	

	GetArrayArray(working_set,index,pack,RUNE_BLOCK_SIZE);

	_id = pack[Id];
	GetTempGenVec(pack,g_ori,_ori);
	GetTempGenVec(pack,g_ang,_ang);
	_force = pack[g_force];
	_disabled = pack[g_StartDisabled];

	decl String:s_disabled[24];
	s_disabled[0] = '\0';
	strcopy(s_disabled,sizeof(s_disabled),"(disabled) ");

	decl String:s_clusters[MAX_CLUSTER_LEN*8];
	s_clusters[0] = '\0';
	new idx;
	new c_cnt = GetClustersContainingId(_id,s_clusters,sizeof(s_clusters),idx);

	decl String:buffer[256];
	Format(buffer,sizeof(buffer),"\n\nid: %d %sOri: %.2f %.2f %.2f\nAng: %.0f %.0f Force %.0f\n%s", 
		_id,
		pack[g_StartDisabled] == true ? s_disabled : "",
		_ori[0], _ori[1], _ori[2],
		_ang[0], _ang[1], _force,
		s_clusters);

	PrintToConsole(client,buffer);
	if(client)
	{
		//ShowHudText(client, -1, buffer);

		new Handle:hBf = StartMessageOne("SayText2", client);
		if(hBf == INVALID_HANDLE)
			return;
		SetGlobalTransTarget(client);
		BfWriteByte(hBf, client);
		BfWriteByte(hBf, 0);
		BfWriteString(hBf, buffer);
		EndMessage();
	}

}

Handle:GetWorkingSet(client)
{
	return g_Player[WorkingSet];
}

stock GetCurrentGen(client, r_gen[RuneGen])
{
	new Handle:set = GetWorkingSet(client);
	new idx = g_Player[CurrentGeneratorIndex];

	if(set == INVALID_HANDLE || idx < 0 || idx > GetArraySize(set))
		return -1;

	GetArrayArray(set,idx,r_gen,RUNE_BLOCK_SIZE);
	return idx;
}


stock SetCurrentCmd(client, args)
{

	new Handle:hWorking = GetWorkingSet(client);
	new currentIdx = g_Player[CurrentGeneratorIndex];

	if(hWorking == INVALID_HANDLE)
	{
		PrintToConsole(client, "Invalid handle on working set");
		return Plugin_Handled;
	}
	if(currentIdx < 0 || currentIdx > GetArraySize(hWorking))
	{
		PrintToConsole(client, "current array idx %d out of bounds", currentIdx);
		return Plugin_Handled;
	}

	if( GetCmdArgs() < 2 )
	{
		PrintToConsole(client, "usage: sm_gen set <test|origin|angle>");
		PrintToConsole(client, "usage: sm_gen set <idx|id|force> <integer>");
		PrintToConsole(client, "usage: sm_gen set <origin|angle> <vector>");
		PrintToConsole(client, "usage: sm_gen set <default> <cluster>");
		return Plugin_Handled;
	}

	new bool:bSpawn = false;
	decl String:param[80];
	GetCmdArg(2, param, sizeof(param));

	if(!strcmp(param,"test"))
	{
		PrintToConsole(client,"spawning rune idx@%d", currentIdx);
		bSpawn = true;
	} else if(!strcmp(param,"idx")) {
		if(GetCmdArgs() < 3)
		{
			PrintToConsole(client,"expected index to set.");
			return Plugin_Handled;
		}
		GetCmdArg(3, param, sizeof(param));
		new idx = StringToInt(param);
		if( idx >= 0 && idx < GetArraySize( hWorking ) )
		{
			PrintToConsole(client,"set current index in working set %d", idx);
			g_Player[CurrentGeneratorIndex] = idx;
			currentIdx = idx;
			bSpawn = true;
		} else {
			PrintToConsole(client,"Invalid working set index %d", idx);
		}
	} else if(!strcmp(param,"id",false)) {
		if(GetCmdArgs() < 3)
		{
			PrintToConsole(client,"expected argument ID to set");
			return Plugin_Handled;
		}
		GetCmdArg(3, param, sizeof(param));
		new old_id = get_id_info(hWorking,currentIdx);
		new _id = StringToInt(param);

		if(_id == 0)
		{
			PrintToConsole(client,"Invalid generator ID number %s",param);
			return Plugin_Handled;
		}
		if(FindIndexInArrayById(hWorking,_id) != -1)
		{
			PrintToConsole(client,"Generator ID %d exists in workingset", _id);
			return Plugin_Handled;
		}

		set_id_info(hWorking, currentIdx, _id);
		PrintToConsole(client, "changed current generator ID %d => %d",old_id, _id);
		new Handle:a_array;
		if(FindIndexInGlobalArrayById(a_array,_id) != -1)
		{
			PrintToConsole(client,"warning; generator %d in %s array will be overwritten on merge.",
				_id, a_array == g_vGen ? " global " : " disabled ");
		}
	} else if(!strcmp(param,"force",false)) {
		if(GetCmdArgs() < 3)
		{
			PrintToConsole(client,"Expected force argument");
			return Plugin_Handled;
		}
		GetCmdArg(3, param, sizeof(param));
		new Float:old_force = get_force_info(hWorking,currentIdx);
		new Float:_force = StringToFloat(param);
		set_force_info(hWorking, currentIdx, _force);
		PrintToConsole(client, "changed current generator force %f => %f",old_force, _force);
	} else if(!strcmp(param,"origin", false)) {
		new Float:_ori[3] = {0.0, 0.0, 0.0}
		if(GetCmdArgs() > 2)
		{
			get_origin_info(hWorking,currentIdx,_ori);
			for(new i; i < 3;++i)
			{
				if(GetCmdArgs() < 3+i)
					break;

				GetCmdArg(3+i,param,sizeof(param));
				new Float:value = 0.0;
				if(!StrEqual(param,"same",false))
					continue;

				if(StringToFloatEx(param,value) < 1)
				{
					PrintToConsole(client,"Expected float value or string same; invalid value %s", param);
					return Plugin_Handled;
				}
				_ori[i] = value;
			}
			PrintToConsole(client,"changed current rune origin to %f %f %f",_ori[0], _ori[1], _ori[2]);
		} else {
			GetClientEyePosition(client,_ori);
			PrintToConsole(client,"updated current rune to spawn at your pos");
		}
		set_origin_info(hWorking, currentIdx, _ori);
		bSpawn = true;
	} else if(!strcmp(param,"angle", false)) {
		new Float:_ang[3] = {0.0, 0.0, 0.0}
		if(GetCmdArgs() > 2)
		{
			get_angle_info(hWorking,currentIdx,_ang);
			for(new i; i < 3;++i)
			{
				if(GetCmdArgs() < 3+i)
					break;

				GetCmdArg(3+i,param,sizeof(param));
				new Float:value = 0.0;
				if(!StrEqual(param,"same",false))
					continue;

				if(StringToFloatEx(param,value) < 1)
				{
					PrintToConsole(client,"Expected float value or string same; invalid value %s", param);
					return Plugin_Handled;
				}
				_ang[i] = value;
			}
			PrintToConsole(client,"changed current rune angle to %f %f %f",_ang[0], _ang[1],_ang[2]);
		} else {
			GetClientEyeAngles(client,_ang);
			PrintToConsole(client,"updated current rune to spawn in your direction");
		}
		set_angle_info(hWorking, currentIdx, _ang);
		bSpawn = true;
	} else {
		PrintToConsole(client,"unknown sm_gen set command %s", param);
	}

	
	if(bSpawn)
		SpawnTestRune( hWorking , currentIdx );


	return Plugin_Continue;
}
// set <id|ori|ang>
// merge //copy workingset to global array
// drop  // drop working set

stock MergeWorkingSetCmd(client)
{
	if( g_Player[WorkingSet] == INVALID_HANDLE )
	{
		PrintToConsole(client, "no working set to merge");
		return -1;
	}
	
	new n_default, n_added, n_updated;

	new Handle:t_arr = g_Player[WorkingSet];
	for(new i; i < GetArraySize(t_arr);++i)
	{
		
		new checkID = get_id_info(t_arr, i);
		new Handle:a_gen = INVALID_HANDLE;
		new g_index = -1; 

		g_index = FindIndexInGlobalArrayById(a_gen, checkID);

		new GenPack[RuneGen];
		GetArrayArray(t_arr, i, GenPack, RUNE_BLOCK_SIZE);

		if(g_index == -1)
		{
			if( GenPack[g_StartDisabled] )
		 		add_rune_info(g_vGenDisabled, GenPack )
			else
				add_rune_info(g_vGen, GenPack);
			++n_added;
		} else {
			SetArrayArray(a_gen, g_index, GenPack, RUNE_BLOCK_SIZE);
			++n_updated;
		}
		
	}

	decl String:buf_cluster[32];
	buf_cluster[0] = '\0';

	Client_PrintToChat(client,false,"Merge Complete; %d updated, %d added%s", n_updated, n_added, buf_cluster[0] == '\0' ? "" : buf_cluster);

	ClearArray( g_Player[WorkingSet] );
	g_Player[WorkingSet] = INVALID_HANDLE;
	g_Player[CurrentGeneratorIndex] = -1;
	return 1;

//	(failed == false ? INVALID_HANDLE : CloneArray(fail_merge_arr));
	//ClearArray(fail_merge_arr);
}

stock DropWorkingSetCmd(client)
{
	new arrSize;
	new Handle:t_arr = g_Player[WorkingSet];
	if( g_Player[WorkingSet] == INVALID_HANDLE || (arrSize = GetArraySize(t_arr)) == 0)
		PrintToConsole(client,"No working set to drop.")
	else 
	{
		new ret;
		new Handle:g_array, _id;
		for(new i; i < arrSize;++i)
		{
			get_id_info(t_arr,i, _id);
			if( FindIndexInGlobalArrayById(g_array,_id) == -1 )
			{
				RemoveFromAllClusters(_id);
				ret++;
			}
		}
		ClearArray(t_arr);
		g_Player[WorkingSet] = INVALID_HANDLE;
		g_Player[CurrentGeneratorIndex] = -1;
		PrintToConsole(client,"Workingset dropped.  %d elements removed from clusters.", ret);
		return Plugin_Continue;
	}
	return Plugin_Handled;
}

stock DisplayWorkingSetCmd(client,args)
{
	new String:displayStr[512];
	new String:buf[32];
	
	new Handle:t_arr = g_Player[WorkingSet];
	new len = strcopy(displayStr,sizeof(displayStr),"Current working set: ");
	new currentIdx = g_Player[CurrentGeneratorIndex];

	new arrSize;
	if( t_arr == INVALID_HANDLE || (arrSize = GetArraySize(t_arr)) == 0)
	{
		new len_buf = strcopy(buf,sizeof(buf), "none.");
		StrCat(displayStr, sizeof(displayStr) - len, buf);
	} else {
		for(new i;i < arrSize;++i)
		{
			new t_ID = get_id_info(t_arr, i);
			len += Format(buf, sizeof(buf),"%5d%c ", t_ID, (i == currentIdx) ? '*' : ' ');
			StrCat(displayStr, sizeof(displayStr) - len, buf);
			if(i == currentIdx)
				DisplayGenToClient(client, t_arr, i);
		}
	}
	PrintToConsole(client,"%s", displayStr);
	return Plugin_Continue;
}

stock DisplayGlobalSet(client)
{
	new String:displayStr[1024];
	new String:buf[32];
	
	new String:s_desc[2][] = { "Global set: ", "Disabled set: " };
	new Handle:t_arr[2];
	t_arr[0] = g_vGen;
  t_arr[1] = g_vGenDisabled;


	for(new i; i < 2;++i)
	{
		new len = strcopy(displayStr,sizeof(displayStr),s_desc[i]);
		new arrSize;
		if( t_arr[i] == INVALID_HANDLE || (arrSize = GetArraySize(t_arr[i])) == 0)
		{
			new len_buf = strcopy(buf,sizeof(buf), "none.");
			StrCat(displayStr, sizeof(displayStr) - len, buf);
		} else {
			for(new j;j < arrSize;++j)
			{
				new t_ID = get_id_info(t_arr[i], j);
				len += Format(buf, sizeof(buf),"%5d ", t_ID);
				StrCat(displayStr, sizeof(displayStr) - len, buf);
			}
		}
		PrintToConsole(client,"%s", displayStr);
	}
	return Plugin_Continue;
}

public Action:ClusterWorkingSetCmd(client,args)
{
	if(GetCmdArgs() < 3)
	{
		PrintToConsole(client,"sm_gen cluster <create> <cluster name>");
		PrintToConsole(client,"sm_gen cluster <add|remove> <cluster name> [id|lower-upper]");
		PrintToConsole(client,"sm_gen cluster <enable|disable> <cluster name>");
		PrintToConsole(client,"sm_gen cluster <load|drop> <cluster name>");
		PrintToConsole(client,"sm_gen cluster <spawn> <cluster name>");
		return Plugin_Stop;
	}
	decl String:cmd[80];

	GetCmdArg(2,cmd,sizeof(cmd));
	new Handle:id_array = INVALID_HANDLE;

	if(!strcmp("create", cmd))
	{
		decl String:s_cluster[MAX_CLUSTER_LEN+1];
		GetCmdArg(3, s_cluster,sizeof(s_cluster));
		if(strlen(s_cluster) >= MAX_CLUSTER_LEN)
		{
			PrintToConsole(client,"Cluster name too long.");
			return Plugin_Stop;
		}
		CreateCluster(s_cluster,id_array);
		PrintToConsole(client, "created cluster %s", s_cluster);

		new cnt;
		new Handle:event_arr = INVALID_HANDLE;
		if(FindEventCluster(s_cluster,event_arr) == -1)
		{
			new a_Cluster[RuneCluster];
			strcopy( a_Cluster[ClusterName], sizeof(a_Cluster[ClusterName]), s_cluster);
			a_Cluster[ EventArray ] = CreateArray(EVENT_BLOCK_SIZE);
			add_cluster_info(g_vGenEvent,a_Cluster);
			event_arr = a_Cluster[ EventArray ];
		}

		if( g_Player[WorkingSet] == INVALID_HANDLE)
			return Plugin_Stop;

		for(new i;i < GetArraySize( g_Player[WorkingSet] );++i)
		{
			new _id;
			get_id_info( g_Player[WorkingSet], i, _id);
			if(AddToCluster(s_cluster,_id) != -1)
				++cnt;
		}
		PrintToConsole(client,"Added %d elements from workingset", cnt);
	} else if(!strcmp("add",cmd) || !strcmp("remove",cmd)) {
		new bool:add = false;
		if(!strcmp("add",cmd))
			add = true;
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetCmdArg(3, s_cluster,sizeof(s_cluster));
		new Handle:c_array;
		if(!GetTrieValue(g_vGenCluster, s_cluster, c_array))
		{
			PrintToConsole(client,"invalid cluster %s", s_cluster);
			return Plugin_Stop;
		}
		if(GetCmdArgs() < 4)
		{
			PrintToConsole(client,"Expected list of generator identifiers or a range N-M");
			return Plugin_Stop;
		}
		new ret;
		for(new iarg = 4; iarg <= GetCmdArgs();++iarg)
		{
			decl String:id_arg[16];
			GetCmdArg(iarg,id_arg,sizeof(id_arg));
			TrimString(id_arg);
			
			new idx;
			if((idx = FindCharInString(id_arg, '-')) != -1)
			{
				new id_lower = StringToInt(id_arg);
				new id_upper = StringToInt(id_arg[idx+1]);
				if(id_upper <= id_lower || id_upper == 0 || id_lower == 0)
				{
					PrintToConsole(client,"invalid range; lower: %d < upper %d", id_lower, id_upper);
					continue;
				}
				for(new i = id_lower;i <= id_upper;++i)
				{
					new idx;
					if(add && IsValidGenId(i))
					{
						++ret;
						AddToCluster(s_cluster,i);
					} else if(!add) {
						if( (idx = FindValueInArray(c_array, i)) != -1)
						{
							RemoveFromArray(c_array,idx);
							++ret;
						}
					}
				}
			} else {
				new id = StringToInt(id_arg);
				if(id != 0)
				{
					if(add && IsValidGenId(id) && ++ret)
						AddToCluster(s_cluster,id);
					else if(!add && (idx = FindValueInArray(c_array,id)) != -1 && ++ret)
						RemoveFromArray(c_array,idx);
					else
						PrintToConsole(client,"gen identifier %d not found.", id);
				} else
					PrintToConsole(client,"Invalid identifier %s", id_arg);
			}
		}
		
		new String:buffer[128];
		Format(buffer,sizeof(buffer), "%d entities ");
		if(add)
			StrCat(buffer,sizeof(buffer), "added.");
		else
			StrCat(buffer,sizeof(buffer), "removed.");
		PrintToConsole(client,buffer);
	
	} else if(!strcmp(cmd,"enable") || !strcmp(cmd,"disable")) {
		new bool:enable = false;

		if(!strcmp(cmd,"enable"))
			enable = true;

		if(GetCmdArgs() < 3)
		{
			PrintToConsole(client,"expected cluster name");
			return Plugin_Stop;
		}
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetCmdArg(3, s_cluster,sizeof(s_cluster));
		new Handle:array_ids;
		new Handle:a_gen;
		
		if(!GetTrieValue(g_vGenCluster,s_cluster,array_ids))
		{
			PrintToConsole(client,"invalid cluster name %s", s_cluster);
			return Plugin_Stop;
		}

		if(array_ids == INVALID_HANDLE || GetArraySize(array_ids) == 0)
		{
			PrintToConsole(client,"cluster %s has invalid id array", s_cluster);
			return Plugin_Stop;
		}

		new n_changed;
		new n_same;
		for(new i; i < GetArraySize(array_ids);++i)
		{
			new t_Gen[RuneGen];
			new idx;
			new id = GetArrayCell(array_ids,i);
			if( (  (a_gen = g_Player[WorkingSet]) != INVALID_HANDLE && (idx = FindIndexInArrayById(a_gen,id)) != -1) || (idx = FindIndexInGlobalArrayById(a_gen, id)) != -1 )
			{
				GetArrayArray(a_gen,idx, t_Gen,RUNE_BLOCK_SIZE);
				if(enable)
				{
					if(t_Gen[g_StartDisabled] == false)
						++n_same;
					else
					{
						++n_changed;
						t_Gen[g_StartDisabled] = false;
						SetArrayArray(a_gen,idx, t_Gen,RUNE_BLOCK_SIZE);
					}
				} else {
					if(t_Gen[g_StartDisabled] == true)
						++n_same;
					else
					{
						++n_changed;
						t_Gen[g_StartDisabled] = true;
						SetArrayArray(a_gen,idx, t_Gen,RUNE_BLOCK_SIZE);
					}
				}
			}
		}
		PrintToConsole(client,"StartDisabled %s: %d changed, %d not changed",s_cluster, n_changed, n_same);
	} else if(!strcmp(cmd,"load") || !strcmp(cmd,"drop")) {
		if(GetCmdArgs() < 3)
		{
			PrintToConsole(client,"expected cluster name");
			return Plugin_Stop;
		}
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetCmdArg(3, s_cluster,sizeof(s_cluster));
		new Handle:array_ids;
		new Handle:a_gen;
		
		if(!GetTrieValue(g_vGenCluster,s_cluster,array_ids))
		{
			PrintToConsole(client,"invalid cluster name %s", s_cluster);
			return Plugin_Stop;
		}

		if(array_ids == INVALID_HANDLE || GetArraySize(array_ids) == 0)
		{
			PrintToConsole(client,"cluster %s has invalid id array", s_cluster);
			return Plugin_Stop;
		}
	
		if(!strcmp(cmd,"load"))
		{
			new n_load;
			for(new i; i < GetArraySize(array_ids);++i)
			{
				new id = GetArrayCell(array_ids,i);
				new ret;
				if( (ret =ClientLoadGen(client, g_Player[WorkingSet],id)) > 0)
					n_load += ret;
			}
			PrintToConsole(client,"Loaded %d generators into the working set from cluster %s",n_load,s_cluster);
		} else if(!strcmp(cmd,"drop")) {
			new n_drop;
			for(new i; i < GetArraySize(array_ids);++i)
			{
				new id = GetArrayCell(array_ids,i);
				new idx;
				if( (idx = FindIndexInArrayById( g_Player[WorkingSet], id)) != -1)
				{
					n_drop++;
					RemoveFromArray( g_Player[WorkingSet], idx);
					if( g_Player[CurrentGeneratorIndex] >= idx)
						g_Player[CurrentGeneratorIndex]--;
				}
			}
			if(GetArraySize( g_Player[WorkingSet] ) == 0)
				g_Player[CurrentGeneratorIndex] = -1;

			PrintToConsole(client,"Dropped %d generators from the working set that are apart of cluster %s",n_drop,s_cluster);
		}


 	} else if(!strcmp(cmd,"spawn")) {
 		if(GetCmdArgs() < 3)
		{
			PrintToConsole(client,"expected cluster name");
			return Plugin_Stop;
		}
		decl String:s_cluster[MAX_CLUSTER_LEN];
		GetCmdArg(3, s_cluster,sizeof(s_cluster));
		new Handle:array_ids;
		new Handle:a_gen;
		
		if(!GetTrieValue(g_vGenCluster,s_cluster,array_ids))
		{
			PrintToConsole(client,"invalid cluster name %s", s_cluster);
			return Plugin_Stop;
		}

		new a_size;
		if(array_ids == INVALID_HANDLE || (a_size = GetArraySize(array_ids)) == 0)
		{
			PrintToConsole(client,"cluster %s has invalid id array", s_cluster);
			return Plugin_Stop;
		}
		new Handle:pack;

		CreateDataTimer(0.075,SpawnTestRuneTimerBlock,pack,TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);

		for(new i; i < a_size;++i)
		{
			WritePackCell(pack,GetArrayCell(array_ids,i));
		}

		ResetPack(pack);
		
		PrintToConsole(client,"spawning cluster %s",s_cluster);

		} else {
		PrintToConsole(client, "Unknown cluster command %s", cmd);
		return Plugin_Stop;
	}
	
	return Plugin_Changed;
}

public Action:EventClusterSetCmd(client,args)
{
	if(GetCmdArgs() < 3)
	{
		PrintToConsole(client,"sm_gen event <cluster> <create|remove|clone>");
		return Plugin_Handled;
	}
	decl String:s_cluster[MAX_CLUSTER_LEN];
	GetCmdArg(2, s_cluster,sizeof(s_cluster));

	decl String:cmd[16];
	GetCmdArg(3,cmd,sizeof(cmd));
	new Handle:event_arr = INVALID_HANDLE;

	FindEventCluster(s_cluster,event_arr);

	if(!strcmp(cmd,"clone"))
	{
		if(event_arr == INVALID_HANDLE)
		{
			PrintToConsole(client,"destination cluster has invalid event array");
			return Plugin_Handled;
		}
		if(GetCmdArgs() < 4)
		{
			PrintToConsole(client,"sm_gen event <cluster> clone <src cluster>");
			return Plugin_Handled;
		}
		decl String:s_cluster_src[MAX_CLUSTER_LEN];
		GetCmdArg(4,s_cluster_src,sizeof(s_cluster_src));
		new Handle:event_arr_src = INVALID_HANDLE;
		FindEventCluster(s_cluster_src, event_arr_src);
		if(event_arr_src == INVALID_HANDLE)
		{
			PrintToConsole(client,"source cluster has invalid event array");
			return Plugin_Handled;
		}
		new ret = CopyClusterEvents(event_arr, event_arr_src, false);	
		PrintToConsole(client,"Copied %d events from %s to %s", ret,s_cluster_src, s_cluster);
		return Plugin_Continue;
		
	}

	if(!strcmp(cmd,"create"))
	{
		new a_Event[GenEvent];

		if(GetCmdArgs() < 4)
		{
			PrintToConsole(client,"sm_gen event <cluster> create <Add|Remove|Spawn|Toggle>");
			return Plugin_Handled;
		}

		decl String:s_type[16];
		StrCopy(s_type,sizeof(s_type),"GenEvent_");
		GetCmdArg(4, s_type[9],sizeof(s_type)-9);

		if( (a_Event[ClusterType] = CheckClusterType(s_type)) == -1)
		{
			PrintToConsole(client,"Invalid ClusterGenType %s", s_type);
			return Plugin_Handled;
		}

		if(GetCmdArgs() < 5)
		{
			PrintToConsole(client,"sm_gen event <cluster> create ClusterGenType <ByClass|ByEnt|BySubClass>");
			return Plugin_Handled;
		}
		decl String:h_type[20];
		StrCopy(h_type,sizeof(h_type),"GenEvent_");
		GetCmdArg(5,h_type[9],sizeof(h_type)-9);

		if( (a_Event[EventType] = CheckEntityType(h_type)) == -1)
		{
			PrintToConsole(client,"Invalid HookType %s", s_type);
			return Plugin_Handled;
		}

		if(GetCmdArgs() < 6)
		{
			PrintToConsole(client,"sm_gen event <cluster> create ClusterGenType HookType <class name or entity name>");
			return Plugin_Handled;
		}

		decl String:e_type[MAX_EVENTGEN_CNAME];
		GetCmdArg(6,e_type,sizeof(e_type));

		if( a_Event[EventType] != GenEvent_ByEnt )
		{
			new idx;
			if(!GetTrieValue(g_CNameTrie,e_type,idx))
			{
				PrintToConsole(client,"Class name %s not supported", e_type);
				new String:buffer[512];
				strcopy(buffer,sizeof(buffer),"Supported class names: ");
				for(new i;i < sizeof(class_type);++i)
				{
					StrCat(buffer,sizeof(buffer),class_type[i]);
					StrCat(buffer,sizeof(buffer)," ");
				}
				PrintToConsole(client,buffer);
				return Plugin_Handled;
			}
			strcopy( a_Event[EventCName],sizeof(a_Event[EventCName]), e_type);
			a_Event[EventEntity][0] = '\0';
		} else {
			new class_idx;
			if(FindMapEntity(e_type,class_idx) == -1)
			{
				PrintToConsole(client,"Could not find entity named %s", e_type);
				DisplayMapEntities(e_type,client);
				return Plugin_Handled;
			}
			strcopy( a_Event[EventEntity], sizeof(a_Event[EventEntity]), e_type);
			strcopy( a_Event[EventCName], sizeof(a_Event[EventCName]), class_type[class_idx]);
		}

		if(GetCmdArgs() < 7)
		{
			PrintToConsole(client,"sm_gen event <cluster> create ClusterGenType HookType Name <Output>");
			DisplayEntOutputs(a_Event[EventCName], client);
			return Plugin_Handled;
		}

		decl String:out_type[MAX_EVENT_OUTPUTNAME];
		GetCmdArg(7,out_type,sizeof(out_type));

		if(CheckOutput(a_Event[EventCName], out_type) == -1)
		{
			PrintToConsole(client,"invalid output %s", out_type);
			DisplayEntOutputs(a_Event[EventCName], client);
			return Plugin_Handled;
		}
		strcopy(a_Event[EventOutput], sizeof(a_Event[EventOutput]), out_type);

		new bool:created = false;

		new t_Cluster[RuneCluster];
		if(event_arr == INVALID_HANDLE)
		{
			strcopy( t_Cluster[ClusterName], sizeof(t_Cluster[ClusterName]), s_cluster);
			t_Cluster[EventArray] = CreateArray(EVENT_BLOCK_SIZE);
			event_arr = t_Cluster[EventArray];
			created = true;
		}	
		if(add_event_info(event_arr, a_Event) == -1)
		{
			PrintToConsole(client,"Failed to add event to event stack.");
			return Plugin_Handled;
		}

		if(created)
		{
			add_cluster_info(g_vGenEvent,t_Cluster);
		}
		HookGenEvent(s_cluster, a_Event);
		PrintToConsole(client,"Event added");
	
	} else if(!strcmp(cmd,"remove")){ 
		if(event_arr == INVALID_HANDLE)
		{
			PrintToConsole(client,"invalid event array for cluster %s",s_cluster);
			return Plugin_Handled;
		}

		if(GetCmdArgs() < 4)
		{
			PrintToConsole(client,"sm_gen event %s remove <#event idx>",s_cluster);
			return Plugin_Handled;
		}
		new String:e_idx[8];
		TrimString(e_idx);
		GetCmdArg(4,e_idx,sizeof(e_idx));
		new idx = StringToInt(e_idx);
		
		if( (idx == 0 && e_idx[0] != '0') || idx < 0 || idx >= GetArraySize(event_arr))
		{
			PrintToConsole(client,"Invalid index %s in event stack. 0 to %d",
				e_idx, GetArraySize(event_arr));
			return Plugin_Handled;
		}
		new a_Event[GenEvent];
		GetArrayArray(event_arr,idx, a_Event, EVENT_BLOCK_SIZE);
		HookGenEvent(s_cluster, a_Event,true);
		RemoveFromArray(event_arr, idx);
		PrintToConsole(client,"Event removed.");
		
	} else
		PrintToConsole(client,"unknown subcommand %s", cmd);

	
		
	return Plugin_Continue;
}




public Action:OnGenCmd(client, args)
{
	decl String:cmdstr[128];

	if( args < 1 )
	{
		DisplayGlobalSet(client);
		DisplayWorkingSetCmd(client,args);
		return Plugin_Continue;
	}
	//GetCmdArgString(cmdstr, sizeof(cmdstr));

	decl String:param[14];
	GetCmdArg(1, param, sizeof(param));

	if(!strcmp(param,"load",false)) {
		LoadGenCmd(client,args);
	} else if(!strcmp(param,"create")) {
		CreateGenClientCmd(client);
	} else if(!strcmp(param,"display")) {
		DisplayWorkingSetCmd(client,args);
	} else if(!strcmp(param,"set")) {
		SetCurrentCmd(client,args);
	} else if(!strcmp(param,"merge")) {
		MergeWorkingSetCmd(client);
	} else if(!strcmp(param,"drop")) {
		DropWorkingSetCmd(client);
	} else if(!strcmp(param,"cluster")) {
		ClusterWorkingSetCmd(client,args);
	} else if(!strcmp(param,"event")) {
		EventClusterSetCmd(client,args);
	} else {
		PrintToConsole(client,"Unknown cmd %s\nUsage: sm_gen <create|display|set|merge|drop|cluster|event>", param);
	}

	return Plugin_Continue;
}

stock SpawnTestRune(const Handle:array, index)
{

	new Float:ori[3];
	new Float:ang[3];
	new Float:vel[3];

	new Float:force;

	new ent = CreatePropRune();
	//SetEntProp(ent, Prop_Send, "m_usSolidFlags", 0);    
	SetEntProp(ent, Prop_Send, "m_nSolidType", SOLID_NONE);
	DispatchSpawn(ent);
	ActivateEntity(ent);

	get_origin_info(array,index,ori);
	get_angle_info(array,index, ang);
	get_force_info(array, index, force);

#if 0
	PrintToServer("SpawnTest:: origin: %f %f %f\nangle: %f %f %f",
		ori[0], ori[1], ori[2],
		ang[0], ang[1], ang[2]);
#endif

	GetAngleVectors(ang, vel, NULL_VECTOR, NULL_VECTOR);
	ScaleVector(vel, force);
	CreateTimer(15.0, Timer_Test_RuneGen, ent);

	TeleportEntity(ent, ori, ang, vel);
}

public Action:Timer_Test_RuneGen(Handle:timer, any:data)
{
	AcceptEntityInput(data, "Kill");
}

stock IterateRuneGen(client, const Handle:vGen)
{
	if( g_Player[WorkingSet] != INVALID_HANDLE )
	{
		g_Player[CurrentGeneratorIndex] = 0;
		IterateTeleportPlayer(client, g_Player[WorkingSet], g_Player[CurrentGeneratorIndex]);
	} else {
		IterateTeleportPlayer(client, vGen, g_Player[CurrentGeneratorIndex]);
	}
}

static g_CurrentRuneId[MAXPLAYERS] = {0}

stock IterateTeleportPlayer(client, const Handle:vGen, &index, bool:advance_fwd = true)
{
	new size, _Id;

	if(advance_fwd)
		index++;
	else
		index--;
	
	if((size = GetArraySize(vGen)) == 0)
		return -1;

	if(index >= size || index < 0)
		index = (advance_fwd ? 0 : size-1);

	get_id_info(vGen,index, _Id);
	g_CurrentRuneId[client] = _Id;
	PrintToConsole(client,"Rune %d", _Id);

	new Float:ori[3];
	new Float:ang[3];

	get_origin_info(vGen,index, ori);
	get_angle_info(vGen,index, ang);
	TeleportEntity(client,ori,ang,NULL_VECTOR);

	SpawnTestRune(vGen, index);
	return 1;
}



stock ResetRuneArrays()
{

	new Handle:to_global = CreateStack(RUNE_BLOCK_SIZE);
	new Handle:to_disabled = CreateStack(RUNE_BLOCK_SIZE);

	for(new i;i < GetArraySize(g_vGen);)
	{
		new a_Gen[RuneGen];
		GetArrayArray(g_vGen,i,a_Gen,RUNE_BLOCK_SIZE);
		if(a_Gen[g_StartDisabled] == true)
		{
			PushStackArray(to_disabled,a_Gen,RUNE_BLOCK_SIZE);
			RemoveFromArray(g_vGen,i);
			continue;
		}
		i++;
	}

	for(new i;i< GetArraySize(g_vGenDisabled);)
	{
		new a_Gen[RuneGen];
		GetArrayArray(g_vGenDisabled,i,a_Gen,RUNE_BLOCK_SIZE);
		if(a_Gen[g_StartDisabled] == false)
		{
			PushStackArray(to_global,a_Gen,RUNE_BLOCK_SIZE);
			RemoveFromArray(g_vGenDisabled,i);
			continue;
		}
		i++;
	}


	new cnt;
	while(!IsStackEmpty(to_global))
	{
		++cnt;
		new a_Gen[GenEvent];
		PopStackArray(to_global,a_Gen,RUNE_BLOCK_SIZE);
		PushArrayArray(g_vGen,a_Gen,RUNE_BLOCK_SIZE);
	}

	while(!IsStackEmpty(to_disabled))
	{
		++cnt;
		new a_Gen[GenEvent];
		PopStackArray(to_disabled,a_Gen,RUNE_BLOCK_SIZE);
		PushArrayArray(g_vGenDisabled,a_Gen,RUNE_BLOCK_SIZE);
	}


	CloseHandle(to_global);
	CloseHandle(to_disabled);
	return cnt;
}

