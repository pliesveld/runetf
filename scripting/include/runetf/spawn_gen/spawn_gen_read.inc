

new String:mapListPath[PLATFORM_MAX_PATH];

bool:ReadRuneFile(client = 0)
{
	decl String:mapPath[PLATFORM_MAX_PATH];
	Format(mapPath,sizeof(mapPath),"%s%s.cfg", mapListPath, g_mapName);
 
	ClearGlobalRuneArray();
	ResetGlobalVars();
	g_bEventsHooked = false;
	
	if(!FileExists(mapPath))
	{
		PrintToConsole(client,"Failed to open %s", mapPath);
		return false;
	}


	new Handle:hSMC = SMC_CreateParser()
	SMC_SetParseStart(hSMC, SMCStartParse);
	SMC_SetParseEnd(hSMC, SMCEndParse);

	return ProcessFile(hSMC,mapPath) == SMCError_Okay;
}

stock SMCError:ProcessFile(&Handle:hSMC, const String:aRuneMapFile[])
{
	new iLine;
	new SMCError:ReturnedError = SMC_ParseFile(hSMC, aRuneMapFile, iLine);
	if (ReturnedError != SMCError_Okay)
	{
		decl String:sError[256];
		SMC_GetErrorString(ReturnedError, sError, sizeof(sError));
		if (iLine > 0)
		{
			LogError("[PP] Could not parse file (Line: %d, File \"%s\"): %s.", iLine, aRuneMapFile, sError);
			CloseHandle(hSMC);
			return ReturnedError;
		}
		
		LogError("[PP] Parser encountered error (File: \"%s\"): %s.", aRuneMapFile, sError);
	}
	CloseHandle(hSMC);
	return SMCError_Okay;
}

ResetGlobalVars()
{
	g_NextGenId = 1;
	g_vGen = CreateArray(RUNE_BLOCK_SIZE);
	g_vGenDisabled = CreateArray(RUNE_BLOCK_SIZE);
	g_vGenCluster = CreateTrie();
	g_vGenEvent = CreateArray(CLUSTER_BLOCK_SIZE);
	g_ClusterNames = CreateArray(MAX_CLUSTER_LEN);
}

public SMCStartParse(Handle:hSMC)
{
	SMC_SetReaders(hSMC, SMCNewSection, SMCReadKeyValues_Generator, SMCEndSection_Generator);
}

public SMCEndParse(Handle:smc, bool:halted, bool:failed)
{
#if 0
	new _id;
	new Float:_ori[3];
	new Float:_ang[3];
	new Float:_force;

	new aSize = GetArraySize(g_vGen);
	for(new i = 0; i < aSize; i++)
	{
		get_id_info(g_vGen,     i, _id);
		get_origin_info(g_vGen, i, _ori);
		get_angle_info(g_vGen,  i, _ang)
		get_force_info(g_vGen,  i, _force);


	PrintToServer("ID %d\nOri %f %f %f\nAng %f %f %f\nForce %f\n", _id,
			_ori[0], _ori[1], _ori[2],
			_ang[0], _ang[1], _ang[2],
			_force);
	}
#endif
	//InitGenEvents(); // Call from TF2_WaitingForPlayersEnd?
	g_bEventsHooked = false;
}

public SMCResult:SMCNewSection(Handle:smc, const String:name[], bool:opt_quotes)
{

	//LogMessage("NewSection %s ======== ", name);

	switch(name[0])
	{
		case 'c':
		{
			if(!strcmp(name,"cluster"))
			{
				//ResetTempCluster(t_Cluster);
		//		LogMessage("cluster");
				SMC_SetReaders(smc, SMCNewSection_Cluster, SMCReadKeyValues_Cluster, SMCEndSection_Cluster);
				return SMCParse_Continue;
			}
			return SMCParse_Halt;
		}
		case 'g':
		{
			if(!strcmp(name, "generator"))
			{
				ResetTempGen(t_Array);
				SMC_SetReaders(smc, SMCNewSection, SMCReadKeyValues_Generator, SMCEndSection_Generator);
				return SMCParse_Continue;
			}

			return SMCParse_Halt;
		}
		default:
		{
			return SMCParse_Halt;
		}
	}

	return SMCParse_Continue;
}

public SMCResult:SMCReadKeyValues_Generator(Handle:smc, const String:key[], const String:value[], bool:key_quotes, bool:value_quotes)
{
	//LogMessage("Section::Gen %s => %s", key, value);
	//LogMessage("SMC::kv %-10s => %-10s", key, value);
	new Float:aVec[3];
	new Float:aFloat;
	new aInt;
	new bool:aBool;
	new String:buffer[32];

	switch(key[0])
	{
		case 'v':
		{
			new i = SplitString(value,",",buffer,sizeof(buffer)) // Should Check if SplitString returns -1 
			aVec[0] = StringToFloat(buffer);
			i += SplitString(value[i],",", buffer, sizeof(buffer))
			aVec[1] = StringToFloat(buffer); // should probably write a helper function so this isn't so ugly.
			Format(buffer, sizeof(buffer), "%s", value[i]);
			aVec[2] = StringToFloat(buffer);
		}
		case 'f':
		{
			aFloat = StringToFloat(value);
		}
		case 'i':
		{
			aInt = StringToInt(value);
		}
		case 'b':
		{
			if(StrEqual(value,"true",false))
				aBool = true;
			else if(StrEqual(value,"false",false))
				aBool = false;
			else 
			{
				if(IsCharNumeric(value[0]) && value[0] != '0')
					aBool = true;
				else
					aBool = false;
			}
		}
		case 's':
		{
		}

		default:
		{
			LogError("Unknown key type %c", key[0]);
			return SMCParse_Halt;
		}
	}

	if(!strncmp(key[1],"Origin",6))
	{
		SetTempGenVec(t_Array,_:g_ori, aVec);
	} else if(!strncmp(key[1],"Angle",5)) {
		SetTempGenVec(t_Array,_:g_ang, aVec);
	} else if(!strncmp(key[1],"Force",5)) {
		SetTempGenInt(t_Array,_:g_force, aFloat);
	} else if(!strncmp(key[1],"Id",2)) {
		SetTempGenInt(t_Array,_:Id, aInt);
		if(aInt >= g_NextGenId)
			g_NextGenId = aInt + 1;
	} else if(!strncmp(key[1],"Cluster",7)) {
		new r_id;
		GetTempGenInt(t_Array, _:Id, r_id);
		AddToCluster(value, r_id, true);
	} else if(!strncmp(key[1],"Disabled",8)) {
		t_Array[g_StartDisabled] = aBool;
	} else {
		LogError("Unknown key %s", key);
		return SMCParse_Halt;
	}
	
	return SMCParse_Continue;
}

public SMCResult:SMCEndSection_Generator(Handle:smc)
{
	//LogMessage("EndSection_Generator");
	if( t_Array[g_StartDisabled] )
		add_rune_info(g_vGenDisabled, t_Array);
	else 
		add_rune_info(g_vGen, t_Array);
		
	return SMCParse_Continue;

}


