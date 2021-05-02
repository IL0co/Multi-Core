#include <sourcemod>

public void LoadNatives()
{
	g_arPlugins = new ArrayList(MAX_UNIQUE_LENGTH);
	g_mapNowRegisteredPlugins = new StringMap();
	g_mapPriorities = new StringMap();
	g_mapPlugins = view_as<MC_PluginMap>(new StringMap());

	CreateNative("MC_PrecacheFile", Native_MC_PrecacheFile);
	CreateNative("MC_IsCoreLoaded", Native_MC_IsCoreLoaded);

	CreateNative("MC_GetPluginIdsArrayList", Native_MC_GetPluginIdsArrayList);
	CreateNative("MC_GetPluginItemsArrayList", Native_MC_GetPluginItemsArrayList);

	CreateNative("MC_GetPluginDisplayName", Native_MC_GetPluginDisplayName);
	CreateNative("MC_GetItemDisplayName", Native_MC_GetItemDisplayName);

	CreateNative("MC_CallItemPreview", Native_MC_CallItemPreview);
	CreateNative("MC_CallItemSelected", Native_MC_CallItemSelected);
	CreateNative("MC_CallPluginSelected", Native_MC_CallPluginSelected);

	CreateNative("MC_IsPluginHavePreviewItems", Native_MC_IsPluginHavePreviewItems);
	CreateNative("MC_IsItemHavePreview", Native_MC_IsItemHavePreview);

	CreateNative("MC_IsValidPluginUnique", Native_MC_IsValidPluginUnique);
	CreateNative("MC_GetPluginsCountRegisteredByMe", Native_MC_GetPluginsCountRegisteredByMe);

	CreateNative("MC_RegisterPlugin", Native_MC_RegisterPlugin);
	CreateNative("MC_SetPluginCallBacks", Native_MC_SetPluginCallBacks);
	CreateNative("MC_StartItem", Native_MC_StartItem);
	CreateNative("MC_SetItemCallBacks", Native_MC_SetItemCallBacks);
	CreateNative("MC_EndItem", Native_MC_EndItem);
	CreateNative("MC_EndPlugin", Native_MC_EndPlugin);
	CreateNative("MC_UnRegisterMe", Native_MC_UnRegisterMe);

	CreateNative("MC_GetClientActiveItem", Native_MC_GetClientActiveItem);
	CreateNative("MC_GetClientSelectedItem", Native_MC_GetClientSelectedItem);
	CreateNative("MC_SetClientSelectedItem", Native_MC_SetClientSelectedItem);
}

public int Native_MC_RegisterPlugin(Handle plugin, int numParams)
{
	char plugin_id[MAX_UNIQUE_LENGTH];

	GetNativeString(1, plugin_id, sizeof(plugin_id));
	
	if(IsValidPluginUnique(plugin_id))
		Error(PLUGIN_IS_ALREADY_REGISTERED, _, plugin_id);

	MC_PluginMap plugin_map;
	if((plugin_map = Get_NowRegisteredMap(plugin)) != null)
		Error(COMPLETE_REGISTRATION_PREVIOUSLY_PLUGIN, plugin_map);

	plugin_map = new MC_PluginMap();
	plugin_map.SetUnique(plugin_id);
	plugin_map.Plugin = plugin;
	plugin_map.Cookie = new Cookie(plugin_id, plugin_id, CookieAccess_Public);
	
	char buff[32];
	FormatEx(buff, sizeof(buff), "%i", plugin);

	g_mapNowRegisteredPlugins.SetValue(buff, plugin_map);

	plugin_map.DeathTimer = CreateTimer(5.0, Timer_DeathPluginRegister, plugin_map);
}

public Action Timer_DeathPluginRegister(Handle timer, MC_PluginMap plugin_map)
{
	RemoveFromNowRegisteredMap(plugin_map.Plugin);
	plugin_map.UnregisterMe();
}

public int Native_MC_SetPluginCallBacks(Handle plugin, int numParams)
{
	MC_PluginMap plugin_map;

	if((plugin_map = Get_NowRegisteredMap(plugin)) == null)
		Error(START_THE_REGISTERING_PLUGIN);
	
	DataPack data = plugin_map.GetCallBacksPack();
	data.WriteFunction(GetNativeFunction(1));
	data.WriteFunction(GetNativeFunction(2));
}

public int Native_MC_StartItem(Handle plugin, int numParams)
{
	MC_PluginMap plugin_map;

	if((plugin_map = Get_NowRegisteredMap(plugin)) == null)
		Error(START_THE_REGISTERING_PLUGIN);
	
	MC_ItemMap item_map;
	char item_unique[MAX_UNIQUE_LENGTH];

	GetNativeString(1, item_unique, sizeof(item_unique));

	if(plugin_map.IsValidItem(item_unique))
		Error(ITEM_IS_ALREADY_REGISTERED, _, item_unique);

	if(plugin_map.OpenForRegistration)
		Error(COMPLETE_REGISTRATION_PREVIOUSLY_ITEM, plugin_map);
	
	item_map = new MC_ItemMap();
	item_map.SetUnique(item_unique);
	plugin_map.SaveItem(item_map, item_unique);
	plugin_map.OpenForRegistration = true;

	return true;
}

public int Native_MC_SetItemCallBacks(Handle plugin, int numParams)
{
	MC_PluginMap plugin_map;

	if((plugin_map = Get_NowRegisteredMap(plugin)) == null)
		Error(START_THE_REGISTERING_PLUGIN);
	
	MC_ItemMap item_map;
	plugin_map.GetLastItemMap(item_map);

	DataPack data = item_map.GetCallBacksPack();
	data.WriteFunction(GetNativeFunction(1));

	Function func = GetNativeFunction(2);
	data.WriteFunction(func);

	data.WriteFunction(GetNativeFunction(3));

	if(!plugin_map.IsHavePreviewItem && func != INVALID_FUNCTION)
		plugin_map.IsHavePreviewItem = true;
}

public int Native_MC_EndItem(Handle plugin, int numParams)
{
	MC_PluginMap plugin_map;

	if((plugin_map = Get_NowRegisteredMap(plugin)) == null)
		Error(START_THE_REGISTERING_PLUGIN);
	
	if(!plugin_map.OpenForRegistration)
		Error(START_THE_REGISTERING_ITEM);
	
	plugin_map.OpenForRegistration = false;
}

public int Native_MC_EndPlugin(Handle plugin, int numParams)
{
	MC_PluginMap plugin_map;

	if((plugin_map = Get_NowRegisteredMap(plugin)) == null)
		Error(START_THE_REGISTERING_PLUGIN);
		
	if(plugin_map.OpenForRegistration)
		Error(COMPLETE_REGISTRATION_PREVIOUSLY_ITEM, plugin_map);

	char plugin_id[MAX_UNIQUE_LENGTH];
	plugin_map.GetUnique(plugin_id, sizeof(plugin_id));

	RemoveFromNowRegisteredMap(plugin);
	delete plugin_map.DeathTimer;
	plugin_map.DeathTimer = null;

	plugin_map.SaveGlobal();
	CallForward_OnPluginRegistered(plugin_id);
}

public int Native_MC_UnRegisterMe(Handle plugin, int numParams)
{
	char plugin_id[MAX_UNIQUE_LENGTH];
	MC_PluginMap map;

	for(int id; id < g_arPlugins.Length; id++)
	{
		g_arPlugins.GetString(id, plugin_id, sizeof(plugin_id));

		if(!plugin_id[0])
			continue;

		g_mapPlugins.GetValue(plugin_id, map);

		if(map.Plugin != plugin)
			continue;
		
		CallForward_OnPluginUnRegistered_Pre(plugin_id);

		map.UnregisterMe();
		g_mapPlugins.Remove(plugin_id);
		g_arPlugins.SetString(id, "");

		CallForward_OnPluginUnRegistered(plugin_id);
	}
}

public any Native_MC_GetPluginIdsArrayList(Handle plugin, int numParams)
{
	ArrayList ar = g_arPlugins.Clone();
	int index;

	while((index = ar.FindString("")) >= 0)
		ar.Erase(index);
	
	return ar;
}

public any Native_MC_GetPluginItemsArrayList(Handle plugin, int numParams)
{
	MC_PluginMap map = GetPluginMapNative(1);
	return map.GetItemsArray().Clone();
}

public any Native_MC_GetPluginDisplayName(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(client >= MaxClients || client < 0)
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	GetPluginMapNative(2, plugin_id, sizeof(plugin_id));
	
	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(3, core_type, sizeof(core_type));

	char name[128];
	if(!CallBack_OnPluginUniqueDisplay(client, plugin_id, core_type, name, sizeof(name)))
		return false;

	SetNativeString(4, name, GetNativeCell(5));
	return true;
}

public any Native_MC_GetItemDisplayName(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(client >= MaxClients || client < 0)
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	MC_PluginMap map = GetPluginMapNative(2, plugin_id, sizeof(plugin_id));

	char item_id[MAX_UNIQUE_LENGTH];
	GetItemMapNative(3, map, item_id, sizeof(item_id));

	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(4, core_type, sizeof(core_type));

	char name[128];
	if(!CallBack_OnItemDisplay(client, plugin_id, item_id, core_type, name, sizeof(name)))
		return false;

	SetNativeString(5, name, GetNativeCell(6));
	return true;
}

public any Native_MC_CallItemPreview(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(client >= MaxClients || client < 0)
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	MC_PluginMap map = GetPluginMapNative(2, plugin_id, sizeof(plugin_id));

	char item_id[MAX_UNIQUE_LENGTH];
	GetItemMapNative(3, map, item_id, sizeof(item_id));

	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(4, core_type, sizeof(core_type));

	CallBack_OnItemPreview(client, plugin_id, item_id, core_type);
}

public any Native_MC_CallItemSelected(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(client >= MaxClients || client < 0)
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	MC_PluginMap map = GetPluginMapNative(2, plugin_id, sizeof(plugin_id));

	char item_id[MAX_UNIQUE_LENGTH];
	MC_ItemMap item_map = GetItemMapNative(3, map, item_id, sizeof(item_id));

	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(4, core_type, sizeof(core_type));

	return CallBack_OnItemSelected(client, map, item_map, plugin_id, item_id, core_type);
}

public any Native_MC_CallPluginSelected(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(client >= MaxClients || client < 0)
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	MC_PluginMap map = GetPluginMapNative(2, plugin_id, sizeof(plugin_id));
	
	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(3, core_type, sizeof(core_type));

	return CallBack_OnPluginSelected(client, map, plugin_id, core_type);
}

public any Native_MC_IsPluginHavePreviewItems(Handle plugin, int numParams)
{
	MC_PluginMap map = GetPluginMapNative(1);
	return map.IsHavePreviewItem;
}

public any Native_MC_IsItemHavePreview(Handle plugin, int numParams)
{
	MC_PluginMap map = GetPluginMapNative(1);
	MC_ItemMap item_map = GetItemMapNative(2, map);

	return (item_map.GetCallBack(ITEM_DATAPACKPOS_PREVIEW) != INVALID_FUNCTION);
}

public int Native_MC_IsValidPluginUnique(Handle plugin, int numParams)
{
	char plugin_id[MAX_UNIQUE_LENGTH];
	GetNativeString(1, plugin_id, sizeof(plugin_id));

	if(!plugin_id[0] || !IsValidPluginUnique(plugin_id))
		return false;
	
	return true;
}

public int Native_MC_GetPluginsCountRegisteredByMe(Handle plugin, int numParams)
{
	int count;
	char plugin_id[MAX_UNIQUE_LENGTH];
	MC_PluginMap map;

	for(int index; index < g_arPlugins.Length; index++)
	{
		g_arPlugins.GetString(index, plugin_id, sizeof(plugin_id));

		if(!plugin_id[0])
			continue;

		GetPluginMap(plugin_id, map);
		
		if(map.Plugin == plugin)
			count++;
	}

	return count;
}

public int Native_MC_PrecacheFile(Handle plugin, int numParams)
{
	static int table = INVALID_STRING_TABLE;
	char file[256];

	GetNativeString(1, file, sizeof(file));

	if(!file[0])
		Error(FILE_NOT_EXIST, _, file);

	TrimString(file);

	MC_DataType type = view_as<MC_DataType>(GetNativeCell(2));
	int index;

	if(type == Type_Sound)
	{
		char buffer[256];
		Format(buffer, sizeof(buffer), "sound/%s", file);

		if(FileExists(buffer, false) || FileExists(buffer, true))
			PrecacheSound(file, true);
		else
			Error(FILE_NOT_EXIST, _, buffer);
	}
	else if(type == Type_Particle)
	{
		bool save = LockStringTables(false);
		AddToStringTable(FindStringTable("EffectDispatch"), "ParticleEffect");
		LockStringTables(save);
		
		if(table == INVALID_STRING_TABLE)
			table = FindStringTable("ParticleEffectNames");

		save = LockStringTables(false);
		AddToStringTable(table, file);
		LockStringTables(save);
	}
	else
	{
		if(!FileExists(file, true) || !FileExists(file, false))
			Error(FILE_NOT_EXIST, _, file);

		if(type == Type_Model)
			index = PrecacheModel(file, true);
		else if(type == Type_Sprite)
			index = PrecacheDecal(file, true);
		else if(type == Type_ParticleFile)
			index = PrecacheGeneric(file, true);
	}

	return index;
}

public int Native_MC_IsCoreLoaded(Handle plugin, int numParams)
{
	return g_bIsCoreLoaded;
}

public int Native_MC_GetClientActiveItem(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(!IsValidPlayer(client))
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	GetPluginMapNative(2, plugin_id, sizeof(plugin_id));
	
	char item_unique[MAX_UNIQUE_LENGTH];
	if(!GetPlayerActiveItem(client, plugin_id, item_unique, sizeof(item_unique)))
		return false;

	SetNativeString(3, item_unique, GetNativeCell(4));
	return true;
}

public int Native_MC_GetClientSelectedItem(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(!IsValidPlayer(client))
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	GetPluginMapNative(2, plugin_id, sizeof(plugin_id));
	
	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(3, core_type, sizeof(core_type));

	char item_unique[MAX_UNIQUE_LENGTH];
	
	if(!GetPlayerSelectedItem(client, plugin_id, core_type, item_unique, sizeof(item_unique)))
		return false;

	SetNativeString(4, item_unique, GetNativeCell(5));
	return true;
}

public int Native_MC_SetClientSelectedItem(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(!IsValidPlayer(client))
		Error(INVALID_CLIENT, client);

	char plugin_id[MAX_UNIQUE_LENGTH];
	GetPluginMapNative(2, plugin_id, sizeof(plugin_id));
	
	char core_type[MAX_UNIQUE_LENGTH];
	GetNativeString(3, core_type, sizeof(core_type));

	char item_unique[MAX_UNIQUE_LENGTH];
	GetNativeString(4, item_unique, sizeof(item_unique));
	
	SetPlayerSelectedItem(client, plugin_id, core_type, item_unique);
}

bool IsValidPluginUnique(char[] plugin_id)
{
	if(g_arPlugins.FindString(plugin_id) != -1)
		return true;

	return false;
}

MC_PluginMap Get_NowRegisteredMap(Handle plugin)
{
	char buff[32];
	FormatEx(buff, sizeof(buff), "%i", plugin);

	MC_PluginMap map;
	g_mapNowRegisteredPlugins.GetValue(buff, map);

	return map;
}

void RemoveFromNowRegisteredMap(Handle plugin)
{
	char buff[32];
	FormatEx(buff, sizeof(buff), "%i", plugin);

	if(buff[0])
		g_mapNowRegisteredPlugins.Remove(buff);
}

MC_PluginMap GetPluginMapNative(int native_arg, char[] plugin_id = "", int maxlen = MAX_UNIQUE_LENGTH)
{
	GetNativeString(native_arg, plugin_id, maxlen);
	
	MC_PluginMap map;
	if(!GetPluginMap(plugin_id, map))
		Error(ERROR_INVALID_PLUGIN_UNIQUE, _, plugin_id);
	
	return map;
}

MC_ItemMap GetItemMapNative(int native_arg, MC_PluginMap plugin_map, char[] item_id = "", int maxlen = MAX_UNIQUE_LENGTH)
{
	GetNativeString(native_arg, item_id, maxlen);
	
	if(!plugin_map.IsValidItem(item_id))
		Error(ERROR_INVALID_ITEM_UNIQUE, _, item_id);
	
	return plugin_map.GetItemMap(item_id);
}
