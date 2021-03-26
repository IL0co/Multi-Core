#include <sourcemod>

public void Shop_Started()
{
	char plugin_unique[MAX_UNIQUE_LENGTH];

	for(int index; index < g_arPluginUniques.Length; index++)
	{
		g_arPluginUniques.GetString(index, plugin_unique, sizeof(plugin_unique));
		Load_Shop(plugin_unique);
	}
}

public void Load_Shop(char[] plugin_unique)
{
	if(!Check_IsAllowRegisterInCore(plugin_unique, Core_Shop))
		return;

	StringMap category_map, item_map;
	ArrayList ar;
	char category_unique[MAX_UNIQUE_LENGTH], category_name[MAX_UNIQUE_LENGTH], item[MAX_UNIQUE_LENGTH], item_name[MAX_UNIQUE_LENGTH], in_category[MAX_UNIQUE_LENGTH];
	CategoryId category_id;
	int pos;

	Get_PluginMap(plugin_unique, _, category_map);
	category_map.GetValue("items array", ar);

	ar = ar.Clone();

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

					if(strcmp(category_unique, plugin_unique, false) != 0)
					{
						g_kvItemsToShop.GetString("In Category", in_category, sizeof(in_category));
						if(!in_category[0] || strcmp(in_category, plugin_unique, false) != 0)
						{
							continue;
						}
					}
					
					if(!Shop_StartItem(category_id, item))
					{
						continue;
					}

					category_map.GetValue(item, item_map);

					CallBack_OnItemDisplay(0, plugin_unique, item, category_map, Core_Shop, item_name, sizeof(item_name));

					Shop_SetInfo(item_name, "", g_kvItemsToShop.GetNum("Price"), g_kvItemsToShop.GetNum("Sell Price"), Item_Togglable, g_kvItemsToShop.GetNum("Duration"), g_kvItemsToShop.GetNum("Gold Price"), g_kvItemsToShop.GetNum("Gold Sell Price"));
					Shop_SetLuckChance(g_kvItemsToShop.GetNum("Luck Chance"));
					Shop_SetCallbacks(_, CallBack_Shop_OnItemToggled, _, CallBack_Shop_OnItemDisplay, .preview = (Check_IsItemHavePreview(item_map) ? CallBack_Shop_OnItemPreview : INVALID_FUNCTION));
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
}

public bool CallBack_Shop_OnItemDisplay(int client, CategoryId category_id, const char[] category, ItemId item_id, const char[] item, ShopMenu menu, bool &disabled, const char[] name, char[] buffer, int maxlen)
{
	if(CallBack_OnPluginUniqueDisplay(client, category, _, Core_Shop, buffer, maxlen))
		return true;

	return false;
}

public void CallBack_Shop_OnItemPreview(int client, CategoryId category_id, const char[] category, ItemId item_id, const char[] item)
{
	char in_category[MAX_UNIQUE_LENGTH];
	Get_CategoryUniqueOfThisItem(category, item, in_category, sizeof(in_category));

	CallBack_OnItemPreview(client, in_category, item, _, Core_Shop);
}

public ShopAction CallBack_Shop_OnItemToggled(int client, CategoryId category_id, const char[] category, ItemId item_id, char[] item, bool isOn, bool elapsed)
{
	char in_category[MAX_UNIQUE_LENGTH];
	Get_CategoryUniqueOfThisItem(category, item, in_category, sizeof(in_category));

	if(isOn || elapsed)
	{
		Set_ClientItem(client, in_category, "shop", "");
		return Shop_UseOff;
	}
		
	Set_ClientItem(client, in_category, "shop", item);
	return Shop_UseOn;
}

void Get_CategoryUniqueOfThisItem(const char[] shop_category, const char[] item, char[] buffer, int maxlen)		// FIXME: сделать нормально, заменить на ранее сделанную карту
{
	g_kvItemsToShop.Rewind();
	g_kvItemsToShop.JumpToKey(shop_category);
	g_kvItemsToShop.JumpToKey(item);

	g_kvItemsToShop.GetString("In Category", buffer, maxlen);
}