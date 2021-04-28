#include <sourcemod>

GlobalForward forward_OnCoreChangeStatus;

public void LoadForwards()
{
	forward_OnCoreChangeStatus = new GlobalForward("MC_OnCoreChangeStatus", ET_Ignore, Param_String, Param_Cell, Param_Cell);
}

void CallForward_OnCoreChangeStatus(const char[] core_name, MC_CoreTypeBits core_type, bool isLoaded)
{
	Call_StartForward(forward_OnCoreChangeStatus);
	Call_PushString(core_name);
	Call_PushCell(core_type);
	Call_PushCell(isLoaded);
	Call_Finish();
}

bool CallBack_OnPluginUniqueDisplay(int client, const char[] plugin_unique, MC_CoreTypeBits core_type, char[] buffer, int maxlen)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_unique, map))
		return false;

    Function func = map.GetCallBack(PLUGIN_DATAPACKPOS_DISPLAY);
    bool result;

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(GetPluginId(plugin_unique));
        Call_PushCell(core_type);
        Call_PushStringEx(buffer, maxlen, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
        Call_PushCell(maxlen);
        Call_Finish(result);
    }

    if(!result) 
        FormatEx(buffer, maxlen, plugin_unique);

    return result;
}

bool CallBack_OnPluginSelected(int client, MC_PluginMap map, const char[] plugin_unique, MC_CoreTypeBits core_type)
{
    Function func = map.GetCallBack(PLUGIN_DATAPACKPOS_SELECTED);
    bool result = true;

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(GetPluginId(plugin_unique));
        Call_PushCell(core_type);
        Call_Finish(result);
    }

    return result;
}

// bool CallBack_OnItemDisplay(int client, const char[] plugin_unique, char[] item_unique, StringMap map = null, MC_CoreTypeBits core_type, char[] buffer, int maxlen)
bool CallBack_OnItemDisplay(int client, const char[] plugin_unique, const char[] item_unique, MC_CoreTypeBits core_type, char[] buffer, int maxlen)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_unique, map))
		return false;

    Function func = map.GetItemMap(item_unique).GetCallBack(ITEM_DATAPACKPOS_DISPLAY);
    bool result;

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(GetPluginId(plugin_unique));
        Call_PushString(item_unique);
        Call_PushCell(core_type);
        Call_PushStringEx(buffer, maxlen, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
        Call_PushCell(maxlen);
        Call_Finish(result);
    }

    if(!result) 
        FormatEx(buffer, maxlen, item_unique);

    return result;
}

void CallBack_OnItemPreview(int client, const char[] plugin_unique, const char[] item_unique, MC_CoreTypeBits core_type)
{
	MC_PluginMap map;
	if(!GetPluginMap(plugin_unique, map))
		return;

    Function func = map.GetItemMap(item_unique).GetCallBack(ITEM_DATAPACKPOS_PREVIEW);

    if(Check_IsValidFunctionCallBack(func, map.Plugin))
    {
        Call_StartFunction(map.Plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(GetPluginId(plugin_unique));
        Call_PushString(item_unique);
        Call_PushCell(core_type);
        Call_Finish();
    }
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
