#include <sourcemod>

char g_LoadCoreType[][] = 		   {"shop",    "vip_core"};
MC_CoreTypeBits g_LoadCoreBits[] = {Core_Shop, Core_VIP};
MC_CoreTypeBits g_IsCoreLoadBits;

char g_hCookieKeys[][] = {"all", "vip", "shop"};

enum MC_PluginMap {};
enum MC_ItemMap {};
enum MC_PluginId {};
enum MC_Plugin {};
enum MC_Error {};
enum MC_Client {};

ArrayList g_arPlugins;
MC_PluginMap g_mapPlugins;
StringMap g_mapNowRegisteredPlugins;
StringMap g_mapPriorities;	
KeyValues g_kvItemsToShop;
KeyValues g_kvItemsToAll;

stock DataPackPos PLUGIN_DATAPACKPOS_DISPLAY	= view_as<DataPackPos>(0);

stock DataPackPos ITEM_DATAPACKPOS_DISPLAY		= view_as<DataPackPos>(0);
stock DataPackPos ITEM_DATAPACKPOS_PREVIEW		= view_as<DataPackPos>(1);

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
		data.Position = pos; 

		return data.ReadFunction();
	}

	public void Remove()
	{
		delete this.GetCallBacksPack();
		delete this;
	}
}

methodmap MC_Plugin < Handle
{
	property MC_PluginMap NowRegisteredMap
	{
		public get()
		{
			char buff[32];
			FormatEx(buff, sizeof(buff), "%i", this);

			MC_PluginMap map;
			g_mapNowRegisteredPlugins.GetValue(buff, map);

			return map;
		}
		public set(MC_PluginMap map)
		{
			char buff[32];
			FormatEx(buff, sizeof(buff), "%i", this);

			g_mapNowRegisteredPlugins.SetValue(buff, map);
		}
	}

	public void Remove()
	{
		char buff[32];
		FormatEx(buff, sizeof(buff), "%i", this);

		g_mapNowRegisteredPlugins.Remove(buff);
	}
}

methodmap MC_PluginId < ArrayList
{
	public bool IsValid()
	{
		int pos = view_as<int>(this);

		if(pos == -1 || pos >= g_arPlugins.Length)
			return false;
	
		char buff[1];
		g_arPlugins.GetString(pos, buff, sizeof(buff));

		if(!buff[0])
			return false;

		return true;
	}

	public bool GetUnique(char[] buffer, int maxlen)
	{
		if(g_arPlugins.GetString(view_as<int>(this), buffer, maxlen) == -1)
			return false;

		return true;
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

	public void SetUnique(char[] plugin_unique)
	{
		this.SetString("Unique", plugin_unique);
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

	public int SaveGlobal()
	{
		char plugin_unique[MAX_UNIQUE_LENGTH];
		this.GetUnique(plugin_unique, sizeof(plugin_unique));

		g_mapPlugins.SetValue(plugin_unique, this, true);
		return g_arPlugins.PushString(plugin_unique);
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

	public bool IsValid(char[] plugin_unique)
	{
		if(!plugin_unique[0])
			return false;

		StringMap map;

		if(this.GetValue(plugin_unique, map))
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

	property MC_CoreTypeBits DontLoadInCores
	{
		public get()
		{
			MC_CoreTypeBits bits;
			this.GetValue("Dont Load In Cores", bits);

			return bits;
		}
		public set(MC_CoreTypeBits bits)
		{
			this.SetValue("Dont Load In Cores", bits, true);
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

methodmap MC_Client
{
	public bool IsValid(bool check_kv = false)
	{
		int client = view_as<int>(this);

		if(check_kv && !g_kvActiveClientList[client])
			return false;

		if(!IsClientAuthorized(client) || !IsClientInGame(client) || IsFakeClient(client))
			return false;

		return true;
	}

	public bool GetSelectedItem(const char[] plugin_unique, char[] core_type, char[] buffer = "", int maxlen = 0)
	{
		int client = view_as<int>(this);

		g_kvActiveClientList[client].Rewind();

		if(!g_kvActiveClientList[client].JumpToKey(plugin_unique))
			return false;

		g_kvActiveClientList[client].GetString(core_type, buffer, maxlen);

		if(buffer[0])
			return true;

		return false;
	}

	public void SaveCookie(char[] plugin_unique)
	{
		int client = view_as<int>(this);

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

	public void LoadCookies()
	{
		int client = view_as<int>(this);
		char plugin_unique[MAX_UNIQUE_LENGTH];
		char buffer[256], exp[sizeof(g_hCookieKeys)][64];
		MC_PluginMap map;
		Cookie cookie;

		for(int id; id < g_arPlugins.Length; id++)
		{
			g_arPlugins.GetString(id, plugin_unique, sizeof(plugin_unique));
			
			if(!plugin_unique[0] || !g_mapPlugins.GetValue(plugin_unique, map))
				continue;

			g_kvActiveClientList[this].Rewind();
			g_kvActiveClientList[this].JumpToKey(plugin_unique, true);

			cookie = map.Cookie;

			if(cookie.GetClientTime(client) == 0)
			{
				g_kvItemsToAll.Rewind();
				ArrayList ar = view_as<ArrayList>(g_kvItemsToAll.GetNum(plugin_unique));

				if(ar)
				{
					ar.GetString(0, buffer, sizeof(buffer));
					g_kvActiveClientList[this].SetString(g_hCookieKeys[All], buffer);
				}
			}
			else
			{
				for(int num; num < sizeof(exp); num++)
					exp[num][0] = '\0';

				cookie.Get(client, buffer, sizeof(buffer));
				ExplodeString(buffer, "|", exp, sizeof(exp), sizeof(exp[]));

				for(int num; num < sizeof(g_hCookieKeys); num++)
					g_kvActiveClientList[this].SetString(g_hCookieKeys[num], exp[num]);
			}
		}
	}

	public void SetSelectedItem(char[] plugin_unique, char[] core_type, char[] item_unique)
	{
		g_kvActiveClientList[this].Rewind();
		g_kvActiveClientList[this].JumpToKey(plugin_unique, true);
		g_kvActiveClientList[this].SetString(core_type, item_unique);

		this.SaveCookie(plugin_unique);
	}


	public bool GetActiveItem(char[] plugin_unique, char[] buffer, int maxlen)
	{
		ArrayList ar;

		if(!g_mapPriorities.GetValue(plugin_unique, ar))
			return false;

		MC_PluginMap map;
		GetPluginMap(plugin_unique, map);
		ArrayList arItems = map.GetItemsArray();

		if(!arItems.Length) 
			return false;
		
		char item_unique[MAX_UNIQUE_LENGTH], core_type[32];

		for(int num; num < ar.Length; num++)
		{
			ar.GetString(num, core_type, sizeof(core_type));

			if(!this.GetSelectedItem(plugin_unique, core_type, item_unique, sizeof(item_unique)))
				continue;

			if(arItems.FindString(item_unique) == -1)
				continue;

			FormatEx(buffer, maxlen, item_unique);
			return true;
		}

		return false;
	}
}

public bool GetPluginMap(const char[] plugin_unique, MC_PluginMap &map)
{
	if(g_mapPlugins.GetValue(plugin_unique, map))
		return true;

	return false;
}

public int GetPluginId(const char[] plugin_unique)
{
	return g_arPlugins.FindString(plugin_unique);
}