#include <sourcemod>

public void LoadNatives()
{
	g_arPlugins = new ArrayList(MAX_UNIQUE_LENGTH);
	g_mapNowRegisteredPlugins = new StringMap();
	g_mapPriorities = new StringMap();
	g_mapPlugins = view_as<MC_PluginMap>(new StringMap());

	CreateNative("MC_PrecacheFile", Native_MC_PrecacheFile);
	CreateNative("MC_IsCoreLoaded", Native_MC_IsCoreLoaded);
	CreateNative("MC_GetSettingsConfigKV", Native_MC_GetSettingsConfigKV);

	CreateNative("MC_IsValidPluginIndex", Native_MC_IsValidPluginId);
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
	plugin_map.DontLoadInCores = GetNativeCell(2);
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

	DataPack data = new DataPack();
	data.WriteString(plugin_id);
	CreateTimer(1.0, Timer_Delay_LoadInCores, data, TIMER_HNDL_CLOSE);

	RemoveFromNowRegisteredMap(plugin);
	delete plugin_map.DeathTimer;
	plugin_map.DeathTimer = null;

	return plugin_map.SaveGlobal();
}

public Action Timer_Delay_LoadInCores(Handle timer, DataPack data)
{
	data.Reset();

	char plugin_id[MAX_UNIQUE_LENGTH];
	data.ReadString(plugin_id, sizeof(plugin_id));

	Load_Vip(plugin_id);
	Load_Shop(plugin_id);

	return Plugin_Stop;
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
		
		map.UnregisterMe();
		g_mapPlugins.Remove(plugin_id);
		g_arPlugins.SetString(id, "");

		
		if(Check_IsCoreLoaded(Core_VIP) && VIP_IsValidFeature(plugin_id))
		{
			VIP_UnregisterFeature(plugin_id);
		}
	}

	if(Check_IsCoreLoaded(Core_Shop))
	{
		Shop_UnregisterMe();		// FIXME: исправить костыль, сделать нормальную выгрузку с учётом наличия такого натива (Shop_UnregisterItem)
		Shop_Started();
	}
}

public int Native_MC_IsValidPluginId(Handle plugin, int numParams)
{
	if(IsValidPluginId(GetNativeCell(1)))
		return true;
	
	return false;
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
		PrecacheSound(file, true);
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
	else if((FileExists(file, true) || FileExists(file, false)))
	{
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
	return Check_IsCoreLoaded(GetNativeCell(1));
}

public int Native_MC_GetClientActiveItem(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(!IsValidPlayer(client))
		Error(INVALID_CLIENT, client);

	int plugin_index = GetNativeCell(2);

	if(!IsValidPluginId(plugin_index))					
		Error(ERROR_INVALID_PLUGIN_ID, plugin_index);
	
	char plugin_id[MAX_UNIQUE_LENGTH], item_unique[MAX_UNIQUE_LENGTH];
	GetPluginUniqueById(plugin_index, plugin_id, sizeof(plugin_id));
	
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

	int plugin_index = GetNativeCell(2);

	if(!IsValidPluginId(plugin_index))				
		Error(ERROR_INVALID_PLUGIN_ID, plugin_index);
	
	int core_type = GetNativeCell(3);
	char plugin_id[MAX_UNIQUE_LENGTH], item_unique[MAX_UNIQUE_LENGTH];
	GetPluginUniqueById(plugin_index, plugin_id, sizeof(plugin_id));
	
	if(!GetPlayerSelectedItem(client, plugin_id, g_hCookieKeys[core_type], item_unique, sizeof(item_unique)))
		return false;

	SetNativeString(4, item_unique, GetNativeCell(5));
	return true;
}

public int Native_MC_SetClientSelectedItem(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(!IsValidPlayer(client))
		Error(INVALID_CLIENT, client);

	int plugin_index = GetNativeCell(2);

	if(!IsValidPluginId(plugin_index))					
		Error(ERROR_INVALID_PLUGIN_ID, plugin_index);
	
	int core_type = GetNativeCell(3);
	char plugin_id[MAX_UNIQUE_LENGTH], item_unique[MAX_UNIQUE_LENGTH];
	GetPluginUniqueById(plugin_index, plugin_id, sizeof(plugin_id));
	
	SetPlayerSelectedItem(client, plugin_id, g_hCookieKeys[core_type], item_unique);
}

public any Native_MC_GetSettingsConfigKV(Handle plugin, int numParams)
{
	char path[256];
	KeyValues kv;

	int core_type = GetNativeCell(1);

	kv = new KeyValues(g_hFileHeads[core_type]);

	if(!kv.ImportFromFile(g_hFilePaths[core_type]))
	{
		delete kv;
		Error(FILE_NOT_EXIST, _, path);
	}

	return kv;
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

bool IsValidPluginId(int plugin_index)
{
	if(plugin_index == -1 || plugin_index >= g_arPlugins.Length)
		return false;

	char buff[1];
	g_arPlugins.GetString(plugin_index, buff, sizeof(buff));

	if(!buff[0])
		return false;

	return true;
}

bool GetPluginUniqueById(int plugin_index, char[] buffer, int maxlen)
{
	if(g_arPlugins.GetString(plugin_index, buffer, maxlen) == -1)
		return false;

	return true;
}