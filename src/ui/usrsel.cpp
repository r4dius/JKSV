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

        static ui::touchTrack track;
		unsigned x = 93, y = 187;
		static unsigned tiX = 0, tiY = 0;

		//Selected rectangle X and Y.
		static unsigned selRectX = x, selRectY = y;
		static std::string title = "";

		if(maxTitles == 24) y = 3;
		unsigned endUser = start + maxTitles;
		if(start + maxTitles > (int)data::users.size())
			endUser = data::users.size();

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
					// drawTitlebox(username, tX, y - 63, 48);
					tiX = tX, tiY = y;
				}
				data::users[i].drawHalf(tX, y);
			}
		}

		//Update invisible buttons
		for(int i = 0; i < maxTitles; i++)
		{
            selButtons[i].update(p);
            if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
            {
				data::curUser = data::users[selected];

				mstate = TTL_SEL;
			}
			else if(selButtons[i].getEvent() == BUTTON_RELEASED)
			{
				if(start + i < (int)data::users.size())
					selected = start + i;
			}
		}

		//Update nav
		for(unsigned i = 0; i < usrNav.size(); i++)
			usrNav[i].update(p);

		memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);
			
		while(true)
        {
            hidScanInput();
            uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
			uint64_t held = hidKeysHeld(CONTROLLER_P1_AUTO);
            touchPosition p;
            hidTouchRead(&p, 0);

			if((held & KEY_RIGHT) || (held & KEY_LEFT) || (held & KEY_UP) || (held & KEY_DOWN))
				movespeed++;
			else {
				movespeed = 0;
				move = false;
			}

			if(movespeed >= 15) {
				move = true;
				movespeed = 12;
			} else move = false;

			if(clrAdd)
			{
				clrSh += 5;
				if(clrSh > 100) {
					if(clrSh > 254) clrSh = 254;
					clrAdd = false;
				}
			}
			else
			{
				clrSh -= 10;
				if(clrSh <= 0) {
					if(clrSh < 0) clrSh = 0;
					clrAdd = true;
				}
			}

			if(down & KEY_RIGHT || ((held & KEY_RIGHT) && move))
			{
				if(selected < (int)data::users.size() - 1)
					selected++;

				if(selected >= (int)start + 18)
					start += 6;

				if(start < 0) start = 0;
				if(selected == 12) maxTitles = 24;
                break;
			}
			else if(down & KEY_LEFT || ((held & KEY_LEFT) && move))
			{
				if(selected > 0)
					selected--;

				if(selected - 6 < (int)start)
					start -= 6;

				if(start < 0) start = 0;
				if(selected == 5) maxTitles = 18;
				break;
			}
			else if(down & KEY_UP || ((held & KEY_UP) && move))
			{
				selected -= 6;
				if(selected < 0)
					selected = 0;

				if(selected - 6 < start)
					start -= 6;

				if(start < 0) start = 0;
				if(selected >= 0 && selected < 6) maxTitles = 18;
				break;
			}
			else if(down & KEY_DOWN || ((held & KEY_DOWN) && move))
			{
				selected += 6;
				if(selected > (int)data::users.size() - 1)
					selected = data::users.size() - 1;

				if(selected - start >= 18)
					start += 6;

				if(selected > 11 && selected < 18) maxTitles = 24;
				break;
			}
			else if(down & KEY_A || usrNav[0].getEvent() == BUTTON_RELEASED)
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
					for(unsigned i = 0; i < data::users.size(); i++)
						fs::dumpAllUserSaves(data::users[i]);
                }
                break;
			}
			else if(down & KEY_PLUS)
			{
				ui::finish = true;
				break;
			}

			gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			drawGlowElem(selRectX, selRectY, 178, 178, clrSh, ui::iconSel, 2);
			drawTitlebox(title, tiX, tiY - 63, 48);
			gfxEndFrame();
		}
    }
}
