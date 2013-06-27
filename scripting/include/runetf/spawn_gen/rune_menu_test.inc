

public Action:OnTestTime(client,args)
{
	if (args < 1)
	{
		ReplyToCommand(client, "[SM] Usage: sm_addtime <amount>");
		return Plugin_Handled;
	}

	decl String:cmdArg[32];
	GetCmdArg(1, cmdArg, sizeof(cmdArg));

	new entityTimer = FindEntityByClassname(-1, "team_round_timer");
	if (entityTimer > -1)
	{

		decl String:mapName[32];
		GetCurrentMap(mapName, sizeof(mapName));

		SetVariantInt(5800);
		AcceptEntityInput(entityTimer, "SetMaxTime");

		if (strncmp(mapName, "pl_", 3) == 0)
		{
			decl String:buffer[32];
			Format(buffer, sizeof(buffer), "0 %i", StringToInt(cmdArg));
			SetVariantString(buffer);
			AcceptEntityInput(entityTimer, "AddTeamTime");
		}
		else
		{
			SetVariantInt(StringToInt(cmdArg));
			AcceptEntityInput(entityTimer, "AddTime");
		}
	}
	else
	{
		new Handle:timelimit = FindConVar("mp_timelimit");
		SetConVarFloat(timelimit, GetConVarFloat(timelimit) + (StringToFloat(cmdArg) / 60));
		CloseHandle(timelimit);
	}

	return Plugin_Handled;
}




