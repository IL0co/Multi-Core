#include <sourcemod>

public void LoadNatives()
{
	g_arPluginUniques = new ArrayList(64);
	g_mapNowRegisteredItems = new StringMap();
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

public int Native_MC_RegisterPlugin(Handle plugin, int numParams)		// TODO: ? добавить таймер на удаление регистрации, что бы закрыть хендлы 
{
	MC_PluginId enum_plugin;

	if(Get_NowRegisterPluginId(plugin, enum_plugin))
		Error_Native_FinishRegisterThePreviousPluginUnique(enum_plugin);

	GetNativeString(1, enum_plugin.Unique, sizeof(enum_plugin.Unique));

	if(g_arPluginUniques.FindString(enum_plugin.Unique) != -1)
		Error_Native_PluginUniqueIsAlreadyRegistered(enum_plugin.Unique);
	
	enum_plugin.Plugin = plugin;

	char buff[96];
	Format(buff, sizeof(buff), "%i", plugin);
	g_mapNowRegisteredItems.SetArray(buff, enum_plugin, sizeof(enum_plugin));

	Format(buff, sizeof(buff), "MultiCore_%s", enum_plugin.Unique);
	enum_plugin.Cookie = new Cookie(buff, buff, CookieAccess_Private);

	enum_plugin.CB_OnDisplay = GetNativeFunction(2);
	enum_plugin.ArItems = new ArrayList(MAX_UNIQUE_LENGTH);
	enum_plugin.MapItems = new StringMap();
	enum_plugin.IgnoreCoresBits = GetNativeCell(3);
}

public int Native_MC_StartItem(Handle plugin, int numParams)
{
	MC_PluginId enum_plugin;

	if(Get_NowRegisterPluginId(plugin, enum_plugin))
		Error_Native_FinishRegisterThePreviousPluginUnique(enum_plugin);

	char item[MAX_UNIQUE_LENGTH];
	GetNativeString(1, item, sizeof(item));

	if(enum_plugin.IsRegisteredNowItem)
	{
		enum_plugin.ArItems.GetString(enum_plugin.ArItems.Length-1, item, sizeof(item));
		Error_Native_FinishRegisterThePreviousItem(item);
	}

	if(enum_plugin.ArItems.FindString(item) != -1)
	{
		Error_Native_ItemIsAlreadyRegistered(item, enum_plugin.Unique);
	}

	MC_Item enum_item;
	enum_item.Unique = item;

	enum_plugin.MapItems.SetArray(item, enum_item, sizeof(enum_item));
	enum_plugin.ArItems.PushString(item);

	enum_plugin.IsRegisteredNowItem = true;
	return true;
}

public int Native_MC_EndItem(Handle plugin, int numParams)
{
	MC_PluginId enum_plugin;

	if(!Get_NowRegisterPluginId(plugin, enum_plugin))
		Error_Native_NoPluginUniqueBeingRegistered();

	MC_Item enum_item;

	if(!enum_plugin.LastItem(enum_item))
	{
		Error_Native_NoItemUniqueBeingRegistered();
	}

	enum_plugin.IsRegisteredNowItem = false;
}

public int Native_MC_EndPlugin(Handle plugin, int numParams)
{
	MC_PluginId enum_plugin;

	if(!Get_NowRegisterPluginId(plugin, enum_plugin))
		Error_Native_NoPluginUniqueBeingRegistered();

	if(enum_plugin.IsRegisteredNowItem == true)
	{
		MC_Item enum_item;
		enum_plugin.LastItem(enum_item);

		Error_Native_FinishRegisterThePreviousItem(enum_item.Unique);
	}

	int index = g_arPluginUniques.PushString(enum_plugin.Unique);
	g_mapPluginUniques.SetArray(enum_plugin.Unique, enum_plugin, sizeof(enum_plugin));
	
	Load_InCores(enum_plugin.Unique);
	g_mapNowRegisteredItems.Remove(enum_plugin.Unique);
	
	return index;
}

public int Native_MC_SetItemCallBacks(Handle plugin, int numParams)
{
	MC_PluginId enum_plugin;

	if(!Get_NowRegisterPluginId(plugin, enum_plugin))
	{
		Error_Native_NoPluginUniqueBeingRegistered();
	}

	MC_Item enum_item;

	if(!enum_plugin.LastItem(enum_item))
	{
		Error_Native_NoItemUniqueBeingRegistered();
	}

	enum_item.CB_OnDisplay = GetNativeFunction(1);
	enum_item.CB_OnPreview = GetNativeFunction(2);
}

public int Native_MC_UnRegisterMe(Handle plugin, int numParams)
{
	MC_PluginId enum_plugin;
	char plugin_unique[MAX_UNIQUE_LENGTH];

	for(int index; index < g_arPluginUniques.Length; index++)
	{
		g_arPluginUniques.GetString(index, plugin_unique, sizeof(plugin_unique));
		g_mapPluginUniques.GetArray(plugin_unique, enum_plugin, sizeof(enum_plugin));

		if(enum_plugin.Plugin != plugin)
			continue;

		delete enum_plugin.MapItems;
		delete enum_plugin.ArItems;
		delete enum_plugin.Cookie;

		Unload_InCores(enum_plugin.Unique);

		for(int i = 1; i <= MaxClients; i++)	if(Check_IsValidClient(i))
			UnLoad_ClientCookieData(i, enum_plugin.Unique);
		
		g_arPluginUniques.Erase(index);
		g_mapPluginUniques.Remove(enum_plugin.Unique);

		if(Get_NowRegisterPluginId(plugin, enum_plugin))
		{
			delete enum_plugin.MapItems;
			delete enum_plugin.ArItems;
			delete enum_plugin.Cookie;
		}
		
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
