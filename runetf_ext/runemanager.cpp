#include <algorithm>
using std::equal;

#include "runemanager.hpp"
#include "extension.h"

RunePluginManager g_Runes;


void RunePluginManager::Initialize()
{
}

void RunePluginManager::Shutdown()
{
}


int RunePluginManager::AddRune(const char* name, IPluginFunction *f_pickup, IPluginFunction *f_drop, int rune_id)
{
	int id;
	Rune **r;
	if( (r = m_tRune.retrieve(name)) != NULL && *r != NULL)
		return -1;

	Rune *t_rune = new Rune();
	t_rune->Id = rune_id;
	t_rune->f_pPickup = f_pickup;
	t_rune->f_pDrop = f_drop;
	t_rune->name = sm_strdup(name);

	m_tRune.insert(name, t_rune);
	const char* rune_name = sm_strdup(name);
	m_vRuneStr.push_back(rune_name);
	return rune_id;
}

Rune *RunePluginManager::FindRuneByName(const char *name) 
{ 
	Rune **pRune = m_tRune.retrieve(name);
	if(pRune)
		return *pRune;
	return NULL;
}


Rune *RunePluginManager::FindRuneById(int id)
{
	Rune *r;
	rune_iterator it = plugin_begin();
	for(; it != plugin_end(); ++it)
	{
		if( (*it)->p->m_nRuneCount <= 0)
			continue;

		if( (*it)->p->m_PlugId != ((id/1000000)*1000))
			continue;
		if((r = (*it)->p->FindRuneById(id)) != NULL)
			return r;
	}
	return NULL;
}

int RunePluginManager::RemoveRune(const char *name)
{
	Rune **rune;
	bool found = false;
	if( (rune = m_tRune.retrieve(name)) != NULL  && *rune != NULL)
	{

		g_Players.RemoveRuneFromAllPlayers((*rune)->Id);
		m_tRune.remove(name);
		if((*rune)->name)
			delete [] (*rune)->name;
		delete (*rune);
		*rune = NULL;
		found = true;
	}

	CVector<const char*>::iterator it = m_vRuneStr.begin();
	for(; it != m_vRuneStr.end();++it)
	{
		if(!strcmp((*it),name))
		{
			found = true;
			delete [] (*it);
			m_vRuneStr.erase(it);
			break;
		}
	}

	return found == true;
}

const char *RunePluginManager::RandomRune()
{
	int s;
	if( ( s = m_vRuneStr.size()) > 1 )
	{
		return m_vRuneStr.at( engrandom->RandomInt(0, s - 1 ) );
	} else if( s == 1 ) {
		return m_vRuneStr.at(0);
	}
	return NULL;
}

int RunePluginManager::GetAllRuneIds(int array[], int len)
{
	int total = 0;
	rune_iterator it = m_PluginInfo.begin();
	int i = 0;
	
	int found = 0;
	it = m_PluginInfo.begin();
	for(;it != m_PluginInfo.end();++it)
	{
			found += (*it)->p->GetAllRuneIds(array,len, i);
	}
		
	assert(i == found);
	return found;
}
/*
int RunePluginManager::GetAllRuneIds(int array[], int len)
{
	int total = 0;
	rune_iterator it = m_PluginInfo.begin();
	for(;it != m_PluginInfo.end();++it)
		if( (*it) && (*it)->p )
			total += (*it)->p->m_nRuneCount;

	if(!total)
		return 0;
	assert(total < len);

	*array = new int[total + 1];
	int i = 0;
	
	int found = 0;
	it = m_PluginInfo.begin();
	for(;it != m_PluginInfo.end();++it)
		if( (*it) && (*it)->p )
			found += (*it)->p->GetAllRuneIds(*array,total, i);
		
	assert(i == found);
	return found;
}

*/

void RunePluginManager::StartRunes()
{
	rune_iterator it = m_PluginInfo.begin();
	for(;it != m_PluginInfo.end();++it)
		if( (*it) && (*it)->p )
			(*it)->p->Startup();
}

void RunePluginManager::StopRunes()
{
	rune_iterator it = m_PluginInfo.begin();
	for(;it != m_PluginInfo.end();++it)
		if( (*it) && (*it)->p )
			(*it)->p->Shutdown();
}


int RunePluginManager::RegisterPlugin(IPluginContext *pl, IPluginFunction *cb_start, IPluginFunction *cb_stop, const char *filename)
{
	// search for existing plugin context

	rune_iterator it = m_PluginInfo.begin();
	for(; it != m_PluginInfo.end();++it)
	{
		if( (*it) && (*it)->p->GetPlugin() == pl)
			return pl->ThrowNativeError("Plugin %s already registered as a rune.", filename);
	}
	
	RunePlugin *runePlugin = new RunePlugin(pl,cb_start,cb_stop);
	RunePluginInfo* t = new RunePluginInfo(sm_strdup(filename), runePlugin);
	
	m_PluginInfo.push_back( t );

	//assert( runePlugin == GetPluginCtx(pl) );
	
	return 1;
}


void RunePluginManager::RemovePlugin(IPluginContext *pc)
{
	rune_iterator it = m_PluginInfo.begin();
	for(; it != m_PluginInfo.end();++it)
	{
		if(!(*it) && !(*it)->p)
			continue;
		if((*it)->p->GetPlugin() == pc)
		{

			RunePlugin *pl_ctx = GetPluginCtx(pc);
			List<RuneInfo*>::iterator it2;
			while(!pl_ctx->m_RuneInfo.empty() )
			{
				it2 = pl_ctx->m_RuneInfo.begin();
				fprintf(stderr,"Checking: %s\n", (*it2)->s_rune);
				RemoveRune((*it2)->s_rune);
				delete [] (*it2)->s_rune;
				delete *it2;
				pl_ctx->m_RuneInfo.erase(it2);
				it2 = pl_ctx->m_RuneInfo.begin();
			}


			//delete [] it->name;
			m_PluginInfo.erase(it);
			break;
		}
	}
}

RunePlugin *RunePluginManager::GetPluginCtx(IPluginContext *p)
{

	RunePluginInfo *r_plugin;
	RunePlugin *t;

	rune_iterator it;
	for(it = m_PluginInfo.begin(); it != m_PluginInfo.end(); ++it)
	{
		r_plugin = NULL;
		t = NULL;
		if((r_plugin = (*it)) == NULL)
			continue;

		if((t = r_plugin->p) == NULL)
			continue;

		if(t->GetPlugin() == NULL)
			continue;
		if( t->GetPlugin() == p)
			return t;	
	}
	return NULL;
}

/*
int RunePluginManager::PauseRunePlugin(IPluginContext *pc)
{
	using SourcePawn;
	SourcePawn::IPluginRuntime * pl;
	if(pc)
	{
		if( (pl = pc->GetRuntime()) != NULL)
		{
			if(pl->GetPluginStatus() == Plugin_Running)
			{
				pl->SetPauseState(true);
				return 1;
			}
		}
	}
	return 0;
}

int RunePluginManager::UnpauseRunePlugin(IPluginContext *pc)
{
	using SourcePawn;
	SourcePawn::IPluginRuntime * pl;
	if(pc)
	{
		if( (pl = pc->GetRuntime()) != NULL)
		{
			if(pl->IsPaused())
			{
				pl->SetPauseState(false);
				return 1;
			}
		}
	}
	return 0;
}

*/

