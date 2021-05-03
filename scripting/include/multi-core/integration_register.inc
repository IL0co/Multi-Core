
/*
 * Call a select callback of the plugin for the player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param item_unique       Item unique name 
 * @param core_type         Core type
 *	-
 * @return true if given permission, false otherwise
*/
native bool MC_CallPluginSelected(int client, char[] plugin_id, char[] core_type);
 
/*
 * Call a preview of the item for the player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param item_unique       Item unique name 
 * @param core_type         Core type
 *	-
 * @noreturn
*/
native void MC_CallItemPreview(int client, char[] plugin_id, const char[] item_unique, char[] core_type);
 
/*
 * Call a select callback of the item for the player
 *	-
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param item_unique       Item unique name 
 * @param core_type         Core type
 *	-
 * @return true if given permission, false otherwise
*/
native bool MC_CallItemSelected(int client, char[] plugin_id, const char[] item_unique, char[] core_type);
 