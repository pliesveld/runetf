#include "extension.h"
#include "player.hpp"
#include "runemanager.hpp"
#include "natives.h"

cell_t Native_RuneSpawn(IPluginContext *pContext, const cell_t *params)
{
	if( params[1] == -1 )
	{
		return pContext->ThrowNativeError("Invalid Spawn location");
	}
	float fLoc = sp_ctof(params[1]);
	return 0;
}

cell_t Native_GetPlayerRuneId(IPluginContext *pContext, const cell_t *params)
{
	cell_t p_ent = params[1];
	Rune *rune = g_Players.RuneHeld(p_ent);
	if(rune)
	{
		return rune->Id;
	}
	return 0;
}


cell_t Native_RunePluginsStart(IPluginContext *p, const cell_t *arg)
{
	g_Runes.StartRunes();
	return 0;
}


cell_t Native_RunePluginsStop(IPluginContext *p, const cell_t *arg)
{
	g_Runes.StopRunes();
	return 0;
}


cell_t Native_RuneCount(IPluginContext *p, const cell_t *arg)
{
	cell_t ret = -1;
	return ret;
}

cell_t Native_PassByRef(IPluginContext *pContext, const cell_t *params)
{
	cell_t *addr;
	pContext->LocalToPhysAddr(params[1], &addr);
	*addr += 1;
	return 1;
}

cell_t Native_GetPlayerRuneName(IPluginContext *pContext, const cell_t *arg)
{
	char *str;
	
	cell_t p_ent = arg[1];
	Rune *rune = g_Players.RuneHeld(p_ent);
	
	if(rune)
	{
		str = rune->name;
		pContext->StringToLocal(arg[2],arg[3],str);
		return 1;
	}
	return 0;
	
}

cell_t Native_AddRune(IPluginContext *p, const cell_t *arg)
{
	char *str;
	p->LocalToString(arg[1], &str);

	//RunePluginManager::rune_iterator it = g_Runes.pluginCtx(p);
	RunePlugin *pl_ctx = g_Runes.GetPluginCtx(p);

	cell_t ret;
	cell_t r_id = 1;
	if(arg[0] < 2 || arg[2] == 0 || arg[3] == 0)
		return p->ThrowNativeError("Invalid arguments");
	if(!pl_ctx || !pl_ctx->plugin)
		return p->ThrowNativeError("Invalid plugin context");

	if(arg[0] > 3)
		r_id = arg[4];

	if(pl_ctx && pl_ctx->plugin)
		r_id = (pl_ctx->m_PlugId * 1000) + r_id;
	else
		r_id = 999000 + r_id;

	IPluginFunction *f_pick_cb = p->GetFunctionById(static_cast<funcid_t>(arg[2]))
		, *f_drop_cb 						 = p->GetFunctionById(static_cast<funcid_t>(arg[3]));

	ret = g_Runes.AddRune(str, f_pick_cb,f_drop_cb,r_id);
	if(ret == -1)
		return p->ThrowNativeError("RunePlugin::AddRune %s already exists.", str);
	
	Rune* p_rune = g_Runes.FindRuneByName(str);
	if(!pl_ctx->AddRuneToPlugin( str, p_rune ))
		return	p->ThrowNativeError("RunePlugin already had a rune named %s.", str);
	
	return r_id;
}

cell_t Native_PlayerPickupRandomRune(IPluginContext *p, const cell_t *arg)
{
	const char *str;
	if(arg[0] < 1)
		return p->ThrowNativeError("expecting client id");
	cell_t player = arg[1];

	str = g_Runes.RandomRune();

	return g_RuneTF.PlayerPickupRune(player, str);
}


cell_t Native_PlayerPickupRune(IPluginContext *p, const cell_t *arg)
{
	char *str;
	if(arg[0] < 1)
		return p->ThrowNativeError("expecting client id");
	else if(arg[0] < 2)
		return p->ThrowNativeError("expecting rune name");

	cell_t player = arg[1];
	p->LocalToString(arg[2], &str);

	if(str[0] == '\0')
		return p->ThrowNativeError("invalid rune string");
	return g_RuneTF.PlayerPickupRune(player, str);
}

cell_t Native_PlayerDropRune(IPluginContext *p, const cell_t *arg)
{
	char *str;
	if(arg[0] < 1)
		return p->ThrowNativeError("expecting client id");
	else if(arg[0] < 2)
		return p->ThrowNativeError("expecting force option");

	cell_t player = arg[1];
	return g_RuneTF.PlayerDropRune(player,arg[2]);
}


cell_t Native_RuneNameById(IPluginContext *p, const cell_t *arg)
{
	char *str;
	
	cell_t rune_id = arg[1];
	Rune *rune = g_Runes.FindRuneById(rune_id);
	if(rune)
	{
		str = rune->name;
		p->StringToLocal(arg[2],arg[3],str);
		return 1;
	}
	return 0;
}

cell_t Native_RuneIdByName(IPluginContext *p, const cell_t *arg)
{
	char *str;
	if(arg[0] < 1)
		return p->ThrowNativeError("Expected string");

	p->LocalToString(arg[1], &str);

	if(str == NULL || str[0] == '\0')
		return p->ThrowNativeError("Invalid string argument");

	Rune *rune = g_Runes.FindRuneByName(str);
	if(!rune)
		return 0;
	return rune->Id;
}

cell_t Native_GetRuneIds(IPluginContext *p, const cell_t *arg)
{

	cell_t *array;

	if(arg[2] < 1)
	{
		return 0;
	}

	p->LocalToPhysAddr(arg[1], &array);
	
	return g_Runes.GetAllRuneIds(array, arg[2]);
}


cell_t Native_RunesPause(IPluginContext *p, const cell_t *arg)
{
	if(arg[0] < 1)
		return p->ThrowNativeError("Expected plugin handle to pause.");

	Handle_t hndl = static_cast<Handle_t>(arg[1]);
	HandleError h_err;

	IPlugin *plug;

	if ((plug = plsys->PluginFromHandle(hndl,&h_err)) != NULL)
		return plug->SetPauseState(true);
	
	return 0;

}

cell_t Native_RunesUnpause(IPluginContext *p, const cell_t *arg)
{
	if(arg[0] < 1)
		return p->ThrowNativeError("Expected plugin handle to pause.");

	Handle_t hndl = static_cast<Handle_t>(arg[1]);
	HandleError h_err;

	IPlugin *plug;

	if ((plug = plsys->PluginFromHandle(hndl,&h_err)) != NULL)
		return plug->SetPauseState(false);
	
	return 0;


}
