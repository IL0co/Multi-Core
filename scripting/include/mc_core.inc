#include <sourcemod>

#define MAX_UNIQUE_LENGTH   64      // Максимальная длинна строки

enum MC_CoreTypeBits    // Типы ядер
{   
    Core_Unloaded = 0,      // Никакое из ядер не загружено     |
    Core_MultiCore = 1,     // Ядро Multi-Core                  | multicore
    Core_Shop = 2,          // Ядро шопа                        | shop
    Core_VIP = 4,           // Ядро випки                       | vip_core
}

enum MC_DataType        // Типы данных
{   
    Type_Model = 0,         // Модель (.mdl)
    Type_Sound,             // Звук (.mp3 .wav). Без папки sound/
    Type_Particle,          // Партикл
    Type_ParticleFile,      // Файл партикла (.pcf)
    Type_Sprite,            // Текстура (.vmt)
}

enum PluginId
{
    INVALID_PLUGIN_ID = -1,
}

enum ItemId
{
    INVALID_ITEM_ID = -1,
}

typeset MC_OnMenuDisplayCallback
{
    /** 
    *	Обратный вызов отображения имени плагина в меню
    *	
    *	@param client           Индекс игрока
    *	@param plugin_unique    Идентификатор плагина
    *	@param plugin_id        Индекс плагина
    *   @param core_type        Тип ядра
    *	@param buffer           Буфер, куда будет помещён результат           
    *	@param maxlen           Размер буфера           
    *	
    *	@return true - что бы отобразить новое название или не менять его, false - что бы скрыть
    */
	function bool (int client, const char[] plugin_unique, PluginId plugin_id, MC_CoreTypeBits core_type, char[] buffer, int maxlen);

    /**
    *	Обратный вызов отображения имени предмета в меню
    *	
    *	@param client           Индекс игрока
    *	@param plugin_unique    Идентификатор плагина
    *	@param plugin_id        Индекс плагина
    *	@param item             Идентификатор предмета
    *	@param item_id          Индекс предмета
    *   @param core_type        Тип ядра
    *	@param buffer           Буфер, куда будет помещён результат           
    *	@param maxlen           Размер буфера           
    *	
    *	@return true - что бы отобразить новое название или не менять его, false - что бы скрыть
    */
	function bool (int client, const char[] plugin_unique, PluginId plugin_id, const char[] item, ItemId item_id, MC_CoreTypeBits core_type, char[] buffer, int maxlen);
}

/**
*	Обратный вызов превью предмета в меню
*	
*	@param client           Индекс игрока
*	@param plugin_unique    Идентификатор плагина
*	@param plugin_id        Индекс плагина
*	@param item             Идентификатор предмета
*	@param item_id          Индекс предмета
*   @param core_type        Тип ядра
*	
*	@noreturn
*/
typedef MC_OnItemPreviewCallback = function void (int client, const char[] plugin_unique, PluginId plugin_id, const char[] item, ItemId item_id, MC_CoreTypeBits core_type);

/*
 * Вызывается после загрузки ядра.
 *	-
 * @param core_name     Имя (id) ядра
 * @param core_type     Тип ядра
 * @param isLoaded      true = ядро загрузилось
 *                      false = выгрузилось  
 *	-
 * @noreturn			
*/
forward void MC_OnCoreChangeStatus(char[] core_name, MC_CoreTypeBits core_type, bool isLoaded);

/*
 * Прекеширует файл.
 *	-
 * @param file          Путь к файлу. Название партикла и тд
 * @param data_type     Тип файла
 *	-
 * @return  индекс файла или -1			
*/
native int MC_PrecacheFile(char[] file, MC_DataType data_type);

/*
 * Проверка на загруженность ядра.
 *	-
 * @param core_type     Тип ядра
 *	-
 * @return  true - загружено, 
            false - нет
*/
native bool MC_IsCoreLoaded(MC_CoreTypeBits core_type = Core_MultiCore);

/*
 * Получить конфиг ядра
 *	-
 * @param core_type     Тип ядра
 *	-
 * @return  KeyValues handle
*/
native KeyValues MC_GetSettingsConfigKV(MC_CoreTypeBits core_type);
 
/*
 * Зарегистрировать идентификатор плагина
 *	-
 * @param plugin_unique     Идентификатор плагина
 * @param OnDisplay         Обратный вызов отображения в меню. Название Vip функции.
 * @param dont_load         Bits индексов ядер, в которые не регистрировать данный идентификатор плагина 
 *	-
 * @return      Индекс плагина 
*/
native PluginId MC_RegisterPlugin(char[] plugin_unique, MC_OnMenuDisplayCallback OnDisplay, MC_CoreTypeBits dont_load = Core_Unloaded);
 
/*
 * Начать регистрацию предмета
 *	-
 * @param plugin_id         Индекс плагина
 * @param item              Уникальное имя предмета
 *	-
 * @return      true if all is well, false otherwise 
*/
native bool MC_StartItem(PluginId plugin_id, char[] item);

/*
 * Добавить обратные вызова предмета
 *	-
 * @param OnDisplay         Обратный вызов отображения в меню
 * @param OnPreview         Обратный вызов превью предмета
 *	-
 * @noreturn 
*/
native void MC_SetItemCallBacks(MC_OnMenuDisplayCallback OnDisplay = INVALID_FUNCTION,
                                MC_OnItemPreviewCallback OnPreview = INVALID_FUNCTION);

/*
 * Закончить регистрацию предмета
 *	-
 * @noparams
 *	-
 * @return      Индекс предмета
*/
native ItemId MC_EndItem();
 
/*
 * Закончить регистрацию плагина
 *	-
 * @param plugin_id         Индекс плагина
 *	-
 * @noreturn
*/
native void MC_EndPlugin(PluginId plugin_id);
 
/*
 * Выгрузить данные плагина (полностью). Обязательно должен вызываться в OnPluginEnd()
 *	-
 * @noparams
 *	-
 * @noreturn     
*/
native void MC_UnRegisterMe();
 
/*
 * Получение активного предмета у игрока 
 *	-
 * @param client            Индекс игрока
 * @param plugin_id         Индекс плагина
 * @param buffer            Буфер, куда будет помещён результат           
 * @param maxlen            Размер буфера           
 *	-
 * @return      true if all is well, false otherwise 
*/
native bool MC_GetClientActiveItem(int client, PluginId plugin_id, char[] buffer, int maxlen);
 
/*
 * Установление активного предмета у игрока 
 *	-
 * @param client            Индекс игрока
 * @param plugin_id         Индекс плагина
 * @param core_type         Тип ядра
 * @param item              Уникальное имя предмета, которое и поставится активным
 *	-
 * @return      true if all is well, false otherwise 
*/
native bool MC_SetClientActiveItem(int client, PluginId plugin_id, MC_CoreTypeBits core_type, char[] item);
 
/*
 * Получить конфиг модуля из папки 'addons/sourcemod/configs/multi-core/modules/'
 *	-
 * @param file_name     Имя файла
 * @param key_name      Название структуры (первый ключ)
 *	-
 * @return  KeyValues handle
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
