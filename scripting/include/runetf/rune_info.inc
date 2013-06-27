#if defined __rune_info__
 #endinput
#endif

#define __rune_info__

public SharedPlugin:__pl_rune_info = 
{
	name = "rune_info",
	file = "rune_info.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};
 
native RuneTF_display_rune_info(client, String:rune_name[]);

public __pl_rune_info_SetNTVOptional()
{
	MarkNativeAsOptional("RuneTF_display_rune_info");
}
