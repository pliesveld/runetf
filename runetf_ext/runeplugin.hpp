#ifndef __runeplugin_hpp_
#define __runeplugin_hpp_

#include <string>
using std::string;
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include "extension.h"
#include "player.hpp"
#include "rune.hpp"

#include <sourcehook/sh_list.h>
#include <sm_trie_tpl.h>
#include "util.h"

struct RuneInfo
{
	const char *s_rune;
	Rune 			 *p_rune;
};


class RunePlugin
{
	public:
	RunePlugin(IPluginContext *_p, IPluginFunction *pl_start, IPluginFunction *pl_stop);

	cell_t Startup();
	cell_t Shutdown();

	IPluginContext  *plugin;
protected:
	IPluginFunction *plugin_start;
	IPluginFunction *plugin_stop;

protected:
	SourceHook::List<RuneInfo*> m_RuneInfo; //Shallow copy of Rune*
public:
	bool AddRuneToPlugin(const char* name,Rune* rune);
	bool RemoveRuneFromPlugin(const char *s_rname);
	Rune *FindRuneByName(const char *s_rname);
	Rune *FindRuneById(int);
	int  GetAllRuneIds(int runeid_array[],int len, int &offset);
	

public:
	int m_PlugId;
	int m_nRuneCount;

protected:
public:



	IPluginContext* GetPlugin() const;

	friend class RunePluginManager;
};




#endif
