#include <sourcemod>

public void OnClientPostAdminCheck(int client)
{
	if(g_kvActiveClientList[client])
		delete g_kvActiveClientList[client];

	g_kvActiveClientList[client] = new KeyValues("My Data");
	LoadPlayerCookies(client);
}

public Action Event_PlayerDisconnect(Event event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));

	if(g_kvActiveClientList[client])
		delete g_kvActiveClientList[client];
}

bool IsValidPlayer(int client, bool check_kv = false)
{
    if(check_kv && !g_kvActiveClientList[client])
        return false;

    if(!IsClientAuthorized(client) || !IsClientInGame(client) || IsFakeClient(client))
        return false;

    return true;
}

bool GetPlayerSelectedItem(int client, const char[] plugin_unique, char[] core_type, char[] buffer = "", int maxlen = 0)
{
    g_kvActiveClientList[client].Rewind();

    if(!g_kvActiveClientList[client].JumpToKey(plugin_unique))
        return false;

    g_kvActiveClientList[client].GetString(core_type, buffer, maxlen);

    if(buffer[0])
        return true;

    return false;
}

void SetPlayerSelectedItem(int client, char[] plugin_unique, char[] core_type, char[] item_unique)
{
    g_kvActiveClientList[client].Rewind();
    g_kvActiveClientList[client].JumpToKey(plugin_unique, true);
    g_kvActiveClientList[client].SetString(core_type, item_unique);

    SavePlayerCookie(client, plugin_unique);
}

bool GetPlayerActiveItem(int client, char[] plugin_unique, char[] buffer, int maxlen)
{
    ArrayList arPriorities;

    if(!g_mapPriorities.GetValue(plugin_unique, arPriorities))
        return false;

    MC_PluginMap map;
    GetPluginMap(plugin_unique, map);
    ArrayList arItems = map.GetItemsArray();

    if(!arItems.Length) 
        return false;
    
    char item_unique[MAX_UNIQUE_LENGTH], core_type[32];

    for(int num; num < arPriorities.Length; num++)
    {
        arPriorities.GetString(num, core_type, sizeof(core_type));

        if(!GetPlayerSelectedItem(client, plugin_unique, core_type, item_unique, sizeof(item_unique)))
            continue;

        if(arItems.FindString(item_unique) == -1)
            continue;

        FormatEx(buffer, maxlen, item_unique);
        return true;
    }

    return false;
}

void SavePlayerCookie(int client, char[] plugin_unique)
{
    g_kvActiveClientList[client].Rewind();
    g_kvActiveClientList[client].JumpToKey(plugin_unique);

    char buffer[256], item_unique[MAX_UNIQUE_LENGTH];

    for(int id; id < sizeof(g_hCookieKeys); id++)
    {
        g_kvActiveClientList[client].GetString(g_hCookieKeys[id], item_unique, sizeof(item_unique));
        Format(buffer, sizeof(buffer), "%s|", item_unique);
    }
    
    MC_PluginMap map;
    GetPluginMap(plugin_unique, map);

    map.Cookie.Set(client, buffer);
}


void LoadPlayerCookies(int client)
{
    char plugin_unique[MAX_UNIQUE_LENGTH];
    char buffer[256], exp[sizeof(g_hCookieKeys)][64];
    MC_PluginMap map;
    Cookie cookie;

    for(int id; id < g_arPlugins.Length; id++)
    {
        g_arPlugins.GetString(id, plugin_unique, sizeof(plugin_unique));
        
        if(!plugin_unique[0] || !g_mapPlugins.GetValue(plugin_unique, map))
            continue;

        g_kvActiveClientList[client].Rewind();
        g_kvActiveClientList[client].JumpToKey(plugin_unique, true);

        cookie = map.Cookie;

        if(cookie.GetClientTime(client) == 0)
        {
            g_kvItemsToAll.Rewind();
            ArrayList ar = view_as<ArrayList>(g_kvItemsToAll.GetNum(plugin_unique));

            if(ar)
            {
                ar.GetString(0, buffer, sizeof(buffer));
                g_kvActiveClientList[client].SetString(g_hCookieKeys[All], buffer);
            }
        }
        else
        {
            for(int num; num < sizeof(exp); num++)
                exp[num][0] = '\0';

            cookie.Get(client, buffer, sizeof(buffer));
            ExplodeString(buffer, "|", exp, sizeof(exp), sizeof(exp[]));

            for(int num; num < sizeof(g_hCookieKeys); num++)
                g_kvActiveClientList[client].SetString(g_hCookieKeys[num], exp[num]);
        }
    }
}