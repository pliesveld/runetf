//#define __DEBUG_HANDLER__

#define MAX_MENU_LEN 28
#define MAX_CMD_LEN 80


//menu_iterator_select.inc
#define ITER_TYPE_GLOBAL		"%%GLOBAL%%"
#define ITER_TYPE_DISABLED	"%%DISABLED%%"
#define ITER_TYPE_WORKING   "%%WORKING%%"

//menu_iterate.inc
#define GEN_ITER_NEXT "%%NEXT%%"
#define GEN_ITER_PREV "%%PREV%%"
#define GEN_ITER_SPAWN "%%SPAWN%%"
#define GEN_MODIFY 		"%%MODIFY%%"
#define GEN_MERGE 		"%%MERGE%%"
#define GEN_LOAD 			"%%LOAD%%"
#define GEN_DROP 			"%%DELETE%%"




//menu_modify_generator.inc
#define GEN_SET_ORIGIN  		"%%ORIGIN%%"
#define GEN_SET_ANGLE   		"%%ANGLE%%"
#define GEN_FORCE_MENU 			"%%FORCE%%" 
#define GEN_TOGGLE_DISABLE 	"%%DISABLE%%" 
#define GEN_CLUSTER_MENU		"%%CLUSTER%%"
//#define GEN_MENU_CREATE 		"%%CREATE%%"


enum IterState
{
  IterType:it_type,
  Handle:it_array,
  MoveType:it_player,
  it_idx
}                                                                                         
                                                                                          
new g_iState[IterState];

#include "menu_util_handler.inc"
#include "menu_build.inc"
#include "menu_main.inc"
#include "generator_stock.inc"
#include "menu_iterator_select.inc"
#include "menu_iterate.inc"
#include "menu_modify_generator.inc"
#include "menu_cluster.inc"
#include "menu_event.inc"

#include "menu_create.inc"

#include "rune_menu_test.inc"




RegisterMenus()
{
	RegAdminCmd("menu_rune", OnMenuCmd, ADMFLAG_CONFIG, "rune generator commands");
	RegAdminCmd("addtime", OnTestTime, ADMFLAG_RCON, "rune generator commands");
}


public Action:OnMenuCmd(client,args)
{

	DisplayMainMenu(client);
	return Plugin_Continue;
}



