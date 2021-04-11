#include <sourcemod>

bool g_bPreviewMode[MAXPLAYERS+1];
char g_cLastSeePluginUnique[MAXPLAYERS+1][MAX_UNIQUE_LENGTH];

public void VIP_OnVIPLoaded()
{
	char plugin_unique[MAX_UNIQUE_LENGTH];

	for(int index; index < g_arPluginUniques.Length; index++)
	{
		g_arPluginUniques.GetString(index, plugin_unique, sizeof(plugin_unique));
		Load_Vip(plugin_unique);
	}
}

void Load_Vip(char[] plugin_unique)
{
	if(!Check_IsCoreLoaded(Core_VIP))
		return;
		
	if(!Check_IsAllowRegisterInCore(plugin_unique, Core_VIP) || VIP_IsValidFeature(plugin_unique))
		return;

	VIP_RegisterFeature(plugin_unique, STRING, SELECTABLE, CallBack_VIP_OnItemSelected, CallBack_VIP_OnItemDisplayed, .bCookie = false);
}

public bool CallBack_VIP_OnItemDisplayed(int client, const char[] plugin_unique, char[] display, int maxlength)
{
	bool result = CallBack_OnPluginUniqueDisplay(client, plugin_unique, _, Core_VIP, display, maxlength);

	if(Check_Vip_IsHaveToggleState(client, plugin_unique))
	{
		Format(display, maxlength, "%s%s", display, Get_ClientItem(client, plugin_unique, "vip") ? " [Включено]" : " [Выключено]");
	}

	return result;
}

bool Check_Vip_IsHaveToggleState(int client, const char[] plugin_unique)
{
	StringMap plugin_map;

	if(!Get_PluginMap(plugin_unique, _, plugin_map))
		return false;

	if(Check_IsPluginUniqueHavePreview(plugin_map))
		return false;

	char buff[MAX_UNIQUE_LENGTH];

	VIP_GetClientFeatureString(client, plugin_unique, buff, sizeof(buff));

	if(strcmp(buff, "all", false) != 0 || FindCharInString(buff, '|') == -1)
		return false;

	StringMap item_map;
	if(!Get_ItemMap(plugin_unique, _, buff, item_map))
		return false;

	if(Check_IsItemHavePreview(item_map))
		return false;
	
	return true;
}

public bool CallBack_VIP_OnItemSelected(int client, const char[] plugin_unique)
{
	if(Check_Vip_IsHaveToggleState(client, plugin_unique))
	{
		if(Get_ClientItem(client, plugin_unique, "vip"))
		{
			Set_ClientItem(client, plugin_unique, "vip", "");
		}
		else
		{
			char buff[MAX_UNIQUE_LENGTH];
			VIP_GetClientFeatureString(client, plugin_unique, buff, sizeof(buff));

			Set_ClientItem(client, plugin_unique, "vip", buff);
		}

		return true;
	}

	g_bPreviewMode[client] = false;

	Menu_Vip_SelectItem(client, plugin_unique).Display(client, 0);
	Format(g_cLastSeePluginUnique[client], sizeof(g_cLastSeePluginUnique[]), plugin_unique);

	return false;
}

public Menu Menu_Vip_SelectItem(int client, const char[] plugin_unique)
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
	StringMap plugin_map, item_map;
	bool selected;

	Get_PluginMap(plugin_unique, _, plugin_map);
	Get_ClientItem(client, plugin_unique, "vip", selected_item, sizeof(selected_item));

	Menu menu = new Menu(MenuHandler_Vip_SelectItem);
	menu.ExitBackButton = true;

	CallBack_OnPluginUniqueDisplay(client, plugin_unique, plugin_map, Core_VIP, translate, sizeof(translate));
	Format(translate, sizeof(translate), "%s\n ", translate);
	menu.SetTitle(translate);

	if(Check_IsPluginUniqueHavePreview(plugin_map))
	{
		Format(translate, sizeof(translate), "Превью режим %s", g_bPreviewMode[client] ? "[Включено]" : "[Выключено]");
		menu.AddItem("p", translate);
	}

	Format(translate, sizeof(translate), "Выключить %s\n ", selected_item[0] ? "" : "[Выбрано]");
	menu.AddItem("o", translate, (g_bPreviewMode[client] || !selected_item[0]) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);

	VIP_GetClientFeatureString(client, plugin_unique, buff, sizeof(buff));

	if(strcmp(buff, "all", false) == 0)
	{
		ArrayList ar;
		plugin_map.GetValue("items_array", ar);

		char item[MAX_UNIQUE_LENGTH];

		for(int num; num < ar.Length; num++)
		{
			ar.GetString(num, item, sizeof(item));
			plugin_map.GetValue(item, item_map);

			if(g_bPreviewMode[client] && !Check_IsItemHavePreview(item_map))
				continue;

			selected = (strcmp(selected_item, item) == 0);
			Fill_MenuByItems(menu, client, selected, plugin_unique, item, item_map);
		}
	}
	else
	{
		char exp[64][MAX_UNIQUE_LENGTH];
		int count = ExplodeString(buff, ";", exp, sizeof(exp), sizeof(exp[]));

		for(int num; num < count; num++)
		{
			if(!Get_ItemMap(plugin_unique, _, exp[num], item_map))
				continue;
			
			if(g_bPreviewMode[client] && !Check_IsItemHavePreview(item_map))
				continue;

			selected = (strcmp(selected_item, exp[num]) == 0);
			Fill_MenuByItems(menu, client, selected, plugin_unique, exp[num], item_map);
		}
	}

	return menu;
}

void Fill_MenuByItems(Menu menu, int client, bool selected, const char[] plugin_unique, char[] item, StringMap map)
{
	char buff[128];

	CallBack_OnItemDisplay(client, plugin_unique, item, map, Core_VIP, buff, sizeof(buff));

	if(!g_bPreviewMode[client] && selected)
	{
		Format(buff, sizeof(buff), "%s [Выбрано]", buff);
	}

	menu.AddItem(item, buff, (!g_bPreviewMode[client] && selected) ? ITEMDRAW_DISABLED : ITEMDRAW_DEFAULT);
}

public int MenuHandler_Vip_SelectItem(Menu menu, MenuAction action, int client, int item)
{
	if(action == MenuAction_Select)
	{
		char buff[MAX_UNIQUE_LENGTH];
		menu.GetItem(item, buff, sizeof(buff));

		if(item == 0 && buff[0] == 'p')
		{
			g_bPreviewMode[client] = !g_bPreviewMode[client];
		}
		else if(buff[0] == 'o' && (item == 0 || item == 1))
		{
			Set_ClientItem(client, g_cLastSeePluginUnique[client], "vip", "");
		}
		else if(g_bPreviewMode[client])
		{
			CallBack_OnItemPreview(client, g_cLastSeePluginUnique[client], buff, _, Core_VIP);
		}
		else
		{
			Set_ClientItem(client, g_cLastSeePluginUnique[client], "vip", buff);
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
