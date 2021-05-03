#include <sourcemod>

bool IsValidPlayer(int client)
{
    if(!IsClientAuthorized(client) || !IsClientInGame(client) || IsFakeClient(client))
        return false;

    return true;
}

bool GetPlayerSelectedItem(int client, const char[] plugin_id, char[] core_type, char[] buffer = "", int maxlen = 0)
{
    if(CallBack_OnGetIntegrationClientItem(client, plugin_id, core_type, buffer, maxlen))
        return true;

    return false;
}

bool GetPlayerActiveItem(int client, char[] plugin_id, char[] buffer, int maxlen)
{
    ArrayList arPriorities;

    if(!g_mapPriorities.GetValue(plugin_id, arPriorities))
        return false;

    MC_PluginMap map;
    GetPluginMap(plugin_id, map);
    ArrayList arItems = map.GetItemsArray();

    if(!arItems.Length) 
        return false;
    
    char item_unique[MAX_UNIQUE_LENGTH], core_type[32];

    for(int num; num < arPriorities.Length; num++)
    {
        arPriorities.GetString(num, core_type, sizeof(core_type));

        if(!GetPlayerSelectedItem(client, plugin_id, core_type, item_unique, sizeof(item_unique)))
            continue;

        if(arItems.FindString(item_unique) == -1)
            continue;

        FormatEx(buffer, maxlen, item_unique);
        return true;
    }

    return false;
}