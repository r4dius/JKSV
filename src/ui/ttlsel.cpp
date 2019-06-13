#include <string>
#include <fstream>
#include <vector>

#include "ui.h"
#include "uiupdate.h"
#include "file.h"
#include "util.h"

extern std::vector<ui::button> ttlNav;

namespace ui
{
	void updateTitleMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
	{
		//Static so they don't get reset every loop
		//Where to start in titles, selected title
		static int start = 0, selected = 0, maxTitles = 18, movespeed = 0;
		static bool move = false;

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true;

		std::vector<ui::button> selButtons;
		static ui::touchTrack track;
		unsigned x = 93, y = 187;
		static unsigned tiX = 0, tiY = 0;

		//Selected rectangle X and Y.
		static unsigned selRectX = x, selRectY = y;
		static std::string title = "";
		static int retEvent = MENU_NOTHING;

		bool updatemenu = false;
		static bool swiping = false;

		ttlNav.clear();

		unsigned list_size = data::curUser.titles.size();

		if(maxTitles == 24)
			y = 3;

		unsigned endTitle = start + maxTitles;
		if(start + maxTitles > (int)list_size)
			endTitle = list_size;

		for(unsigned i = start; i < endTitle; y += 184)
		{
			unsigned endRow = i + 6;
			for(unsigned tX = x; i < endRow; i++, tX += 184)
			{
				if(i == endTitle)
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
				data::curUser.titles[i].icon.drawResize(tX, y, 174, 174);
				ui::button newSelButton("", tX, y, 174, 174);
				selButtons.push_back(newSelButton);
			}
		}

		unsigned endX = 1218, butSize = 0;
		std::string butTxt = "Select";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonA, frameBuffer, endX -= 38, 672);
		ui::button ttlSel("", endX, 656, butSize + 38, 64);
		ttlNav.push_back(ttlSel);
		endX -= 41;
		butTxt = "Back";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonB, frameBuffer, endX -= 38, 672);
		ui::button ttlBck("", endX, 656, butSize + 38, 64);
		ttlNav.push_back(ttlBck);
		endX -= 41;
		if(list_size > 0)
		{
			butTxt = "Blacklist";
			butSize = textGetWidth(butTxt.c_str(), shared, 18);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
			texDraw(buttonX, frameBuffer, endX -= 38, 672);
			ui::button ttlBlk("", endX, 656, butSize + 38, 64);
			ttlNav.push_back(ttlBlk);
			endX -= 41;
			butTxt = "Backup All";
			butSize = textGetWidth(butTxt.c_str(), shared, 18);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
			texDraw(buttonY, frameBuffer, endX -= 38, 672);
			ui::button ttlDmp("", endX, 656, butSize + 38, 64);
			ttlNav.push_back(ttlDmp);
			endX -= 41;
		}
		butTxt = "View Blacklisted";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonM, frameBuffer, endX -= 38, 672);
		ui::button ttlBll("", endX, 656, butSize + 38, 64);
		ttlNav.push_back(ttlBll);
		endX -= 41;
		butTxt = "Exit";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonP, frameBuffer, endX -= 38, 672);
		ui::button ttlExt("", endX, 656, butSize + 38, 64);
		ttlNav.push_back(ttlExt);

		memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);

		while(true)
		{
			hidScanInput();
			uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
			uint64_t held = hidKeysHeld(CONTROLLER_P1_AUTO);
			touchPosition p;
			hidTouchRead(&p, 0);

			if(clrAdd)
			{
				clrSh += 5;
				if(clrSh > 100)
				{
					if(clrSh > 254) clrSh = 254;
					clrAdd = false;
				}
			}
			else
			{
				clrSh -= 10;
				if(clrSh <= 0)
				{
					if(clrSh < 0) clrSh = 0;
					clrAdd = true;
				}
			}

			if((held & KEY_RIGHT) || (held & KEY_LEFT) || (held & KEY_UP) || (held & KEY_DOWN))
				movespeed++;
			else
			{
				movespeed = 0;
				move = false;
			}

			if(movespeed >= 10)
			{
				move = true;
				movespeed = 12;
			} else move = false;

			//Update touchtracking
			track.update(p); 
			switch(track.getEvent()) 
			{
				case TRACK_SWIPE_UP:
					if(start + 18 < (int)list_size)
					{
						swiping = true;
						selected += 6;
						if(selected > (int)list_size - 1)
							selected = list_size - 1;

						if(maxTitles == 24)
							start += 6;

						if((int)list_size > 12)
							maxTitles = 24;

						updatemenu = true;
						return;
					}
					break;

				case TRACK_SWIPE_DOWN:
					if(maxTitles != 18)
					{
						swiping = true;
						selected -= 6;
						if(selected < 0)
							selected = 0;

						start -= 6;
						if(start < 0)
						{
							start = 0;
							maxTitles = 18;
						}

						updatemenu = true;
						return;
					}
					break;
			}

			//Update nav
			for(unsigned i = 0; i < ttlNav.size(); i++)
				ttlNav[i].update(p);

			//Update invisible buttons
			for(int i = 0; (unsigned)i < endTitle - start; i++)
			{
				selButtons[i].update(p);
				if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
				{
					if(!swiping) {
						data::curData = data::curUser.titles[selected];
						if(fs::mountSave(data::curUser, data::curData))
						{
							util::makeTitleDir(data::curUser, data::curData);
							folderMenuInfo = util::getInfoString(data::curUser, data::curData);

							mstate = FLD_SEL;
							prevState = TTL_SEL;
						}

						retEvent = MENU_DOUBLE_REL;
						break;
					}
				}
				else if(selButtons[i].getEvent() == BUTTON_RELEASED)
				{
					if(!swiping)
					{
						if(start + i < (int)list_size)
							selected = start + i;

						retEvent = MENU_NOTHING;
						updatemenu = true;
						
						if(maxTitles == 24)
						{
							if(selected < start + 6)
							{
								start -= 6;
								if(start < 0) {
									start = 0;
									maxTitles = 18;
								}

								updatemenu = true;
								return;
							}
							else if(selected >= start + 18)
							{
								start += 6;

								updatemenu = true;
								return;
							}
						}
						else if(selected >= start + 12)
						{
							maxTitles = 24;

							updatemenu = true;
							return;
						}
					}
				}
				else
				{
					retEvent = MENU_NOTHING;
				}
			}

			// reset swiping check
			if(swiping && hidTouchCount() <= 0)
				swiping = false;

			gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			if(list_size > 0)
			{
				drawGlowButton(selRectX, selRectY, 174, 174, clrSh, BUTTON_ICON, 2);
				drawTitlebox(title, tiX, tiY - 63, 48);
			}
			else
			{
				std::string message = "No saved game, please have a look in the blacklist";
				drawText(message.c_str(), frameBuffer, ui::shared, (1280 - textGetWidth(message.c_str(), ui::shared, 22)) / 2, 340, 22, mnuTxt);
			}
			gfxEndFrame(ui::shared);

			if(updatemenu == true)
				break;

			if(list_size > 0)
			{
				if(down & KEY_RIGHT || ((held & KEY_RIGHT) && move))
				{
					if(selected < (int)list_size - 1)
						selected++;

					if(selected >= (int)start + 18)
						start += 6;

					if(start < 0)
						start = 0;

					if(selected == 12)
						maxTitles = 24;
					break;
				}
				else if(down & KEY_LEFT || ((held & KEY_LEFT) && move))
				{
					if(selected > 0)
						selected--;

					if(selected - 6 < (int)start)
						start -= 6;

					if(start < 0)
						start = 0;

					if(selected == 5)
						maxTitles = 18;
					break;
				}
				else if(down & KEY_UP || ((held & KEY_UP) && move))
				{
					selected -= 6;
					if(selected < 0)
						selected = 0;

					if(selected - 6 < start)
						start -= 6;

					if(start < 0)
						start = 0;

					if(selected >= 0 && selected < 6)
						maxTitles = 18;
					break;
				}
				else if(down & KEY_DOWN || ((held & KEY_DOWN) && move))
				{
					selected += 6;
					if(selected > (int)list_size - 1)
						selected = list_size - 1;

					if(selected - start >= 18)
						start += 6;

					if(selected > 11 && selected < 18)
						maxTitles = 24;
					break;
				}
				else if(down & KEY_A || ttlNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
				{
					data::curData = data::curUser.titles[selected];
					if(fs::mountSave(data::curUser, data::curData))
					{
						util::makeTitleDir(data::curUser, data::curData);
															
						folderMenuInfo = util::getInfoString(data::curUser, data::curData);

						mstate = FLD_SEL;
						prevState = TTL_SEL;
					}
					break;
				}
				else if(down & KEY_X || ttlNav[2].getEvent() == BUTTON_RELEASED)
				{
					std::string confStr = "Are you sure you want to add \"" + data::curUser.titles[selected].getTitle() + \
										  "\" to your blacklist? It will apply to all users";
					if(ui::confirm(confStr, "Blacklist")) {
						data::blacklistAdd(data::curUser, data::curUser.titles[selected]);
						// deleting last icon
						if((unsigned)selected == list_size - 1)
						{
							if(selected > 0)
								selected--;

							if(selected - 6 < (int)start)
								start -= 6;

							if(start < 0)
								start = 0;

							if(selected == 5)
								maxTitles = 18;
						}
					}
					break;
				}
				else if(down & KEY_Y || ttlNav[3].getEvent() == BUTTON_RELEASED)
				{
					if(confirm("Are you sure you want to backup all saves?", "Backup"))
					{
						fs::dumpAllUserSaves(data::curUser);
					}
					break;
				}
			}
			if(down & KEY_B || ttlNav[1].getEvent() == BUTTON_RELEASED)
			{
				start = 0;
				selected = 0;
				maxTitles = 18;
				selRectX = 93, selRectY = 187;
				mstate = USR_SEL;
				return;
			}
			else if(down & KEY_MINUS || ttlNav[ttlNav.size() - 2].getEvent() == BUTTON_RELEASED)
			{
				mstate = BKL_SEL;
				break;
			}
			else if(down & KEY_PLUS || ttlNav[ttlNav.size() - 1].getEvent() == BUTTON_RELEASED)
			{
				ui::finish = true;
				break;
			}
		}
/*
		char char_arr[200];
		sprintf(char_arr, "selected %d", selected);
		drawText(char_arr, frameBuffer, ui::shared, 500, 10, 14, txtClr);
		sprintf(char_arr, "endTitle %d", endTitle);
		drawText(char_arr, frameBuffer, ui::shared, 500, 25, 14, txtClr);
		sprintf(char_arr, "start %d", start);
		drawText(char_arr, frameBuffer, ui::shared, 500, 40, 14, txtClr);
*/
	}
}
