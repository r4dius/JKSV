#include "util.h"

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
	clr clearClr, mnutxtClr, tipbgClr, tiptxtClr, sideRect, divClr, sepClr, popupClr, popupbgClr, buttxtClr, butbgovrClr, glowClr, glowbgClr, glowpopbgClr, thatthingontherightClr;

	//textbox pieces
	//I was going to flip them when I draw them, but then laziness kicked in.
	tex *cornerTopHor, *cornerBottomHor, *cornerLeftVer, *cornerRightVer, *cornerTopLeft, *cornerTopRight, *cornerBottomLeft, *cornerBottomRight, *tip, *temp;
	tex *popupTopLeft, *popupTopRight, *popupBotLeft, *popupBotRight, *popupShadowLeft, *popupShadowMiddle, *popupShadowRight;
	tex *iconShadow, *iconSelTopLeft, *iconSelTopRight, *iconSelBotLeft, *iconSelBotRight, *iconSelShadowLeft, *iconSelShadowMiddle, *iconSelShadowRight;
	tex *popupButTopLeft, *popupButTopRight, *popupButBotLeft, *popupButBotRight;
	tex *buttonA, *buttonB, *buttonX, *buttonY, *buttonM, *buttonP;
	tex *icn, *mnuGrad;
	font *shared;

	tex	*screen = texCreate(1280, 720);

	void init()
	{
		ColorSetId gthm;
		setsysGetColorSetId(&gthm);

		// glowR = 89;
		// glowG = 253;
		// glowB = 219;

		popupbgClr = clrCreateU32(0xB2160F05);

		switch(gthm)
		{
			case ColorSetId_Light:
				glowClr = clrCreateRGBA(90, 255, 220, 0xFF);
				glowbgClr = clrCreateU32(0xFFFCFCFC);
				glowpopbgClr = clrCreateU32(0xFFFDFDFD);

				thatthingontherightClr = clrCreateU32(0xFFC6C6C6);

				popupClr = clrCreateU32(0xFFF0F0F0);
				clearClr = clrCreateU32(0xFFEBEBEB);
				mnutxtClr = clrCreateU32(0xFF282828);

				tipbgClr = clrCreateU32(0xF0FFFFFF);
				tiptxtClr = clrCreateU32(0xFFCBC000);

				sideRect = clrCreateU32(0xFFDCDCDC);
				divClr = clrCreateU32(0xFF2D2D2D);
				sepClr = clrCreateU32(0xFFCDCDCD);
				buttxtClr = clrCreateU32(0xFFF15230);
				// butbgovrClr = clrCreateU32(0x28FFFA00);
				butbgovrClr = clrCreateU32(0x26FFFF40);

				tip = texLoadPNGFile("romfs:/img/mnu/tboxTipLght.png");
				icn = texLoadPNGFile("romfs:/img/icn/icnDrk.png");
				mnuGrad = texLoadPNGFile("romfs:/img/mnu/gradLght.png");

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

				//Dark buttons
				buttonA = texLoadPNGFile("romfs:/img/button/buttonA_drk.png");
				buttonB = texLoadPNGFile("romfs:/img/button/buttonB_drk.png");
				buttonX = texLoadPNGFile("romfs:/img/button/buttonX_drk.png");
				buttonY = texLoadPNGFile("romfs:/img/button/buttonY_drk.png");
				buttonM = texLoadPNGFile("romfs:/img/button/buttonM_drk.png");
				buttonP = texLoadPNGFile("romfs:/img/button/buttonP_drk.png");

				break;
			default:
			case ColorSetId_Dark:
				glowClr = clrCreateRGBA(145, 250, 250, 0xFF);
				glowbgClr = clrCreateU32(0xFF27221F);
				glowpopbgClr = clrCreateU32(0xFF423D3A);

				thatthingontherightClr = clrCreateU32(0xFF565656);

				popupClr = clrCreateU32(0xFF464646);
				clearClr = clrCreateU32(0xFF2D2D2D);
				mnutxtClr = clrCreateU32(0xFFFFFFFF);

				tipbgClr = clrCreateU32(0xF04F4F4F);
				tiptxtClr = clrCreateU32(0xFFFDBD1B);

				sideRect = clrCreateU32(0xFF373737);
				divClr = clrCreateU32(0xFFFFFFFF);
				sepClr = clrCreateU32(0xFF4D4D4D);
				buttxtClr = clrCreateU32(0xFFC8FF00);
				butbgovrClr = clrCreateU32(0x28FFE600);

				tip = texLoadPNGFile("romfs:/img/mnu/tboxTipDrk.png");
				icn = texLoadPNGFile("romfs:/img/icn/icnLght.png");
				mnuGrad = texLoadPNGFile("romfs:/img/mnu/gradDrk.png");

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

				//Light buttons
				buttonA = texLoadPNGFile("romfs:/img/button/buttonA_lght.png");
				buttonB = texLoadPNGFile("romfs:/img/button/buttonB_lght.png");
				buttonX = texLoadPNGFile("romfs:/img/button/buttonX_lght.png");
				buttonY = texLoadPNGFile("romfs:/img/button/buttonY_lght.png");
				buttonM = texLoadPNGFile("romfs:/img/button/buttonM_lght.png");
				buttonP = texLoadPNGFile("romfs:/img/button/buttonP_lght.png");

				break;
		}

		iconShadow = texLoadPNGFile("romfs:/img/mnu/iconShadow.png");

		temp = texLoadPNGFile("romfs:/img/mnu/iconSel.png");
		texSwapColors(temp, clrCreateRGBA(0xFF, 0xFF, 0xFF, 0xFF), glowClr);
		iconSelTopLeft = texCreateFromPart(temp, 0, 0, 15, 15);
		iconSelTopRight = texCreateFromPart(temp, 15, 0, 15, 15);
		iconSelBotLeft = texCreateFromPart(temp, 0, 15, 15, 15);
		iconSelBotRight = texCreateFromPart(temp, 15, 15, 15, 15);
		iconSelShadowLeft = texCreateFromPart(temp, 0, 30, 5, 6);
		iconSelShadowMiddle = texCreateFromPart(temp, 15, 30, 1, 6);
		iconSelShadowRight = texCreateFromPart(temp, 25, 30, 5, 6);

		temp = texLoadPNGFile("romfs:/img/mnu/popup.png");
		texSwapColors(temp, clrCreateRGBA(0xFF, 0xFF, 0xFF, 0xFF), popupClr);
		popupTopLeft = texCreateFromPart(temp, 0, 0, 15, 15);
		popupTopRight = texCreateFromPart(temp, 15, 0, 15, 15);
		popupBotLeft = texCreateFromPart(temp, 0, 15, 15, 15);
		popupBotRight = texCreateFromPart(temp, 15, 15, 15, 15);
		popupShadowLeft = texCreateFromPart(temp, 0, 30, 10, 10);
		popupShadowMiddle = texCreateFromPart(temp, 15, 30, 1, 10);
		popupShadowRight = texCreateFromPart(temp, 20, 30, 10, 10);

		temp = texLoadPNGFile("romfs:/img/mnu/popupsel.png");
		texSwapColors(temp, clrCreateRGBA(0xFF, 0xFF, 0xFF, 0xFF), glowClr);
		popupButTopLeft = texCreateFromPart(temp, 0, 0, 15, 15);
		popupButTopRight = texCreateFromPart(temp, 15, 0, 15, 15);
		popupButBotLeft = texCreateFromPart(temp, 0, 15, 15, 15);
		popupButBotRight = texCreateFromPart(temp, 15, 15, 15, 15);

		if(fs::fileExists(fs::getWorkDir() + "font.ttf"))
			shared = fontLoadTTF(std::string(fs::getWorkDir() + "font.ttf").c_str());
		else
			shared = fontLoadSharedFonts();

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

		texDestroy(iconShadow);

		texDestroy(iconSelTopLeft);
		texDestroy(iconSelTopRight);
		texDestroy(iconSelBotLeft);
		texDestroy(iconSelBotRight);
		texDestroy(iconSelShadowLeft);
		texDestroy(iconSelShadowMiddle);
		texDestroy(iconSelShadowRight);

		texDestroy(popupTopLeft);
		texDestroy(popupTopRight);
		texDestroy(popupBotLeft);
		texDestroy(popupBotRight);
		texDestroy(popupShadowLeft);
		texDestroy(popupShadowMiddle);
		texDestroy(popupShadowRight);

		texDestroy(popupButTopLeft);
		texDestroy(popupButTopRight);
		texDestroy(popupButBotLeft);
		texDestroy(popupButBotRight);

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

	void drawUI()
	{
		if(background == NULL)
			texClearColor(frameBuffer, clearClr);
		else
			texDrawNoAlpha(background, frameBuffer, 0, 0);

		texDraw(icn, frameBuffer, 66, 27);
		drawText("JKSV", frameBuffer, shared, 130, 40, 22, mnutxtClr);
		//drawText(VER_STRING, frameBuffer, shared, 8, 702, 12, mnutxtClr);
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
