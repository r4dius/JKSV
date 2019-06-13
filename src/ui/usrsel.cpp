#include <string>
#include <fstream>
#include <vector>
#include <switch.h>

#include "ui.h"
#include "uiupdate.h"
#include "file.h"

extern std::vector<ui::button> usrNav;

namespace ui
{
	void updateUserMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
	{
		//Static so they don't get reset every loop
		//Where to start in titles, selected user
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

		usrNav.clear();

		unsigned list_size = data::users.size();

		if(maxTitles == 24)
			y = 3;

		unsigned endUser = start + maxTitles;
		if(start + maxTitles > (int)list_size)
			endUser = list_size;

		for(unsigned i = start; i < endUser; y += 184)
		{
			unsigned endRow = i + 6;
			for(unsigned tX = x; i < endRow; i++, tX += 184)
			{
				if(i == endUser)
					break;

				if((int)i == selected)
				{
					if(selRectX != tX || selRectY != y)
					{
						selRectX = tX;
						selRectY = y;
					}

					title = data::users[selected].getUsername();
					tiX = tX, tiY = y;
				}
				data::users[i].drawResize(tX, y, 174, 174);
				ui::button newSelButton("", tX, y, 174, 174);
				selButtons.push_back(newSelButton);
			}
		}

		unsigned endX = 1218, butSize = 0;
		std::string butTxt = "Select";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonA, frameBuffer, endX -= 38, 672);
		ui::button usrSel("", endX, 656, butSize + 38, 64);
		usrNav.push_back(usrSel);
		endX -= 41;
		butTxt = "Backup All";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonY, frameBuffer, endX -= 38, 672);
		ui::button usrDmp("", endX, 656, butSize + 38, 64);
		usrNav.push_back(usrDmp);
		endX -= 41;
		butTxt = "Exit";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675, 18, mnuTxt);
		texDraw(buttonP, frameBuffer, endX -= 38, 672);
		ui::button usrExt("", endX, 656, butSize + 38, 64);
		usrNav.push_back(usrExt);

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
			for(unsigned i = 0; i < usrNav.size(); i++)
				usrNav[i].update(p);

			//Update invisible buttons
			for(int i = 0; (unsigned)i < endUser - start; i++)
			{
				selButtons[i].update(p);
				if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
				{
					if(!swiping) {
						data::curUser = data::users[selected];

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
				std::string message = "No user with saved game, try playing something and get back";
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
				else if(down & KEY_A || usrNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
				{
					data::curUser = data::users[selected];
					//Reset this
					//start = 0;
					//selected = 0;
					//selRectX = 93, selRectY = 187;
					mstate = TTL_SEL;
					break;
				}
				else if(down & KEY_Y || usrNav[1].getEvent() == BUTTON_RELEASED)
				{
					if(confirm("Are you sure you want to backup all users saves?", "Backup"))
					{
						for(unsigned i = 0; i < list_size; i++)
							fs::dumpAllUserSaves(data::users[i]);
					}
					break;
				}
			}
			if(down & KEY_PLUS || usrNav[2].getEvent() == BUTTON_RELEASED)
			{
				ui::finish = true;
				break;
			}
		}
	}
}
