#ifndef UI_H
#define UI_H

#include <vector>
#include <string>

#include "data.h"
#include "gfx.h"

//ui headers - split up to keep a bit more organized
#include "miscui.h"
#include "uiupdate.h"

enum menuState
{
    USR_SEL,
    TTL_SEL,
    FLD_SEL,
    BKL_SEL
};

enum menuTouch
{
	MENU_NOTHING,
	MENU_DOUBLE_REL
};

namespace ui
{

    //Current menu/ui state
    extern int mstate, prevState, glowR, glowG, glowB;

    //Both UI modes need access to this
    extern std::string folderMenuInfo;

    //Colors to use now that I added theme detection
    extern clr clearClr, mnuTxt, txtClr, tboxClr, sideRect, sepClr, boundClr, popupClr, popupbgClr, blurClr, butselClr;

	extern tex *screen;
    //Button tex
	extern tex *iconSelTopLeft, *iconSelTopRight, *iconSelBotLeft, *iconSelBotRight, *iconSelShadowLeft, *iconSelShadowBot, *iconSelShadowRight;
    extern tex *buttonA, *buttonB, *buttonX, *buttonY, *buttonM, *buttonP;
    //Textbox graphics
    extern tex *cornerTopHor, *cornerBottomHor, *cornerLeftVer, *cornerRightVer, *cornerTopLeft, *cornerTopRight, *cornerBottomLeft, *cornerBottomRight, *tip, *horEdgeTop, *horEdgeBot, *vertEdgeLeft, *vertEdgeRight;
	extern tex *popupTopLeft, *popupTopRight, *popupBotLeft, *popupBotRight, *popupBotShadow;
    extern tex *popupButTopLeft, *popupButTopRight, *popupButBotLeft, *popupButBotRight;
	extern bool finish;

    //Shared font
    extern font *shared;

    // extern std::vector<ui::button> selButtons;

    //Loads in the A, B, X, Y button graphics
    void init();
    void exit();

    //Clears and draws general stuff used by multiple screens
    void drawUI();

    //switch case so we don't have problems with multiple main loops like 3DS
    void runApp(const uint64_t& down, const uint64_t& held, const touchPosition& p);
}

#endif
