#include <sourcemod>

void Error_Native_NoPluginUniqueBeingRegistered()
{
	ThrowNativeError(SP_ERROR_PARAM, "No Plugin_Unique is being registered!");
}

void Error_Native_PluginUniqueIsAlreadyRegistered(char[] plugin_unique)
{
	ThrowNativeError(SP_ERROR_PARAM, "Plugin_Unique '%s' is already registered!", plugin_unique);
}

// void Error_Native_ItemIsBeginRegistered()
// {
// 	ThrowNativeError(SP_ERROR_PARAM, "Item is being registered!");
// }

void Error_Native_ItemIsAlreadyRegistered(char[] item, int plugin_id)
{
	char buff[MAX_UNIQUE_LENGTH];
	Convert_PluginId(_, plugin_id, buff, sizeof(buff));

	ThrowNativeError(SP_ERROR_PARAM, "Item '%s' is already registered on this Plugin_Unique '%s'!", item, buff);
}

void Error_Native_InvalidPluginId(int plugin_id)
{
	ThrowNativeError(SP_ERROR_PARAM, "Invalid PluginId %i", plugin_id);
}

// void Error_Native_InvalidPluginUnique(char[] unique)
// {
// 	ThrowNativeError(SP_ERROR_PARAM, "Invalid Plugin '%s'", unique);
// }

void Error_Native_InvalidItemUnique(char[] plugin_unique, char[] item)
{
	ThrowNativeError(SP_ERROR_PARAM, "Invalid Item '%s' from Plugin_Unique '%s'", item, plugin_unique);
}

void Error_Native_FinishRegisterThePreviousItem(char[] unique)
{
	ThrowNativeError(SP_ERROR_PARAM, "Finish registering the previous item '%s'!", unique);
}

void Error_Native_NoItemToRegistered()
{
	ThrowNativeError(SP_ERROR_PARAM, "There is no item that is being registered");
}

// void Error_Native_FileIsNotExist(char[] path)
// {
// 	ThrowNativeError(SP_ERROR_PARAM, "File is not exist '%s'!", path);
// }

void Error_FailState_FileIsNotExist(char[] path)
{
	SetFailState("File is not exist '%s'!", path);
}