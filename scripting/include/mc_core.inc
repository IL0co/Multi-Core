#include <sourcemod>

#define MAX_UNIQUE_LENGTH   64      // Максимальная длинна строки

enum MC_CoreTypeBits    // Типы ядер
{                           // Описание:                        | Идентификатор ядра (библиотеки):
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

enum MC_PluginId
{
    INVALID_PLUGIN_ID = -1,
}

enum MC_ValueOfCore
{
    All = 0,            // Общедоступный
    VIP = 1,            // Из Випки
    Shop = 2,           // Из шопа
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
	function bool (int client, const char[] plugin_unique, MC_PluginId plugin_id, MC_CoreTypeBits core_type, char[] buffer, int maxlen);

    /**
    *	Обратный вызов отображения имени предмета в меню
    *	
    *	@param client           Индекс игрока
    *	@param plugin_unique    Идентификатор плагина
    *	@param plugin_id        Индекс плагина
    *	@param item_unique      Идентификатор предмета
    *   @param core_type        Тип ядра
    *	@param buffer           Буфер, куда будет помещён результат           
    *	@param maxlen           Размер буфера           
    *	
    *	@return true - что бы отобразить новое название или не менять его, false - что бы скрыть
    */
	function bool (int client, const char[] plugin_unique, MC_PluginId plugin_id, const char[] item_unique, MC_CoreTypeBits core_type, char[] buffer, int maxlen);
}

/**
*	Обратный вызов превью предмета в меню
*	
*	@param client           Индекс игрока
*	@param plugin_unique    Идентификатор плагина
*	@param plugin_id        Индекс плагина
*	@param item_unique      Идентификатор предмета
*   @param core_type        Тип ядра
*	
*	@noreturn
*/
typedef MC_OnItemPreviewCallback = function void (int client, const char[] plugin_unique, MC_PluginId plugin_id, const char[] item_unique, MC_CoreTypeBits core_type);

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
 * @param type          Тип ядра
 *	-
 * @return  KeyValues handle
*/
native KeyValues MC_GetSettingsConfigKV(MC_ValueOfCore type);
 
/*
 * Зарегистрировать идентификатор плагина
 *	-
 * @param plugin_unique     Идентификатор плагина
 * @param dont_load         Bits индексов ядер, в которые не регистрировать данный идентификатор плагина 
 *	-
 * @noreturn   
*/
native void MC_RegisterPlugin(char[] plugin_unique, MC_CoreTypeBits dont_load = Core_Unloaded);
 
/*
 * Добавить обратные вызова плагина
 *	-
 * @param OnDisplay         Обратный вызов отображения в меню
 *	-
 * @noreturn 
*/
native void MC_SetPluginCallBacks(MC_OnMenuDisplayCallback OnDisplay = INVALID_FUNCTION);

/*
 * Начать регистрацию предмета
 *	-
 * @param item_unique       Уникальное имя предмета
 *	-
 * @return      true if all is well, false otherwise 
*/
native bool MC_StartItem(char[] item_unique);

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
 * @noreturn   
*/
native void MC_EndItem();
 
/*
 * Закончить регистрацию плагина
 *	-
 * @noparams
 *	-
 * @return      Индекс плагина 
*/
native MC_PluginId MC_EndPlugin();
 
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
native bool MC_GetClientActiveItem(int client, MC_PluginId plugin_id, char[] buffer, int maxlen);
 
/*
 * Получение выбраного предмета у игрока 
 *	-
 * @param client            Индекс игрока
 * @param plugin_id         Индекс плагина
 * @param type              Тип ядра
 * @param buffer            Буфер, куда будет помещён результат           
 * @param maxlen            Размер буфера           
 *	-
 * @return      true if all is well, false otherwise 
*/
native bool MC_GetClientSelectedItem(int client, MC_PluginId plugin_id, MC_ValueOfCore type, char[] buffer, int maxlen);
 
/*
 * Установить игроку выбранный предмет
 *	-
 * @param client            Индекс игрока
 * @param plugin_id         Индекс плагина
 * @param type              Тип ядра
 * @param item_unique       Уникальное имя предмета (установится активным)
 *	-
 * @noreturn
*/
native void MC_SetClientSelectedItem(int client, MC_PluginId plugin_id, MC_ValueOfCore type, char[] item_unique);
 
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
