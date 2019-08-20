#include <string>
#include <fstream>
#include <vector>

#include "ui.h"
#include "uiupdate.h"
#include "file.h"
#include "util.h"
#include "snd.h"

extern std::vector<ui::button> blkNav;

namespace ui
{
	void updateBlacklistMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
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
		unsigned x = 93, y = 187, j = 0, selY = 0;
		static unsigned tiX = 0, tiY = 0;

		//Selected rectangle X and Y.
		static unsigned selRectX = x, selRectY = y;
		static std::string title = "";
		static int retEvent = MENU_NOTHING;

		bool updatemenu = false;
		static bool holding = false;
		static bool swiping = false;
		static bool touching = false;

		blkNav.clear();

		unsigned list_size = data::curUser.blktitles.size();

		if(maxTitles == 24)
			y = 3;
		
		if(start < 0)
			start = 0;

		if(selected < 0)
			selected = 0;

		unsigned endTitle = start + maxTitles;
		if(start + maxTitles > (int)list_size)
			endTitle = list_size;

		for(unsigned i = start; i < endTitle; y += 184, j++)
		{
			unsigned endRow = i + 6;
			for(unsigned tX = x; i < endRow; i++, tX += 184)
			{
				unsigned selH = 174;
				if(i == endTitle)
					break;

				if((int)i == selected)
				{
					if(selRectX != tX || selRectY != y)
					{
						selRectX = tX;
						selRectY = y;
					}

					title = data::curUser.blktitles[selected].getTitle();
					tiX = tX, tiY = y;
				}
				data::curUser.blktitles[i].icon.drawResize(tX, y, 174, 174);
				texDrawLimit(iconShadow, frameBuffer, tX - 5, y - 5);
				selY = y;
				if(maxTitles == 24) {
					if(j < 1) {
						selY = y + 85;
						selH = 89;
					}
					if(j > 2)
						selH = 92;
				}
				else
					if(j > 1)
						selH = 92;
				ui::button newSelButton("", tX, selY, 174, selH);
				selButtons.push_back(newSelButton);
			}
		}

		unsigned endX = 1218, butSize = 0;
		std::string butTxt = "Select";
		butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
		texDraw(buttonA, frameBuffer, endX -= 37, 672);
		ui::button blkSel("", endX, 656, butSize + 38, 64);
		blkNav.push_back(blkSel);
		endX -= 41;
		butTxt = "Back";
		butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
		texDraw(buttonB, frameBuffer, endX -= 37, 672);
		ui::button blkBck("", endX, 656, butSize + 38, 64);
		blkNav.push_back(blkBck);
		endX -= 41;
		if(list_size > 0)
		{
			butTxt = "Unblacklist";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonX, frameBuffer, endX -= 37, 672);
			ui::button blkUnb("", endX, 656, butSize + 38, 64);
			blkNav.push_back(blkUnb);
			endX -= 41;
			butTxt = "Backup All";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonY, frameBuffer, endX -= 37, 672);
			ui::button blkDmp("", endX, 656, butSize + 38, 64);
			blkNav.push_back(blkDmp);
			endX -= 41;
		}
		butTxt = "Exit";
		butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
		texDraw(buttonP, frameBuffer, endX -= 37, 672);
		ui::button blkExt("", endX, 656, butSize + 38, 64);
		blkNav.push_back(blkExt);

		drawScrollBar(start, maxTitles, list_size, SCROLL_ICON);

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
				holding = false;
				movespeed = 0;
				move = false;
			}

			if(movespeed >= 10)
			{
				move = true;
				movespeed = 12;
			} else move = false;

			//Update touchtracking
			track.update(p, 6); 
			switch(track.getEvent()) 
			{
				case TRACK_SWIPE_UP:
					if((maxTitles == 18 && start + 12 < (int)list_size) || start + 18 < (int)list_size)
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
			for(unsigned i = 0; i < blkNav.size(); i++)
			{
				blkNav[i].update(p);
				if(blkNav[i].getEvent() == BUTTON_PRESSED)
				{
					if(!touching)
						sndPlay(SND_TICK);
					touching = true;
				}
			}

			//Update invisible buttons
			for(int i = 0; (unsigned)i < endTitle - start; i++)
			{
				selButtons[i].update(p);
				if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
				{
					if(!swiping) {
						data::curData = data::curUser.blktitles[selected];
						if(fs::mountSave(data::curUser, data::curData))
						{
							util::makeTitleDir(data::curUser, data::curData);
							folderMenuInfo = util::getInfoString(data::curUser, data::curData);

							mstate = FLD_SEL;
							prevState = BKL_SEL;
						}

						retEvent = MENU_DOUBLE_REL;
						break;
					}
				}
				else if(selButtons[i].getEvent() == BUTTON_RELEASED)
				{
					sndPlay(SND_TOUCHOUT);
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
								if(start < 0)
								{
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
				else if(selButtons[i].getEvent() == BUTTON_PRESSED)
				{
					if(!touching)
						sndPlay(SND_TICK);
					touching = true;
				}
				else
				{
					retEvent = MENU_NOTHING;
				}
			}

			if(hidTouchCount() <= 0)
				touching = false;

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
				std::string message = "No blacklisted game, see ya";
				drawText(message.c_str(), frameBuffer, ui::shared, (1280 - textGetWidth(message.c_str(), ui::shared, 22)) / 2, 340, 22, mnutxtClr);
			}

// char char_arr[200];
// sprintf(char_arr, "selected %d", selected);
// drawText(char_arr, frameBuffer, ui::shared, 500, 10, 14, mnutxtClr);
// sprintf(char_arr, "endTitle %d", endTitle);
// drawText(char_arr, frameBuffer, ui::shared, 500, 25, 14, mnutxtClr);
// sprintf(char_arr, "start %d", start);
// drawText(char_arr, frameBuffer, ui::shared, 500, 40, 14, mnutxtClr);
// sprintf(char_arr, "list_size %d", list_size);
// drawText(char_arr, frameBuffer, ui::shared, 750, 10, 14, mnutxtClr);
// sprintf(char_arr, "maxTitles %d", maxTitles);
// drawText(char_arr, frameBuffer, ui::shared, 750, 25, 14, mnutxtClr);
// sprintf(char_arr, "start %d", start);
// drawText(char_arr, frameBuffer, ui::shared, 750, 40, 14, mnutxtClr);

			gfxEndFrame(ui::shared);

			if(updatemenu == true)
				break;

			if(list_size > 0)
			{
				if(down & KEY_RIGHT || ((held & KEY_RIGHT) && move))
				{
					if(selected < (int)list_size - 1)
					{
						selected++;
						sndPlay(SND_TICK);
					}
					else if(!holding)
					{
						holding = true;
						sndPlay(SND_BOUNDS);
					}

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
					{
						selected--;
						sndPlay(SND_TICK);
					}
					else if(!holding)
					{
						holding = true;
						sndPlay(SND_BOUNDS);
					}

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
					if(selected > 0)
						sndPlay(SND_TICK);
					else if(!holding)
					{
						holding = true;
						sndPlay(SND_BOUNDS);
					}

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
					if(selected < (int)list_size - 1)
						sndPlay(SND_TICK);
					else if(!holding)
					{
						holding = true;
						sndPlay(SND_BOUNDS);
					}

					selected += 6;
					if(selected > (int)list_size - 1)
						selected = list_size - 1;

					if(selected - start >= 18)
						start += 6;

					if(selected > 11 && selected < 18)
						maxTitles = 24;
					break;
				}
				else if(down & KEY_A || blkNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
				{
					sndPlay(SND_SELECT);
					data::curData = data::curUser.blktitles[selected];
					if(fs::mountSave(data::curUser, data::curData))
					{
						util::makeTitleDir(data::curUser, data::curData);
															
						folderMenuInfo = util::getInfoString(data::curUser, data::curData);

						mstate = FLD_SEL;
						prevState = BKL_SEL;
					}
					break;
				}
				else if(down & KEY_X || blkNav[2].getEvent() == BUTTON_RELEASED)
				{
					sndPlay(SND_SELECT);
					data::blacklistRemove(data::curUser, data::curUser.blktitles[selected]);
					// deleting last icon
					if((unsigned)selected == list_size - 1)
						if(selected > 0)
							selected--;

					if(maxTitles == 24 && (int)list_size - 1 <= 12)
						maxTitles = 18;

					if(maxTitles == 24 && start + 12 < (int)list_size)
						start -= 6;

					if(start < 0)
						start = 0;

					break;
				}
				else if(down & KEY_Y || blkNav[3].getEvent() == BUTTON_RELEASED)
				{
					sndPlay(SND_POPUP);
					if(confirm("Are you sure you want to backup all saves from blacklisted games?", "Backup"))
					{
						sndPlay(SND_LOADING);
						fs::dumpAllUserSavesBlacklisted(data::curUser);
						sndPlay(SND_BING);
					}
					break;
				}
			}
			if(down & KEY_B || blkNav[1].getEvent() == BUTTON_RELEASED)
			{
				sndPlay(SND_BACK);
				start = 0;
				selected = 0;
				maxTitles = 18;
				selRectX = 93, selRectY = 187;
				mstate = TTL_SEL;
				return;
			}
			else if(down & KEY_PLUS || blkNav[blkNav.size() - 1].getEvent() == BUTTON_RELEASED)
			{
				ui::finish = true;
				break;
			}
		}
	}
}
