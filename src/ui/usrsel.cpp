#include "ui.h"
#include "uiupdate.h"
#include "file.h"
#include "snd.h"

extern std::vector<ui::button> usrNav;

namespace ui
{
	void updateUserMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
	{
		static std::vector<ui::button> selButtons;

		//Static so they don't get reset every loop
		//Where to start in titles, selected user
		static int start = 0, selected = 0, listShow = 18, moveSpeed = 0;

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true;

		static ui::touchTrack track;
		unsigned x = 93, y = 187, tX = 0;
		static unsigned tiX = 0, tiY = 0;

		//Selected rectangle X and Y.
		static unsigned selRectX = x, selRectY = y;
		std::string title = "";
		int retEvent = MENU_NOTHING;

		static bool holding = false;
		static bool moving = false;
		static bool swiping = false;
		static bool touching = false;
		//Limit redraw of unchanged stuff
		static bool updateMenu = true;

		unsigned listSize = data::users.size();
		
		if(start < 0)
			start = 0;

		if(selected < 0)
			selected = 0;

		unsigned listLast = start + listShow;
		if(start + listShow > (int)listSize)
			listLast = listSize;

		if(start > 0)
		{
			for(unsigned i = 0; i < 6; i++, tX += 184)
			{
				if(updateMenu)
				{
					data::users[i].drawResize(tX, y, 174, 89);
					texDrawLimit(iconShadow, frameBuffer, tX - 5, 84);
				}
			}
		}

		for(unsigned i = start, j = 0; i < listLast; y += 184, j++)
		{
			unsigned endRow = i + 6;
			for(unsigned tX = x; i < endRow; i++, tX += 184)
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

					title = data::users[selected].getUsername();
					tiX = tX, tiY = y;
				}

				if(updateMenu)
				{
					data::users[i].drawResize(tX, y, 174, 174);
					texDraw(iconShadow, frameBuffer, tX - 5, y - 5);
				}

				ui::button newSelButton("", tX, y, 174, selH);
				selButtons.push_back(newSelButton);
			}
		}
				// selY = y;
				// if(listShow == 24) {
					// if(j < 1) {
						// selY = y + 85;
						// selH = 89;
					// }
					// if(j > 2)
						// selH = 92;
				// }
				// else
					// if(j > 1)
						// selH = 92;

		if(updateMenu)
		{
			updateMenu = false;
			usrNav.clear();

			unsigned endX = 1218, butSize = 0;
			std::string butTxt = "Select";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonA, frameBuffer, endX -= 37, 672);
			ui::button usrSel("", endX, 656, butSize + 38, 64);
			usrNav.push_back(usrSel);
			endX -= 41;
			butTxt = "Backup All";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonY, frameBuffer, endX -= 37, 672);
			ui::button usrDmp("", endX, 656, butSize + 38, 64);
			usrNav.push_back(usrDmp);
			endX -= 41;
			butTxt = "Exit";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= butSize, 675.5, 17.5, mnutxtClr);
			texDraw(buttonP, frameBuffer, endX -= 37, 672);
			ui::button usrExt("", endX, 656, butSize + 38, 64);
			usrNav.push_back(usrExt);

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
				if((listShow == 18 && start + 12 < (int)listSize) || start + 18 < (int)listSize)
				{
					swiping = true;
					selected += 6;
					if(selected > (int)listSize - 1)
						selected = listSize - 1;

					if(listShow == 24)
						start += 6;

					if((int)listSize > 12)
						listShow = 24;

					updateMenu = true;
					return;
				}
				break;

			case TRACK_SWIPE_DOWN:
				if(listShow != 18)
				{
					swiping = true;
					selected -= 6;
					if(selected < 0)
						selected = 0;

					start -= 6;
					if(start < 0)
					{
						start = 0;
						listShow = 18;
					}

					updateMenu = true;
					return;
				}
				break;
		}

		//Update nav
		for(unsigned i = 0; i < usrNav.size(); i++)
		{
			usrNav[i].update(p);
			if(usrNav[i].getEvent() == BUTTON_PRESSED)
			{
				if(!touching)
					soundPlay(SND_TICK);
				touching = true;
			}
		}

		// draw captured screen so we can overlay "dynamic" stuff
		texDraw(screen, frameBuffer, 0, 0);

		//Update invisible buttons
		for(int i = 0; (unsigned)i < listLast - start; i++)
		{
			selButtons[i].update(p);
			if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
			{
				if(!swiping) {
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
					
					if(listShow == 24)
					{
						if(selected < start + 6)
						{
							start -= 6;
							if(start < 0)
							{
								start = 0;
								listShow = 18;
							}

							updateMenu = true;
							return;
						}
						else if(selected >= start + 18)
						{
							start += 6;

							updateMenu = true;
							return;
						}
					}
					else if(selected >= start + 12)
					{
						listShow = 24;

						updateMenu = true;
						return;
					}
				}
			}
			else if(selButtons[i].getEvent() == BUTTON_PRESSED)
			{
				selButtons[i].draw();
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

		if(listSize > 0)
		{
			drawGlowButton(selRectX, selRectY, 174, 174, clrSh, BUTTON_ICON, 2);
			drawTitlebox(title, tiX, tiY - 63, 48);
		}
		else
		{
			std::string message = "No user with saved game, try playing something and get back";
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

				if(selected >= (int)start + 18)
				{
					updateMenu = true;
					start += 6;
				}

				if(start < 0)
					start = 0;

				if(selected == 12)
				{
					updateMenu = true;
					listShow = 24;
				}
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

				if(selected - 6 < (int)start && start != 0)
				{
					updateMenu = true;
					start -= 6;
				}

				if(start < 0)
					start = 0;

				if(selected == 5 && listShow != 18)
				{
					updateMenu = true;
					listShow = 18;
				}
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

				if(selected - 6 < start && start != 0)
				{
					updateMenu = true;
					start -= 6;
				}

				if(start < 0)
					start = 0;

				if(selected >= 0 && selected < 6 && listShow != 18)
				{
					updateMenu = true;
					listShow = 18;
				}
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

				if(selected - start >= 18)
				{
					updateMenu = true;
					start += 6;
				}

				if(selected > 11 && selected < 18 && listShow != 24)
				{
					updateMenu = true;
					listShow = 24;
				}
			}
			else if(down & KEY_A || usrNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
			{
				updateMenu = true;
				data::curUser = data::users[selected];
				mstate = TTL_SEL;
				soundPlay(SND_SELECT);
			}
			else if(down & KEY_Y || usrNav[1].getEvent() == BUTTON_RELEASED)
			{
				updateMenu = true;
				soundPlay(SND_POPUP);
				if(confirm("Are you sure you want to backup all users saves?", "Backup"))
				{
					soundPlay(SND_LOADING);
					for(unsigned i = 0; i < listSize; i++)
						fs::dumpAllUserSaves(data::users[i]);
					soundPlay(SND_BING);
				}
			}
		}
	}
}
