#if !defined __spawn_gen_inc__
#define __spawn_gen_inc__

public SharedPlugin:__p1_spawn_gen =
{
	name =  "spawn_gen",
	file = "spawn_gen.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif 
};

#endif

