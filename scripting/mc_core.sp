#include <sdktools>
#include <sdkhooks>
#include <sourcemod>
#include <mc_core>
#include <clientprefs>

#pragma newdecls required
#pragma semicolon 1

public Plugin myinfo =
{
	name		= "[Multi-Core] Core",
	author	  	= "iLoco",
	description = "Ядро, контролирующее регистрацию предметов в других ядрах",
	version	 	= "0.2.0",
	url			= "http://hlmod.ru"
};

#include "multi_core/core/globals.inc"
#include "multi_core/core/errors.inc"
#include "multi_core/core/player_manager.inc"
#include "multi_core/core/natives.inc"
#include "multi_core/core/forwards.inc"

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{	
	LoadForwards();
	LoadNatives();

	RegPluginLibrary("multicore");
	return APLRes_Success;
}

public void OnPluginStart()
{
	ArrayList ar;
	char exp[32][MAX_UNIQUE_LENGTH], plugin_id[MAX_UNIQUE_LENGTH], buff[256];
	
	BuildPath(Path_SM, buff, sizeof(buff), "configs/multi-core/settings_to_all.cfg");	//FIXME: добавить варны что идентификатора повторяются
	g_kvItemsToAll = new KeyValues("MC Give To All");
	if(!g_kvItemsToAll.ImportFromFile(buff))
		Error(FILE_NOT_EXIST, _, buff);
	
	if(g_kvItemsToAll.GotoFirstSubKey(false))
	{
		do
		{
			g_kvItemsToAll.GetSectionName(plugin_id, sizeof(plugin_id));
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
			kv.GetSectionName(plugin_id, sizeof(plugin_id));

			if(g_mapPriorities.GetValue(plugin_id, ar))		// TODO: добавить варн, что предмет повторяется
				continue;

			kv.GetString(NULL_STRING, buff, sizeof(buff));

			if(!buff[0])
				continue;

			ar = new ArrayList();
			g_mapPriorities.SetValue(plugin_id, ar);

			int count = ExplodeString(buff, " ", exp, sizeof(exp), sizeof(exp[]));
			for(int num; num < count; num++)		if(exp[num][0])
				ar.PushString(exp[num]);
		}
		while(kv.GotoNextKey(false));
	}

	delete kv;

	RegAdminCmd("sm_mc_dump", Command_Dump, ADMFLAG_ROOT);
	LoadTranslations("mc_core.phrases");

	g_bIsCoreLoaded = true;
}

public Action Command_Dump(int client, int args)
{
	char path[] = "addons/mc_dump.txt";
	char plugin_id[MAX_UNIQUE_LENGTH], item_unique[MAX_UNIQUE_LENGTH], buff[128];
	KeyValues kv = new KeyValues("Multi-Core Dump");

	MC_PluginMap mc_plugin;
	MC_ItemMap mc_item;
	// StringMap map;
	ArrayList ar;

	for(int index; index < g_arPlugins.Length; index++)
	{
		g_arPlugins.GetString(index, plugin_id, sizeof(plugin_id));

		if(!g_mapPlugins.GetValue(plugin_id, mc_plugin))
			continue;

		kv.JumpToKey(plugin_id, true);
		
		ar = mc_plugin.GetItemsArray();

		GetPluginInfo(mc_plugin.Plugin, PlInfo_Name, buff, sizeof(buff));
		kv.SetString("Registered Plugin Name", buff);
		kv.SetString("Plugin Unique", plugin_id);

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

	kv.Rewind();
	kv.ExportToFile(path);
	PrintToServer("[MC Core] Dumped in: %s", path);
	return Plugin_Handled;
}

public Action Timer_Delay_StartCore(Handle timer)
{
	CallForward_OnCoreLoaded();
}
