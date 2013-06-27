#include "player.hpp"
#include "extension.h"

#include "rune.hpp"

PlayerManager g_Players;

bool player_t::hasRune() const { return m_rune != NULL; }
Rune *player_t::RuneHeld() const { return m_rune; }


int player_t::OnPickup(Rune *r)
{
	m_rune = r;
	m_runeId = r->Id;
	m_PickupCooldown = 0;
	m_UsageCooldown = 0;
	m_active = true;
	return 1;
}

int player_t::OnDrop()
{
	m_rune = NULL;
	m_runeId = 0;
	m_PickupCooldown = 0;
	m_UsageCooldown = 0;
	m_active = false;
	return 1;
}

Rune *PlayerManager::RuneHeld(int client) const { return v_players[client].RuneHeld(); }
bool PlayerManager::hasRune(int client) const { return v_players[client].hasRune();  }



void PlayerManager::OnClientDisconnecting(int client)
{
	player_t &p(g_Players.v_players[client]);  
	Rune *rune = p.m_rune;

	//Msg("Client %d disconnecting\n", client);

	if(rune)
	{
		rune->Drop(client);
	}
	p.OnDrop();
	
	v_players[client] = player_t();
	v_players[client].m_active = false;
}

void PlayerManager::OnClientPutInServer(int client)
{
	v_players[client] = player_t();
	v_players[client].m_active = true;
}

void PlayerManager::RemoveRuneFromAllPlayers(int id)
{
	player_iterator it = v_players.begin();
	for(;it != v_players.end();++it)
	{
		if(it->m_runeId == id)
			it->OnDrop();
	}
}
