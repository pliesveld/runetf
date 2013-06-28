
static g_User = 0;

public Action:OnSaveCmd(client, args)
{
	if( (g_vGen == INVALID_HANDLE && g_vGenDisabled) || (GetArraySize(g_vGen) < 1 && GetArraySize(g_vGenDisabled) < 1))
	{
		PrintToConsole(client,"invalid rune generator array or empty");
		return Plugin_Stop;
	}
	new count;

	decl String:mapPath[PLATFORM_MAX_PATH];
	PrintToConsole(client,"Saving rune generator file");
	g_User = client;
	new Handle:g_array[2];
	g_array[0] = g_vGen;
	g_array[1] = g_vGenDisabled;
#if 0
	new Handle:hSMC = SMC_CreateParser();

  SMC_SetParseStart(hSMC, SMCStartParseTest);
  SMC_SetParseEnd(hSMC, SMCEndParseTest);

	GetTestGenConfigPath(client,mapPath, sizeof(mapPath));
	for(new i; i < 2;++i)
	{
		if( (n_wrote = WriteConfig(mapPath,g_array[i])) < 1)
		{
			new String:buffer[128];
			SMC_GetErrorString(count, buffer, sizeof(buffer));
			PrintToConsole(client,"temp write file: %s",buffer);
			CloseHandle(hSMC);
			return Plugin_Stop;
		}
		count += n_wrote;
	}

	if( ProcessFile(hSMC, mapPath) != SMCError_Okay )
	{
		new String:buffer[128];
		SMC_GetErrorString(count, buffer, sizeof(buffer));
		PrintToConsole(client,"temp read file: %s",buffer);
		CloseHandle(hSMC);
		return Plugin_Stop;
	}
	PrintToConsole(client, "test write okay: #gen %d written",count);
#endif
	

	GetRuneGenMapConfigPath(mapPath, sizeof(mapPath));
	new Handle:File = WriteConfigHeader(mapPath);

	for(new i; i < 2;++i)
	{
		new n_wrote = WriteRuneGenerators(File, g_array[i]);
		// if( n_wrote == -1 . . .
		count += ( n_wrote != -1 ? n_wrote : 0);
	}

	new n_events;
	new n_clusters = WriteRuneClusters(File, g_vGenEvent,n_events);
	FlushFile(File);
	CloseHandle(File);

	decl String:buffer[256]="";
	Format(buffer,sizeof(buffer),"Wrote to file; wrote %d generators.  wrote %d clusters.  wrote %d events.", count, n_clusters,n_events);

	if(client > 0)
		Client_PrintToChat(client,false,buffer);

	LogMessage(buffer);
	return Plugin_Changed;
}

static Handle:test_vGen;


public SMCStartParseTest(Handle:hSMC)
{
	test_vGen = CreateArray(RUNE_BLOCK_SIZE);
	SMC_SetReaders(hSMC, SMCNewSection, SMCReadKeyValues_Generator, SMCEndSectionTest);
}

public SMCEndParseTest(Handle:smc, bool:halted, bool:failed)                                  
{
  new _id;
  new Float:_ori[3];
  new Float:_ang[3];
  new Float:_force;                                                                       

  new aSize = GetArraySize(g_vGen);
  for(new i = 0; i < aSize; i++)
  {
    get_id_info(g_vGen,     i, _id);                                                      
    get_origin_info(g_vGen, i, _ori);
    get_angle_info(g_vGen,  _:i, _ang)                                                    
    get_force_info(g_vGen,  i, _:_force);                                                 


    PrintToConsole(g_User,"ID %d\nOri %f %f %f\nAng %f %f %f\nForce %f\n", _id,                     
      _ori[0], _ori[1], _ori[2],                                                          
      _ang[0], _ang[1], _ang[2],                                                          
      _force); 
  }                                                                                       

}



public SMCResult:SMCEndSectionTest(Handle:smc)
{
	add_rune_info(test_vGen, t_Array);
}

GetRuneGenMapConfigPath(String:file[], len)
{
	Format(file,len,"%s%s.cfg", mapListPath, g_mapName);
}

stock GetTestGenConfigPath(client,String:file[], len)
{
	Format(file,len,"%stemp_%d_%s.cfg", client,mapListPath, g_mapName);
}
Handle:WriteConfigHeader(const String:runeGenFile[])
{
	new Handle:File = OpenFile(runeGenFile,"w");
	if(File == INVALID_HANDLE)
		return ThrowError("Could not open %s for writing.", runeGenFile);
	return File;
}

stock Handle:WriteConfig(const String:runeGenFile[], Handle:array, Handle:array2)
{
	new Handle:File = OpenFile(runeGenFile,"w");
	if(File == INVALID_HANDLE)
		return ThrowError("Could not open %s for writing.", runeGenFile);

	return INVALID_HANDLE;
}

WriteRuneGenerators(Handle:File, Handle:array)
{
	new i;
	for(i = 0; i < GetArraySize(array); ++i)
	{
		new Float:_ori[3];
		new Float:_ang[3];
		new Float:_force;
		new bool:disabled = false;
		new _id;

		new a_Gen[RuneGen];

		GetArrayArray(array, i, a_Gen, RUNE_BLOCK_SIZE);
		
		_id = a_Gen[Id];
		GetTempGenVec(a_Gen, _:g_ori, _ori);
		GetTempGenVec(a_Gen, _:g_ang, _ang);
		_force = a_Gen[g_force];
		disabled = a_Gen[g_StartDisabled];
		

		WriteFileLine(File, "generator\n{");
		WriteFileLine(File, "\t\"iId\" \"%d\"", _id);
		WriteFileLine(File, "\t\"vOrigin\" \"%f,%f,%f\"", _ori[0], _ori[1], _ori[2]);
		WriteFileLine(File, "\t\"vAngle\" \"%f,%f,%f\"", _ang[0], _ang[1], _ang[2]);
		WriteFileLine(File, "\t\"fForce\" \"%f\"", _force);
		//WriteFileLine(File, "\t\"bDisabled\" \"%s\"",  (disabled ? "true" : "false"));
		if(disabled)
			WriteFileLine(File,"\t\"bDisabled\" \"true\"");
		WriteClusterString(File, _id);
		WriteFileLine(File, "}");
	}
	return i;
}

WriteClusterEventsArray(Handle:File, Handle:arr_events)
{
	new size;
	if(arr_events == INVALID_HANDLE || (size=GetArraySize(arr_events)) < 1)
		return 0;


	new i;
	for(; i < size;++i)
	{
		new a_Event[GenEvent];
		GetArrayArray(arr_events, i, a_Event, EVENT_BLOCK_SIZE);
		WriteFileLine(File,"\tevent\n\t{");
		WriteFileLine(File, "\t\t\"action\" \"%s\"", actionString[ a_Event[ClusterType] ]);
		WriteFileLine(File, "\t\t\"type\" \"%s\"", entityString[ a_Event[EventType] ]);
		WriteFileLine(File, "\t\t\"cname\" \"%s\"", a_Event[EventCName] );
		if(a_Event[EventEntity][0] != '\0')
			WriteFileLine(File, "\t\t\"entity\" \"%s\"", a_Event[EventEntity]);
		WriteFileLine(File, "\t\t\"output\" \"%s\"", a_Event[EventOutput] );

		WriteFileLine(File,"\t}\n");
	}
	return i;
}


WriteRuneClusters(Handle:File, Handle:arr_clusters, &event_counter)
{
	new i;
	for(i = 0; i < GetArraySize(arr_clusters);++i)
	{
		new a_Cluster[GenEvent];
		GetArrayArray(arr_clusters, i,a_Cluster, CLUSTER_BLOCK_SIZE);
		WriteFileLine(File, "cluster\n{\n\tcluster_name \"%s\"", a_Cluster[ClusterName]);
		event_counter += WriteClusterEventsArray(File, a_Cluster[EventArray]);
		WriteFileLine(File,"}\n");
	}
	return i;
}


WriteClusterString(Handle:File, id)
{
	decl String:cluster_name[MAX_CLUSTER_LEN];
	new size;
	if( (size = GetArraySize(g_ClusterNames)) < 1)
		return 0;

	new cnt;
	for(new i; i < size;++i)
	{
		GetArrayString(g_ClusterNames,i,cluster_name, sizeof(cluster_name));
		new Handle:c_array;
		if(!GetTrieValue(g_vGenCluster, cluster_name, c_array))
			continue;
		if(FindValueInArray(c_array, id) == -1)
			continue;
		WriteFileLine(File, "\t\"sCluster\" \"%s\"\n",cluster_name);
		cnt++;
	}
	return cnt;
}
