#include <sourcemod>

bool g_bIsCoreLoaded;

char g_hCookieKeys[][] = {"all", "vip", "shop"};

enum MC_PluginMap {};
enum MC_ItemMap {};
enum MC_Error {};

ArrayList g_arPlugins;
MC_PluginMap g_mapPlugins;
StringMap g_mapNowRegisteredPlugins;
StringMap g_mapPriorities;	
KeyValues g_kvItemsToAll;

stock DataPackPos PLUGIN_DATAPACKPOS_DISPLAY	= view_as<DataPackPos>(0);
stock DataPackPos PLUGIN_DATAPACKPOS_SELECTED	= view_as<DataPackPos>(1);

stock DataPackPos ITEM_DATAPACKPOS_DISPLAY		= view_as<DataPackPos>(0);
stock DataPackPos ITEM_DATAPACKPOS_PREVIEW		= view_as<DataPackPos>(1);
stock DataPackPos ITEM_DATAPACKPOS_SELECTED		= view_as<DataPackPos>(2);

KeyValues g_kvActiveClientList[MAXPLAYERS+1];	

methodmap MC_ItemMap < StringMap
{
	public MC_ItemMap()
	{
		StringMap map = new StringMap();
		map.SetValue("CallBacks", new DataPack());

		return view_as<MC_ItemMap>(map);
	}

	public bool GetUnique(char[] buffer, int maxlen)
	{
		if(this.GetString("Unique", buffer, maxlen))
			return true;

		return false;
	}

	public void SetUnique(char[] item_unique)
	{
		this.SetString("Unique", item_unique);
	}

	public DataPack GetCallBacksPack()
	{
		DataPack data;
		this.GetValue("CallBacks", data);

		return data;
	}

	public Function GetCallBack(DataPackPos pos)
	{
		DataPack data = this.GetCallBacksPack();

		if(!data)
			return INVALID_FUNCTION;

		data.Position = pos; 

		return data.ReadFunction();
	}

	public void Remove()
	{
		delete this.GetCallBacksPack();
		delete this;
	}
}

methodmap MC_PluginMap < StringMap
{
	public MC_PluginMap()
	{
		StringMap map = new StringMap();
		map.SetValue("Items Map", new StringMap());
		map.SetValue("Items Array", new ArrayList(MAX_UNIQUE_LENGTH));
		map.SetValue("CallBacks", new DataPack());

		return view_as<MC_PluginMap>(map);
	}

	public bool GetUnique(char[] buffer, int maxlen)
	{
		if(this.GetString("Unique", buffer, maxlen))
			return true;

		return false;
	}

	public void SetUnique(char[] plugin_id)
	{
		this.SetString("Unique", plugin_id);
	}

	public ArrayList GetItemsArray()
	{
		ArrayList ar;
		this.GetValue("Items Array", ar);

		return ar;
	}

	public StringMap GetItemsMap()
	{
		StringMap map;
		this.GetValue("Items Map", map);

		return map;
	}

	public void SaveGlobal()
	{
		char plugin_id[MAX_UNIQUE_LENGTH];
		this.GetUnique(plugin_id, sizeof(plugin_id));

		g_mapPlugins.SetValue(plugin_id, this, true);
		g_arPlugins.PushString(plugin_id);
	}

	public void SaveItem(MC_ItemMap item_map, char[] item_unique)
	{
		this.GetItemsArray().PushString(item_unique);
		this.GetItemsMap().SetValue(item_unique, item_map, true);
	}

	public MC_ItemMap GetItemMap(const char[] item_unique)
	{
		StringMap map = this.GetItemsMap();

		map.GetValue(item_unique, map);
		return view_as<MC_ItemMap>(map);
	}

	public bool GetLastItemMap(MC_ItemMap &item_map)
	{
		ArrayList ar = this.GetItemsArray();

		char item_unique[MAX_UNIQUE_LENGTH];

		ar.GetString(ar.Length-1, item_unique, sizeof(item_unique));

		if(this.GetItemsMap().GetValue(item_unique, item_map))
			return true;

		return false;
	}

	public bool IsValid(char[] plugin_id)
	{
		if(!plugin_id[0])
			return false;

		StringMap map;

		if(this.GetValue(plugin_id, map))
			return true;

		return false;
	}

	public bool IsValidItem(char[] item_unique)
	{
		if(!item_unique[0])
			return false;
			
		ArrayList ar = this.GetItemsArray();

		if(!ar || ar.FindString(item_unique) == -1)
			return false;

		return true;
	}

	public void UnregisterItems()
	{
		ArrayList ar = this.GetItemsArray();
		char item_unique[MAX_UNIQUE_LENGTH];
		MC_ItemMap item_map;

		for(int id; id < ar.Length; id++)
		{
			ar.GetString(id, item_unique, sizeof(item_unique));

			if(!item_unique[0])
				continue;

			item_map = this.GetItemMap(item_unique);
			item_map.Remove();
		}
	}

	public DataPack GetCallBacksPack()
	{
		DataPack data;
		this.GetValue("CallBacks", data);

		return data;
	}

	public Function GetCallBack(DataPackPos pos)
	{
		DataPack data = this.GetCallBacksPack();

		if(!data)
			return INVALID_FUNCTION;

		data.Position = pos; 

		return data.ReadFunction();
	}

	property bool OpenForRegistration
	{
		public get()
		{
			bool state;
			this.GetValue("Open For Registration", state);

			return state;
		}
		public set(bool state)
		{
			this.SetValue("Open For Registration", state, true);
		}
	}

	property bool IsHavePreviewItem
	{
		public get()
		{
			bool state;
			this.GetValue("Have Preview Item", state);

			return state;
		}
		public set(bool state)
		{
			this.SetValue("Have Preview Item", state, true);
		}
	}

	property Handle Plugin
	{
		public get()
		{
			Handle plugin;
			this.GetValue("Plugin", plugin);

			return plugin;
		}
		public set(Handle plugin)
		{
			this.SetValue("Plugin", plugin, true);
		}
	}
	property Cookie Cookie
	{
		public get()
		{
			Cookie cookie;
			this.GetValue("Cookie", cookie);

			return cookie;
		}
		public set(Cookie cookie)
		{
			this.SetValue("Cookie", cookie, true);
		}
	}

	property Handle DeathTimer
	{
		public get()
		{
			Handle timer;
			this.GetValue("Death Timer", timer);

			return timer;
		}
		public set(Handle timer)
		{
			this.SetValue("Death Timer", timer, true);
		}
	}

	public void UnregisterMe()
	{
		delete this.Cookie;
		delete this.GetCallBacksPack();
		this.UnregisterItems();
		delete this.GetItemsMap();
		delete this.GetItemsArray();
		delete this;
	}
}

public bool GetPluginMap(const char[] plugin_id, MC_PluginMap &map)
{
	if(g_mapPlugins.GetValue(plugin_id, map))
		return true;

	return false;
}