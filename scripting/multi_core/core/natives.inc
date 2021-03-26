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

public int Native_MC_RegisterPlugin(Handle plugin, int numParams)		// TODO: ? добавить таймер на удаление регистрации, что бы закрыть DataPack 
{
	if(Jump_ToPluginHandle(plugin))
		Error_Native_FinishRegisterThePreviousPluginUnique();

	char plugin_unique[MAX_UNIQUE_LENGTH];

	GetNativeString(1, plugin_unique, sizeof(plugin_unique));

	if(g_arPluginUniques.FindString(plugin_unique))
		Error_Native_PluginUniqueIsAlreadyRegistered(plugin_unique);
	
	char buff[96];
	StringMap map = new StringMap();

	map.SetValue("plugin", plugin);

	Format(buff, sizeof(buff), "%i", plugin);
	g_kvNowRegisteredItems.SetNum(buff, view_as<int>(map));

	map.SetString("plugin_unique", plugin_unique);

	Format(buff, sizeof(buff), "MultiCore_%s", plugin_unique);
	map.SetValue("cookie", new Cookie(buff, buff, CookieAccess_Private));

	DataPack data = new DataPack();
	data.WriteFunction(GetNativeFunction(2));
	map.SetValue("callbacks", data);

	map.SetValue("items_array", new ArrayList(MAX_UNIQUE_LENGTH));
	map.SetValue("items_map", new StringMap());

	map.SetValue("dontLoadCoreBits", GetNativeCell(3));
}

public int Native_MC_StartItem(Handle plugin, int numParams)
{
	char item[MAX_UNIQUE_LENGTH];
	StringMap plugin_map;

	GetNativeString(1, item, sizeof(item));

	if(!Jump_ToPluginHandle(plugin, plugin_map))
		Error_Native_CategoryIsNoRegisteredBefore(item);

	StringMap item_map;

	if(plugin_map.GetValue("registered_item", item_map))
	{
		item_map.GetString("item_unique", item, sizeof(item));
		Error_Native_FinishRegisterThePreviousItem(item);
	}

	ArrayList ar;

	plugin_map.GetValue("items_array", ar);

	if(ar.FindString(item) != -1)
	{
		char plugin_unique[MAX_UNIQUE_LENGTH];
		plugin_map.GetString("plugin_unique", plugin_unique, sizeof(plugin_unique));
		
		Error_Native_ItemIsAlreadyRegistered(item, plugin_unique);
	}

	item_map = new StringMap();
	plugin_map.SetValue("registered_item", item_map);
	item_map.SetString("item_unique", item);

	return true;
}

public int Native_MC_EndItem(Handle plugin, int numParams)
{
	StringMap plugin_map;

	if(!Jump_ToPluginHandle(plugin, plugin_map))
	{
		Error_Native_NoItemUniqueBeingRegistered();
	}

	StringMap item_map;

	if(!plugin_map.GetValue("registered_item", item_map))
	{
		Error_Native_NoItemUniqueBeingRegistered();
	}

	char item_unique[MAX_UNIQUE_LENGTH];
	item_map.GetString("item_unique", item_unique, sizeof(item_unique));
	item_map.Remove("item_unique");

	ArrayList items_array;
	plugin_map.GetValue("items_array", items_array);
	items_array.PushString(item_unique);

	StringMap plugin_items_map;
	plugin_map.GetValue("items_array", plugin_items_map);
	plugin_items_map.SetValue(item_unique, item_map);

	plugin_map.Remove("registered_item");
}

public int Native_MC_EndPlugin(Handle plugin, int numParams)
{
	StringMap plugin_map;

	if(!Jump_ToPluginHandle(plugin, plugin_map))
	{
		Error_Native_NoPluginUniqueBeingRegistered();
	}

	char plugin_unique[MAX_UNIQUE_LENGTH];
	plugin_map.GetString("plugin_unique", plugin_unique, sizeof(plugin_unique));
	plugin_map.Remove("plugin_unique");

	int index = g_arPluginUniques.PushString(plugin_unique);
	g_mapPluginUniques.SetValue(plugin_unique, plugin_map);

	g_kvNowRegisteredItems.DeleteThis();
	g_kvNowRegisteredItems.Rewind();

	Load_InCores(plugin_unique);
	
	return index;
}

public int Native_MC_SetItemCallBacks(Handle plugin, int numParams)
{
	StringMap plugin_map;

	if(!Jump_ToPluginHandle(plugin, plugin_map))
	{
		Error_Native_NoPluginUniqueBeingRegistered();
	}

	StringMap item_map;

	if(!plugin_map.GetValue("registered_item", item_map))
	{
		Error_Native_NoItemUniqueBeingRegistered();
	}

	DataPack data;
	
	if(item_map.GetValue("callbacks", data))
	{
		char plugin_unique[MAX_UNIQUE_LENGTH], item_unique[MAX_UNIQUE_LENGTH];
		plugin_map.GetString("plugin_unique", plugin_unique, sizeof(plugin_unique));
		item_map.GetString("item_unique", item_unique, sizeof(item_unique));

		Error_Native_CallBacksInItemIsAlreadyRegistered(item_unique, plugin_unique);
	}

	data = new DataPack();
	data.WriteFunction(GetNativeFunction(1));
	data.WriteFunction(GetNativeFunction(2));
	
	item_map.SetValue("callbacks", data);
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

	ArrayList items_array;
	StringMap plugin_map;
	Get_PluginMap(plugin_unique, _, plugin_map);
	plugin_map.GetValue("items_array", items_array);
	
	char item[MAX_UNIQUE_LENGTH];

	for(int num; num < ar.Length; num++)
	{
		ar.GetString(num, item, sizeof(item));

		if(!item[0])
			continue;

		if(!Get_ClientItem(client, plugin_unique, item, item, sizeof(item)))
			continue;

		if(items_array.FindString(item) == -1)
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
