#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <switch.h>

#include "ui.h"
#include "gfx.h"
#include "util.h"
#include "file.h"

#define VER_STRING "v. 01/08/2019"

//background that can be drawn from "/JKSV/back.jpg"
//txtSide and fldSide are to fake alpha blending so the framerate doesn't suffer
static tex *background = NULL, *txtSide = NULL, *fldSide = NULL;

//Nav buttons
std::vector<ui::button> usrNav, ttlNav, blkNav, fldNav;

namespace ui
{
    //Current menu state
    int mstate = USR_SEL, prevState = USR_SEL;
	bool finish = false;

    //Info printed on folder menu
    std::string folderMenuInfo;

    //Touch button vector
    // std::vector<ui::button> selButtons;

    //UI colors
    clr clearClr, mnuTxt, txtClr, tboxClr, sideRect, divClr, sepClr, boundClr, popupClr, popupbgClr, blurClr;

    //textbox pieces
    //I was going to flip them when I draw them, but then laziness kicked in.
    tex *cornerTopHor, *cornerBottomHor, *cornerLeftVer, *cornerRightVer, *cornerTopLeft, *cornerTopRight, *cornerBottomLeft, *cornerBottomRight, *tip, *temp;
    tex *mnuTopLeft, *mnuTopRight, *mnuBotLeft, *mnuBotRight, *mnuBotShadow;
    tex *popupTopLeft, *popupTopRight, *popupBotLeft, *popupBotRight, *popupBotShadow;
    tex *popupButTopLeft, *popupButTopRight, *popupButBotLeft, *popupButBotRight;
    tex *buttonA, *buttonB, *buttonX, *buttonY, *buttonM, *buttonP;
    tex *icn, *mnuGrad, *iconSel, *menuSel, *buttonSel, *buttonLrg;
    font *shared;
	
	tex	*screen = texCreate(1280, 720);

    void init()
    {
        ColorSetId gthm;
        setsysGetColorSetId(&gthm);

		popupClr = clrCreateU32(0xFFF0F0F0);
		popupbgClr = clrCreateU32(0xB2160F05);
		blurClr = clrCreateU32(0xFFF15230);

		temp = texLoadPNGFile("romfs:/img/mnu/popup.png");
		popupTopLeft = texCreateFromPart(temp, 0, 0, 15, 15);
		popupTopRight = texCreateFromPart(temp, 15, 0, 15, 15);
		popupBotLeft = texCreateFromPart(temp, 0, 15, 15, 15);
		popupBotRight = texCreateFromPart(temp, 15, 15, 15, 15);
		popupBotShadow = texCreateFromPart(temp, 15, 15, 1, 15);

		temp = texLoadPNGFile("romfs:/img/mnu/popupsel.png");
		popupButTopLeft = texCreateFromPart(temp, 0, 0, 15, 15);
		popupButTopRight = texCreateFromPart(temp, 15, 0, 15, 15);
		popupButBotLeft = texCreateFromPart(temp, 0, 15, 15, 15);
		popupButBotRight = texCreateFromPart(temp, 15, 15, 15, 15);

        iconSel = texLoadPNGFile("romfs:/img/mnu/iconSel.png");
        menuSel = texLoadPNGFile("romfs:/img/mnu/menuSel.png");
        buttonSel = texLoadPNGFile("romfs:/img/mnu/buttonSel.png");
        buttonLrg = texLoadPNGFile("romfs:/img/mnu/buttonLrg.png");

        switch(gthm)
        {
            case ColorSetId_Light:
                //Light corners
		        temp = texLoadPNGFile("romfs:/img/mnu/tboxLght.png");
				cornerTopHor = texCreateFromPart(temp, 12, 0, 1, 12);
				cornerBottomHor = texCreateFromPart(temp, 12, 52, 1, 12);
				cornerLeftVer = texCreateFromPart(temp, 0, 12, 12, 1);
				cornerRightVer = texCreateFromPart(temp, 52, 12, 12, 1);
				cornerTopLeft = texCreateFromPart(temp, 0, 0, 12, 12);
				cornerTopRight = texCreateFromPart(temp, 52, 0, 12, 12);
				cornerBottomLeft = texCreateFromPart(temp, 0, 52, 12, 12);
				cornerBottomRight = texCreateFromPart(temp, 52, 52, 12, 12);

                tip = texLoadPNGFile("romfs:/img/mnu/tboxTipLght.png");

                //Dark buttons
                buttonA = texLoadPNGFile("romfs:/img/button/buttonA_drk.png");
                buttonB = texLoadPNGFile("romfs:/img/button/buttonB_drk.png");
                buttonX = texLoadPNGFile("romfs:/img/button/buttonX_drk.png");
                buttonY = texLoadPNGFile("romfs:/img/button/buttonY_drk.png");
                buttonM = texLoadPNGFile("romfs:/img/button/buttonM_drk.png");
                buttonP = texLoadPNGFile("romfs:/img/button/buttonP_drk.png");

                icn = texLoadPNGFile("romfs:/img/icn/icnDrk.png");
                mnuGrad = texLoadPNGFile("romfs:/img/mnu/gradLght.png");

				temp = texLoadPNGFile("romfs:/img/mnu/msel.png");
				mnuTopLeft = texCreateFromPart(temp, 0, 0, 15, 15);
				mnuTopRight = texCreateFromPart(temp, 15, 0, 15, 15);
				mnuBotLeft = texCreateFromPart(temp, 0, 15, 15, 15);
				mnuBotRight = texCreateFromPart(temp, 15, 15, 15, 15);
				mnuBotShadow = texCreateFromPart(temp, 15, 15, 1, 15);

                clearClr = clrCreateU32(0xFFEBEBEB);
                // clearClr = clrCreateU32(0xFF505050);
                mnuTxt = clrCreateU32(0xFF282828);
                txtClr = clrCreateU32(0xFFCBC000);
                tboxClr = clrCreateU32(0xF0FFFFFF);
                sideRect = clrCreateU32(0xFFDCDCDC);
                divClr = clrCreateU32(0xFF2D2D2D);
				sepClr = clrCreateU32(0xFFCDCDCD);
				boundClr = clrCreateU32(0xFFFCFCFC);
                break;

            default:
            case ColorSetId_Dark:
                //Dark corners
		        temp = texLoadPNGFile("romfs:/img/mnu/tboxDrk.png");
				cornerTopHor = texCreateFromPart(temp, 12, 0, 1, 12);
				cornerBottomHor = texCreateFromPart(temp, 12, 52, 1, 12);
				cornerLeftVer = texCreateFromPart(temp, 0, 12, 12, 1);
				cornerRightVer = texCreateFromPart(temp, 52, 12, 12, 1);
				cornerTopLeft = texCreateFromPart(temp, 0, 0, 12, 12);
				cornerTopRight = texCreateFromPart(temp, 52, 0, 12, 12);
				cornerBottomLeft = texCreateFromPart(temp, 0, 52, 12, 12);
				cornerBottomRight = texCreateFromPart(temp, 52, 52, 12, 12);

                tip = texLoadPNGFile("romfs:/img/mnu/tboxTipDrk.png");

                //Light buttons
                buttonA = texLoadPNGFile("romfs:/img/button/buttonA_lght.png");
                buttonB = texLoadPNGFile("romfs:/img/button/buttonB_lght.png");
                buttonX = texLoadPNGFile("romfs:/img/button/buttonX_lght.png");
                buttonY = texLoadPNGFile("romfs:/img/button/buttonY_lght.png");
                buttonM = texLoadPNGFile("romfs:/img/button/buttonM_lght.png");
                buttonP = texLoadPNGFile("romfs:/img/button/buttonP_lght.png");
	
                icn = texLoadPNGFile("romfs:/img/icn/icnLght.png");
                mnuGrad = texLoadPNGFile("romfs:/img/mnu/gradDrk.png");

				temp = texLoadPNGFile("romfs:/img/mnu/msel.png");
				mnuTopLeft = texCreateFromPart(temp, 0, 0, 8, 8);
				mnuTopRight = texCreateFromPart(temp, 8, 0, 8, 8);
				mnuBotLeft = texCreateFromPart(temp, 0, 8, 8, 12);
				mnuBotRight = texCreateFromPart(temp, 8, 8, 8, 12);
				mnuBotShadow = texCreateFromPart(temp, 8, 11, 1, 11);

                clearClr = clrCreateU32(0xFF2D2D2D);
                mnuTxt = clrCreateU32(0xFFFFFFFF);
                txtClr = clrCreateU32(0xFFFDBD1B);
                tboxClr = clrCreateU32(0xF04F4F4F);
                sideRect = clrCreateU32(0xFF373737);
                divClr = clrCreateU32(0xFFFFFFFF);
				sepClr = clrCreateU32(0xFFCDCDCD);
				boundClr = clrCreateU32(0xFF27221F);
                break;
        }

        if(fs::fileExists(fs::getWorkDir() + "font.ttf"))
            shared = fontLoadTTF(std::string(fs::getWorkDir() + "font.ttf").c_str());
        else
            shared = fontLoadSharedFonts();

        //setupSelButtons();
        setupNavButtons();

        if(fs::fileExists(fs::getWorkDir() + "back.jpg"))
        {
            background = texLoadJPEGFile(std::string(fs::getWorkDir() + "back.jpg").c_str());
            //Fake alpha Rects
            fldSide = texCreateFromPart(background, 0, 88, 410, 559);
            clr tempRect = sideRect;
            tempRect.a = 0xAA;
            drawRectAlpha(fldSide, 0, 0, 410, 559, tempRect);

            txtSide = texCreateFromPart(background, 0, 88, 448, 559);
            drawRectAlpha(txtSide, 0, 0, 448, 559, tempRect);
        }
    }

    void exit()
    {
        texDestroy(screen);
        texDestroy(cornerTopHor);
        texDestroy(cornerBottomHor);
        texDestroy(cornerLeftVer);
        texDestroy(cornerRightVer);
        texDestroy(cornerTopLeft);
        texDestroy(cornerTopRight);
        texDestroy(cornerBottomLeft);
        texDestroy(cornerBottomRight);
        texDestroy(tip);
        texDestroy(iconSel);
        texDestroy(menuSel);
        texDestroy(buttonSel);
        texDestroy(buttonLrg);

        texDestroy(popupTopLeft);
        texDestroy(popupTopRight);
        texDestroy(popupBotLeft);
        texDestroy(popupBotRight);
        texDestroy(popupBotShadow);

        texDestroy(popupButTopLeft);
        texDestroy(popupButTopRight);
        texDestroy(popupButBotLeft);
        texDestroy(popupButBotRight);

        texDestroy(mnuTopLeft);
        texDestroy(mnuTopRight);
        texDestroy(mnuBotLeft);
        texDestroy(mnuBotRight);
        texDestroy(mnuBotShadow);

        texDestroy(buttonA);
        texDestroy(buttonB);
        texDestroy(buttonX);
        texDestroy(buttonY);
        texDestroy(buttonM);
        texDestroy(buttonP);

        texDestroy(mnuGrad);
        texDestroy(temp);

        if(background != NULL)
            texDestroy(background);
        if(fldSide != NULL)
            texDestroy(fldSide);
        if(txtSide != NULL)
            texDestroy(txtSide);

        fontDestroy(shared);
    }

    // void setupSelButtons()
    // {
        // int x = 93, y = 187;
        // for(int i = 0; i < 12; y += 184)
        // {
            // int endRow = i + 6;
            // for(int tX = x; i < endRow; tX += 184, i++)
            // {
                // Make a new button with no text. We're not drawing them anyway
                // ui::button newSelButton("", tX, y, 174, 174);
                // selButtons.push_back(newSelButton);
            // }
        // }
    // }

    void setupNavButtons()
    {
        //User Select
        // int startX = 754;
        // ui::button sel("", startX, 656, 110, 64);
        // ui::button dmp("", startX += 110, 656, 134, 64);
        // ui::button cls("", startX += 134, 656, 110, 64);
        // ui::button ex("", startX += 110, 656, 110, 64);
        // usrNav.push_back(sel);
        // usrNav.push_back(dmp);
        // usrNav.push_back(cls);
        // usrNav.push_back(ex);

        //Title
        // startX = 804;
        // ui::button ttlSel("", startX, 656, 110, 64);
        // ui::button ttlDmp("", startX += 110, 656, 134, 64);
        // ui::button ttlBlk("", startX += 134, 656, 110, 64);
        // ui::button ttlBck("", startX += 110, 656, 110, 64);
        // ttlNav.push_back(ttlSel);
        // ttlNav.push_back(ttlDmp);
        // ttlNav.push_back(ttlBlk);
        // ttlNav.push_back(ttlBck);

        //Folder. Skip adv since it can't be touch controlled
        // startX = 800;
        // ui::button fldBackup("", startX, 656, 110, 64);
        // ui::button fldRestor("", startX += 110, 656, 110, 64);
        // ui::button fldDelete("", startX += 110, 656, 110, 64);
        // ui::button fldBack("", startX += 110, 672, 110, 64);
        // fldNav.push_back(fldBackup);
        // fldNav.push_back(fldRestor);
        // fldNav.push_back(fldDelete);
        // fldNav.push_back(fldBack);
    }

    void drawUI()
    {
        if(background == NULL)
            texClearColor(frameBuffer, clearClr);
        else
            texDrawNoAlpha(background, frameBuffer, 0, 0);

        texDraw(icn, frameBuffer, 66, 27);
        drawText("JKSV", frameBuffer, shared, 130, 38, 24, mnuTxt);
        //drawText(VER_STRING, frameBuffer, shared, 8, 702, 12, mnuTxt);
        drawRect(frameBuffer, 30, 87, 1220, 1, divClr);
        drawRect(frameBuffer, 30, 647, 1220, 1, divClr);

        switch(mstate)
        {
            case FLD_SEL:
				{
					if(fldSide == NULL)
						texDraw(mnuGrad, frameBuffer, 0, 88);
					else
						texDraw(fldSide, frameBuffer, 30, 88);
                }
                break;
        }
    }

    void runApp(const uint64_t& down, const uint64_t& held, const touchPosition& p)
    {
        //Draw first. Shouldn't, but it simplifies the showX functions
        drawUI();

        switch(mstate)
        {
            case USR_SEL:
                updateUserMenu(down, held, p);
                break;

            case TTL_SEL:
                updateTitleMenu(down, held, p);
                break;

            case BKL_SEL:
                updateBlacklistMenu(down, held, p);
                break;

            case FLD_SEL:
                updateFolderMenu(down, held, p);
                break;
        }
    }
}
