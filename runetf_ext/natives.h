#ifndef _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
#define _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_


#define NATIVE_STATIC(func) { #func, Native_##func },
#define NATIVE_PROTO(func)  cell_t Native_##func(IPluginContext *pContext, const cell_t *params);


NATIVE_PROTO(AddRune)
NATIVE_PROTO(PlayerPickupRune)
NATIVE_PROTO(PlayerDropRune)
NATIVE_PROTO(PlayerPickupRandomRune)
NATIVE_PROTO(RunePluginsStart)
NATIVE_PROTO(RunePluginsStop)
NATIVE_PROTO(GetPlayerRuneId)
NATIVE_PROTO(GetPlayerRuneName)
NATIVE_PROTO(GetRuneIds);
NATIVE_PROTO(RuneSpawn)
NATIVE_PROTO(RuneNameById);
NATIVE_PROTO(RuneIdByName);
NATIVE_PROTO(RunesPause);
NATIVE_PROTO(RunesUnpause);




const sp_nativeinfo_t g_Natives[] = 
{
	NATIVE_STATIC(AddRune)
	NATIVE_STATIC(PlayerPickupRune)
	NATIVE_STATIC(PlayerDropRune)
	NATIVE_STATIC(PlayerPickupRandomRune)

	NATIVE_STATIC(RunePluginsStart)
	NATIVE_STATIC(RunePluginsStop)

	NATIVE_STATIC(GetPlayerRuneId)
	NATIVE_STATIC(GetPlayerRuneName)
	NATIVE_STATIC(GetRuneIds)
	NATIVE_STATIC(RuneNameById)
	NATIVE_STATIC(RuneIdByName)

	NATIVE_STATIC(RunesPause)
	NATIVE_STATIC(RunesUnpause)




	NATIVE_STATIC(RuneSpawn)
	{NULL,					NULL},
};

extern RuneSDK g_RuneTF;

#endif
