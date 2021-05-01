#include <sourcemod>

public Action Command_MCMenu(int args, int client)
{
    if(client && IsClientInGame(client))
        Menu_MC_Main(client);

    return Plugin_Handled;
}

public Menu Menu_MC_Main(int client)
{
    /*
        Multi-Core: настройки

        Перечисление плагинов (идентификаторов)

        Выход
    */
    char buff[128], translate[128];

    Menu menu = new Menu(MenuHandler_MC_Main);
    menu.ExitBackButton = true;

    Format(translate, sizeof(translate), "");
    menu.SetTitle(translate);

    

    Format(translate, sizeof(translate), "");
    menu.AddItem("", translate);

    return menu;
}

public int MenuHandler_MC_Main(Menu menu, MenuAction action, int client, int item)
{
    if(action == MenuAction_Select)
    {
        char buff[64];
        menu.GetItem(item, buff, sizeof(buff));

        
    }
    // else if(action == MenuAction_Cancel && item == MenuCancel_ExitBack) 
    else if(action == MenuAction_End) 
        delete menu;
}