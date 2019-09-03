#include "ui.h"
#include "uiupdate.h"
#include "snd.h"
#include "util.h"

extern std::vector<ui::button> ttlNav;

namespace ui
{
	void updateTitleMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
	{
		static std::vector<ui::button> selButtons;

		//Static so they don't get reset every loop
		//Where to start in titles, selected title
		static int start = 0, selected = 0, listShow = 12, moveSpeed = 0;

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true;

		static ui::touchTrack track;
		unsigned x = 93, y = 187, j = 0, tX = x;
		static unsigned tiX = 0, tiY = 0;

		//Selected rectangle X and Y.
		static unsigned selRectX = x, selRectY = y;
		std::string title = "";
		int retEvent = MENU_NOTHING;

		static bool holding = false;
		static bool moving = false;
		static bool swiping = false;
		//Limit redraw of unchanged stuff
		static bool updateMenu = true;

		unsigned listSize = data::curUser.titles.size();

		if(start < 0)
			start = 0;

		if(selected < 0)
			selected = 0;

		unsigned listLast = start + listShow;
		if(listLast > listSize)
			listLast = listSize;

		if(start > 0)
		{
			for(int i = start - 6, tX = x; i < start; i++, tX += 184)
			{
				if(updateMenu)
				{
					data::curUser.titles[i].icon.drawResize(tX, 3, 174, 174);
					texDrawLimit(iconShadow, frameBuffer, tX - 5, -2);
				}
			}
		}

		for(unsigned i = start; i < listLast; y += 184, j++)
		{
			unsigned endRow = i + 6;
			for(tX = x; i < endRow; i++, tX += 184)
			{
				unsigned selH = 174;
				if(i == listLast)
					break;

				if((int)i == selected)
				{
					if(selRectX != tX || selRectY != y)
					{
						selRectX = tX;
						selRectY = y;
					}

					title = data::curUser.titles[selected].getTitle();
					tiX = tX, tiY = y;
				}

				if(updateMenu)
				{
					data::curUser.titles[i].icon.drawResize(tX, y, 174, 174);
					texDrawLimit(iconShadow, frameBuffer, tX - 5, y - 5);

					ui::button newSelButton("", tX, y, 174, selH);
					selButtons.push_back(newSelButton);
				}
			}
		}

		if(start + 12 < (int)listSize)
		{
			unsigned listBottomLast = start + 18;
			if(listBottomLast > listSize)
				listBottomLast = listSize;

			for(unsigned i = start + 12, tX = x; i < listBottomLast; i++, tX += 184)
			{
				if(updateMenu)
				{
					data::curUser.titles[i].icon.drawResize(tX, 555, 174, 174);
					texDrawLimit(iconShadow, frameBuffer, tX - 5, 550);
				}
			}
		}

		if(updateMenu)
		{
			updateMenu = false;
			ttlNav.clear();

			std::string butTxt = "";
			unsigned endX = 1218, butSize = 0;
			if(listSize > 0)
			{
				butTxt = "Select";
				butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
				drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
				texDraw(buttonA, frameBuffer, endX -= 37, 672);
				ui::button ttlSel("", endX - 20, 653, butSize + 78, 62);
				ttlNav.push_back(ttlSel);
				endX -= 41;
			}
			else
			{
				// empty button to pad B button to [1]
				ui::button ttlSel("", 0, 0, 0, 0);
				ttlNav.push_back(ttlSel);
			}
			butTxt = "Back";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonB, frameBuffer, endX -= 37, 672);
			ui::button ttlBck("", endX - 20, 653, butSize + 78, 62);
			ttlNav.push_back(ttlBck);
			endX -= 41;
			if(listSize > 0)
			{
				butTxt = "Blacklist";
				butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
				drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
				texDraw(buttonX, frameBuffer, endX -= 37, 672);
				ui::button ttlBlk("", endX - 20, 653, butSize + 78, 62);
				ttlNav.push_back(ttlBlk);
				endX -= 41;
				butTxt = "Backup All";
				butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
				drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
				texDraw(buttonY, frameBuffer, endX -= 37, 672);
				ui::button ttlDmp("", endX - 20, 653, butSize + 78, 62);
				ttlNav.push_back(ttlDmp);
				endX -= 41;
			}
			butTxt = "View Blacklisted";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonM, frameBuffer, endX -= 37, 672);
			ui::button ttlBll("", endX - 20, 653, butSize + 78, 62);
			ttlNav.push_back(ttlBll);
			endX -= 41;
			butTxt = "Exit";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonP, frameBuffer, endX -= 37, 672);
			ui::button ttlExt("", endX - 20, 653, butSize + 78, 62);
			ttlNav.push_back(ttlExt);

			drawScrollBar(start, listShow, listSize, SCROLL_ICON);

			// capture screen before adding "selected" stuff
			memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);
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

		if((held & KEY_RIGHT) || (held & KEY_LEFT) || (held & KEY_UP) || (held & KEY_DOWN))
			moveSpeed++;
		else
		{
			holding = false;
			moveSpeed = 0;
			moving = false;
		}

		if(moveSpeed >= 10)
		{
			moving = true;
			if(moveSpeed > 11)
				moveSpeed = 10;
		}
		else
			moving = false;

		//Update touchtracking
		track.update(p, 6); 
		switch(track.getEvent()) 
		{
			case TRACK_SWIPE_UP:
				if(start + 12 < (int)listSize)
				{
					swiping = true;
					selected += 6;
					if(selected > (int)listSize - 1)
						selected = listSize - 1;

					start += 6;
					updateMenu = true;
				}
				break;

			case TRACK_SWIPE_DOWN:
				if(start > 0 && (int)listSize > 12)
				{
					swiping = true;
					selected -= 6;
					if(selected < 0)
						selected = 0;

					start -= 6;

					updateMenu = true;
				}
				break;
		}

		// draw captured screen so we can overlay "dynamic" stuff
		texDraw(screen, frameBuffer, 0, 0);

		//Update nav
		for(unsigned i = 0; i < ttlNav.size(); i++)
		{
			ttlNav[i].update(p);
			if(ttlNav[i].getEvent() == BUTTON_PRESSED)
			{
				if(!swiping)
					ttlNav[i].draw();
			}
		}

		//Update invisible buttons
		for(int i = 0; (unsigned)i < listLast - start; i++)
		{
			selButtons[i].update(p);
			if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
			{
				if(!swiping)
				{
					retEvent = MENU_DOUBLE_REL;
					break;
				}
			}
			else if(selButtons[i].getEvent() == BUTTON_RELEASED)
			{
				soundPlay(SND_TOUCHOUT);
				if(!swiping)
				{
					if(start + i < (int)listSize)
						selected = start + i;

					retEvent = MENU_NOTHING;
				}
			}
			else if(selButtons[i].getEvent() == BUTTON_PRESSED)
			{
				if(!swiping)
					selButtons[i].draw();
			}
			else
			{
				retEvent = MENU_NOTHING;
			}
		}

		// reset swiping check
		if(swiping && hidTouchCount() <= 0)
			swiping = false;

		if(listSize > 0)
		{
			drawGlowButton(selRectX, selRectY, 174, 174, clrSh, BUTTON_ICON, 2);
			drawTitlebox(title, tiX, tiY - 63, 48);
		}
		else
		{
			std::string message = "No saved game, please have a look in the blacklist";
			drawText(message.c_str(), frameBuffer, ui::shared, (1280 - textGetWidth(message.c_str(), ui::shared, 22)) / 2, 340, 22, mnutxtClr);
		}

		if(listSize > 0)
		{
			if(down & KEY_RIGHT || ((held & KEY_RIGHT) && moving && moveSpeed >= 11))
			{
				if(selected < (int)listSize - 1)
				{
					selected++;
					soundPlay(SND_TICK);
				}
				else if(!holding)
				{
					holding = true;
					soundPlay(SND_BOUNDS);
				}

				if(selected >= (int)start + 12)
				{
					updateMenu = true;
					start += 6;
				}

				if(start < 0)
					start = 0;
			}
			else if(down & KEY_LEFT || ((held & KEY_LEFT) && moving && moveSpeed >= 11))
			{
				if(selected > 0)
				{
					selected--;
					soundPlay(SND_TICK);
				}
				else if(!holding)
				{
					holding = true;
					soundPlay(SND_BOUNDS);
				}

				if(selected < (int)start && start != 0)
				{
					updateMenu = true;
					start -= 6;
				}

				if(start < 0)
					start = 0;
			}
			else if(down & KEY_UP || ((held & KEY_UP) && moving && moveSpeed >= 11))
			{
				if(selected > 0)
					soundPlay(SND_TICK);
				else if(!holding)
				{
					holding = true;
					soundPlay(SND_BOUNDS);
				}

				selected -= 6;
				if(selected < 0)
					selected = 0;

				if(selected < start && start != 0)
				{
					updateMenu = true;
					start -= 6;
				}

				if(start < 0)
					start = 0;
			}
			else if(down & KEY_DOWN || ((held & KEY_DOWN) && moving && moveSpeed >= 11))
			{
				if(selected < (int)listSize - 1)
					soundPlay(SND_TICK);
				else if(!holding)
				{
					holding = true;
					soundPlay(SND_BOUNDS);
				}

				selected += 6;
				if(selected > (int)listSize - 1)
					selected = listSize - 1;

				if(selected - start >= 12)
				{
					updateMenu = true;
					start += 6;
				}
			}
			else if(down & KEY_A || ttlNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
			{
				updateMenu = true;
				soundPlay(SND_SELECT);
				data::curData = data::curUser.titles[selected];
				if(fs::mountSave(data::curUser, data::curData))
				{
					util::makeTitleDir(data::curUser, data::curData);
					folderMenuInfo = util::getInfoString(data::curUser, data::curData);

					mstate = FLD_SEL;
					prevState = TTL_SEL;
				}
			}
			else if(down & KEY_X || ttlNav[2].getEvent() == BUTTON_RELEASED)
			{
				updateMenu = true;
				soundPlay(SND_POPUP);
				std::string confStr = "Are you sure you want to add \"" + data::curUser.titles[selected].getTitle() + "\" to your blacklist? It will apply to all users";
				if(ui::confirm(confStr, "Blacklist")) {
					data::blacklistAdd(data::curUser, data::curUser.titles[selected]);
					// deleting last icon
					if((unsigned)selected == listSize - 1)
						if(selected > 0)
							selected--;

					if((int)listSize - (int)start <= 7)
					{
						updateMenu = true;
						start -= 6;
					}

					if(start < 0)
						start = 0;
				}
			}
			else if(down & KEY_Y || ttlNav[3].getEvent() == BUTTON_RELEASED)
			{
				updateMenu = true;
				soundPlay(SND_POPUP);
				if(confirm("Are you sure you want to backup all saves?", "Backup"))
				{
					soundPlay(SND_LOADING);
					fs::dumpAllUserSaves(data::curUser);
					soundPlay(SND_BING);
				}
			}
		}
		if(down & KEY_B || ttlNav[1].getEvent() == BUTTON_RELEASED)
		{
			updateMenu = true;
			soundPlay(SND_BACK);
			start = 0;
			selected = 0;
			selRectX = 93, selRectY = 187;
			mstate = USR_SEL;
		}
		else if(down & KEY_MINUS || ttlNav[ttlNav.size() - 2].getEvent() == BUTTON_RELEASED)
		{
			updateMenu = true;
			soundPlay(SND_SELECT);
			mstate = BKL_SEL;
		}
		else if(ttlNav[ttlNav.size() - 1].getEvent() == BUTTON_RELEASED)
		{
			ui::finish = true;
		}
	}
}
