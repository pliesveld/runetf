#if !defined __rune_vote__
#define __rune_vote__

public SharedPlugin:__p1_rune_vote =
{
	name = "rune_vote",
	file = "rune_vote.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};


#endif
