/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include <sourcemod_version.h>
#include "extension.h"

#include "player.hpp"
#include "runemanager.hpp"

#include "natives.h"

/**
 * @file extension.cpp
 * @brief RuneTF extension manages the rune state of players who hold the runes.  It is responsible for calling the plugins' start/stop rune call back.
 */

RuneSDK g_RuneTF;

SMEXT_LINK(&g_RuneTF);

IUniformRandomStream *engrandom = NULL;
IGameEventManager2 *gameevents = NULL;
CGlobalVars *gpGlobals = NULL;


bool RuneSDK::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	g_pSM->LogMessage(myself,"SDK::Load");
	sharesys->AddNatives(myself,g_Natives);

	
	plsys->AddPluginsListener(this);
	playerhelpers->AddClientListener(this);
	if (!gameevents->FindListener(this, "player_death"))
	{
		/* Then add ourselves */
		if (!gameevents->AddListener(this, "player_death", true))
		{
			/* If event doesn't exist... */
			return false;
		}
	}

	
	return true;
}

/* IGameEventListener2::FireGameEvent */
void RuneSDK::FireGameEvent(IGameEvent *pEvent)
{
	int userid = pEvent->GetInt("userid");
	int client = playerhelpers->GetClientOfUserId(userid);

  player_t &p(g_Players.v_players[client]);
  Rune *rune = p.m_rune;
	
	if(rune)
	{
		rune->Drop(client);
	}

	p.OnDrop();


}


void RuneSDK::SDK_OnAllLoaded()
{
	g_pSM->LogMessage(myself,"runetf::AllLoad");
	g_Runes.Initialize();
}

void RuneSDK::SDK_OnUnload()
{
	g_pSM->LogMessage(myself,"runetf::UnLoad");
	plsys->RemovePluginsListener(this);
	gameevents->RemoveListener(this);
	playerhelpers->RemoveClientListener(this);
	g_Runes.Shutdown();
}

void RuneSDK::SDK_OnPauseChange(bool paused)
{
	g_pSM->LogMessage(myself,"SDK::Pause");
}

bool RuneSDK::QueryRunning(char *error, size_t maxlength)
{
	return true;
}

#if defined SMEXT_CONF_METAMOD

bool RuneSDK::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	gpGlobals = ismm->GetCGlobals();

  GET_V_IFACE_CURRENT(GetEngineFactory, engrandom, IUniformRandomStream, VENGINE_SERVER_RANDOM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);

	//g_pSM->LogMessage(myself,"MMSDK::Load");
	return true;
}

bool RuneSDK::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	g_pSM->LogMessage(myself,"MMSDK::UnLoad");
	return true;
}

bool RuneSDK::SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength)
{
	g_pSM->LogMessage(myself,"MMSDK::Pause");
	return true;
}
#endif

const char *RuneSDK::GetExtensionVerString()
{
  return SM_VERSION_STRING;
}

const char *RuneSDK::GetExtensionDateString()
{
  return SM_BUILD_TIMESTAMP;
}


void RuneSDK::OnClientDisconnecting(int client)
{
	return g_Players.OnClientDisconnecting(client);
}

void RuneSDK::OnClientPutInServer(int client)
{
	return g_Players.OnClientPutInServer(client);
}

void RuneSDK::OnPluginLoaded(IPlugin *plugin)
{
	IPluginFunction *cb1 = NULL, *cb2 = NULL;
	const char* filen = plugin->GetFilename();
	IPluginContext* pc = plugin->GetBaseContext();

	cb1 = pc->GetFunctionByName("RunePluginStart");
	cb2 = pc->GetFunctionByName("RunePluginStop");

	if( cb1 == NULL || cb2 == NULL)
		return;

	g_Runes.RegisterPlugin(pc,cb1,cb2,filen);
}

void RuneSDK::OnPluginUnloaded(IPlugin *plugin)
{
	IPluginContext* pc = plugin->GetBaseContext();
	g_Runes.RemovePlugin(pc);
}

void RuneSDK::OnPluginPauseChange(IPlugin *plugin, bool paused)
{
}


int RuneSDK::PlayerPickupRune(int player, const char *rune_name)
{
	player_t &p(g_Players.v_players[player]);

	Rune *t_rune;
		
	if(p.m_rune != NULL)
		return 0;

	t_rune = g_Runes.FindRuneByName(rune_name);
	if(t_rune == NULL )
		return -1;
	
	p.OnPickup(t_rune);
	t_rune->Pickup(player);
	return 1;
}

int RuneSDK::PlayerDropRune(int player, bool force)
{
	player_t &p(g_Players.v_players[player]);

	Rune *t_rune = p.m_rune;

	if(t_rune == NULL )
		return -1;
	
	if(!force && t_rune->flags & TFRUNE_FLAG_NODROP)
		return 0;

	t_rune->Drop(player);
	p.OnDrop();
	return 1;
}
