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

		ThrowNativeError(4, "Сначала закончите регистрацию предыдущего предмета! '%s'", item_unique);
	}
	else if(error == COMPLETE_REGISTRATION_PREVIOUSLY_PLUGIN)
	{
		char plugin_unique[MAX_UNIQUE_LENGTH];
		view_as<MC_PluginMap>(data).GetUnique(plugin_unique, sizeof(plugin_unique));

		ThrowNativeError(4, "Сначала закончите регистрацию предыдущего плагина! '%s'", unique);
	}
	else if(error == START_THE_REGISTERING_PLUGIN)
	{
		ThrowNativeError(4, "Сначала начните регистрацию плагина!");
	}
	else if(error == START_THE_REGISTERING_ITEM)
	{
		ThrowNativeError(4, "Сначала начните регистрацию предмета!");
	}
	else if(error == PLUGIN_IS_ALREADY_REGISTERED)
	{
		ThrowNativeError(4, "Плагин уже зарегистрирован! %s", unique);
	}
	else if(error == INVALID_CLIENT)
	{
		if(!IsClientInGame(data))
			ThrowNativeError(4, "Игрок вышел! %s", data);
		else if(IsFakeClient(data))
			ThrowNativeError(4, "Этот игрок - бот! %L", data);
		else 
			ThrowNativeError(4, "Невалидный игрок! %L", data);

	}
	else if(error == ERROR_INVALID_PLUGIN_ID)
	{
		ThrowNativeError(4, "Invalid PluginId %i", data);
	}
	else if(error == FILE_NOT_EXIST)
	{
		SetFailState("File is not exist '%s'!", unique);
	}
}