#include <sourcemod>

char g_LoadCoreType[][] = 		   {"shop",    "vip_core"};
MC_CoreTypeBits g_LoadCoreBits[] = {Core_Shop, Core_VIP};
MC_CoreTypeBits g_IsCoreLoadBits;

char g_hCookieKeys[][] = {"all", "vip", "shop"};

KeyValues g_kvItemsToAll;
/*
	"Plugin Unique"		"ArrayList"		// Массив с идентификаторами предметов
*/

KeyValues g_kvItemsToShop;

KeyValues g_kvActiveClientList[MAXPLAYERS+1];	// Карта с данными игрока

/* Карта Cookie:
	all|vip|shop
*/

StringMap g_mapPriorities;					// Карта с ArrayList'ами приоритетов
/*	"Plugin Unique"		"ArrayList"		*/

StringMap g_mapNowRegisteredItems;		
ArrayList g_arPluginUniques;		// Массив с идентификаторами плагинов
StringMap g_mapPluginUniques;
/*
	g_arPluginUniques = ArrayList: // Перечисление плагинов. Тут же есть ид плагинов

	g_mapPluginUniques = StringMap:		
						"plugin unique"		"StringMap"

	StringMap (plugin unique):
		"plugin"			"Handle плагина" 			// Handle плагина, который зарегистрировал этот плагин
		"cookie"			"char"						// Активный предмет игрока (бд)
		"callbacks"			"DataPack"					// Обратные вызова 
		"dontLoadCoreBits"	"int (MC_CoreTypeBits)"		// Индекса ядер, в которые не регистрировать 

		"items_array"		"ArrayList"					// Перечисление зарегистрированных предметов
		"item unique"		"StringMap"					// Карта данных о предмете

			StringMap (item unique):
TODO:			"plugin"		"Handle плагина"	// Handle плагина, который зарегистрировал этот предметы
				"item_id"		"int"				// Индекс предмета (ид предмета)
				"callbacks"		"DataPack"			// Обратные вызова
				
*/

stock DataPackPos PLUGIN_DATAPACKPOS_DISPLAY	= view_as<DataPackPos>(0);

stock DataPackPos ITEM_DATAPACKPOS_DISPLAY		= view_as<DataPackPos>(0);
stock DataPackPos ITEM_DATAPACKPOS_PREVIEW		= view_as<DataPackPos>(1);


// ArrayList g_arPluginUniques;		// Массив с идентификаторами плагинов
// StringMap g_mapPluginUniques;		// "Plugin_Unique"		"MC_PluginId"

enum struct MC_Item
{
	Function CB_OnDisplay;
	Function CB_OnPreview;
	char Unique[MAX_UNIQUE_LENGTH];
}

enum struct MC_PluginId
{
	Handle Plugin;
	Cookie Cookie;
	Function CB_OnDisplay;
	int IgnoreCoresBits;
	char Unique[MAX_UNIQUE_LENGTH];

	MC_Item	Items;

	ArrayList ArItems;
	StringMap MapItems;

	bool IsRegisteredNowItem;

	bool Item(char[] item_unique, MC_Item item = 0)
	{
		if(this.MapItems.GetArray(item_unique, item, sizeof(item)))
			return true;

		return false;
	}

	bool LastItem(MC_Item item = 0)
	{
		char item_unique[MAX_UNIQUE_LENGTH];
		this.ArItems.GetString(this.ArItems.Length-1, item_unique, sizeof(item_unique));

		if(this.MapItems.GetArray(item_unique, item, sizeof(item)))
			return true;

		return false;
	}

	void CB_SetItemDisplay(char[] item_unique, Function func)
	{
		MC_Item item;
		this.Item(item_unique, item);
		item.CB_OnPreview = func;
	}
	
	Function CB_GetItemDisplay(char[] item_unique)
	{
		MC_Item item;
		this.Item(item_unique, item);
		return item.CB_OnDisplay;
	}

	void CB_SetItemPreview(char[] item_unique, Function func)
	{
		MC_Item item;
		this.Item(item_unique, item);
		item.CB_OnPreview = func;
	}

	Function CB_GetItemPreview(char[] item_unique)
	{
		MC_Item item;
		this.Item(item_unique, item);
		return item.CB_OnPreview;
	}
}
