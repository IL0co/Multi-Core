#include <sourcemod>

enum Errors
{
	COMPLETE_REGISTRATION_PREVIOUSLY_PLUGIN,		
	COMPLETE_REGISTRATION_PREVIOUSLY_ITEM,
	START_THE_REGISTERING_PLUGIN,
	START_THE_REGISTERING_ITEM,
	PLUGIN_IS_ALREADY_REGISTERED,
	ITEM_IS_ALREADY_REGISTERED,
	INVALID_CLIENT,
	ERROR_INVALID_PLUGIN_ID,
	FILE_NOT_EXIST,
}

void Error(Errors error, any data = -1, char[] unique = "")
{
	if(error == COMPLETE_REGISTRATION_PREVIOUSLY_ITEM)
	{
		char item_unique[MAX_UNIQUE_LENGTH];
		MC_ItemMap item_map;

		if(view_as<MC_PluginMap>(data).GetLastItemMap(item_map))
			item_map.GetUnique(item_unique, sizeof(item_unique));

		ThrowNativeError(4, "%T", "ERROR COMPLETE REGISTRATION PREVIOUSLY ITEM", 0, item_unique);
	}
	else if(error == COMPLETE_REGISTRATION_PREVIOUSLY_PLUGIN)
	{
		char plugin_id[MAX_UNIQUE_LENGTH];
		view_as<MC_PluginMap>(data).GetUnique(plugin_id, sizeof(plugin_id));

		ThrowNativeError(4, "%T", "ERROR COMPLETE REGISTRATION PREVIOUSLY PLUGIN", 0, plugin_id);
	}
	else if(error == START_THE_REGISTERING_PLUGIN)
	{
		ThrowNativeError(4, "%T", "ERROR START THE REGISTERING PLUGIN", 0);
	}
	else if(error == START_THE_REGISTERING_ITEM)
	{
		ThrowNativeError(4, "%T", "ERROR START THE REGISTERING ITEM", 0);
	}
	else if(error == PLUGIN_IS_ALREADY_REGISTERED)
	{
		ThrowNativeError(4, "%T", "ERROR PLUGIN IS ALREADY REGISTERED", 0, unique);
		ThrowNativeError(4, "%T", "ERROR PLAYER LEAVE", 0, unique);
	}
	else if(error == INVALID_CLIENT)
	{
		if(!IsClientInGame(data))
			ThrowNativeError(4, "%T", "ERROR PLAYER LEAVE", 0, data);
		else if(IsFakeClient(data))
			ThrowNativeError(4, "%T", "ERROR PLAYER IS BOT", 0, data);
		else 
			ThrowNativeError(4, "%T", "ERROR INVALID PLAYER", 0, data);

	}
	else if(error == ERROR_INVALID_PLUGIN_ID)
	{
		ThrowNativeError(4, "%T", "ERROR INVALID PLUGIN INDEX", 0, data);
	}
	else if(error == FILE_NOT_EXIST)
	{
		ThrowNativeError(4, "%T", "ERROR FILE DOES NOT EXISTS", 0, unique);
	}
}