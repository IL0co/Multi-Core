#include <sourcemod>

bool Check_IsValidPluginId(const char[] plugin_unique = "", int plugin_id = -1)
{
	if(Convert_PluginId(plugin_unique, plugin_id) == -1)
		return false;

	return true;
}

bool Check_IsValidItemUnique(char[] plugin_unique, char[] item)
{
	if(Get_ItemMap(plugin_unique, _, item))
		return true;

	return false;
}

bool Check_IsItemRegisteredStartedOnPlugin(Handle plugin, char[] buffer = "", int maxlen = 0)
{
	char buff[32];
	Format(buff, sizeof(buff), "%i", plugin);
	
	g_kvNowRegisteredItems.Rewind();
	bool result = g_kvNowRegisteredItems.JumpToKey(buff);
	
	if(maxlen > 0)
		g_kvNowRegisteredItems.GetString("unique", buffer, maxlen);
	
	return result;
}

void Check_AllLibraries()
{
	g_IsCoreLoadBits = Core_MultiCore;

	for(int id; id < sizeof(g_LoadCoreType); id++)		if(LibraryExists(g_LoadCoreType[id]))
		g_IsCoreLoadBits |= g_LoadCoreBits[id];
}

void Check_IsLoadLibraryName(const char[] name, bool isLoad)
{
	for(int id; id < sizeof(g_LoadCoreType); id++)		
	{
		if(strcmp(name, g_LoadCoreType[id], false) != 0)
			continue;

		if(isLoad)
			g_IsCoreLoadBits |= g_LoadCoreBits[id];
		else
			g_IsCoreLoadBits &= ~g_LoadCoreBits[id];

		CallForward_OnCoreChangeStatus(name, g_LoadCoreBits[id], isLoad);

		break;
	}
}

bool Check_IsValidClient(int client, bool check_kv = true)
{
	if(check_kv && !g_kvActiveClientList[client])
		return false;

	if(!IsClientAuthorized(client) || !IsClientInGame(client) || IsFakeClient(client))
		return false;

	return true;
}


bool Check_IsAllowRegisterInCore(char[] plugin_unique, MC_CoreTypeBits type)
{
	StringMap map;
	if(!Get_PluginMap(plugin_unique, _, map))
		return false;

	MC_CoreTypeBits core_bits;
	map.GetValue("dontLoadCoreBits", core_bits);

	if(core_bits == Core_Unloaded)
		return true;

	if(core_bits & type)
		return false;

	return true;
}

bool Check_IsCoreLoaded(MC_CoreTypeBits type)
{
	if(g_IsCoreLoadBits & type)
		return true;

	return false;
}

bool Check_IsValidFunctionCallBack(Function func, Handle plugin)
{
    if(!Check_IsValidFunction(func) || !Check_IsValidPlugin(plugin))
        return false;
    
    return true;
}

bool Check_IsValidFunction(Function func)
{
    if(func == INVALID_FUNCTION)
        return false;
    
    return true;
}

bool Check_IsValidPlugin(Handle plugin)
{
    if(!plugin)
        return false;
        
	Handle hIterator = GetPluginIterator();
	bool result = false;
	
	while(MorePlugins(hIterator))
	{
		if(plugin == ReadPlugin(hIterator))
		{
			result = (GetPluginStatus(plugin) == Plugin_Running);
			break;
		}
	}
	
	delete hIterator;
	return result;
}

bool Check_IsPluginUniqueHavePreview(StringMap plugin_map)
{
	int buff;
	if(plugin_map.GetValue("preview", buff))
		return true;

	return false;
}

bool Check_IsItemHavePreview(StringMap item_map)
{
	if(Check_IsValidFunction(Get_ItemCallBackFunc(item_map, ITEM_DATAPACKPOS_PREVIEW)))
		return true;

	return false;
}