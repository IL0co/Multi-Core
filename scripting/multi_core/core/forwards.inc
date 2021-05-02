#include <sourcemod>

GlobalForward forward_OnCoreLoaded,
              forward_OnPluginRegistered,
              forward_OnPluginUnRegistered_Pre,
              forward_OnPluginUnRegistered;

public void LoadForwards()
{
	forward_OnCoreLoaded = new GlobalForward("MC_OnCoreLoaded", ET_Ignore);
	forward_OnPluginRegistered = new GlobalForward("MC_OnPluginRegistered", ET_Ignore, Param_String, Param_Cell);
	forward_OnPluginUnRegistered_Pre = new GlobalForward("MC_OnPluginUnRegistered_Pre", ET_Ignore, Param_String, Param_Cell);
	forward_OnPluginUnRegistered = new GlobalForward("MC_OnPluginUnRegistered", ET_Ignore, Param_String, Param_Cell);
}

void CallForward_OnCoreLoaded()
{
	Call_StartForward(forward_OnCoreLoaded);
	Call_Finish();
}

void CallForward_OnPluginRegistered(char[] plugin_id)
{
	Call_StartForward(forward_OnPluginRegistered);
    Call_PushString(plugin_id);
	Call_Finish();
}

void CallForward_OnPluginUnRegistered_Pre(char[] plugin_id)
{
	Call_StartForward(forward_OnPluginUnRegistered_Pre);
    Call_PushString(plugin_id);
	Call_Finish();
}

void CallForward_OnPluginUnRegistered(char[] plugin_id)
{
	Call_StartForward(forward_OnPluginUnRegistered);
    Call_PushString(plugin_id);
	Call_Finish();
}

bool CallBack_OnPluginUniqueDisplay(int client, const char[] plugin_id, char[] core_type, char[] buffer, int maxlen)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_id, map))
		return false;

    Function func = map.GetCallBack(PLUGIN_DATAPACKPOS_DISPLAY);
    bool result;

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_id);
        Call_PushString(core_type);
        Call_PushStringEx(buffer, maxlen, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
        Call_PushCell(maxlen);
        Call_Finish(result);
    }

    if(!result) 
        FormatEx(buffer, maxlen, plugin_id);

    return result;
}

bool CallBack_OnPluginSelected(int client, MC_PluginMap map, const char[] plugin_id, char[] core_type)
{
    Function func = map.GetCallBack(PLUGIN_DATAPACKPOS_SELECTED);
    bool result = true;

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_id);
        Call_PushString(core_type);
        Call_Finish(result);
    }

    return result;
}

bool CallBack_OnItemDisplay(int client, const char[] plugin_id, const char[] item_unique, char[] core_type, char[] buffer, int maxlen)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_id, map))
		return false;

    Function func = map.GetItemMap(item_unique).GetCallBack(ITEM_DATAPACKPOS_DISPLAY);
    bool result;

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_id);
        Call_PushString(item_unique);
        Call_PushString(core_type);
        Call_PushStringEx(buffer, maxlen, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
        Call_PushCell(maxlen);
        Call_Finish(result);
    }

    if(!result) 
        FormatEx(buffer, maxlen, item_unique);

    return result;
}

void CallBack_OnItemPreview(int client, const char[] plugin_id, const char[] item_unique, char[] core_type)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_id, map))
		return;

    Function func = map.GetItemMap(item_unique).GetCallBack(ITEM_DATAPACKPOS_PREVIEW);

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_id);
        Call_PushString(item_unique);
        Call_PushString(core_type);
        Call_Finish();
    }
}

bool CallBack_OnItemSelected(int client, MC_PluginMap plugin_map, MC_ItemMap item_map, const char[] plugin_id, const char[] item_unique, char[] core_type)
{
    Function func = item_map.GetCallBack(ITEM_DATAPACKPOS_SELECTED);
    bool result = true;

    if(Check_IsValidFunctionCallBack(func, plugin_map.Plugin))
    {
        Call_StartFunction(plugin_map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_id);
        Call_PushString(item_unique);
        Call_PushString(core_type);
        Call_Finish(result);
    }

    return result;
}

bool Check_IsValidFunctionCallBack(Function func, Handle plugin)
{
    if(!Check_IsValidFunction(func) || !Check_IsValidPlugin(plugin))
        return false;
    
    return true;
}

bool Check_IsValidFunction(Function func)
{
    if(func == INVALID_FUNCTION)
        return false;
    
    return true;
}

bool Check_IsValidPlugin(Handle plugin)
{
    if(!plugin)
        return false;
        
	Handle hIterator = GetPluginIterator();
	bool result = false;
	
	while(MorePlugins(hIterator))
	{
		if(plugin == ReadPlugin(hIterator))
		{
			result = (GetPluginStatus(plugin) == Plugin_Running);
			break;
		}
	}
	
	delete hIterator;
	return result;
}
