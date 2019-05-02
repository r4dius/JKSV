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
        static int start = 0, selected = 0;

        //Color shift for rect
        static uint8_t clrSh = 0;
        //Whether or not we're adding or subtracting from clrShft
        static bool clrAdd = true;

        //Selected rectangle X and Y.
        static unsigned selRectX = 86, selRectY = 180;

        static ui::touchTrack track;

        unsigned x = 93, y = 187;
        unsigned endUser = start + 18;
        if(start + 18 > (int)data::users.size())
            endUser = data::users.size();

		if(clrAdd)
        {
            clrSh += 4;
            if(clrSh > 63)
                clrAdd = false;
        }
        else
        {
            clrSh--;
            if(clrSh == 0)
                clrAdd = true;
        }

		drawBoundBox(selRectX + 5, selRectY + 5, 178, 178, clrSh);

        for(unsigned i = start; i < endUser; y += 184)
        {
            unsigned endRow = i + 8;
            for(unsigned tX = x; i < endRow; i++, tX += 184)
            {
                if(i == endUser)
                    break;

                if((int)i == selected)
                {
                    if(selRectX != tX - 7 || selRectY != y - 7)
                    {
                        selRectX = tX - 7;
                        selRectY = y - 7;
                    }

                    std::string username = data::users[selected].getUsername();
					drawTitlebox(username, tX, y - 63, 48);
                }
                data::users[i].drawIconHalf(tX, y);
            }
        }


        //Update invisible buttons
        for(int i = 0; i < 18; i++)
        {
            selButtons[i].update(p);
            if(selected == i && selButtons[i].getEvent() == BUTTON_RELEASED)
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

        //Update touch tracking
        track.update(p);

        if(down & KEY_RIGHT)
        {
            if(selected < (int)data::users.size() - 1)
                selected++;

            if(selected >= (int)start + 18)
                start += 6;
        }
        else if(down & KEY_LEFT)
        {
            if(selected > 0)
                selected--;

            if(selected < start)
                start -= 6;
        }
        else if(down & KEY_UP)
        {
            selected -= 6;
            if(selected < 0)
                selected = 0;

            if(selected - start >= 18)
                start -= 6;
        }
        else if(down & KEY_DOWN)
        {
            selected += 6;
            if(selected > (int)data::users.size() - 1)
                selected = data::users.size() - 1;

            if(selected - start >= 18)
                start += 6;
        }
        else if(down & KEY_A || usrNav[0].getEvent() == BUTTON_RELEASED)
        {
            data::curUser = data::users[selected];
            //Reset this
            start = 0;
            //selected = 0;
            selRectX = 86, selRectY = 180;
            mstate = TTL_SEL;
        }
        else if(down & KEY_Y || usrNav[1].getEvent() == BUTTON_RELEASED)
        {
            for(unsigned i = 0; i < data::users.size(); i++)
                fs::dumpAllUserSaves(data::users[i]);
        }
    }
}
