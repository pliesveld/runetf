//#include "debug_files.inc"
#include <runetf/defines_debug>
#define MENU_MSG_LEN 64

stock LogNothing(any:...)
{
}

#if defined DEBUG_GEN_MENU
#define LOG_MENU_HANDLER LogHandler(m_action,menu)
#define LOG_MENU_START LogMenuStart(menu)
#define LOG_MENU_CANCELLED LogMenuCancel(param2)
#define LOG_MENU_CANCEL LogMenuCancel(param2)
#define LOG_MENU_END LogMenuEnd(param1)
#define LOG_MENU_SELECT LogMenuSelect(menu,param2,"")
#define LOG_MENU_SELECT_CUSTOM(%1) LogMenuSelect(menu,param2,%1)
#else
#define LOG_MENU_HANDLER LogNothing()
#define LOG_MENU_START LogNothing()
#define LOG_MENU_CANCELLED LogNothing()
#define LOG_MENU_CANCEL LogNothing()
#define LOG_MENU_END LogNothing()
#define LOG_MENU_SELECT LogNothing()
#define LOG_MENU_SELECT_CUSTOM(%1) LogNothing(%1)

#endif

#define RUNE_MENU_ACTIONS_DEFAULT MENU_ACTIONS_DEFAULT|MenuAction_Start


#if !defined __debug_files__
#define LOGCHANNEL LogMessage
#else
#define LOGCHANNEL LogBuffer
#endif
#define MAX_MENU_TITLE 32

static Handle:g_MenuHandleTrie = INVALID_HANDLE;


stock bool:LogMenuExternalHandle(Handle:menu,String:t_name[])
{
	decl String:s_menu[16];
	Format(s_menu,sizeof(s_menu),"%X",menu);
	return GetTrieString(g_MenuHandleTrie,s_menu,t_name, MAX_MENU_TITLE);
}


stock LogMenuStart(Handle:menu)
{
#if defined __DEBUG_HANDLER__
	LOGCHANNEL("Start %X: ",menu);
#endif
}

stock LogMenuSelect(Handle:menu,param2,const String:format[], any:...)
{
#if defined __DEBUG_HANDLER__
	decl String:item[64];
	GetMenuItem(menu, param2, item,sizeof(item));
	
	decl String:buffer[256];
	new len = strcopy(buffer,sizeof(buffer),item);

	VFormat(buffer[len],sizeof(buffer) - len,format,4);
	LOGCHANNEL("Select %s: %s", item, buffer);
#endif
}

stock LogMenuCancel(param2)
{
#if defined __DEBUG_HANDLER__
	LOGCHANNEL("Cancel %s",
		param2 == MenuCancel_Disconnected ? "disconnect" : param2 == MenuCancel_Interrupted ? "interrupted" : param2 == MenuCancel_Exit ? "exit" : param2 == MenuCancel_NoDisplay ? "display" : param2 == MenuCancel_ExitBack ? "back" : param2 == MenuCancel_Timeout ? "timeout" : "unknown");

#endif
}

stock LogMenuEnd(param1)
{
#if defined __DEBUG_HANDLER__
	LOGCHANNEL("End %s",
		param1 == MenuEnd_Selected ? "selected" : param1 == MenuEnd_Cancelled ? "cancel" : param1 == MenuEnd_Exit ? "exit" : param1 == MenuEnd_ExitBack ? "back" : "unknown");
#endif

}

stock LogHandler(MenuAction:action, Handle:menu = INVALID_HANDLE)
{
#if defined __DEBUG_HANDLER__

	LOGCHANNEL("Handler%X %s",menu,
		action == MenuAction_Start ? "Start" : action == MenuAction_Display ? "Display" :
 action == MenuAction_Select ? "Select" : action == MenuAction_Cancel ? "Cancel" : action == MenuAction_End ? "End" : action == MenuAction_DrawItem ? "DrawItem" : action == MenuAction_DisplayItem ? "DisplayItem" : "unknown")

#endif
}

#if defined __test__menu__

public DefaultBuildMenu(&Handle:menu)
{
	decl String:buffer[16];
	for(new i; i < 5;++i)
	{
		Format(buffer,sizeof(buffer), "default_%d", i);
		AddMenuItem(menu,buffer,buffer);
	}
}


public OnPluginEnd()
{
#if defined __debug_files__
	EndDebugLogger();
#endif
}


#endif
