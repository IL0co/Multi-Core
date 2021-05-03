#if defined _mc_core_included
 #endinput
#endif
#define _mc_core_included

#define MAX_UNIQUE_LENGTH   64      // Maximum length of identifier string

enum MC_DataType        // Data types
{   
    Type_Model = 0,         // Model (.mdl)
    Type_Sound,             // Sound file (.mp3 .wav). No folder 'sound/'
    Type_Particle,          // Particle (name)
    Type_ParticleFile,      // Particle file (.pcf)
    Type_Sprite,            // Texture (.vmt)
}

#include <multi-core/item_register.inc>
#include <multi-core/integration_register.inc>

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
 *	-
 * @noreturn			
*/
forward void MC_OnPluginRegistered(const char[] plugin_id);

/*
 * Called before the plugin id was unregistered
 *	-
 * @param plugin_id         Plugin Id
 *	-
 * @noreturn			
*/
forward void MC_OnPluginUnRegistered_Pre(const char[] plugin_id);

/*
 * Called after the plugin id was unregistered
 *	-
 * @param plugin_id         Plugin Id
 *	-
 * @noreturn			
*/
forward void MC_OnPluginUnRegistered(const char[] plugin_id);

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
 * @noparams
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_IsCoreLoaded();

/*
 * Check for items with a preview in the plugin id
 *	-
 * @param plugin_id         Plugin Id
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_IsPluginHavePreviewItems(char[] plugin_id);

/*
 * Check if the item has a preview
 *	-
 * @param plugin_id         Plugin Id
 * @param item_unique       Item Id (unique)
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_IsItemHavePreview(char[] plugin_id, char[] item_unique);

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
 * Get an array of items. The handle must be closed!
 *	-
 * @param plugin_id         Plugin Id
 *	-
 * @return ArrayList Handle      
*/
native ArrayList MC_GetPluginItemsArrayList(char[] plugin_id);

/*
 * Check the validity of the plugin by its identifier
 *	-
 * @param plugin_id     Plugin Id
 *	-
 * @return true if all is well, false otherwise      
*/
native bool MC_IsValidPluginUnique(const char[] plugin_id);
 
/*
 * Get the number of plugins I have registered
 *	-
 * @noparams
 *	-
 * @return number of registered plugin ids      
*/
native int MC_GetPluginsCountRegisteredByMe();
 
/*
 * Get the display name of the plugin
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param core_type         Core type
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetPluginDisplayName(int client, char[] plugin_id, char[] core_type, char[] buffer, int maxlen);
 
/*
 * Get the display name of the item
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param item_unique       Item unique name 
 * @param core_type         Core type
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetItemDisplayName(int client, char[] plugin_id, const char[] item_unique, char[] core_type, char[] buffer, int maxlen);
 
/*
 * Getting an active item from a player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetClientActiveItem(int client, char[] plugin_id, char[] buffer, int maxlen);
 
/*
 * Getting an selected item from the player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param core_type         Core type
 * @param buffer            The buffer where the result will be placed           
 * @param maxlen            Buffer size           
 *	-
 * @return true if all is well, false otherwise 
*/
native bool MC_GetClientSelectedItem(int client, char[] plugin_id, char[] core_type, char[] buffer, int maxlen);
 
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

    MarkNativeAsOptional("MC_RegisterPlugin");
    MarkNativeAsOptional("MC_SetPluginCallBacks");
    MarkNativeAsOptional("MC_StartItem");
    MarkNativeAsOptional("MC_SetItemCallBacks");
    MarkNativeAsOptional("MC_EndItem");
    MarkNativeAsOptional("MC_EndPlugin");
    MarkNativeAsOptional("MC_UnRegisterMe");

    MarkNativeAsOptional("MC_GetPluginIdsArrayList");
    MarkNativeAsOptional("MC_GetPluginItemsArrayList");

    MarkNativeAsOptional("MC_IsValidPluginUnique");

    MarkNativeAsOptional("MC_GetPluginsCountRegisteredByMe");

    MarkNativeAsOptional("MC_GetPluginDisplayName");
    MarkNativeAsOptional("MC_GetItemDisplayName");

    MarkNativeAsOptional("MC_IsPluginHavePreviewItems");
    MarkNativeAsOptional("MC_IsItemHavePreview");

    MarkNativeAsOptional("MC_CallPluginSelected");
    MarkNativeAsOptional("MC_CallItemPreview");
    MarkNativeAsOptional("MC_CallItemSelected");
    MarkNativeAsOptional("MC_RegisterIntegration");

    MarkNativeAsOptional("MC_GetClientActiveItem");
    MarkNativeAsOptional("MC_GetClientSelectedItem");
}
#endif
