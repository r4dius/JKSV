#include <string>
#include <vector>

#include <fstream>

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
        static int start = 0, selected = 0, maxTitles = 18;

        //Color shift for rect
        static uint8_t clrSh = 0;
        //Whether or not we're adding or subtracting from clrShft
        static bool clrAdd = true;

        static ui::touchTrack track;
        unsigned x = 93, y = 187;
		static unsigned tiX = 0, tiY = 0;

        //Selected rectangle X and Y.
        static unsigned selRectX = x, selRectY = y;
		static std::string title = "";

		if(maxTitles == 24) y = 3;
        unsigned endTitle = start + maxTitles;
        if(start + maxTitles > (int)data::curUser.titles.size())
            endTitle = data::curUser.titles.size();

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
                    //drawTitlebox(title, tX, y - 63, 48);
					tiX = tX, tiY = y;
                }
                data::curUser.titles[i].icon.drawHalf(tX, y);
            }
        }

        //Buttons
        for(int i = 0; i < maxTitles; i++)
        {
            selButtons[i].update(p);
            if(i == selected - start && selButtons[i].getEvent() == BUTTON_RELEASED)
            {
                data::curData = data::curUser.titles[selected];
                if(fs::mountSave(data::curUser, data::curData))
                {
                    util::makeTitleDir(data::curUser, data::curData);
                    folderMenuPrepare(data::curUser, data::curData);
                    folderMenuInfo = util::getInfoString(data::curUser, data::curData);

                    mstate = FLD_SEL;
                }
            }
            else if(selButtons[i].getEvent() == BUTTON_RELEASED)
            {
                if(start + i < (int)data::curUser.titles.size())
                    selected = start + i;
            }
        }

        //Nav
        for(unsigned i = 0; i < ttlNav.size(); i++)
            ttlNav[i].update(p);

		ui::screen = texCreate(1280, 720);
		memcpy(screen->data, frameBuffer->data, frameBuffer->size * 4);

		while(true)
        {
            hidScanInput();
            uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
            touchPosition p;
            hidTouchRead(&p, 0);
			//Update touchtracking
			track.update(p); 

			if(clrAdd)
			{
				clrSh += 10;
				if(clrSh > 60)
					clrAdd = false;
			}
			else
			{
				clrSh -= 10;
				if(clrSh == 0)
					clrAdd = true;
			}

			switch(track.getEvent())
			{
				case TRACK_SWIPE_UP:
					{
						if(start + maxTitles < (int)data::curUser.titles.size())
						{
							start += 6;
							selected += 6;
							if(selected > (int)data::curUser.titles.size() - 1)
								selected = data::curUser.titles.size() - 1;
							maxTitles = 24;
						}
					}
					break;

				case TRACK_SWIPE_DOWN:
					{
						if(start - 6 >= 0)
						{
							start -= 6;
							selected -= 6;
						} else maxTitles = 18;
					}
					break;

				break;
			}

			if(down & KEY_RIGHT)
			{
				if(selected < (int)data::curUser.titles.size() - 1)
					selected++;

				if(selected >= (int)start + 18)
					start += 6;

				if(start < 0) start = 0;

				if(selected == 12) maxTitles = 24;
				break;
			}
			else if(down & KEY_LEFT)
			{
				if(selected > 0)
					selected--;

				if(selected - 6 < (int)start)
					start -= 6;

				if(start < 0) start = 0;

				if(selected == 5) maxTitles = 18;
				break;
			}
			else if(down & KEY_UP)
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
			else if(down & KEY_DOWN)
			{
				selected += 6;
				if(selected > (int)data::curUser.titles.size() - 1)
					selected = data::curUser.titles.size() - 1;

				if(selected - start >= 18)
					start += 6;

				if(selected > 11 && selected < 18) maxTitles = 24;
				break;
			}
			else if(down & KEY_A || ttlNav[0].getEvent() == BUTTON_RELEASED)
			{
				data::curData = data::curUser.titles[selected];
				if(fs::mountSave(data::curUser, data::curData))
				{
					util::makeTitleDir(data::curUser, data::curData);
					folderMenuPrepare(data::curUser, data::curData);
					folderMenuInfo = util::getInfoString(data::curUser, data::curData);

					mstate = FLD_SEL;
				}
				break;
			}
			else if(down & KEY_Y || ttlNav[1].getEvent() == BUTTON_RELEASED)
			{
				fs::dumpAllUserSaves(data::curUser);
			}
			else if(down & KEY_X || ttlNav[2].getEvent() == BUTTON_RELEASED)
			{
				std::string confStr = "Are you 100% sure you want to add \"" + data::curUser.titles[selected].getTitle() + \
									  "\" to your blacklist?";
				if(ui::confirm(confStr, "Blacklist"))
					data::blacklistAdd(data::curUser, data::curUser.titles[selected]);
				break;
			}
			else if(down & KEY_B || ttlNav[3].getEvent() == BUTTON_RELEASED)
			{
				start = 0;
				//selected = 0;
				maxTitles = 18;
				selRectX = 93, selRectY = 187;
				mstate = USR_SEL;
				return;
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
