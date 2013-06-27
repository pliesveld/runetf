#if !defined __eff_particles__
#define __eff_particles__

#include <sdktools_functions>
#include <sdktools_entinput>
stock CreateParticleAtEntity(iEntity, String:sParticle[], Float:lifetime=10.0, bool:bAttach = false, Float:delay = 0.0)
{
	new iParticle = CreateEntityByName("info_particle_system");
	if (IsValidEdict(iParticle))
	{
		decl Float:fPosition[3];
		GetEntPropVector(iEntity, Prop_Send, "m_vecOrigin", fPosition);
		
		if(delay != 0.0)
			DispatchKeyValue(iParticle,"start_active","0");

		TeleportEntity(iParticle, fPosition, NULL_VECTOR, NULL_VECTOR);
		DispatchKeyValue(iParticle, "effect_name", sParticle);
		
		if (bAttach)
		{
			SetVariantString("!activator");
			AcceptEntityInput(iParticle, "SetParent", iEntity, iParticle, 0);			
		}

		DispatchSpawn(iParticle);
		ActivateEntity(iParticle);

		decl String:sInput[32]="";
		if(delay != 0.0)
			AcceptEntityInput(iParticle, "Start");
		else
		{
			Format(sInput,sizeof(sInput),
				"OnUser1 !self:Start::%f:-1",delay);
			SetVariantString(sInput);
			AcceptEntityInput(iParticle, "AddOutput");
		}

		if(lifetime != 0.0)
		{
			Format(sInput,sizeof(sInput),
				"OnUser1 !self:Kill::%f:-1", lifetime);
			SetVariantString(sInput);
			AcceptEntityInput(iParticle, "AddOutput");
		}

		if(delay != 0.0 || lifetime != 0.0)
			AcceptEntityInput(iParticle, "FireUser1");
	}
	return iParticle;
}

stock CreateParticle(iEntity, String:sParticle[], bool:bAttach = false)
{
	return CreateParticleAtEntity(iEntity,sParticle,_,bAttach);
}

#endinput
