#include "global.h"
#include "battle.h"
#include "coins.h"
#include "credits.h"
#include "data.h"
#include "daycare.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_message_box.h"
#include "field_screen_effect.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "list_menu.h"
#include "m4a.h"
#include "main.h"
#include "main_menu.h"
#include "malloc.h"
#include "map_name_popup.h"
#include "menu.h"
#include "money.h"
#include "naming_screen.h"
#include "new_game.h"
#include "overworld.h"
#include "palette.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "region_map.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "pokemon_summary_screen.h"
#include "constants/abilities.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/species.h"

#define CHEAT_MAIN_MENU_WIDTH 13
#define CHEAT_MAIN_MENU_HEIGHT 8

#define CHEAT_NUMBER_DISPLAY_WIDTH 10
#define CHEAT_NUMBER_DISPLAY_HEIGHT 4
#define CHEAT_NUMBER_DISPLAY_SOUND_WIDTH 20
#define CHEAT_NUMBER_DISPLAY_SOUND_HEIGHT 6

#define CHEAT_NUMBER_DIGITS_FLAGS 4
#define CHEAT_NUMBER_DIGITS_VARIABLES 5
#define CHEAT_NUMBER_DIGITS_VARIABLE_VALUE 5
#define CHEAT_NUMBER_DIGITS_ITEMS 4
#define CHEAT_NUMBER_DIGITS_ITEM_QUANTITY 3

#define CHEAT_NUMBER_ICON_X 210
#define CHEAT_NUMBER_ICON_Y 50

void Cheat_ShowMainMenu(void);
static void Cheat_DestroyMainMenu(u8);
static void CheatAction_Cancel(u8);
static void CheatTask_HandleMainMenuInput(u8);
static void CheatAction_DestroyExtraWindow(u8 taskId);

static void CheatAction_Util_HealParty(u8 taskId);
static void CheatAction_Give_Item(u8 taskId);
static void CheatAction_Give_Item_SelectId(u8 taskId);
static void CheatAction_Give_Item_SelectQuantity(u8 taskId);
static void CheatAction_Give_MaxMoney(u8 taskId);
static void CheatAction_Give_MaxCoins(u8 taskId);
static void CheatAction_Give_DayCareEgg(u8 taskId);
static void CheatAction_Flags_EncounterOnOff(u8);
static void CheatAction_Util_SetWallClock(u8);

extern u8 PlayersHouse_2F_EventScript_SetWallClock[];

enum {
	CHEAT_UTIL_MENU_ITEM_HEAL_PARTY,
	CHEAT_GIVE_MENU_ITEM_ITEM_X,
	CHEAT_GIVE_MENU_ITEM_MAX_MONEY,
    CHEAT_GIVE_MENU_ITEM_MAX_COINS,
	CHEAT_GIVE_MENU_ITEM_DAYCARE_EGG,
	CHEAT_FLAG_MENU_ITEM_ENCOUNTER_ONOFF,
    CHEAT_UTIL_MENU_ITEM_SETWALLCLOCK,
    CHEAT_MENU_ITEM_CANCEL,
};

static const u8 gCheatText_Util_HealParty[] = _("Heal Party");
static const u8 gCheatText_Give_GiveItem[] =  _("Give item XXXX");
static const u8 gCheatText_Give_MaxMoney[] =  _("Max Money");
static const u8 gCheatText_Give_MaxCoins[] =  _("Max Coins");
static const u8 gCheatText_Give_DaycareEgg[] = _("Daycare Egg");
static const u8 gCheatText_Flags_SwitchEncounter[] = _("Encounter ON/OFF");
static const u8 gCheatText_Util_SetWallClock[] = _("Set Wall Clock");
static const u8 gCheatText_Cancel[] = _("Cancel");

static const u8 digitInidicatorCheat_1[] =        _("{LEFT_ARROW}+1{RIGHT_ARROW}        ");
static const u8 digitInidicatorCheat_10[] =       _("{LEFT_ARROW}+10{RIGHT_ARROW}       ");
static const u8 digitInidicatorCheat_100[] =      _("{LEFT_ARROW}+100{RIGHT_ARROW}      ");
static const u8 digitInidicatorCheat_1000[] =     _("{LEFT_ARROW}+1000{RIGHT_ARROW}     ");
static const u8 digitInidicatorCheat_10000[] =    _("{LEFT_ARROW}+10000{RIGHT_ARROW}    ");
static const u8 digitInidicatorCheat_100000[] =   _("{LEFT_ARROW}+100000{RIGHT_ARROW}   ");
static const u8 digitInidicatorCheat_1000000[] =  _("{LEFT_ARROW}+1000000{RIGHT_ARROW}  ");
static const u8 digitInidicatorCheat_10000000[] = _("{LEFT_ARROW}+10000000{RIGHT_ARROW} ");
const u8 * const gCheatText_DigitIndicator[] =
{
    digitInidicatorCheat_1,
    digitInidicatorCheat_10,
    digitInidicatorCheat_100,
    digitInidicatorCheat_1000,
    digitInidicatorCheat_10000,
    digitInidicatorCheat_100000,
    digitInidicatorCheat_1000000,
    digitInidicatorCheat_10000000
};
static const s32 sPowersOfTenCheat[] =
{
             1,
            10,
           100,
          1000,
         10000,
        100000,
       1000000,
      10000000,
     100000000,
    1000000000,
};

static const u8 gCheatText_ItemQuantity[] = _("Quantity:       \n{STR_VAR_1}    \n\n{STR_VAR_2}");
static const u8 gCheatText_ItemID[] =       _("Item Id: {STR_VAR_3}\n{STR_VAR_1}    \n\n{STR_VAR_2}");

static const struct ListMenuItem sCheatMenuItems[] =
{
	[CHEAT_UTIL_MENU_ITEM_HEAL_PARTY] = {gCheatText_Util_HealParty, CHEAT_UTIL_MENU_ITEM_HEAL_PARTY},
	[CHEAT_GIVE_MENU_ITEM_ITEM_X] = {gCheatText_Give_GiveItem, CHEAT_GIVE_MENU_ITEM_ITEM_X},
	[CHEAT_GIVE_MENU_ITEM_MAX_MONEY] = {gCheatText_Give_MaxMoney, CHEAT_GIVE_MENU_ITEM_MAX_MONEY},
    [CHEAT_GIVE_MENU_ITEM_MAX_COINS] = {gCheatText_Give_MaxCoins, CHEAT_GIVE_MENU_ITEM_MAX_COINS},
    [CHEAT_GIVE_MENU_ITEM_DAYCARE_EGG] = {gCheatText_Give_DaycareEgg, CHEAT_GIVE_MENU_ITEM_DAYCARE_EGG},
	[CHEAT_FLAG_MENU_ITEM_ENCOUNTER_ONOFF]  = {gCheatText_Flags_SwitchEncounter, CHEAT_FLAG_MENU_ITEM_ENCOUNTER_ONOFF},
	[CHEAT_UTIL_MENU_ITEM_SETWALLCLOCK]     = {gCheatText_Util_SetWallClock,     CHEAT_UTIL_MENU_ITEM_SETWALLCLOCK},
    [CHEAT_MENU_ITEM_CANCEL] = {gCheatText_Cancel, CHEAT_MENU_ITEM_CANCEL}
};

static void (*const sCheatMenuActions[])(u8) =
{
	[CHEAT_UTIL_MENU_ITEM_HEAL_PARTY] = CheatAction_Util_HealParty,
	[CHEAT_GIVE_MENU_ITEM_ITEM_X] = CheatAction_Give_Item,
	[CHEAT_GIVE_MENU_ITEM_MAX_MONEY] = CheatAction_Give_MaxMoney,
    [CHEAT_GIVE_MENU_ITEM_MAX_COINS] = CheatAction_Give_MaxCoins,
    [CHEAT_GIVE_MENU_ITEM_DAYCARE_EGG] = CheatAction_Give_DayCareEgg,
	[CHEAT_FLAG_MENU_ITEM_ENCOUNTER_ONOFF]  = CheatAction_Flags_EncounterOnOff,
	[CHEAT_UTIL_MENU_ITEM_SETWALLCLOCK]     = CheatAction_Util_SetWallClock,
    [CHEAT_MENU_ITEM_CANCEL] = CheatAction_Cancel
};

static const struct WindowTemplate sCheatMenuWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = CHEAT_MAIN_MENU_WIDTH,
    .height = 2 * CHEAT_MAIN_MENU_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

static const struct ListMenuTemplate sCheatMenuListTemplate =
{
    .items = sCheatMenuItems,
    .moveCursorFunc = ListMenuDefaultCursorMoveFunc,
    .totalItems = ARRAY_COUNT(sCheatMenuItems),
    .maxShowed = CHEAT_MAIN_MENU_HEIGHT,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = 1,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = 1,
    .cursorKind = 0
};

static const struct WindowTemplate sCheatNumberDisplayWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = 6 + CHEAT_MAIN_MENU_WIDTH,
    .tilemapTop = 1,
    .width = CHEAT_NUMBER_DISPLAY_WIDTH,
    .height = 2 * CHEAT_NUMBER_DISPLAY_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 1,
};

void Cheat_ShowMainMenu(void) {
    struct ListMenuTemplate menuTemplate;
    u8 windowId;
    u8 menuTaskId;
    u8 inputTaskId;

    // create window
    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sCheatMenuWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    // create list menu
    menuTemplate = sCheatMenuListTemplate;
    menuTemplate.windowId = windowId;
    menuTaskId = ListMenuInit(&menuTemplate, 0, 0);

    // draw everything
    CopyWindowToVram(windowId, 3);

    // create input handler task
    inputTaskId = CreateTask(CheatTask_HandleMainMenuInput, 3);
    gTasks[inputTaskId].data[0] = menuTaskId;
    gTasks[inputTaskId].data[1] = windowId;
}

static void Cheat_DestroyMainMenu(u8 taskId)
{
    DestroyListMenuTask(gTasks[taskId].data[0], NULL, NULL);
    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);
    DestroyTask(taskId);
    EnableBothScriptContexts();
}

static void CheatTask_HandleMainMenuInput(u8 taskId)
{
    void (*func)(u8);
    u32 input = ListMenu_ProcessInput(gTasks[taskId].data[0]);

    if (gMain.newKeys & A_BUTTON)
    {
        PlaySE(SE_SELECT);
        if ((func = sCheatMenuActions[input]) != NULL)
            func(taskId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Cheat_DestroyMainMenu(taskId);
    }
}

static void CheatAction_Cancel(u8 taskId)
{
    Cheat_DestroyMainMenu(taskId);
}

static void CheatAction_DestroyExtraWindow(u8 taskId)
{
    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);

    ClearStdWindowAndFrame(gTasks[taskId].data[2], TRUE);
    RemoveWindow(gTasks[taskId].data[2]);

    DestroyTask(taskId);
    EnableBothScriptContexts();
}

static void CheatAction_Util_HealParty(u8 taskId)
{
    PlaySE(SE_USE_ITEM);
    HealPlayerParty();
    EnableBothScriptContexts();
    Cheat_DestroyMainMenu(taskId);
}

static void CheatAction_Give_MaxMoney(u8 taskId)
{
    SetMoney(&gSaveBlock1Ptr->money, 999999);
}

static void CheatAction_Give_MaxCoins(u8 taskId)
{
    SetCoins(9999);
}

static void CheatAction_Give_DayCareEgg(u8 taskId)
{
    TriggerPendingDaycareEgg();
}

#define ITEM_TAG 0xFDF3
static void CheatAction_Give_Item(u8 taskId)
{
    u8 windowId;

    ClearStdWindowAndFrame(gTasks[taskId].data[1], TRUE);
    RemoveWindow(gTasks[taskId].data[1]);

    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    windowId = AddWindow(&sCheatNumberDisplayWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);

    CopyWindowToVram(windowId, 3);

    //Display initial ID
    StringCopy(gStringVar2, gCheatText_DigitIndicator[0]);
    ConvertIntToDecimalStringN(gStringVar3, 1, STR_CONV_MODE_LEADING_ZEROS, CHEAT_NUMBER_DIGITS_ITEMS);
    CopyItemName(1, gStringVar1);
    StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
    StringExpandPlaceholders(gStringVar4, gCheatText_ItemID);
    AddTextPrinterParameterized(windowId, 1, gStringVar4, 1, 1, 0, NULL);

    gTasks[taskId].func = CheatAction_Give_Item_SelectId;
    gTasks[taskId].data[2] = windowId;
    gTasks[taskId].data[3] = 1;            //Current ID
    gTasks[taskId].data[4] = 0;            //Digit Selected
    gTasks[taskId].data[6] = AddItemIconSprite(ITEM_TAG, ITEM_TAG, gTasks[taskId].data[3]);
    gSprites[gTasks[taskId].data[6]].x2 = CHEAT_NUMBER_ICON_X+10;
    gSprites[gTasks[taskId].data[6]].y2 = CHEAT_NUMBER_ICON_Y+10;
    gSprites[gTasks[taskId].data[6]].oam.priority = 0;
}
static void CheatAction_Give_Item_SelectId(u8 taskId)
{
    if (gMain.newKeys & DPAD_ANY)
    {
        PlaySE(SE_SELECT);

        if(gMain.newKeys & DPAD_UP)
        {
            gTasks[taskId].data[3] += sPowersOfTenCheat[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] >= ITEMS_COUNT)
                gTasks[taskId].data[3] = ITEMS_COUNT - 1;
        }
        if(gMain.newKeys & DPAD_DOWN)
        {
            gTasks[taskId].data[3] -= sPowersOfTenCheat[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] < 1)
                gTasks[taskId].data[3] = 1;
        }
        if(gMain.newKeys & DPAD_LEFT)
        {
            if(gTasks[taskId].data[4] > 0)
                gTasks[taskId].data[4] -= 1;
        }
        if(gMain.newKeys & DPAD_RIGHT)
        {
            if(gTasks[taskId].data[4] < CHEAT_NUMBER_DIGITS_ITEMS-1)
                gTasks[taskId].data[4] += 1;
        }

        StringCopy(gStringVar2, gCheatText_DigitIndicator[gTasks[taskId].data[4]]);
        CopyItemName(gTasks[taskId].data[3], gStringVar1);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        ConvertIntToDecimalStringN(gStringVar3, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, CHEAT_NUMBER_DIGITS_ITEMS);
        StringExpandPlaceholders(gStringVar4, gCheatText_ItemID);
        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);

        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[6]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[6]]);       //Destroy item icon
        gTasks[taskId].data[6] = AddItemIconSprite(ITEM_TAG, ITEM_TAG, gTasks[taskId].data[3]);
        gSprites[gTasks[taskId].data[6]].x2 = CHEAT_NUMBER_ICON_X+10;
        gSprites[gTasks[taskId].data[6]].y2 = CHEAT_NUMBER_ICON_Y+10;
        gSprites[gTasks[taskId].data[6]].oam.priority = 0;
    }

    if (gMain.newKeys & A_BUTTON)
    {
        gTasks[taskId].data[5] = gTasks[taskId].data[3];
        gTasks[taskId].data[3] = 1;
        gTasks[taskId].data[4] = 0;

        StringCopy(gStringVar2, gCheatText_DigitIndicator[gTasks[taskId].data[4]]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, CHEAT_NUMBER_DIGITS_ITEM_QUANTITY);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, gCheatText_ItemQuantity);
        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);

        gTasks[taskId].func = CheatAction_Give_Item_SelectQuantity;
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[6]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[6]]);       //Destroy item icon

        PlaySE(SE_SELECT);
        CheatAction_DestroyExtraWindow(taskId);
    }
}
static void CheatAction_Give_Item_SelectQuantity(u8 taskId)
{
    if (gMain.newKeys & DPAD_ANY)
    {
        PlaySE(SE_SELECT);

        if(gMain.newKeys & DPAD_UP)
        {
            gTasks[taskId].data[3] += sPowersOfTenCheat[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] >= 1000)
                gTasks[taskId].data[3] = 999;
        }
        if(gMain.newKeys & DPAD_DOWN)
        {
            gTasks[taskId].data[3] -= sPowersOfTenCheat[gTasks[taskId].data[4]];
            if(gTasks[taskId].data[3] < 1)
                gTasks[taskId].data[3] = 1;
        }
        if(gMain.newKeys & DPAD_LEFT)
        {
            if(gTasks[taskId].data[4] > 0)
                gTasks[taskId].data[4] -= 1;
        }
        if(gMain.newKeys & DPAD_RIGHT)
        {
            if(gTasks[taskId].data[4] < 2)
                gTasks[taskId].data[4] += 1;
        }

        StringCopy(gStringVar2, gCheatText_DigitIndicator[gTasks[taskId].data[4]]);
        ConvertIntToDecimalStringN(gStringVar1, gTasks[taskId].data[3], STR_CONV_MODE_LEADING_ZEROS, CHEAT_NUMBER_DIGITS_ITEM_QUANTITY);
        StringCopyPadded(gStringVar1, gStringVar1, CHAR_SPACE, 15);
        StringExpandPlaceholders(gStringVar4, gCheatText_ItemQuantity);
        AddTextPrinterParameterized(gTasks[taskId].data[2], 1, gStringVar4, 1, 1, 0, NULL);
    }

    if (gMain.newKeys & A_BUTTON)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[6]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[6]]);       //Destroy item icon

        PlaySE(MUS_OBTAIN_ITEM);
        AddBagItem(gTasks[taskId].data[5], gTasks[taskId].data[3]);
        CheatAction_DestroyExtraWindow(taskId);
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[6]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[6]]);       //Destroy item icon

        PlaySE(SE_SELECT);
        CheatAction_DestroyExtraWindow(taskId);
    }
}

static void CheatAction_Flags_EncounterOnOff(u8 taskId)
{
    if(FlagGet(FLAG_SYS_NO_ENCOUNTER))
    {
        FlagClear(FLAG_SYS_NO_ENCOUNTER);
        PlaySE(SE_PC_OFF);
    }else{
        FlagSet(FLAG_SYS_NO_ENCOUNTER);
        PlaySE(SE_PC_LOGIN);
    }
}

static void CheatAction_Util_SetWallClock(u8 taskId)
{
    Cheat_DestroyMainMenu(taskId);
    ScriptContext2_Enable();
    ScriptContext1_SetupScript(PlayersHouse_2F_EventScript_SetWallClock);
}
