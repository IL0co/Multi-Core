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

KeyValues g_kvNowRegisteredItems;			
/*
	"plugin Handle"
	{
		unique			char		// Уникальное имя предмета
		plugin_id		int			// Индекс плагина в которой регистрируется
		callbacks		DataPack	// Обратные вызова

		"item_unique"
		{
			callbacks		DataPack	// Обратные вызова
		}
	}
*/

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
		"isLoaded"			"int"						// Обозначение что плагин ещё регистрируется 
		"dontLoadCoreBits"	"int (MC_CoreTypeBits)"		// Индекса ядер, в которые не регистрировать 

		"items array"		"ArrayList"					// Перечисление зарегистрированных предметов
		"item unique"		"StringMap"					// Карта данных о предмете

			StringMap (item unique):
				"plugin"		"Handle плагина"	// Handle плагина, который зарегистрировал этот предметы
				"item_id"		"int"				// Индекс предмета (ид предмета)
				"callbacks"		"DataPack"			// Обратные вызова
				
*/

stock DataPackPos PLUGIN_DATAPACKPOS_DISPLAY	= view_as<DataPackPos>(0);

stock DataPackPos ITEM_DATAPACKPOS_DISPLAY		= view_as<DataPackPos>(0);
stock DataPackPos ITEM_DATAPACKPOS_PREVIEW		= view_as<DataPackPos>(1);
