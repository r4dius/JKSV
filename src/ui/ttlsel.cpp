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
        //Static vars so they don't change on every loop
        //Where to start in titles, selected title
        static int start = 0, selected = 0, maxTitles = 18;

        //Color shift for rect
        static uint8_t clrShft = 0;
        //Whether or not we're adding or subtracting from clrShft
        static bool clrAdd = true;

        //Selected rectangle X and Y.
        static unsigned selRectX = 86, selRectY = 180;

        static ui::touchTrack track;

        //Color swapping
        clr clrPrev = clrCreateRGBA(0x00, 0x60 + clrShft, 0xBB + clrShft, 0xFF);

        if(clrAdd)
        {
            clrShft += 6;
            if(clrShft > 63)
                clrAdd = false;
        }
        else
        {
            clrShft--;
            if(clrShft == 0)
                clrAdd = true;
        }

        //Updated sel
        clr clrUpdt = clrCreateRGBA(0x00, 0x60 + clrShft, 0xBB + clrShft, 0xFF);

        texSwapColors(ui::selBox, clrPrev, clrUpdt);

        unsigned x = 93, y = 187;
		
		if(maxTitles == 24) y = 3;
		
        unsigned endTitle = start + maxTitles;
        if(start + maxTitles > (int)data::curUser.titles.size())
            endTitle = data::curUser.titles.size();

        //draw Rect so it's always behind icons
        texDraw(ui::selBox, frameBuffer, selRectX, selRectY);

        for(unsigned i = start; i < endTitle; y += 184)
        {
            unsigned endRow = i + 6;
            for(unsigned tX = x; i < endRow; i++, tX += 184)
            {
                if(i == endTitle)
                    break;

                if((int)i == selected)
                {
                    if(selRectX != tX - 7 || selRectY != y - 7)
                    {
                        selRectX = tX - 7;
                        selRectY = y - 7;
                    }

					std::string title = data::curUser.titles[selected].getTitle();
                    drawTitlebox(title, tX, y - 63, 48);
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

        //Update touchtracking
        track.update(p);

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
        }

        if(down & KEY_RIGHT)
        {
            if(selected < (int)data::curUser.titles.size() - 1)
                selected++;

			if(selected >= (int)start + 18)
				start += 6;

			if(start < 0) start = 0;

			if(selected == 12) maxTitles = 24;
        }
        else if(down & KEY_LEFT)
        {
            if(selected > 0)
                selected--;

			if(selected - 6 < (int)start)
				start -= 6;

			if(start < 0) start = 0;

			if(selected == 5) maxTitles = 18;
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
        }
        else if(down & KEY_DOWN)
        {
            selected += 6;
            if(selected > (int)data::curUser.titles.size() - 1)
                selected = data::curUser.titles.size() - 1;

            if(selected - start >= 18)
                start += 6;

			if(selected > 11 && selected < 18) maxTitles = 24;
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
        }
        else if(down & KEY_Y || ttlNav[1].getEvent() == BUTTON_RELEASED)
        {
            fs::dumpAllUserSaves(data::curUser);
        }
        else if(down & KEY_X || ttlNav[2].getEvent() == BUTTON_RELEASED)
        {
            std::string confStr = "Are you 100% sure you want to add \"" + data::curUser.titles[selected].getTitle() + \
                                  "\" to your blacklist?";
            if(ui::confirm(confStr))
                data::blacklistAdd(data::curUser, data::curUser.titles[selected]);
        }
        else if(down & KEY_B || ttlNav[3].getEvent() == BUTTON_RELEASED)
        {
            start = 0;
            selected = 0;
            maxTitles = 18;
            selRectX = 86;
            selRectY = 180;
            mstate = USR_SEL;
            return;
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
