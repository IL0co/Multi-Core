#include <sourcemod>

public void Shop_Started()
{
	char plugin_id[MAX_UNIQUE_LENGTH];

	for(int index; index < g_arPlugins.Length; index++)
	{
		g_arPlugins.GetString(index, plugin_id, sizeof(plugin_id));
		Load_Shop(plugin_id, false);
	}
}

stock void Load_Shop(char[] plugin_id, bool check_is_core_loaded = true)
{
	if(!plugin_id[0] || (check_is_core_loaded && !Check_IsCoreLoaded(Core_Shop)))
		return;
		
	MC_PluginMap map;
	GetPluginMap(plugin_id, map);

	if(map.DontLoadInCores & Core_Shop)
		return;

	char category_unique[MAX_UNIQUE_LENGTH], category_name[MAX_UNIQUE_LENGTH], item[MAX_UNIQUE_LENGTH], item_name[MAX_UNIQUE_LENGTH], in_category[MAX_UNIQUE_LENGTH];
	CategoryId category_id;
	int pos;
	MC_ItemMap item_map;

	ArrayList ar = map.GetItemsArray().Clone();

	g_kvItemsToShop.Rewind();
	if(g_kvItemsToShop.GotoFirstSubKey())
	{
		do
		{
			g_kvItemsToShop.GetSectionName(category_unique, sizeof(category_unique));
			g_kvItemsToShop.GetString("Name", category_name, sizeof(category_name));

			g_kvItemsToShop.SavePosition();
			if(g_kvItemsToShop.GotoFirstSubKey())
			{
				category_id = Shop_RegisterCategory(category_unique, (category_name[0] ? category_name : category_unique), "");

				do
				{
					g_kvItemsToShop.GetSectionName(item, sizeof(item));

					if((pos = ar.FindString(item)) == -1)
					{
						continue;
					}

					if(strcmp(category_unique, plugin_id, false) != 0)
					{
						g_kvItemsToShop.GetString("In Category", in_category, sizeof(in_category));
						
						if(!in_category[0] || strcmp(in_category, plugin_id, false) != 0)
						{
							continue;
						}
					}
					
					if(!Shop_StartItem(category_id, item))
					{
						continue;
					}

					item_map = map.GetItemMap(item);

					CallBack_OnItemDisplay(0, plugin_id, item, Core_Shop, item_name, sizeof(item_name));

					Shop_SetInfo(item_name, "", g_kvItemsToShop.GetNum("Price"), g_kvItemsToShop.GetNum("Sell Price"), Item_Togglable, g_kvItemsToShop.GetNum("Duration"), g_kvItemsToShop.GetNum("Gold Price"), g_kvItemsToShop.GetNum("Gold Sell Price"));
					Shop_SetLuckChance(g_kvItemsToShop.GetNum("Luck Chance"));
					Shop_SetCallbacks(_, CallBack_Shop_OnItemToggled, _, CallBack_Shop_OnItemDisplay, .preview = (item_map.GetCallBack(ITEM_DATAPACKPOS_PREVIEW) ? CallBack_Shop_OnItemPreview : INVALID_FUNCTION));
					Shop_SetHide(view_as<bool>(g_kvItemsToShop.GetNum("Hide", 0)));
					Shop_EndItem();

					ar.Erase(pos);
				}
				while(g_kvItemsToShop.GotoNextKey());
			
				g_kvItemsToShop.GoBack();
			}
		}
		while(g_kvItemsToShop.GotoNextKey());
	}

	delete ar;
}

public bool CallBack_Shop_OnItemDisplay(int client, CategoryId category_id, const char[] category, ItemId item_id, const char[] item_unique, ShopMenu menu, bool &disabled, const char[] name, char[] buffer, int maxlen)
{
	char plugin_id[MAX_UNIQUE_LENGTH];
	Get_CategoryUniqueOfThisItem(category, item_unique, plugin_id, sizeof(plugin_id));
	
	if(CallBack_OnItemDisplay(client, plugin_id, item_unique, Core_Shop, buffer, maxlen))
		return true;

	return false;
}

public void CallBack_Shop_OnItemPreview(int client, CategoryId category_id, const char[] category, ItemId item_id, const char[] item_unique)
{
	char plugin_id[MAX_UNIQUE_LENGTH];
	Get_CategoryUniqueOfThisItem(category, item_unique, plugin_id, sizeof(plugin_id));

	CallBack_OnItemPreview(client, plugin_id, item_unique, Core_Shop);
}

public ShopAction CallBack_Shop_OnItemToggled(int client, CategoryId category_id, const char[] category, ItemId item_id, char[] item_unique, bool isOn, bool elapsed)
{
	char plugin_id[MAX_UNIQUE_LENGTH];
	Get_CategoryUniqueOfThisItem(category, item_unique, plugin_id, sizeof(plugin_id));

	MC_PluginMap map;
	if(!GetPluginMap(plugin_id, map))
		return Shop_Raw;

	if(isOn || elapsed)
	{
		if(CallBack_OnItemSelected(client, map, plugin_id, "", Core_Shop))
			SetPlayerSelectedItem(client, g_cLastSeePluginUnique[client], g_hCookieKeys[Shop], "");
			
		return Shop_UseOff;
	}
		
	if(CallBack_OnItemSelected(client, map, plugin_id, item_unique, Core_Shop))
		SetPlayerSelectedItem(client, g_cLastSeePluginUnique[client], g_hCookieKeys[Shop], item_unique);

	return Shop_UseOn;
}

void Get_CategoryUniqueOfThisItem(const char[] shop_category, const char[] item, char[] buffer, int maxlen)		// FIXME: сделать нормально, заменить на ранее сделанную карту
{
	g_kvItemsToShop.Rewind();
	g_kvItemsToShop.JumpToKey(shop_category);
	g_kvItemsToShop.JumpToKey(item);

	g_kvItemsToShop.GetString("In Category", buffer, maxlen);
}