
typeset MC_OnMenuDisplayCallbacks
{
    /* 
     * Callback to display plugin name in menu
     *  -
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - to display the new name or not change it, false - to hide
    */
	function bool (int client, const char[] plugin_id, const char[] core_type, char[] buffer, int maxlen);

    /*
     * Callback to display item name in menu
     *  -
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param item_unique      Item Id
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - to display the new name or not change it, false - to hide
    */
	function bool (int client, const char[] plugin_id, const char[] item_unique, const char[] core_type, char[] buffer, int maxlen);
}

typeset MC_OnMenuSelectedCallbacks
{
    /*
     * Callback when selecting a plugin from the menu
     *  -
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param core_type        Core type
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *  -
     * @return true - continue the action, false - stop the action
    */
	function bool (int client, const char[] plugin_id, const char[] core_type);

    /*
     * Callback when selecting a item from the menu
     *	-
     * @param client           Player Index
     * @param plugin_id        Plugin Id
     * @param core_type        Core type
     * @param item_unique      Item Id (can be empty if turned off)
     * @param buffer           The buffer where the result will be placed           
     * @param maxlen           Buffer size           
     *	-
     * @return true - continue the action, false - stop the action
    */
	function bool (int client, const char[] plugin_id, const char[] item_unique, const char[] core_type);
}

/*
 * Callback for menu item preview
 *  -
 * @param client            Player Index
 * @param plugin_id         Plugin Id
 * @param item_unique       Item Id
 * @param core_type         Core type
 *	-
 * @noreturn
*/
typedef MC_OnItemPreviewCallback = function void (int client, const char[] plugin_id, const char[] item_unique, const char[] core_type);

/*
 * Register Plugin Id
 *	-
 * @param plugin_id         Plugin Id
 *	-
 * @noreturn   
*/
native void MC_RegisterPlugin(const char[]plugin_id);
 
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
native bool MC_StartItem(const char[]item_unique);

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
 * @noreturn 
*/
native void MC_EndPlugin();
 