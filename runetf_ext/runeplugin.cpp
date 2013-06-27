#include "runeplugin.hpp"
using namespace SourceHook;

static int COUNT = 0;

RunePlugin::RunePlugin(IPluginContext *_p, IPluginFunction *pl_start, IPluginFunction *pl_stop) 
	: plugin(_p), plugin_start(pl_start), plugin_stop(pl_stop),
		m_PlugId((++COUNT)*1000), m_nRuneCount(0)
{ 
	assert(plugin_start != NULL);
	assert(plugin_stop  != NULL);

}

IPluginContext* RunePlugin::GetPlugin() const
{ 
	return plugin;
}

bool RunePlugin::AddRuneToPlugin(const char* name,Rune *rune)
{
	if( !rune || FindRuneByName( name ) )
		return false;
	
	RuneInfo *t;
	t = new RuneInfo();
	t->s_rune = sm_strdup( name );
	t->p_rune = rune;
	m_RuneInfo.push_back(t);
	++m_nRuneCount;
	return true;
	
}

bool RunePlugin::RemoveRuneFromPlugin(const char *s_rname)
{
	Rune *t_rune;

	List<RuneInfo*>::iterator it = m_RuneInfo.begin();
	for(;it != m_RuneInfo.end();++it)
	{
		if(!strcmp(s_rname, (*it)->s_rune))
		{
			delete [] (*it)->s_rune;
			(*it)->s_rune = NULL;
			(*it)->p_rune = NULL;
			delete (*it);
			m_RuneInfo.erase(it);
			--m_nRuneCount;
			return true;
		}
	}
	return false;
}

	
Rune *RunePlugin::FindRuneByName(const char *s_rname)
{
	List<RuneInfo*>::iterator it = m_RuneInfo.begin();
	for(;it != m_RuneInfo.end();++it)
	{
		if(!strcmp(s_rname, (*it)->s_rune))
			return (*it)->p_rune;
		
	}
	return NULL;
}
	
Rune *RunePlugin::FindRuneById(int runeid)
{
	List<RuneInfo*>::iterator it = m_RuneInfo.begin();
	for(;it != m_RuneInfo.end();++it)
	{
		Rune *r;
		if( (r = (*it)->p_rune) == NULL )
			continue;
		if( r->Id == runeid )
			return r;
	}
	return NULL;
}
	
int  RunePlugin::GetAllRuneIds(int runeid_array[],int len, int &offset)
{

	List<RuneInfo*>::iterator it = m_RuneInfo.begin();
	int i;
	for(i = 0;it != m_RuneInfo.end();++it)
	{
		if(offset >= len)
			break;
		i++;
		assert((*it)->p_rune != NULL);
		runeid_array[offset++] = (*it)->p_rune->Id;
//		fprintf(stderr,"DBGDBG:Plugin %X Rune %X %d\n", plugin,it->p_rune,it->p_rune->Id);
	}
	return i;
}

cell_t RunePlugin::Startup()
{
	cell_t ret;
	assert(plugin_start != NULL);
	plugin_start->Execute(&ret);
	return ret;
}


cell_t RunePlugin::Shutdown()
{
	cell_t ret;
	assert(plugin_stop != NULL);
	plugin_stop->Execute(&ret);
	return ret;
}




