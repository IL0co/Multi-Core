#include <sourcemod>

bool g_bPreviewMode[MAXPLAYERS+1];
char g_cLastSeePluginUnique[MAXPLAYERS+1][MAX_UNIQUE_LENGTH];

public void VIP_OnVIPLoaded()
{
	char plugin_id[MAX_UNIQUE_LENGTH];

	for(int index; index < g_arPlugins.Length; index++)
	{
		g_arPlugins.GetString(index, plugin_id, sizeof(plugin_id));
		Load_Vip(plugin_id);
	}
}

void Load_Vip(char[] plugin_id)
{
	if(!plugin_id[0] || !Check_IsCoreLoaded(Core_VIP))
		return;

	MC_PluginMap map;
	GetPluginMap(plugin_id, map);
	
	if(map.DontLoadInCores & Core_VIP || VIP_IsValidFeature(plugin_id))
		return;

	VIP_RegisterFeature(plugin_id, STRING, SELECTABLE, CallBack_VIP_OnItemSelected, CallBack_VIP_OnItemDisplayed, .bCookie = false);
}

public bool CallBack_VIP_OnItemDisplayed(int client, const char[] plugin_id, char[] display, int maxlength)
{
	if(CallBack_OnPluginUniqueDisplay(client, plugin_id, Core_VIP, display, maxlength))
	{
		MC_PluginMap map;
		GetPluginMap(plugin_id, map);

		if(!map.IsHavePreviewItem)
		{
			Format(display, maxlength, "%s%T", display, (GetPlayerSelectedItem(client, plugin_id, g_hCookieKeys[VIP]) ? "ENABLED" : "DISABLED"), client);
		}

		return true;
	}

	return false;
}

public bool CallBack_VIP_OnItemSelected(int client, const char[] plugin_id)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_id, map))
		return false;

	if(!CallBack_OnPluginSelected(client, map, plugin_id, Core_VIP))
		return true;

	if(!map.IsHavePreviewItem)
	{
		if(GetPlayerSelectedItem(client, plugin_id, g_hCookieKeys[VIP]))
		{
			SetPlayerSelectedItem(client, g_cLastSeePluginUnique[client], g_hCookieKeys[VIP], "");
		}
		else
		{
			char item_unique[MAX_UNIQUE_LENGTH];
			VIP_GetClientFeatureString(client, plugin_id, item_unique, sizeof(item_unique));

			SetPlayerSelectedItem(client, g_cLastSeePluginUnique[client], g_hCookieKeys[VIP], item_unique);
		}

		return true;
	}

	g_bPreviewMode[client] = false;

	Menu_Vip_SelectItem(client, plugin_id).Display(client, 0);
	Format(g_cLastSeePluginUnique[client], sizeof(g_cLastSeePluginUnique[]), plugin_id);

	return false;
}

public Menu Menu_Vip_SelectItem(int client, const char[] plugin_id)
{
/* 
	Имя идентификатора плагина

	Превью режим [Вкл/Выкл]
	Выключить [Выбрано]

	Перечисление предметов...

	Назад
	Выход
*/
	char buff[256], translate[128], selected_item[MAX_UNIQUE_LENGTH];
	bool selected;

	MC_PluginMap plugin_map;
	MC_ItemMap item_map;
	GetPluginMap(plugin_id, plugin_map);

	GetPlayerSelectedItem(client, plugin_id, g_hCookieKeys[VIP], selected_item, sizeof(selected_item));

	Menu menu = new Menu(MenuHandler_Vip_SelectItem);
	menu.ExitBackButton = true;

	CallBack_OnPluginUniqueDisplay(client, plugin_id, Core_VIP, translate, sizeof(translate));
	Format(translate, sizeof(translate), "%s\n ", translate);
	menu.SetTitle(translate);

	if(plugin_map.IsHavePreviewItem)
	{
		Format(translate, sizeof(translate), "%T%T", "PREVIEW MODE", client, (g_bPreviewMode[client] ? "ENABLED" : "DISABLED"), client);
		menu.AddItem("p", translate);
	}

	if(!selected_item[0])
		Format(translate, sizeof(translate), "%T", "SELECTED", client);
	else 
		translate[0] = '\0';
		
	Format(translate, sizeof(translate), "%T%s\n ", "DISABLE", client, translate);
	menu.AddItem("o", translate, (g_bPreviewMode[client] || !selected_item[0]) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);

	VIP_GetClientFeatureString(client, plugin_id, buff, sizeof(buff));

	if(strcmp(buff, "all", false) == 0)
	{
		ArrayList ar;
		plugin_map.GetValue("items_array", ar);

		char item[MAX_UNIQUE_LENGTH];

		for(int num; num < ar.Length; num++)
		{
			ar.GetString(num, item, sizeof(item));
			item_map = plugin_map.GetItemMap(item);

			if(g_bPreviewMode[client] && !item_map.GetCallBack(ITEM_DATAPACKPOS_PREVIEW))
				continue;

			selected = (strcmp(selected_item, item) == 0);
			Fill_MenuByItems(menu, client, selected, plugin_id, item);
		}
	}
	else
	{
		char exp[64][MAX_UNIQUE_LENGTH];

		int count = ExplodeString(buff, ";", exp, sizeof(exp), sizeof(exp[]));

		for(int num; num < count; num++)
		{
			if(!(item_map = plugin_map.GetItemMap(exp[num])))
				continue;
			
			if(g_bPreviewMode[client] && !item_map.GetCallBack(ITEM_DATAPACKPOS_PREVIEW))
				continue;

			selected = (strcmp(selected_item, exp[num]) == 0);
			Fill_MenuByItems(menu, client, selected, plugin_id, exp[num]);
		}
	}

	return menu;
}

void Fill_MenuByItems(Menu menu, int client, bool selected, const char[] plugin_id, char[] item_unique)
{
	char buff[128];

	CallBack_OnItemDisplay(client, plugin_id, item_unique, Core_VIP, buff, sizeof(buff));

	if(!g_bPreviewMode[client] && selected)
	{
		Format(buff, sizeof(buff), "%s%T", buff, "SELECTED", client);
	}

	menu.AddItem(item_unique, buff, (!g_bPreviewMode[client] && selected) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
}

public int MenuHandler_Vip_SelectItem(Menu menu, MenuAction action, int client, int item)
{
	if(action == MenuAction_Select)
	{
		char item_unique[MAX_UNIQUE_LENGTH];
		menu.GetItem(item, item_unique, sizeof(item_unique));

		MC_PluginMap plugin_map;
		GetPluginMap(g_cLastSeePluginUnique[client], plugin_map);

		MC_ItemMap item_map = plugin_map.GetItemMap(item_unique);
	
		if(item == 0 && item_unique[0] == 'p')
		{
			g_bPreviewMode[client] = !g_bPreviewMode[client];
		}
		else if(item_unique[0] == 'o' && (item == 0 || item == 1))
		{
			if(CallBack_OnItemSelected(client, plugin_map, item_map, g_cLastSeePluginUnique[client], "", Core_VIP))
				SetPlayerSelectedItem(client, g_cLastSeePluginUnique[client], g_hCookieKeys[VIP], "");
		}
		else if(g_bPreviewMode[client])
		{
			CallBack_OnItemPreview(client, g_cLastSeePluginUnique[client], item_unique, Core_VIP);
		}
		else
		{
			if(CallBack_OnItemSelected(client, plugin_map, item_map, g_cLastSeePluginUnique[client], item_unique, Core_VIP))
				SetPlayerSelectedItem(client, g_cLastSeePluginUnique[client], g_hCookieKeys[VIP], item_unique);
		}

		Menu_Vip_SelectItem(client, g_cLastSeePluginUnique[client]).DisplayAt(client, menu.Selection, 0);
	}
	else if(action == MenuAction_Cancel && item == MenuCancel_ExitBack) 
	{
		VIP_SendClientVIPMenu(client, false);
	}
	else if(action == MenuAction_End) 
		delete menu;
}
