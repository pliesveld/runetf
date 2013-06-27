#if defined __runesgentempent_stock__
 #endinput
#endif
#define __runesgentempent_stock__


#include <sdktools_tempents_stocks>

static mdlindex;

stock CacheModels()
{
	mdlindex = PrecacheModel("materials/sprites/healbeam.vmt");
}


stock TE_SetupBeamEnts(StartEntity, EndEntity, ModelIndex, HaloIndex, StartFrame, FrameRate, Float:Life,  
                Float:Width, Float:EndWidth, FadeLength, Float:Amplitude, const Color[4], Speed) 
{ 
    TE_Start("BeamEnts"); 
    TE_WriteEncodedEnt("m_nStartEntity", StartEntity); 
    TE_WriteEncodedEnt("m_nEndEntity", EndEntity); 
    TE_WriteNum("m_nModelIndex", ModelIndex); 
    TE_WriteNum("m_nHaloIndex", HaloIndex); 
    TE_WriteNum("m_nStartFrame", StartFrame); 
    TE_WriteNum("m_nFrameRate", FrameRate); 
    TE_WriteFloat("m_fLife", Life); 
    TE_WriteFloat("m_fWidth", Width); 
    TE_WriteFloat("m_fEndWidth", EndWidth); 
    TE_WriteFloat("m_fAmplitude", Amplitude); 
    TE_WriteNum("r", Color[0]); 
    TE_WriteNum("g", Color[1]); 
    TE_WriteNum("b", Color[2]); 
    TE_WriteNum("a", Color[3]); 
    TE_WriteNum("m_nSpeed", Speed); 
    TE_WriteNum("m_nFadeLength", FadeLength); 
}  

stock TE_PlayerToTeammates(client, clients[], len)
{

	new colour[4];
	colour[0] = 255;
	colour[1] = 0;
	colour[2] = 0;
	colour[3] = 255;

	if(!IsValidEntity(client))
		return;
	if(!IsPlayerAlive(client))
		return;
	for(new i; i < len; ++i)
	{
		new other = clients[i];

		if(!IsValidEntity(other) || !IsPlayerAlive(other))
			continue;

		TE_SetupBeamEnts(client, other, mdlindex, 0, 0, 0, 10.0, 1.0, 1.0, 0, 0.0, colour, 0);
		TE_Send(clients, len, 0.1);
	}
}



#if 0
stock CreateParticle(iEntity, String:sParticle[], bool:bAttach = false)
{
  new iParticle = CreateEntityByName("info_particle_system");
  if (IsValidEdict(iParticle))
  {
    decl Float:fPosition[3];
    GetEntPropVector(iEntity, Prop_Send, "m_vecOrigin", fPosition);

    TeleportEntity(iParticle, fPosition, NULL_VECTOR, NULL_VECTOR);
    DispatchKeyValue(iParticle, "effect_name", sParticle);

    if (bAttach)
    {
      SetVariantString("!activator");
      AcceptEntityInput(iParticle, "SetParent", iEntity, iParticle, 0);
    }

    DispatchSpawn(iParticle);
    ActivateEntity(iParticle);
    AcceptEntityInput(iParticle, "Start");
  }
  return iParticle;
}


#endif

