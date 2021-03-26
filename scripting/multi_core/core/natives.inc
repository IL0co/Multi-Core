#include <sourcemod>

public void LoadNatives()
{
	g_arPluginUniques = new ArrayList(64);
	g_kvNowRegisteredItems = new KeyValues("Now Registered Items");
	g_mapPluginUniques = new StringMap();

	CreateNative("MC_PrecacheFile", Native_MC_PrecacheFile);
	CreateNative("MC_IsCoreLoaded", Native_MC_IsCoreLoaded);
	CreateNative("MC_GetSettingsConfigKV", Native_MC_GetSettingsConfigKV);

	CreateNative("MC_RegisterPlugin", Native_MC_RegisterPlugin);
	CreateNative("MC_StartItem", Native_MC_StartItem);
	CreateNative("MC_SetItemCallBacks", Native_MC_SetItemCallBacks);
	CreateNative("MC_EndItem", Native_MC_EndItem);
	CreateNative("MC_EndPlugin", Native_MC_EndPlugin);
	CreateNative("MC_UnRegisterMe", Native_MC_UnRegisterMe);

	CreateNative("MC_GetClientActiveItem", Native_MC_GetClientActiveItem);
	CreateNative("MC_SetClientActiveItem", Native_MC_SetClientActiveItem);
}

public int Native_MC_RegisterPlugin(Handle plugin, int numParams)
{
	char plugin_unique[MAX_UNIQUE_LENGTH], buff[96];
	StringMap map;
	GetNativeString(1, plugin_unique, sizeof(plugin_unique));

	if(Get_PluginMap(plugin_unique, _, map))
	{
		Handle plugin_owner;
		map.GetValue("plugin", plugin_owner);

		if(plugin == plugin_owner)
			return Convert_PluginId(plugin_unique);
		else
			Error_Native_PluginUniqueIsAlreadyRegistered(plugin_unique);
	}

	Format(buff, sizeof(buff), "MultiCore_%s", plugin_unique);
	Cookie cookie = new Cookie(buff, buff, CookieAccess_Private);
	map = new StringMap();

	g_mapPluginUniques.SetValue(plugin_unique, map);

	map.SetValue("plugin", plugin, true);
	map.SetValue("cookie", cookie, true);
	map.SetValue("dontLoadCoreBits", GetNativeCell(3), true);
	map.SetValue("isLoaded", 1, true);

	DataPack data = new DataPack();
	data.WriteFunction(GetNativeFunction(2));
	map.SetValue("callbacks", data, true);

	for(int i = 1; i <= MaxClients; i++)	if(Check_IsValidClient(i))
		Load_ClientCookieData(i, plugin_unique, map);

	return g_arPluginUniques.PushString(plugin_unique);
}

public int Native_MC_StartItem(Handle plugin, int numParams)
{
	char item[MAX_UNIQUE_LENGTH], buff[64];

	if(Check_IsItemRegisteredStartedOnPlugin(plugin, item, sizeof(item)))
		Error_Native_FinishRegisterThePreviousItem(item);

	int plugin_id = GetNativeCell(1);

	if(!Check_IsValidPluginId(_, plugin_id))
		Error_Native_InvalidPluginId(plugin_id);

	StringMap map;
	GetNativeString(2, item, sizeof(item));
	
	if(Get_ItemMap(_, plugin_id, item, map))
		Error_Native_ItemIsAlreadyRegistered(item, plugin_id);

	g_kvNowRegisteredItems.Rewind();

	Format(buff, sizeof(buff), "%i", plugin);
	g_kvNowRegisteredItems.JumpToKey(buff, true);

	g_kvNowRegisteredItems.SetNum("plugin_id", plugin_id);
	g_kvNowRegisteredItems.SetString("unique", item);

	return true;
}

public int Native_MC_EndItem(Handle plugin, int numParams)
{
	char item[MAX_UNIQUE_LENGTH];
	if(!Check_IsItemRegisteredStartedOnPlugin(plugin, item, sizeof(item)))
		Error_Native_NoItemToRegistered();

	int plugin_id = g_kvNowRegisteredItems.GetNum("plugin_id");
	int item_id, buff;
	StringMap plugin_map;
	StringMap item_map = new StringMap();
	ArrayList ar;
	DataPack data = view_as<DataPack>(g_kvNowRegisteredItems.GetNum("callbacks"));
	
	Get_PluginMap(_, plugin_id, plugin_map);
	plugin_map.SetValue(item, item_map);

	if(!plugin_map.GetValue("items array", ar))
	{
		ar = new ArrayList(64);
		plugin_map.SetValue("items array", ar, true);
	}

	ar.PushString(item);

	Format(item, sizeof(item), "1%i%i", plugin_id, GetRandomInt(0, 999999));
	item_id = StringToInt(item);

	item_map.SetValue("item_id", item_id, true);
	item_map.SetValue("plugin", plugin, true);
	item_map.SetValue("callbacks", data, true);

	if(!plugin_map.GetValue("preview", buff))
	{
		data.Position = ITEM_DATAPACKPOS_PREVIEW;
		Function func = data.ReadFunction();

		if(Check_IsValidFunction(func))
			plugin_map.SetValue("preview", 1, true);
	}

	g_kvNowRegisteredItems.DeleteThis();
	g_kvNowRegisteredItems.Rewind();

	return item_id;
}

public int Native_MC_EndPlugin(Handle plugin, int numParams)
{
	int plugin_id = GetNativeCell(1);
	
	char plugin_unique[MAX_UNIQUE_LENGTH];
	StringMap map;
	int load;

	Convert_PluginId(_, plugin_id, plugin_unique, sizeof(plugin_unique));
	Get_PluginMap(plugin_unique, _, map);

	if(!map.GetValue("isLoaded", load))
		Error_Native_NoPluginUniqueBeingRegistered();
		
	map.Remove("isLoad");
	
	Load_InCores(plugin_unique);
}

public int Native_MC_SetItemCallBacks(Handle plugin, int numParams)
{
	if(!Check_IsItemRegisteredStartedOnPlugin(plugin))
		Error_Native_NoItemToRegistered();

	DataPack data = new DataPack();

	g_kvNowRegisteredItems.SetNum("callbacks", view_as<int>(data));

	data.WriteFunction(GetNativeFunction(1));
	data.WriteFunction(GetNativeFunction(2));
}

public int Native_MC_UnRegisterMe(Handle plugin, int numParams)
{
	ArrayList ar;
	StringMap plugin_map, item_map;
	char plugin_unique[MAX_UNIQUE_LENGTH], item[MAX_UNIQUE_LENGTH];
	Handle plugin_owner;
	DataPack data;
	Cookie cookie;

	for(int index; index < g_arPluginUniques.Length; index++)
	{
		g_arPluginUniques.GetString(index, plugin_unique, sizeof(plugin_unique));
		
		if(!g_mapPluginUniques.GetValue(plugin_unique, plugin_map))
			continue;

		plugin_map.GetValue("plugin", plugin_owner);

		if(plugin != plugin_owner)
			continue;
		
		plugin_map.GetValue("items array", ar);

		if(plugin_map.GetValue("cookie", cookie) && cookie)
			delete cookie;

		if(plugin_map.GetValue("callbacks", data) && data)
			delete data;

		for(int item_index; item_index < ar.Length; item_index++)
		{
			ar.GetString(item_index, item, sizeof(item));

			if(!plugin_map.GetValue(item, item_map))
				continue;

			item_map.GetValue("plugin", plugin_owner);

			if(plugin != plugin_owner)
				continue;
			
			if(item_map.GetValue("callbacks", data) && data)
				delete data;
		}

		g_arPluginUniques.Erase(index);
		g_mapPluginUniques.Remove(plugin_unique);

		Unload_InCores(plugin_unique);

		for(int i = 1; i <= MaxClients; i++)	if(Check_IsValidClient(i))
			UnLoad_ClientCookieData(i, plugin_unique);

		delete plugin_map;
		delete ar;
		delete item_map;

		index -= 1;
	}
}

public int Native_MC_PrecacheFile(Handle plugin, int numParams)
{
	static int table = INVALID_STRING_TABLE;
	char file[256];

	GetNativeString(1, file, sizeof(file));

	if(!file)
		return -1;

	TrimString(file);

	MC_DataType type = view_as<MC_DataType>(GetNativeCell(2));
	int index;

	if(file[0])
	{
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

	if(!Check_IsValidClient(client))
		return false; 		// FIXME: добавить error

	int plugin_id = GetNativeCell(2);

	if(!Check_IsValidPluginId(_, plugin_id))
		Error_Native_InvalidPluginId(plugin_id);

	char plugin_unique[MAX_UNIQUE_LENGTH];
	Convert_PluginId(_, plugin_id, plugin_unique, sizeof(plugin_unique));

	ArrayList ar;
	if(!Get_PluginPriorityArrayList(plugin_unique, _, ar))
		return false;

	char item[MAX_UNIQUE_LENGTH];

	for(int num; num < ar.Length; num++)
	{
		ar.GetString(num, item, sizeof(item));

		if(!item[0])
			continue;

		if(!Get_ClientItem(client, plugin_unique, item, item, sizeof(item)))
			continue;

		SetNativeString(3, item, GetNativeCell(4));
		return true;
	}

	return false;
}

public int Native_MC_SetClientActiveItem(Handle plugin, int numParams)
{
	int client = GetNativeCell(1);

	if(!Check_IsValidClient(client))
		return false; 		// FIXME: добавить error

	int plugin_id = GetNativeCell(2);

	if(!Check_IsValidPluginId(_, plugin_id))
		Error_Native_InvalidPluginId(plugin_id);

	char plugin_unique[MAX_UNIQUE_LENGTH];
	Convert_PluginId(_, plugin_id, plugin_unique, sizeof(plugin_unique));
	ArrayList ar;

	if(!Get_PluginPriorityArrayList(plugin_unique, _, ar))
		return false;

	char item[MAX_UNIQUE_LENGTH];
	GetNativeString(4, item, sizeof(item));

	if(!Check_IsValidItemUnique(plugin_unique, item))
		Error_Native_InvalidItemUnique(plugin_unique, item);

	MC_CoreTypeBits core_type = view_as<MC_CoreTypeBits>(GetNativeCell(3));

	char key[32];
	if(core_type == Core_VIP)
		key = "vip";
	
	Set_ClientItem(client, plugin_unique, key, item);
	
	return true;
}

public int Native_MC_GetSettingsConfigKV(Handle plugin, int numParams)
{
	char buff[256];
	KeyValues kv;

	MC_CoreTypeBits core_type = GetNativeCell(1);

	if(core_type == Core_Shop)
		kv = GetKVFileFromPath("settings_shop", "Shop Config", buff, sizeof(buff));
	else
		ThrowNativeError(4, "Config for this Core not found!");

	if(!kv.ImportFromFile(buff))
		ThrowNativeError(4, "File '%s' does not funded", buff);

	return view_as<int>(kv);
}

public KeyValues GetKVFileFromPath(char[] file_name, char[] core_name, char[] buff, int bufflen)
{
	BuildPath(Path_SM, buff, bufflen, "configs/multi-core/%s.cfg", file_name);
	return (new KeyValues(core_name));
}
