#include <sourcemod>

void Unload_InCores(char[] plugin_unique)
{
	if(Check_IsCoreLoaded(Core_VIP) && VIP_IsValidFeature(plugin_unique))
	{
		VIP_UnregisterFeature(plugin_unique);
	}

	if(Check_IsCoreLoaded(Core_Shop))
	{
		Shop_UnregisterMe();		// FIXME: исправить костыль, сделать нормальную выгрузку с учётом наличия такого натива (Shop_UnregisterItem)
		Shop_Started();
	}
}

void Load_InCores(char[] plugin_unique)
{
	Load_Vip(plugin_unique);
	Load_Shop(plugin_unique);
}

void Set_ClientItem(int client, const char[] plugin_unique, char[] key, char[] newValue)
{
	g_kvActiveClientList[client].Rewind();
	g_kvActiveClientList[client].JumpToKey(plugin_unique, true);
	g_kvActiveClientList[client].SetString(key, newValue);

	Save_ClientCookieData(client, plugin_unique);
}

bool Get_ClientItem(int client, const char[] plugin_unique, char[] key, char[] buffer = "", int maxlen = 0)
{
	g_kvActiveClientList[client].Rewind();

	if(!g_kvActiveClientList[client].JumpToKey(plugin_unique))
		return false;
	
	if(maxlen > 0)
		g_kvActiveClientList[client].GetString(key, buffer, maxlen);

	return true;
}

void Save_ClientCookieData(int client, const char[] plugin_unique)
{
	g_kvActiveClientList[client].Rewind();

	if(!g_kvActiveClientList[client].JumpToKey(plugin_unique))
		return;

	char cookie_buff[256], buff[64];

	for(int id; id < sizeof(g_hCookieKeys); id++)
	{
		g_kvActiveClientList[client].GetString(g_hCookieKeys[id], buff, sizeof(buff));
		Format(cookie_buff, sizeof(cookie_buff), "%s|", buff);
	}
	
	StringMap map;
	Cookie cookie;

	Get_PluginMap(plugin_unique, _, map);
	map.GetValue("cookie", cookie);
	cookie.Set(client, cookie_buff);
}

bool Load_ClientCookieData(int client, char[] plugin_unique, StringMap map)
{
	Cookie cookie;

	if(!map.GetValue("cookie", cookie))
		return false;

	int lastUpdate = cookie.GetClientTime(client);
	char buff[128];

	g_kvActiveClientList[client].Rewind();
	g_kvActiveClientList[client].JumpToKey(plugin_unique, true);
	
	if(lastUpdate == 0)
	{
		g_kvItemsToAll.Rewind();
		ArrayList ar = view_as<ArrayList>(g_kvItemsToAll.GetNum(plugin_unique));

		if(!ar)
			return false;

		ar.GetString(0, buff, sizeof(buff));
		g_kvActiveClientList[client].SetString("all", buff);

		return true;
	}

	char exp[sizeof(g_hCookieKeys)][64];

	cookie.Get(client, buff, sizeof(buff));
	ExplodeString(buff, "|", exp, sizeof(exp), sizeof(exp[]));

	for(int id; id < sizeof(g_hCookieKeys); id++)
		g_kvActiveClientList[client].SetString(g_hCookieKeys[id], exp[id]);

	return true;
}

void UnLoad_ClientCookieData(int client, char[] plugin_unique)
{
	g_kvActiveClientList[client].Rewind();

	if(!g_kvActiveClientList[client].JumpToKey(plugin_unique))
		return;

	g_kvActiveClientList[client].DeleteThis();
	g_kvActiveClientList[client].Rewind();
}

int Convert_PluginId(const char[] plugin_unique = "", int plugin_id = -1, char[] buffer = "", int maxlen = 0)
{
	if(plugin_id >= 0)
	{
		return g_arPluginUniques.FindString(plugin_unique);
	}
	else if(plugin_unique[0])
	{
		g_arPluginUniques.GetString(plugin_id, buffer, maxlen);

		if(buffer[0])
			return 1;
	}
	
	return -1;
}

bool Get_PluginMap(const char[] plugin_unique = "", int plugin_id = -1, StringMap &map = null)
{
	if(plugin_id >= 0)
	{
		char buff[MAX_UNIQUE_LENGTH];
		Convert_PluginId(_, plugin_id, buff, sizeof(buff));

		return g_mapPluginUniques.GetValue(buff, map);
	}

	return g_mapPluginUniques.GetValue(plugin_unique, map);
}

bool Get_ItemMap(const char[] plugin_unique = "", int plugin_id = -1, const char[] item_unique, StringMap &map = null)
{
	StringMap plugin_map;
	Get_PluginMap(plugin_unique, plugin_id, plugin_map);
	
	if(plugin_map.GetValue(item_unique, map))
		return true;

	return false;
}

int Get_PluginPriorityArrayList(const char[] plugin_unique = "", int plugin_id = -1, ArrayList &ar)
{
	if(plugin_id >= 0)
	{
		char buff[MAX_UNIQUE_LENGTH];
		Convert_PluginId(_, plugin_id, buff, sizeof(buff));

		return g_mapPriorities.GetValue(buff, ar);
	}

	return g_mapPriorities.GetValue(plugin_unique, ar);
}
