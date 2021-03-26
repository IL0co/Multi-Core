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

bool CallBack_OnPluginUniqueDisplay(int client, const char[] plugin_unique, StringMap map = null, MC_CoreTypeBits core_type, char[] buffer, int maxlen)
{
    if(map == null)
        Get_PluginMap(plugin_unique, _, map);

    Handle plugin;
    Function func = Get_PluginUniqueCallBackFunc(map, PLUGIN_DATAPACKPOS_DISPLAY);

    map.GetValue("plugin", plugin);

    bool result;

    if(Check_IsValidFunctionCallBack(func, plugin))
    {
        Call_StartFunction(plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(Convert_PluginId(plugin_unique));
        Call_PushCell(core_type);
        Call_PushStringEx(buffer, maxlen, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
        Call_PushCell(maxlen);
        Call_Finish(result);
    }

    if(!result) 
        FormatEx(buffer, maxlen, plugin_unique);

    return result;
}

bool CallBack_OnItemDisplay(int client, const char[] plugin_unique, char[] item_unique, StringMap map = null, MC_CoreTypeBits core_type, char[] buffer, int maxlen)
{
    if(map == null)
        Get_ItemMap(plugin_unique, _, item_unique, map);

    Function func = Get_ItemCallBackFunc(map, ITEM_DATAPACKPOS_DISPLAY);
    Handle plugin;
    bool result;

    map.GetValue("plugin", plugin);

    if(Check_IsValidFunctionCallBack(func, plugin))
    {
        Call_StartFunction(plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(Convert_PluginId(plugin_unique));
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

void CallBack_OnItemPreview(int client, const char[] plugin_unique, const char[] item_unique, StringMap map = null, MC_CoreTypeBits core_type)
{
    if(map == null)
        Get_ItemMap(plugin_unique, _, item_unique, map);

    Function func = Get_ItemCallBackFunc(map, ITEM_DATAPACKPOS_PREVIEW);
    Handle plugin;

    map.GetValue("plugin", plugin);

    if(Check_IsValidFunctionCallBack(func, plugin))
    {
        Call_StartFunction(plugin, func);
        Call_PushCell(client);
        Call_PushString(plugin_unique);
        Call_PushCell(Convert_PluginId(plugin_unique));
        Call_PushString(item_unique);
        Call_PushCell(core_type);
        Call_Finish();
    }
}

Function Get_PluginUniqueCallBackFunc(StringMap plugin_map, DataPackPos pos)
{
    DataPack data;

    plugin_map.GetValue("callbacks", data);

    data.Position = pos;
    return data.ReadFunction();
}

Function Get_ItemCallBackFunc(StringMap item_map, DataPackPos pos)
{
    DataPack data;

    item_map.GetValue("callbacks", data);

    data.Position = pos;
    return data.ReadFunction();
}
