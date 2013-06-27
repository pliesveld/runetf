#include <sourcemod>
#undef REQUIRE_PLUGIN
#include <updater>

#define UPDATE_URL  "https://raw.github.com/pliesveld/runetf/master/autoupdate/updatefile.txt"

public OnPluginStart()
{
    if (LibraryExists("updater"))
    {
        Updater_AddPlugin(UPDATE_URL)
    }
}

public OnLibraryAdded(const String:name[])
{
    if (StrEqual(name, "updater"))
    {
        Updater_AddPlugin(UPDATE_URL)
    }
}

