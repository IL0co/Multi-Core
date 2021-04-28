#include <sourcemod>

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

enum MC_PluginId
{
    INVALID_PLUGIN_ID = -1,
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
     * @param plugin_unique    Plugin Id
     * @param plugin_id        Plugin index
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - to display the new name or not change it, false - to hide
    */
	function bool (int client, const char[] plugin_unique, MC_PluginId plugin_id, MC_CoreTypeBits core_type, char[] buffer, int maxlen);

    /*
     * Callback to display item name in menu
     *  -
     * @param client           Player Index
     * @param plugin_unique    Plugin Id
     * @param plugin_id        Plugin index
     * @param item_unique      Item Id
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - to display the new name or not change it, false - to hide
    */
	function bool (int client, const char[] plugin_unique, MC_PluginId plugin_id, const char[] item_unique, MC_CoreTypeBits core_type, char[] buffer, int maxlen);
}

typeset MC_OnMenuSelectedCallbacks
{
    /*
     * Callback when selecting a plugin from the menu
     *  -
     * @param client           Player Index
     * @param plugin_unique    Plugin Id
     * @param plugin_id        Plugin index
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - continue the action, false - stop the action
    */
	function bool (int client, const char[] plugin_unique, MC_PluginId plugin_id, MC_CoreTypeBits core_type);

    /*
     * Callback when selecting a item from the menu
     *	-
     * @param client           Player Index
     * @param plugin_unique    Plugin Id
     * @param plugin_id        Plugin index
     * @param core_type        Core type
     * @param item_unique      Item Id (can be empty if turned off)
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *	-
     * @return true - continue the action, false - stop the action
    */
	function bool (int client, const char[] plugin_unique, MC_PluginId plugin_id, const char[] item_unique, MC_CoreTypeBits core_type);
}

/*
 * Callback for menu item preview
 *  -
 * @param client           Player Index
 * @param plugin_unique    Plugin Id
 * @param plugin_id        Plugin index
 * @param item_unique      Item Id
 * @param core_type        Core type
 *	-
 * @noreturn
*/
typedef MC_OnItemPreviewCallback = function void (int client, const char[] plugin_unique, MC_PluginId plugin_id, const char[] item_unique, MC_CoreTypeBits core_type);

/*
 * Called after the kernel is loaded
 *	-
 * @param core_name         Core name (id)
 * @param core_type         Core type
 * @param isLoaded          Is the core loaded
 *	-
 * @noreturn			
*/
forward void MC_OnCoreChangeStatus(char[] core_name, MC_CoreTypeBits core_type, bool isLoaded);

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
 * @param plugin_unique     Plugin Id
 * @param dont_load         Bits indexes of cores, which do not register this Plugin Id
 *	-
 * @noreturn   
*/
native void MC_RegisterPlugin(char[] plugin_unique, MC_CoreTypeBits dont_load = Core_Unloaded);
 
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
                                MC_OnItemPreviewCallback OnPreview = INVALID_FUNCTION
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
native MC_PluginId MC_EndPlugin();
 
/*
 * Unload plugin data (completely). Must be called in OnPluginEnd()
 *	-
 * @noparams
 *	-
 * @noreturn     
*/
native void MC_UnRegisterMe();
 
/*
 * Check the validity of the plugin by its index
 *	-
 * @param plugin_id         Plugin index
 *	-
 * @return true if all is well, false otherwise      
*/
native bool MC_IsValidPluginId(MC_PluginId plugin_id);

/*
 * Check the validity of the plugin by its identifier
 *	-
 * @param plugin_unique     Plugin Id
 *	-
 * @return true if all is well, false otherwise      
*/
native bool MC_IsValidPluginUnique(char[] plugin_unique);
 
/*
 * Getting an active item from a player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin index
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetClientActiveItem(int client, MC_PluginId plugin_id, char[] buffer, int maxlen);
 
/*
 * Getting an selected item from the player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin index
 * @param type              Core type
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetClientSelectedItem(int client, MC_PluginId plugin_id, MC_ValueOfCore type, char[] buffer, int maxlen);
 
/*
 * Set the selected item to the player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin index
 * @param type              Core type
 * @param item_unique       Item unique name (becomes active)
 *	-
 * @noreturn
*/
native void MC_SetClientSelectedItem(int client, MC_PluginId plugin_id, MC_ValueOfCore type, char[] item_unique);
 
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
