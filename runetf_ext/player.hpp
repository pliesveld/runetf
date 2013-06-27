#ifndef __player_hpp__
#define __player_hpp__
#include "extension.h"
#include "utlvector.h"
#include <vector>
using std::vector;

class Rune;

class player_t 
{
	public:
	player_t() : m_PickupCooldown(0), m_UsageCooldown(0), m_runeId(-1), m_active(false), m_rune(NULL) { };
	
	bool hasRune() const;
	Rune *RuneHeld() const;

	int OnPickup(Rune *);
	int OnDrop();

	protected:
	int m_PickupCooldown;
	int m_UsageCooldown;
	int m_runeId;
	bool m_active;
	Rune *m_rune;

	friend class RuneSDK;
	friend class PlayerManager;
};

class PlayerManager : public IClientListener
{
	public:
	PlayerManager() : v_players(32) { }

  void OnClientDisconnecting(int client);
  void OnClientPutInServer(int client);

	Rune *RuneHeld(int client) const;
	bool hasRune(int client) const;

	void RemoveRuneFromAllPlayers(int id);

	protected:
	typedef vector<player_t>::iterator player_iterator;
	vector<player_t> v_players;

	friend class RuneSDK;
};

extern PlayerManager g_Players;


#endif
