#if defined __rune_player_stock__
 #endinput
#endif
#define __rune_player_stock__

stock TeammatesNearPlayer(client, clients[], Float:distance = 475.0)
{
	new team = GetClientTeam(client);
	return NearPlayer(client, team, clients, distance);
}

stock EnemiesNearPlayer(client, clients[], Float:distance = 475.0)
{
	new team = GetClientTeam(client);
	team = ((team == 2) ? 3 : 2);
	return NearPlayer(client, team, clients, distance);
}

stock AllNearPlayer(client, clients[], Float:distance = 475.0)
{
	return NearPlayer(client, 0, clients, distance);
}


stock NearPlayer(client, team, clients[], Float:distance = 475.0)
{
	new total = 0;
	if(!IsValidEntity(client)||!IsClientInGame(client))
		return total;

	new Float:vecOri_client[3];
	new Float:vecOri_other[3];
	GetClientAbsOrigin(client, vecOri_client);

	for (new i=1; i<=MaxClients; ++i)
  {
    if (IsClientInGame(i) && IsPlayerAlive(i) && i != client)
    {
			if(team && GetClientTeam(i) != team)
				continue;
			GetClientAbsOrigin(i, vecOri_other);
			if( distance < GetVectorDistance(vecOri_client, vecOri_other))
				continue;
			clients[total++] = i;
    }
  }
	return total;
}

stock OriginNearPlayer(client, Float:loc[3], range = 100)
{
	new Float:ori[3];
	new Float:ang[3];
	GetClientEyePosition(client, ori)
	GetClientEyeAngles(client, ang); 

	loc[0] = (ori[0]+(range*((Cosine(DegToRad(ang[1]))) * (Cosine(DegToRad(ang[0]))))));    
	loc[1] = (ori[1]+(range*((Sine(DegToRad(ang[1]))) * (Cosine(DegToRad(ang[0]))))));      
	ang[0] -= (2*ang[0]);                                                                 
	loc[2] = (ori[2]+(range*(Sine(DegToRad(ang[0])))));                                     

}


stock TF2_GetCurrentWeaponClass(client, String:name[], maxlength)
{
  if( client > 0 )
  {
    new index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if (index > 0)
      GetEntityNetClass(index, name, maxlength);
  }
}

stock TF2_GetHealingTarget(client)
{
  new String:classname[64];
  TF2_GetCurrentWeaponClass(client, classname, sizeof(classname));

  if(StrEqual(classname, "CWeaponMedigun"))
  {
    new index = GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
    if( GetEntProp(index, Prop_Send, "m_bHealing") == 1 )
    {
      return GetEntPropEnt(index, Prop_Send, "m_hHealingTarget");
    }
  }
  return -1;
}


