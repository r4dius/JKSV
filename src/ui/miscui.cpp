#include <cstring>
#include <math.h>
#include <switch.h>

#include "gfx.h"
#include "ui.h"
#include "miscui.h"
#include "util.h"
#include "snd.h"

namespace ui
{
	static int selected = 1;

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
		drawText(head.c_str(), frameBuffer, shared, 325, 297, 19, mnutxtClr);
		drawTextWrap(text.c_str(), frameBuffer, shared, 325, 333, 19, mnutxtClr, 630);

		drawRect(frameBuffer, 325, 451, 630, 10, clrCreateU32(0xFFCCCCCC));
		drawRect(frameBuffer, 325, 451, (unsigned)width, 10, glowClr);

		char tmp[128];
		sprintf(tmp, "%lu KB/%lu KB", prog / 1024, max / 1024);
		int szX = 640 - (textGetWidth(tmp, shared, 19) / 2);

		drawText(tmp, frameBuffer, shared, szX, 416, 19, mnutxtClr);
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
		cur = p;

		//If button was first thing pressed
		if(isOver() && prev.px == 0 && prev.py == 0)
		{
			if(!pressed)
				sndPlay(SND_LIST);
			pressed = true;
			retEvent = BUTTON_PRESSED;
		}
		else if(retEvent == BUTTON_PRESSED && hidTouchCount() == 0 && wasOver())
		{
			pressed = false;
			retEvent = BUTTON_RELEASED;
		}
		else if(retEvent != BUTTON_NOTHING && hidTouchCount() == 0)
		{
			pressed = false;
			retEvent = BUTTON_NOTHING;
		}
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
			drawRectAlpha(frameBuffer, x, y, w, h, butbgovrClr);
			drawText(text.c_str(), frameBuffer, ui::shared, tx, ty, fontsize, buttxtClr);
		}
		else
			drawText(text.c_str(), frameBuffer, ui::shared, tx, ty, fontsize, buttxtClr);
	}

	void touchTrack::update(const touchPosition& p, int multiplier)
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

				if(avY <= -multiplier)
					retTrack = TRACK_SWIPE_UP;
				else if(avY >= multiplier)
					retTrack = TRACK_SWIPE_DOWN;
				else if(retTrack <= -multiplier)
					retTrack = TRACK_SWIPE_LEFT;
				else if(retTrack >= multiplier)
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
		memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);

		button ok("OK", 255, 461, 770, 70);

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true, touched = false;

		while(true)
		{
			hidScanInput();
			uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
			touchPosition p;
			hidTouchRead(&p, 0);

			if(hidTouchCount() > 0)
				touched = true;

			ok.update(p);

			if(clrAdd)
			{
				clrSh += 5;
				if(clrSh > 100)
				{
					if(clrSh > 254)
						clrSh = 254;

					clrAdd = false;
				}
			}
			else
			{
				clrSh -= 10;
				if(clrSh <= 0)
				{
					if(clrSh < 0)
						clrSh = 0;

					clrAdd = true;
				}
			}

			if(touched && (down & KEY_A || down & KEY_RIGHT || down & KEY_UP || down & KEY_DOWN))
				touched = false;
			else
			{
				if(down & KEY_A || down & KEY_B || ok.getEvent() == BUTTON_RELEASED)
				{
					sndPlay(SND_SELECT);
					break;
				}
			}

			gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			drawTextPopupBg(255, 189, 770, 342);
			
			int top = 191 + 266 / 2 - textGetHeight(mess.c_str(), ui::shared, 19, 630) / 2;
			drawTextWrap(mess.c_str(), frameBuffer, ui::shared, 325, top, 19, mnutxtClr, 630);
			drawRectAlpha(frameBuffer, 255, 459, 770, 2, clrCreateU32(0x64A0A0A0));

			if(!touched)
				drawGlowButton(255, 461, 770, 70, clrSh, BUTTON_POPUP, 0);

			ok.draw();
			gfxEndFrame(ui::shared);
		}

		touched = false;
	}

	bool confirm(const std::string& mess, const std::string& buttontext)
	{
		memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);

		button no("Cancel", 255, 461, 384, 70);
		button yes(buttontext, 641, 461, 384, 70);

		bool ret = false;
		//Color shift for rect
		static int clrSh = 0, bX = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true, touched = false;
			
		while(true)
		{
			hidScanInput();
			uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
			touchPosition p;
			hidTouchRead(&p, 0);
			
			bX = 255 + 386 * selected;

			if(hidTouchCount() > 0)
				touched = true;

			yes.update(p);
			no.update(p);

			if(no.getEvent() == BUTTON_PRESSED)
				selected = 0;
			if(yes.getEvent() == BUTTON_PRESSED)
				selected = 1;

			if(touched && (down & KEY_A || down & KEY_LEFT || down & KEY_RIGHT || down & KEY_UP || down & KEY_DOWN))
				touched = false;
			else
			{
				if(down & KEY_A || yes.getEvent() == BUTTON_RELEASED)
				{
					if(selected == 0)
						ret = false;
					else
						ret = true;

					sndPlay(SND_SELECT);
					break;
				}
				else if(down & KEY_B || no.getEvent() == BUTTON_RELEASED)
				{
					ret = false;

					sndPlay(SND_SELECT);
					break;
				}
				else if(down & KEY_LEFT || no.getEvent() == BUTTON_RELEASED)
				{
					if(selected == 1)
					{
						selected = 0;
						sndPlay(SND_TICK);
					}
					else
						sndPlay(SND_BOUNDS);
				}
				else if(down & KEY_RIGHT || yes.getEvent() == BUTTON_RELEASED)
				{
					if(selected == 0)
					{
						selected = 1;
						sndPlay(SND_TICK);
					}
					else
						sndPlay(SND_BOUNDS);
				}
				else if(down & KEY_UP || down & KEY_DOWN)
					sndPlay(SND_BOUNDS);
			}

			if(clrAdd)
			{
				clrSh += 5;
				if(clrSh > 100)
				{
					if(clrSh > 254)
						clrSh = 254;

					clrAdd = false;
				}
			}
			else
			{
				clrSh -= 10;
				if(clrSh <= 0)
				{
					if(clrSh < 0)
						clrSh = 0;

					clrAdd = true;
				}
			}

			gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			drawTextPopupBg(255, 189, 770, 342);
			
			int top = 191 + 266 / 2 - textGetHeight(mess.c_str(), ui::shared, 19, 630) / 2;
			drawTextWrap(mess.c_str(), frameBuffer, ui::shared, 325, top, 19, mnutxtClr, 630);
			drawRectAlpha(frameBuffer, 255, 459, 770, 2, clrCreateU32(0x64A0A0A0));
			drawRectAlpha(frameBuffer, 639, 461, 2, 70, clrCreateU32(0x64A0A0A0));

			if(!touched)
				drawGlowButton(bX, 461, 384, 70, clrSh, BUTTON_POPUP, 0);

			no.draw();
			yes.draw();
			gfxEndFrame(ui::shared);
		}

		selected = 1;
		touched = false;
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
		texDrawHorizontal(ui::cornerTopHor, frameBuffer, rectX - 5 + 12, y - 5, w - 24 + 10);
		texDraw(ui::cornerTopRight, frameBuffer, rectX + w - 12 + 5, y - 5);

		//Middle
		texDrawVertical(ui::cornerLeftVer, frameBuffer, rectX - 5, y - 5 + 12, h - 24 + 10);
		drawRectAlpha(frameBuffer, rectX - 5 + 12, y - 5 + 12, w - 24 + 10, h - 24 + 10, tipbgClr);
		texDrawVertical(ui::cornerRightVer, frameBuffer, rectX + w - 12 + 5, y - 5 + 12, h - 24 + 10);

		//Bottom
		texDraw(ui::cornerBottomLeft, frameBuffer, rectX - 5, y + h - 12 + 5);
		texDrawHorizontal(ui::cornerBottomHor, frameBuffer, rectX - 5 + 12, y + h - 12 + 5, w - 24 + 10);
		texDraw(ui::cornerBottomRight, frameBuffer, rectX + w - 12 + 5, y + h - 12 + 5);
		
		//Tip
		texDraw(ui::tip, frameBuffer, x + 87 - 11, y + h);
		drawText(title.c_str(), frameBuffer, ui::shared, textX + 28, y + 15, fontSize, tiptxtClr);
	}

	void drawTextPopup(int x, int y, int w, int h)
	{
		//Draw shadow first
		texDraw(iconSelShadowLeft, frameBuffer, x, y + h - 5);
		texDrawHorizontal(iconSelShadowMiddle, frameBuffer, x + 10, y + h - 5, w - 20);
		texDraw(iconSelShadowRight, frameBuffer, x + w - 10, y + h - 5);

		//Top
		texDraw(ui::popupTopLeft, frameBuffer, x, y);
		drawRect(frameBuffer, x + 15, y, w - 30, 15, popupClr);
		texDraw(ui::popupTopRight, frameBuffer, x + w - 15, y);

		//middle
		drawRect(frameBuffer, x, y + 15, w, h - 30, popupClr);

		//bottom
		texDraw(ui::popupBotLeft, frameBuffer, x, y + h - 15);
		drawRect(frameBuffer, x + 15, y + h - 15, w - 30, 15, popupClr);
		texDraw(ui::popupBotRight, frameBuffer, x + w - 15, y + h - 15);
	}

	void drawTextPopupBg(int x, int y, int w, int h)
	{
		drawRectAlpha(frameBuffer, 0, 0, 1280, 720, popupbgClr);
		drawTextPopup(x, y, w, h);
	}

	void drawGlowButton(int x, int y, int w, int h, int clrSh, int type, int offset)
	{
		uint8_t glowR = glowClr.r;
		uint8_t glowG = glowClr.g;
		uint8_t glowB = glowClr.b;
		
		int newR = glowR - clrSh; if(newR < 0) newR = 0; if(newR > 254) newR = 254;
		int newG = glowG - clrSh; if(newG < 0) newG = 0; if(newG > 254) newG = 254;
		int newB = glowB - clrSh / 2; if(newB < 0) newB = 0; if(newB > 254) newB = 254;

		clr rectClr = clrCreateRGBA(newR, newG, newB, 0xFF);

		switch(type)
		{
			case BUTTON_ICON:
			case BUTTON_LIST:
				if(type == BUTTON_LIST)
					drawRect(frameBuffer, x - offset, y - offset, w + offset * 2, h + offset * 2, glowbgClr);

				texSwapColors(iconSelTopLeft, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);
				texSwapColors(iconSelTopRight, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);
				texSwapColors(iconSelBotLeft, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);
				texSwapColors(iconSelBotRight, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);

				//Draw shadow first
				texDraw(iconSelShadowLeft, frameBuffer, x - 5 - offset, y + h + 3 + offset);
				texDrawHorizontal(iconSelShadowMiddle, frameBuffer, x - offset, y + h + 3 + offset, w + offset * 2);
				texDraw(iconSelShadowRight, frameBuffer, x + w + offset, y + h + 3 + offset);

				//Top
				texDraw(iconSelTopLeft, frameBuffer, x - 5 - offset, y - 5 - offset);
				drawRect(frameBuffer, x - offset + 10, y - 5 - offset, w + offset * 2 - 20, 5, rectClr);
				drawRect(frameBuffer, x - offset, y - offset, w + offset * 2, offset, glowbgClr);
				texDraw(iconSelTopRight, frameBuffer, x + w + offset - 10, y - 5 - offset);

				//Left
				drawRect(frameBuffer, x - 5 - offset, y - offset + 10, 5, h + offset * 2 - 20, rectClr);
				drawRect(frameBuffer, x - offset, y - offset, offset, h + offset * 2, glowbgClr);

				//Right
				drawRect(frameBuffer, x + w, y - offset, offset, h + offset * 2, glowbgClr);
				drawRect(frameBuffer, x + w + offset, y - offset + 10, 5, h + offset * 2 - 20, rectClr);

				//Bottom
				texDraw(iconSelBotLeft, frameBuffer, x - 5 - offset, y + h + offset - 10);
				drawRect(frameBuffer, x - offset, y + h, w + offset * 2, offset, glowbgClr);
				drawRect(frameBuffer, x - offset + 10 , y + h + offset, w + offset * 2 - 20, 5, rectClr);
				texDraw(iconSelBotRight, frameBuffer, x + w + offset - 10, y + h + offset - 10);

				texSwapColors(iconSelTopLeft, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				texSwapColors(iconSelTopRight, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				texSwapColors(iconSelBotLeft, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				texSwapColors(iconSelBotRight, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				break;

			case BUTTON_POPUP:
				drawRect(frameBuffer, x - offset, y - offset, w + offset * 2, h + offset * 2, glowpopbgClr);

				texSwapColors(popupButTopLeft, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);
				texSwapColors(popupButTopRight, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);
				texSwapColors(popupButBotLeft, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);
				texSwapColors(popupButBotRight, clrCreateRGBA(glowR, glowG, glowB, 0xFF), rectClr);

				//Top
				texDraw(popupButTopLeft, frameBuffer, x - 5 - offset, y - 5 - offset);
				drawRect(frameBuffer, x - offset + 10, y - 5 - offset, w + offset * 2 - 20, 5, rectClr);
				drawRect(frameBuffer, x - offset, y - offset, w + offset * 2, offset, glowpopbgClr);
				texDraw(popupButTopRight, frameBuffer, x + w + offset - 10, y - 5 - offset);

				//Left
				drawRect(frameBuffer, x - 5 - offset, y - offset + 10, 5, h + offset * 2 - 20, rectClr);
				drawRect(frameBuffer, x - offset, y - offset, offset, h + offset * 2, glowpopbgClr);

				//Right
				drawRect(frameBuffer, x + w, y - offset, offset, h + offset * 2, glowpopbgClr);
				drawRect(frameBuffer, x + w + offset, y - offset + 10, 5, h + offset * 2 - 20, rectClr);

				//Bottom
				texDraw(popupButBotLeft, frameBuffer, x - 5 - offset, y + h + offset - 10);
				drawRect(frameBuffer, x - offset, y + h, w + offset * 2, offset, glowpopbgClr);
				drawRect(frameBuffer, x - offset + 10 , y + h + offset, w + offset * 2 - 20, 5, rectClr);
				texDraw(popupButBotRight, frameBuffer, x + w + offset - 10, y + h + offset - 10);

				texSwapColors(popupButTopLeft, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				texSwapColors(popupButTopRight, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				texSwapColors(popupButBotLeft, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));				
				texSwapColors(popupButBotRight, rectClr, clrCreateRGBA(glowR, glowG, glowB, 0xFF));		
				break;
		}
	}

	void drawScrollBar(int start, int maxTitles, int count, int type)
	{
		int line_count = 0, min_count = 0, y = 0;
		float line_view = 0;

		if (type == SCROLL_LIST)
		{
			count++;
			line_count = 1;
			// how much "lines" are visible per screen
			line_view = 7;
			min_count = 7;
		}
		else
		{
			line_count = 6;
			// how much "lines" are visible per screen
			line_view = 2.5;
			min_count = 12;
		}

		if(count <= min_count)
			return;

		int h = 553 * line_view / ceil((float)count / line_count);
		double step_h = (553 - h) / ceil(((float)count - min_count) / line_count);

		if (type == SCROLL_LIST)
			y = 91 + step_h * start;
		else
			y = 91 + step_h * (start == 0 ? (maxTitles == 24 ? 1 : 0) : start / line_count + 1);

		if(h + y > 553 + 91)
			y = 553 + 91 - h;

		drawRectAlpha(frameBuffer, 1263, y, 4, h, thatthingontherightClr);
	}
}
