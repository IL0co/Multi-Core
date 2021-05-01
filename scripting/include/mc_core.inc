#if defined _mc_core_included
 #endinput
#endif
#define _mc_core_included

#define MAX_UNIQUE_LENGTH   64      // Maximum length of identifier string

enum MC_CoreTypeBits    // Core types
{                           //                                  | Core (library) identifier:
    Core_Unloaded = 0,      // None of the cores are loaded     |
    Core_MultiCore = 1,     // Core Multi-Core                  | multicore
    Core_Shop = 2,          // Shop Core                        | shop
    Core_VIP = 4,           // VIP Core                         | vip_core
}

enum MC_DataType        // Data types
{   
    Type_Model = 0,         // Model (.mdl)
    Type_Sound,             // Sound file (.mp3 .wav). No folder 'sound/'
    Type_Particle,          // Particle (name)
    Type_ParticleFile,      // Particle file (.pcf)
    Type_Sprite,            // Texture (.vmt)
}

enum MC_PluginIndex
{
    INVALID_PLUGIN_INDEX = -1,
}

enum MC_ValueOfCore
{
    All = 0,                // Public
    VIP = 1,                // From VIP (by R1KO)
    Shop = 2,               // From the shop (by FrozDark)
}

typeset MC_OnMenuDisplayCallbacks
{
    /* 
     * Callback to display plugin name in menu
     *  -
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param plugin_index     Plugin index
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - to display the new name or not change it, false - to hide
    */
	function bool (int client, const char[] plugin_id, MC_PluginIndex plugin_index, MC_CoreTypeBits core_type, char[] buffer, int maxlen);

    /*
     * Callback to display item name in menu
     *  -
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param plugin_index     Plugin index
     * @param item_unique      Item Id
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - to display the new name or not change it, false - to hide
    */
	function bool (int client, const char[] plugin_id, MC_PluginIndex plugin_index, const char[] item_unique, MC_CoreTypeBits core_type, char[] buffer, int maxlen);
}

typeset MC_OnMenuSelectedCallbacks
{
    /*
     * Callback when selecting a plugin from the menu
     *  -
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param plugin_index     Plugin index
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - continue the action, false - stop the action
    */
	function bool (int client, const char[] plugin_id, MC_PluginIndex plugin_index, MC_CoreTypeBits core_type);

    /*
     * Callback when selecting a item from the menu
     *	-
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param plugin_index     Plugin index
     * @param core_type        Core type
     * @param item_unique      Item Id (can be empty if turned off)
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *	-
     * @return true - continue the action, false - stop the action
    */
	function bool (int client, const char[] plugin_id, MC_PluginIndex plugin_index, const char[] item_unique, MC_CoreTypeBits core_type);
}

/*
 * Callback for menu item preview
 *  -
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param plugin_index      Plugin index
 * @param item_unique       Item Id
 * @param core_type         Core type
 *	-
 * @noreturn
*/
typedef MC_OnItemPreviewCallback = function void (int client, const char[] plugin_id, MC_PluginIndex plugin_index, const char[] item_unique, MC_CoreTypeBits core_type);

/*
 * Precash file
 *	-
 * @param file              The path to the file. Particle name, etc.
 * @param data_type         File type
 *	-
 * @return file index or -1			
*/
native int MC_PrecacheFile(char[] file, MC_DataType data_type);

/*
 * Called after the plugin id has been registered
 *	-
 * @param plugin_id         Plugin Id
 * @param plugin_index      Plugin index
 *	-
 * @noreturn			
*/
forward void MC_OnPluginRegistered(const char[] plugin_id, MC_PluginIndex plugin_index);

/*
 * Called before the plugin id was unregistered
 *	-
 * @param plugin_id         Plugin Id
 * @param plugin_index      Plugin index
 *	-
 * @noreturn			
*/
forward void MC_OnPluginUnRegistered_Pre(const char[] plugin_id, MC_PluginIndex plugin_index);

/*
 * Called after the core is loaded
 *	-
 * @noparams
 *	-
 * @noreturn			
*/
forward void MC_OnCoreLoaded();

/*
 * Core load check
 *	-
 * @param core_type         Core type
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_IsCoreLoaded(MC_CoreTypeBits core_type = Core_MultiCore);

/*
 * Get core config
 *	-
 * @param type              Core type
 *	-
 * @return KeyValues handle
*/
native KeyValues MC_GetSettingsConfigKV(MC_ValueOfCore type);
 
/*
 * Register Plugin Id
 *	-
 * @param plugin_id         Plugin Id
 * @param dont_load         Bits indexes of cores, which do not register this Plugin Id
 *	-
 * @noreturn   
*/
native void MC_RegisterPlugin(char[] plugin_id, MC_CoreTypeBits dont_load = Core_Unloaded);
 
/*
 * Add plugin callbacks
 *	-
 * @param OnDisplay         Menu display callback
 * @param OnSelect          Menu Select Callback
 *	-
 * @noreturn 
*/
native void MC_SetPluginCallBacks(MC_OnMenuDisplayCallbacks OnDisplay = INVALID_FUNCTION,
                                  MC_OnMenuSelectedCallbacks OnSelect = INVALID_FUNCTION);

/*
 * Start item registration
 *	-
 * @param item_unique       Item Id (unique)
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_StartItem(char[] item_unique);

/*
 * Add item callbacks
 *	-
 * @param OnDisplay         Menu display callback
 * @param OnPreview         Item preview callback
 * @param OnSelect          Menu Select Callback
 *	-
 * @noreturn 
*/
native void MC_SetItemCallBacks(MC_OnMenuDisplayCallbacks OnDisplay = INVALID_FUNCTION,
                                MC_OnItemPreviewCallback OnPreview = INVALID_FUNCTION,
                                MC_OnMenuSelectedCallbacks OnSelect = INVALID_FUNCTION);

/*
 * Finish item registration
 *	-
 * @noparams
 *	-
 * @noreturn   
*/
native void MC_EndItem();
 
/*
 * Finish plugin registration
 *	-
 * @noparams
 *	-
 * @return Plugin index 
*/
native MC_PluginIndex MC_EndPlugin();
 
/*
 * Unload plugin data (completely). Must be called in OnPluginEnd()
 *	-
 * @noparams
 *	-
 * @noreturn     
*/
native void MC_UnRegisterMe();
 
/*
 * Get an array of plugin ids. The handle must be closed!
 *	-
 * @noparams
 *	-
 * @return ArrayList Handle      
*/
native ArrayList MC_GetPluginIdsArrayList();

/*
 * Check the validity of the plugin by its index
 *	-
 * @param plugin_index         Plugin index
 *	-
 * @return true if all is well, false otherwise      
*/
native bool MC_IsValidPluginIndex(MC_PluginIndex plugin_index);

/*
 * Check the validity of the plugin by its identifier
 *	-
 * @param plugin_id     Plugin Id
 *	-
 * @return true if all is well, false otherwise      
*/
native bool MC_IsValidPluginUnique(char[] plugin_id);
 
/*
 * Get the number of plugins I have registered
 *	-
 * @noparams
 *	-
 * @return number of registered plugin ids      
*/
native int MC_GetPluginsCountRegisteredByMe();
 
/*
 * Getting an active item from a player
 *	-
 * @param client            Player Index
 * @param plugin_index      Plugin index
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetClientActiveItem(int client, MC_PluginIndex plugin_index, char[] buffer, int maxlen);
 
/*
 * Getting an selected item from the player
 *	-
 * @param client            Player Index
 * @param plugin_index      Plugin index
 * @param type              Core type
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetClientSelectedItem(int client, MC_PluginIndex plugin_index, MC_ValueOfCore type, char[] buffer, int maxlen);
 
/*
 * Set the selected item to the player
 *	-
 * @param client            Player Index
 * @param plugin_index      Plugin index
 * @param type              Core type
 * @param item_unique       Item unique name (becomes active)
 *	-
 * @noreturn
*/
native void MC_SetClientSelectedItem(int client, MC_PluginIndex plugin_index, MC_ValueOfCore type, char[] item_unique);
 
/*
 * Get module config from folder 'addons/sourcemod/configs/multi-core/modules/'
 *	-
 * @param file_name         File name
 * @param key_name          Structure name (first key)
 *	-
 * @return KeyValues handle
*/
stock KeyValues MC_GetModuleConfigKV(char[] file_name, char[] key_name)
{
	char path[256];
	KeyValues kv = new KeyValues(key_name);

	BuildPath(Path_SM, path, sizeof(path), "configs/multi-core/modules/%s", file_name);

	if(!kv.ImportFromFile(path))
		SetFailState("File is not exist '%s'!", path)

	return kv;
}

public SharedPlugin __pl_mc_core = 
{
	name = "multicore",
	file = "mc_core.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_mc_core_SetNTVOptional()
{
	MarkNativeAsOptional("MC_PrecacheFile");
	MarkNativeAsOptional("MC_IsCoreLoaded");
	MarkNativeAsOptional("MC_GetSettingsConfigKV");
	MarkNativeAsOptional("MC_RegisterPlugin");
	MarkNativeAsOptional("MC_SetPluginCallBacks");
	MarkNativeAsOptional("MC_StartItem");
	MarkNativeAsOptional("MC_SetItemCallBacks");
	MarkNativeAsOptional("MC_EndItem");
	MarkNativeAsOptional("MC_EndPlugin");
	MarkNativeAsOptional("MC_UnRegisterMe");
	MarkNativeAsOptional("Native_MC_GetPluginIdsArrayList");
	MarkNativeAsOptional("MC_IsValidPluginIndex");
	MarkNativeAsOptional("MC_IsValidPluginUnique");
	MarkNativeAsOptional("MC_GetClientActiveItem");
	MarkNativeAsOptional("MC_GetClientSelectedItem");
	MarkNativeAsOptional("MC_SetClientSelectedItem");
	MarkNativeAsOptional("MC_GetPluginsCountRegisteredByMe");
}
#endif
