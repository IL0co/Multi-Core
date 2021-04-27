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
	version	 	= "0.2.0",
	url			= "http://hlmod.ru"
};


/*	TODO:
	- перейти с Cookie системы бд на SQL
	- сделать рефакторинг кода
	- добавить меню sm_mc
	- если доступен только один предмет на выбор, то в VIP делать его как togglable
	- поддерка персонального
	- поддержка контроллера
	- поддержка LR и FPS
	- поддержка LK и LK2
	- добавить форвард на регистрацию предмета в каком-то ядре (в процессе регистрации, что бы можно было дополнить своим)

	FIXME:
	- добавить файл перевода
	- добавить описание предмета и "категории"
	- Добавить CB на нажатие в каком-то меню этого предмета.
*/

#include "multi_core/core/globals.inc"
#include "multi_core/core/stuff_errors.inc"
#include "multi_core/core/player_manager.inc"
#include "multi_core/core/natives.inc"
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

void Check_IsLoadLibraryName(const char[] name, bool isLoad)
{
	for(int id; id < sizeof(g_LoadCoreType); id++)		
	{
		if(strcmp(name, g_LoadCoreType[id], false) != 0)
			continue;

		if(isLoad)
			g_IsCoreLoadBits |= g_LoadCoreBits[id];
		else
			g_IsCoreLoadBits &= ~g_LoadCoreBits[id];

		CallForward_OnCoreChangeStatus(name, g_LoadCoreBits[id], isLoad);

		break;
	}
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
	ArrayList ar;
	char exp[32][MAX_UNIQUE_LENGTH], plugin_unique[MAX_UNIQUE_LENGTH], buff[256];
	
	BuildPath(Path_SM, buff, sizeof(buff), "configs/multi-core/settings_to_all.cfg");	//FIXME: добавить варны что идентификатора повторяются
	g_kvItemsToAll = new KeyValues("MC Give To All");
	if(!g_kvItemsToAll.ImportFromFile(buff))
		Error(FILE_NOT_EXIST, _, buff);
	
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
		Error(FILE_NOT_EXIST, _, buff);
	
	HookEvent("player_disconnect", Event_PlayerDisconnect, EventHookMode_Pre);

	for(int i = 1; i <= MaxClients; i++)		if(IsValidPlayer(i))
		OnClientPostAdminCheck(i);

	BuildPath(Path_SM, buff, sizeof(buff), "configs/multi-core/settings_priorities.cfg");
	KeyValues kv = new KeyValues("MC Priorities");
	if(!kv.ImportFromFile(buff))
		Error(FILE_NOT_EXIST, _, buff);

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

	if(!(g_IsCoreLoadBits & Core_MultiCore))
		CreateTimer(0.1, Timer_Delay_StartCore);

	g_IsCoreLoadBits = Core_MultiCore;

	for(int id; id < sizeof(g_LoadCoreType); id++)		if(LibraryExists(g_LoadCoreType[id]))
		g_IsCoreLoadBits |= g_LoadCoreBits[id];

	RegAdminCmd("sm_mc_dump", Command_Dump, ADMFLAG_ROOT);
}

public Action Command_Dump(int client, int args)
{
	char path[] = "addons/mc_dump.txt";
	char plugin_unique[MAX_UNIQUE_LENGTH], item_unique[MAX_UNIQUE_LENGTH];
	KeyValues kv = new KeyValues("Multi-Core Dump");

	MC_PluginMap mc_plugin;
	MC_ItemMap mc_item;
	// StringMap map;
	ArrayList ar;
	StringMapSnapshot snap = g_mapPlugins.Snapshot();

	if(snap)
	{
		for(int id; id < snap.Length; id++)
		{
			snap.GetKey(id, plugin_unique, sizeof(plugin_unique));

			if(!g_mapPlugins.GetValue(plugin_unique, mc_plugin))
				continue;

			kv.JumpToKey(plugin_unique, true);
			
			ar = mc_plugin.GetItemsArray();
			kv.SetNum("MC_CategoryId", g_arPlugins.FindString(plugin_unique));
			kv.SetNum("Items Array", view_as<int>(ar));
			kv.SetNum("Items Map", view_as<int>(mc_plugin.GetItemsMap()));
			kv.SetNum("CallBacks", view_as<int>(mc_plugin.GetCallBacksPack()));
			kv.SetNum("Cookie", view_as<int>(mc_plugin.Cookie));

			for(int num; num < ar.Length; num++)
			{
				ar.GetString(num, item_unique, sizeof(item_unique));

				if(!item_unique[0] || (mc_item = mc_plugin.GetItemMap(item_unique)) == null)
					continue;

				kv.JumpToKey(item_unique, true);
				kv.SetNum("CallBacks", view_as<int>(mc_item.GetCallBacksPack()));
				kv.GoBack();
			}

			kv.GoBack();
		}
	}

	kv.Rewind();
	kv.ExportToFile(path);
	PrintToServer("[MC Core] Dumped in: %s", path);
	return Plugin_Handled;
}

public Action Timer_Delay_StartCore(Handle timer)
{
	CallForward_OnCoreChangeStatus("multicore", Core_MultiCore, true);
}

bool Check_IsCoreLoaded(MC_CoreTypeBits type)
{
	if(g_IsCoreLoadBits & type)
		return true;

	return false;
}
