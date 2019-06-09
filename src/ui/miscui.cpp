#include <cstring>
#include <switch.h>

#include "gfx.h"
#include "ui.h"
#include "miscui.h"
#include "util.h"

namespace ui
{
    progBar::progBar(const uint64_t& _max)
    {
        max = _max;
    }

    void progBar::update(const uint64_t& _prog)
    {
        prog = _prog;

        width = (float)(((float)prog / (float)max) * 630);
    }

    void progBar::draw(const std::string& text, const std::string& head)
    {
        drawTextPopup(255, 189, 770, 342);
		drawText(head.c_str(), frameBuffer, shared, 325, 297, 19, mnuTxt);
		drawTextWrap(text.c_str(), frameBuffer, shared, 325, 333, 19, mnuTxt, 630);

        drawRect(frameBuffer, 325, 448, 630, 14, clrCreateU32(0xFFCCCCCC));
        drawRect(frameBuffer, 325, 448, (unsigned)width, 14, txtClr);

        char tmp[128];
        sprintf(tmp, "%lu KB/%lu KB", prog / 1024, max / 1024);
        int szX = 640 - (textGetWidth(tmp, shared, 19) / 2);

        drawText(tmp, frameBuffer, shared, szX, 416, 19, mnuTxt);
    }

    button::button(const std::string& _txt, unsigned _x, unsigned _y, unsigned _w, unsigned _h)
    {
        x = _x;
        y = _y;
        w = _w;
        h = _h;
        text = _txt;
		fontsize = 21;

        unsigned tw = textGetWidth(text.c_str(), ui::shared, fontsize);
        unsigned th = 24;

        tx = x + (w / 2) - (tw / 2);
        ty = y + (h / 2) - (th / 2);
    }

    void button::update(const touchPosition& p)
    {
        prev = cur;
        cur  = p;

        //If button was first thing pressed
        if(isOver() && prev.px == 0 && prev.py == 0)
        {
            first = true;
            pressed = true;
            retEvent = BUTTON_PRESSED;
        }
        else if(retEvent == BUTTON_PRESSED && hidTouchCount() == 0 && wasOver())
        {
            first = false;
            pressed = false;
            retEvent = BUTTON_RELEASED;
        }
        else if(retEvent != BUTTON_NOTHING && hidTouchCount() == 0)
        {
            first = false;
            pressed = false;
            retEvent = BUTTON_NOTHING;
        }
    }

	void drawBoundBox(int x, int y, int w, int h, int clrSh)
	{
		clr rectClr = clrCreateRGBA(0x59 - clrSh, 0xFD - clrSh, 0xDB - clrSh, 0xFF);

		texSwapColors(ui::iconSel, clrCreateRGBA(0x59, 0xFD, 0xDB, 0xFF), rectClr);
		texDraw(ui::iconSel, frameBuffer, x - 7, y - 7);
		texSwapColors(ui::iconSel, rectClr, clrCreateRGBA(0x59, 0xFD, 0xDB, 0xFF));
	}

    bool button::isOver()
    {
        return (cur.px > x && cur.px < x + w && cur.py > y && cur.py < y + h);
    }

    bool button::wasOver()
    {
        return (prev.px > x && prev.px < x + w && prev.py > y && prev.py < y + h);
    }

    void button::draw()
    {
        if(pressed)
        {
            ui::drawTextboxInvert(x, y, w, h);
            drawText(text.c_str(), frameBuffer, ui::shared, tx, ty, fontsize, blurClr);
        }
        else
        {
            drawText(text.c_str(), frameBuffer, ui::shared, tx, ty, fontsize, blurClr);
        }
    }

    void touchTrack::update(const touchPosition& p)
    {
        if(hidTouchCount() > 0)
        {
            pos[curPos++] = p;
            if(curPos == 5)
            {
                curPos = 0;

                for(unsigned i = 1; i < 5; i++)
                {
                    touchPosition c = pos[i], p = pos[i - 1];
                    avX += c.px - p.px;
                    avY += c.py - p.py;
                }

                avX /= 5;
                avY /= 5;

                if(avY <= -6)
                    retTrack = TRACK_SWIPE_UP;
                else if(avY >= 6)
                    retTrack = TRACK_SWIPE_DOWN;
                else if(retTrack <= -6)
                    retTrack = TRACK_SWIPE_LEFT;
                else if(retTrack >= 6)
                    retTrack = TRACK_SWIPE_RIGHT;
                else
                    retTrack = TRACK_NOTHING;

                std::memset(pos, 0, sizeof(touchPosition) * 5);
            }
            else
                retTrack = TRACK_NOTHING;
        }
        else
        {
            retTrack = TRACK_NOTHING;
            curPos = 0;
        }

    }

    void showMessage(const std::string& mess, const std::string& head)
    {
        button ok("OK", 255, 461, 770, 70);

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true;

        while(true)
        {
            hidScanInput();
            uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
            touchPosition p;
            hidTouchRead(&p, 0);

            ok.update(p);

			if(clrAdd)
			{
				clrSh += 8;
				if(clrSh > 100) {
					if(clrSh > 254) clrSh = 254;
					clrAdd = false;
				}
			}
			else
			{
				clrSh -= 14;
				if(clrSh <= 0) {
					if(clrSh < 0) clrSh = 0;
					clrAdd = true;
				}
			}

            if(down & KEY_A || down & KEY_B || ok.getEvent() == BUTTON_RELEASED)
                break;

            gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			drawTextPopupBg(255, 189, 770, 342);
			
			int top = 191 + 266 / 2 - textGetHeight(mess.c_str(), ui::shared, 19, 630) / 2;
			drawTextWrap(mess.c_str(), frameBuffer, ui::shared, 325, top, 19, mnuTxt, 630);
			drawRect(frameBuffer, 255, 459, 770, 2, clrCreateU32(0xFFD1D1D1));
			drawGlowElem(255, 461, 770, 70, clrSh, ui::buttonLrg, 0);
            ok.draw();
            gfxEndFrame();
        }
    }

    bool confirm(const std::string& mess, const std::string& buttontext)
    {
		memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);

        bool ret = false;
	
        button no("Cancel", 255, 461, 384, 70);
        button yes(buttontext, 641, 461, 384, 70);

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true;
			
        while(true)
        {
            hidScanInput();
            uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
            touchPosition p;
            hidTouchRead(&p, 0);

            yes.update(p);
            no.update(p);

            if(down & KEY_A || yes.getEvent() == BUTTON_RELEASED)
            {
                ret = true;
                break;
            }
            else if(down & KEY_B || no.getEvent() == BUTTON_RELEASED)
            {
                ret = false;
                break;
            }

			if(clrAdd)
			{
				clrSh += 8;
				if(clrSh > 100) {
					if(clrSh > 254) clrSh = 254;
					clrAdd = false;
				}
			}
			else
			{
				clrSh -= 14;
				if(clrSh <= 0) {
					if(clrSh < 0) clrSh = 0;
					clrAdd = true;
				}
			}

			gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			drawTextPopupBg(255, 189, 770, 342);
			
			int top = 191 + 266 / 2 - textGetHeight(mess.c_str(), ui::shared, 19, 630) / 2;
			drawTextWrap(mess.c_str(), frameBuffer, ui::shared, 325, top, 19, mnuTxt, 630);
			drawRect(frameBuffer, 255, 459, 770, 2, clrCreateU32(0xFFD1D1D1));
			drawRect(frameBuffer, 639, 461, 2, 70, clrCreateU32(0xFFD1D1D1));
			drawGlowElem(641, 461, 384, 70, clrSh, ui::buttonSel, 0);
			no.draw();
			yes.draw();
			gfxEndFrame();
        }

        return ret;
    }

    bool confirmTransfer(const std::string& f, const std::string& t)
    {
        std::string confMess = "Are you sure you want to copy #" + f + "# to #" + t +"#?";

        return confirm(confMess, "Copy");
    }

    bool confirmDelete(const std::string& p)
    {
        std::string confMess = "Are you 100% sure you want to delete #" + p + "#? *This is permanent*!";

        return confirm(confMess, "Delete");
    }

    void drawTitlebox(std::string& title, int x, int y, int h)
    {
		int fontSize = 19;
		//std::string title = data::curUser.titles[selected].getTitle();
		unsigned titleWidth = textGetWidth(title.c_str(), ui::shared, fontSize);
		int w = titleWidth + 56, rectX = (x + 87) - (w / 2);


		if(rectX < 40)
			rectX = 40;

		if(rectX + w > 1240)
			rectX = 1240 - w;
		
		int textX = rectX;

		if(w < 174) {
			w = 174;
			rectX = (x + 87) - (174 / 2);
		}

        //Top
        texDraw(ui::cornerTopLeft, frameBuffer, rectX - 5, y - 5);
		texDrawH(ui::cornerTopHor, frameBuffer, rectX - 5 + 12, y - 5, w - 24 + 10);
        texDraw(ui::cornerTopRight, frameBuffer, rectX + w - 12 + 5, y - 5);

        //middle
		texDrawV(ui::cornerLeftVer, frameBuffer, rectX - 5, y - 5 + 12, h - 24 + 10);
        drawRectAlpha(frameBuffer, rectX - 5 + 12, y - 5 + 12,  w - 24 + 10, h - 24 + 10, tboxClr);
		texDrawV(ui::cornerRightVer, frameBuffer, rectX + w - 12 + 5, y - 5 + 12, h - 24 + 10);

        //bottom
        texDraw(ui::cornerBottomLeft, frameBuffer, rectX - 5, y + h - 12 + 5);
		texDrawH(ui::cornerBottomHor, frameBuffer, rectX - 5 + 12, y + h - 12 + 5, w - 24 + 10);
        texDraw(ui::cornerBottomRight, frameBuffer, rectX + w - 12 + 5, y + h - 12 + 5);
		
		//tip
        texDraw(ui::tip, frameBuffer, x + 87 - 11, y + h);
		
		drawText(title.c_str(), frameBuffer, ui::shared, textX + 28, y + 15, fontSize, txtClr);
    }

    void drawTextPopup(int x, int y, int w, int h)
    {
        //Top
        texDraw(ui::popupTopLeft, frameBuffer, x - 5, y - 5);
		drawRect(frameBuffer, x + 10, y, w - 20, 10, popupClr);
        texDraw(ui::popupTopRight, frameBuffer, x + w - 10, y - 5);

        //middle
		drawRect(frameBuffer, x, y + 10, w, h - 20, popupClr);

        //bottom
        texDraw(ui::popupBotLeft, frameBuffer, x - 5, y + h - 10);
		texDrawH(ui::popupBotShadow, frameBuffer, x + 10, y + h - 10, w - 20);
        texDraw(ui::popupBotRight, frameBuffer, x + w - 10, y + h - 10);
    }

    void drawTextPopupBg(int x, int y, int w, int h)
    {
		drawRectAlpha(frameBuffer, 0, 0, 1280, 720, popupbgClr);
		drawTextPopup(x, y, w, h);
    }
	
    void drawPopupButton(int x, int y, int w, int h, int clrSh)
    {
		clr rectClr = clrCreateRGBA(0x59 - clrSh, 0xFD - clrSh, 0xDB - clrSh, 0xFF);

		drawRect(frameBuffer, x, y, w, h, boundClr);
		texSwapColors(ui::buttonSel, clrCreateRGBA(0x59, 0xFD, 0xDB, 0xFF), rectClr);
		texDraw(ui::buttonSel, frameBuffer, x - 5, y - 5);
		texSwapColors(ui::buttonSel, rectClr, clrCreateRGBA(0x59, 0xFD, 0xDB, 0xFF));
    }
	
	void drawGlowElem(int x, int y, int w, int h, int clrSh, tex *elem, int offset)
	{
		int R = 89 - clrSh; if(R < 0) R = 0; if(R > 254) R = 254;
		int G = 253 - clrSh; if(G < 0) G = 0; if(G > 254) G = 254;
		int B = 219 - clrSh / 2; if(B < 0) B = 0; if(B > 254) B = 254;

		clr rectClr = clrCreateRGBA(R, G, B, 0xFF);

		texSwapColors(elem, clrCreateRGBA(0x59, 0xFD, 0xDB, 0xFF), rectClr);
		texDraw(elem, frameBuffer, x - 5 - offset, y - 5 - offset);
		texSwapColors(elem, rectClr, clrCreateRGBA(0x59, 0xFD, 0xDB, 0xFF));
	}

    void drawTextboxInvert(int x, int y, int w, int h)
    {
        clr temp = ui::tboxClr;
        clrInvert(&temp);

        //Top
        texDrawInvert(ui::cornerTopLeft, frameBuffer, x, y);
        drawRect(frameBuffer, x + 32, y, w - 64, 32, temp);
        texDrawInvert(ui::cornerTopRight, frameBuffer, (x + w) - 32, y);

        //middle
        drawRect(frameBuffer, x, y + 32,  w, h - 64, temp);

        //bottom
        texDrawInvert(ui::cornerBottomLeft, frameBuffer, x, (y + h) - 32);
        drawRect(frameBuffer, x + 32, (y + h) - 32, w - 64, 32, temp);
        texDrawInvert(ui::cornerBottomRight, frameBuffer, (x + w) - 32, (y + h) - 32);
    }
}
