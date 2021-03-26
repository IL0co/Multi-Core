#include <sdktools>
#include <sdkhooks>
#include <sourcemod>
#include <mc_core>
#include <clientprefs>

#undef REQUIRE_PLUGIN
#tryinclude <shop>
#tryinclude <vip_core>

#pragma newdecls required
#pragma semicolon 1

public Plugin myinfo =
{
	name		= "[MC] Multi-Core",
	author	  	= "iLoco",
	description = "Ядро, контролирующее регистрацию предметов в других ядрах",
	version	 	= "0.1.2",
	url			= "http://hlmod.ru"
};

/*	TODO:
	- перейти с Cookie системы бд на SQL
	- сделать рефакторинг кода
	- добавить меню sm_mc
	- если доступен только один предмет на выбор, то в VIP делать его как togglable
	- поддерка персонального
	- поддержка контроллера
	- поддержка LR
	- добавить форвард на регистрацию предмета в каком-то ядре (в процессе регистрации, что бы можно было дополнить своим)

	FIXME:
		- убрать с требований при регистрации предмета аргумент с plugin_id, она автоматически будет
*/

#include "multi_core/core/globals.inc"
#include "multi_core/core/natives.inc"
#include "multi_core/core/stuff_any.inc"
#include "multi_core/core/stuff_errors.inc"
#include "multi_core/core/stuff_checks.inc"
#include "multi_core/core/forwards.inc"
#include "multi_core/core/vip_core.inc"
#include "multi_core/core/shop_core.inc"

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{	
	__pl_shop_SetNTVOptional();
	__pl_vip_core_SetNTVOptional();

	MarkNativeAsOptional("VIP_UnregisterMe");
	MarkNativeAsOptional("Shop_SetHide");
	// MarkNativeAsOptional("Shop_UnregisterItem");
	
	LoadForwards();
	LoadNatives();

	RegPluginLibrary("multicore");
	return APLRes_Success;
}

public void OnLibraryAdded(const char[] name)
{
	Check_IsLoadLibraryName(name, true);
}

public void OnLibraryRemoved(const char[] name)
{
	Check_IsLoadLibraryName(name, false);
}

public void OnPluginEnd()
{
	if(Check_IsCoreLoaded(Core_Shop))
		Shop_UnregisterMe();

	if(Check_IsCoreLoaded(Core_VIP))
		VIP_UnregisterMe();

	MC_UnRegisterMe();
}

public void OnPluginStart()
{
	g_mapPriorities = new StringMap();

	ArrayList ar;
	char exp[32][MAX_UNIQUE_LENGTH], plugin_unique[MAX_UNIQUE_LENGTH], buff[256];
	
	BuildPath(Path_SM, buff, sizeof(buff), "configs/multi-core/settings_to_all.cfg");	//FIXME: добавить варны что идентификатора повторяются
	g_kvItemsToAll = new KeyValues("MC Give To All");
	if(!g_kvItemsToAll.ImportFromFile(buff))
		Error_FailState_FileIsNotExist(buff);
	
	if(g_kvItemsToAll.GotoFirstSubKey(false))
	{
		do
		{
			g_kvItemsToAll.GetSectionName(plugin_unique, sizeof(plugin_unique));
			g_kvItemsToAll.GetString(NULL_STRING, buff, sizeof(buff));
			
			ar = new ArrayList();

			int count = ExplodeString(buff, "|", exp, sizeof(exp), sizeof(exp[]));
			for(int num; num < count; num++)		if(exp[num][0])
				ar.PushString(exp[num]);
			
			g_kvItemsToAll.SetNum(NULL_STRING, view_as<int>(ar));
		}
		while(g_kvItemsToAll.GotoNextKey(false));

		g_kvItemsToAll.Rewind();
	}

	BuildPath(Path_SM, buff, sizeof(buff), "configs/multi-core/settings_shop.cfg");		
	g_kvItemsToShop = new KeyValues("Shop Config");
	if(!g_kvItemsToShop.ImportFromFile(buff))
		Error_FailState_FileIsNotExist(buff);
	
	HookEvent("player_disconnect", Event_PlayerDisconnect, EventHookMode_Pre);

	for(int i = 1; i <= MaxClients; i++)		if(Check_IsValidClient(i, false))
		OnClientPostAdminCheck(i);

	BuildPath(Path_SM, buff, sizeof(buff), "configs/multi-core/settings_priorities.cfg");
	KeyValues kv = new KeyValues("MC Priorities");
	if(!kv.ImportFromFile(buff))
		Error_FailState_FileIsNotExist(buff);

	if(kv.GotoFirstSubKey(false))
	{
		do
		{
			kv.GetSectionName(plugin_unique, sizeof(plugin_unique));

			if(g_mapPriorities.GetValue(plugin_unique, ar))		// TODO: добавить варн, что предмет повторяется
				continue;

			kv.GetString(NULL_STRING, buff, sizeof(buff));

			if(!buff[0])
				continue;

			ar = new ArrayList();
			g_mapPriorities.SetValue(plugin_unique, ar);

			int count = ExplodeString(buff, " ", exp, sizeof(exp), sizeof(exp[]));
			for(int num; num < count; num++)		if(exp[num][0])
				ar.PushString(exp[num]);
		}
		while(kv.GotoNextKey(false));
	}

	delete kv;

	Check_AllLibraries();
	CreateTimer(0.1, Timer_Delay);

	RegAdminCmd("sm_mc_dump", Command_Dump, ADMFLAG_ROOT);
}

public Action Command_Dump(int client, int args)
{
	KeyValues kv = new KeyValues("MC Dump");

	ArrayList ar;
	StringMap plugin_map, item_map;
	char plugin_unique[MAX_UNIQUE_LENGTH], item[MAX_UNIQUE_LENGTH];
	Handle plugin;
	DataPack data;
	Cookie cookie;

	for(int index; index < g_arPluginUniques.Length; index++)
	{
		kv.Rewind();

		g_arPluginUniques.GetString(index, plugin_unique, sizeof(plugin_unique));
		
		if(!g_mapPluginUniques.GetValue(plugin_unique, plugin_map))
			continue;

		kv.JumpToKey(plugin_unique, true);

		plugin_map.GetValue("plugin", plugin);
		kv.SetNum("plugin", view_as<int>(plugin));

		plugin_map.GetValue("items array", ar);
		kv.SetNum("items array", view_as<int>(ar));

		plugin_map.GetValue("cookie", cookie);
		kv.SetNum("cookie", view_as<int>(cookie));

		plugin_map.GetValue("callbacks", data);
		kv.SetNum("callbacks", view_as<int>(data));

		for(int item_index; item_index < ar.Length; item_index++)
		{
			ar.GetString(item_index, item, sizeof(item));

			if(!plugin_map.GetValue(item, item_map))
				continue;

			kv.JumpToKey(item, true);

			item_map.GetValue("plugin", plugin);
			kv.SetNum("plugin", view_as<int>(plugin));

			item_map.GetValue("callbacks", data);
			kv.SetNum("callbacks", view_as<int>(data));

			kv.GoBack();
		}
	}

	kv.Rewind();
	kv.ExportToFile("addons/mc_dump.ini");
	ReplyToCommand(client, "Dumped to addons/mc_dump.ini");

	delete kv;
	return Plugin_Handled;
}

public Action Timer_Delay(Handle timer)
{
	CallForward_OnCoreChangeStatus("multicore", Core_MultiCore, true);
}

public void OnClientPostAdminCheck(int client)
{
	if(g_kvActiveClientList[client])
		delete g_kvActiveClientList[client];

	g_kvActiveClientList[client] = new KeyValues("My Data");

	StringMap map;
	char plugin_unique[MAX_UNIQUE_LENGTH];

	for(int index; index < g_arPluginUniques.Length; index++)
	{
		g_arPluginUniques.GetString(index, plugin_unique, sizeof(plugin_unique));

		if(!g_mapPluginUniques.GetValue(plugin_unique, map))
			continue;

		Load_ClientCookieData(client, plugin_unique, map);
	}
}

public Action Event_PlayerDisconnect(Event event, char[] name, bool dontBroadcast)
{
	int client = GetClientOfUserId(event.GetInt("userid"));

	if(g_kvActiveClientList[client])
		delete g_kvActiveClientList[client];
}
