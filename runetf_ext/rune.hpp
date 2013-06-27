#ifndef __rune_hpp__
#define __rune_hpp__


#include "extension.h"
#include "player.hpp"

#define TFRUNE_FLAG_NODROP (1<<1)


class Rune
{
	public:
		Rune() : Id(0), rune_Id(0), ref(0), name(NULL),flags(0), f_pPickup(NULL), f_pDrop(NULL) { };
		int Id; // unique id
		int rune_Id;
		int ref;
		char *   name;
		int flags;

	protected:
		void Pickup(int client);
		void Drop(int client);

	public:
		IPluginFunction* f_pPickup;
		IPluginFunction* f_pDrop;

	friend class RuneSDK;
	friend class PlayerManager;
};

#endif


