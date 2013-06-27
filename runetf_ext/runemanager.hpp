#ifndef __rune_manager__hpp
#define __rune_manager__hpp

#include "extension.h"
#include "utlvector.h"
#include <vector>
using std::vector;

enum RuneBlock
{
	RunePluginFuncStart = 0,
	RunePluginFuncStop = 1,
	RunePluginPickupFwd = 2,
	RunePluginTickFwd = 3
};
#define RUNEBLOCK_SIZE 4

enum RuneFlags
{
	Rune_Any = 0,
	Rune_Pickup = (1<<1),
	Rune_Death  = (2<<1),
	Rune_Use		= (3<<1),
	Rune_Drop		= (4<<1),
	Rune_HurtShield = (5<<1),
	Rune_HurtDmg    = (6<<1),
	Rune_Kill       = (7<<1),
	Rune_KillAssist = (8<<1)
};


//#include "sm_globals.h"
//#include "sourcemm_api.h"
//#include "sm_trie.h"
#include <sh_list.h>
#include <sh_stack.h>
#include <IHandleSys.h>
#include <IForwardSys.h>
#include <IPluginSys.h>
#include <sourcehook/sh_list.h>
#include <sourcehook/sh_vector.h>

#include "runeplugin.hpp"
using std::vector;

using namespace SourceHook;
#define RUNE_CB_PTR(rune_cb) (void (*rune_cb)(Rune *))

struct RunePluginInfo
{
	RunePluginInfo(const char* file, RunePlugin* plug) : name(file), p(plug) { }
	~RunePluginInfo() { if(name) delete [] name; if(p) delete p; } 
	const char* name;
	RunePlugin *p;
};


class RunePluginManager
{
	public:
		int RegisterPlugin(IPluginContext*, IPluginFunction *cb_start, IPluginFunction *cb_end, const char *fn = NULL);
		void RemovePlugin(IPluginContext *pc);

		void Initialize();
		void Shutdown();

		void StartRunes();
		void StopRunes();


	public:

	protected:
		SourceHook::List<RunePluginInfo*> m_PluginInfo;
		KTrie<Rune*>    m_tRune;
		SourceHook::CVector<const char *> m_vRuneStr;

		int  RemoveRune(const char *name);

	public:
		int  AddRune(const char *name, IPluginFunction *f_pickup, IPluginFunction *f_drop, int rune_id = 1);
		Rune *FindRuneByName(const char *name);
		Rune *FindRuneById(int id);
		const char *RandomRune();
		int GetAllRuneIds(int array[], int len);


	public:
		typedef List<RunePluginInfo*>::iterator rune_iterator;

		rune_iterator plugin_begin() { return m_PluginInfo.begin(); }
		rune_iterator plugin_end()   { return m_PluginInfo.end();   }

		RunePlugin *GetPluginCtx(IPluginContext *p);


	//	int PauseRunePlugin(IPluginContext *);
	//	int UnpauseRunePlugin(IPluginContext *);


/// deprecated
		//typedef List<RunePluginInfo*>::const_iterator rune_const_iterator;
	//	vector<RunePlugin> v_runes;
		//Rune *FindFirstRune(const char* str_rune );
		//Rune *FindRune(rune_iterator& it, const char* str_rune );
	//	RunePluginManager::rune_iterator pluginCtx(IPluginContext *p);
	//	void RunePluginPickup(rune_iterator &it,funcid_t fid, funcid_t fhandler);
};

extern RunePluginManager g_Runes;


#endif
