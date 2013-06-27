#if defined __runetf__
 #endif

#define __runetf__
#include <smlib>
#include "defines_debug"

/*
	Natives exposed by the runetf extension.

	Rune plugins are expected:
		1)  have public functions RunePluginStart() and RunePluginStop().
		2)  call Addrune(...) in OnPluginStart
		3)  maintain player state of rune pickups
		4)  rune ability event hooks only when ref == 1 and unhook when ref == 0
		5)  events that are not dependent on callbacks registered from AddRune are expected to verify cvar rune_enable == 1


	Extension will call RemoveRune callback registered from AddRune when:
		1) player drops a rune
		2) player has their rune removed from start of round or end of round
		3) player disconnects (including end of map)
		4) plugin is unloaded.
*/



/*
  Registers public functions with the event handler
  returns # of callbacks registered
  or a handle to a private forward for this plugins' event handler
*/
funcenum RuneTFPickup
{
	public(client),
/*  Pickup */
	public(client,runeid),
/*  Drop */
	public(client,runeid,ref)
}
//native RunePickupFwd(RuneTFPickup:cb,EventHook:hook);

/*
	Calls RunePlugin_(start|stop) in all rune plugins.
*/
native RunePluginsStart();
native RunePluginsStop();

/*
	Registers rune name[] with pickup/drop plugins.
  id should be unique to the plugin

	@return Handle: to a Rune. 
*/
native Handle:AddRune(String:rune[], RuneTFPickup:cb_on_pickup, RuneTFPickup:cb_on_drop,id );

/* returns a unique Id of the rune held by a player.
   0 if no player has no rune
*/
native GetPlayerRuneId(client);

/* returns the rune name the player is holding.
   0 on failure
*/
native GetPlayerRuneName(client,String:rune[],len);

/*
	Calls the rune pickup function of rune.
	returns 
		0 if player has a rune already
		-1 if rune name was not found
		1 on success
*/
native PlayerPickupRune(client,const String:rune[]);

/* Calls the rune drop function of the rune the player is holding.
	returns
		-1 if the player is not holding a rune
		0 if force is not set, and rune has the property TFRUNE_FLAG_NODROP
		1 on success

*/
native PlayerDropRune(client,bool:force = true);


/*
	Picks a random rune for the player to pickup.
  Same return values as PlayerPickupRune.
	
	used by spawn_gen 
  
*/
native PlayerPickupRandomRune(client);

/* retrieves the rune name associated with the runeid.
  returns 1 on success, 0 on failure
*/
native RuneNameById(runeid, String:rune[], size);

/*
	retrieves the rune id associated with the rune name
  returns 0 on failure, Id on success
*/
native RuneIdByName(const String:rune[]);

/*
	Fills in the supplied array with the all rune Ids that are currently registered.
	returns the number of elements copied

	Used by rune_info to map internal rune id to names.
*/
native GetRuneIds(array[], size);


/*
	@param Handle:hndl to a rune plugin
		force drops players runes and pauses the state of the plugin
		1 on success, 0 on failure.
*/
native RunesPause(Handle:hndl);
native RunesUnpause(Handle:hndl);

stock DbgPrint(const String:name[], ch, client = 0)
{
	decl String:dbg_string[256];
	decl String:buf_n[16];
	decl String:buf_cn[16];
	decl String:buf_tn[16];
	decl String:buf_or[32];
	decl String:buf_pn[16];
	decl Float:ori[3];
		
	if(ch > 0 && ch <= GetMaxClients())
	{
		GetClientName(ch, buf_n, sizeof(buf_n));
	} else {
		Entity_GetName(ch,buf_n,sizeof(buf_n));
	}
	Entity_GetClassName(ch,buf_cn,sizeof(buf_cn));
	Entity_GetTargetName(ch,buf_tn,sizeof(buf_tn));
	Entity_GetAbsOrigin(ch,ori);
	Entity_GetParentName(ch,buf_pn,sizeof(buf_pn));

	Format(buf_or,sizeof(buf_or),"(%-5.2f,%-5.2f,%-5.2f)",ori[0],ori[1],ori[2]);

	Format(dbg_string, sizeof(dbg_string),
		"%-10s %-10s:%-4d %-29s cname:%5s tname:%8s pname:%8s(%d) own:%d\n", name, buf_n, ch,
			buf_or, buf_cn, buf_tn, buf_pn,  Entity_GetParent(ch),Entity_GetOwner(ch));
	PrintToConsole(client,dbg_string);
}



public Extension:__ext_runespawn =
{
  name = "runetf",
  file = "runetf.ext",
#if defined AUTOLOAD_EXTENSIONS
  autoload = 1,
#else
	autoload = 0,
#endif

#if defined REQUIRE_EXTENSIONS
  required = 1
#else
	required = 0
#endif
};

/*
public __ext_runespawn_SetNTVOptional()
{
  MarkNativeAsOptional("RuneSpawn");
  MarkNativeAsOptional("PlayerHasRune");
}
*/


