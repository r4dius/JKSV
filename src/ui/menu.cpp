#include <string>
#include <switch.h>

#include "gfx.h"
#include "menu.h"
#include "ui.h"
#include "util.h"
#include "miscui.h"

namespace ui
{
    void menu::setParams(const unsigned& _x, const unsigned& _y, const unsigned& _rW, const int& _fontSize)
    {
        x = _x;
        y = _y;
        rW = _rW;
        rY = _y;
		fontSize = _fontSize;

        optButtons.clear();

        for(unsigned i = 0; i < 7; i++)
        {
            //Init + push invisible options buttons
            ui::button newOptButton("", x, y + i * 71, rW, 71);
            optButtons.push_back(newOptButton);
        }
    }

    void menu::addOpt(const std::string& add)
    {
        if(textGetWidth(add.c_str(), ui::shared, fontSize) < rW - 60 || rW == 0)
            opt.push_back(add);
        else
        {
			opt.push_back(util::cutStr(add, rW - 60, fontSize));
        }
    }

    void menu::editOpt(int ind, const std::string& ch)
    {
        opt[ind] = ch;
    }

    menu::~menu()
    {
        opt.clear();
    }

    void menu::handleInput(const uint64_t& down, const uint64_t& held, const touchPosition& p)
    {
        if( (held & KEY_UP) || (held & KEY_DOWN))
            fc++;
        else
            fc = 0;
        if(fc > 10)
            fc = 0;

        int size = opt.size() - 1;
        if((down & KEY_UP) || ((held & KEY_UP) && fc == 10))
        {
            selected--;
            if(selected < 0)
                selected = size;

            if((start > selected)  && (start > 0))
                start--;
            if(size < 7)
                start = 0;
            if((selected - 6) > start)
                start = selected - 6;
        }
        else if((down & KEY_DOWN) || ((held & KEY_DOWN) && fc == 10))
        {
            selected++;
            if(selected > size)
                selected = 0;

            if((selected > (start + 6)) && ((start + 6) < size))
                start++;
            if(selected == 0)
                start = 0;
        }
        else if(down & KEY_RIGHT)
        {
            selected += 7;
            if(selected > size)
                selected = size;
            if((selected - 6) > start)
                start = selected - 6;
        }
        else if(down & KEY_LEFT)
        {
            selected -= 7;
            if(selected < 0)
                selected = 0;
            if(selected < start)
                start = selected;
        }

        //New touch shit
        for(int i = 0; i < 7; i++)
        {
            optButtons[i].update(p);
            if(selected == i && optButtons[i - start].getEvent() == BUTTON_RELEASED)
            {
                retEvent = MENU_DOUBLE_REL;
                break;
            }
            else if(optButtons[i].getEvent() == BUTTON_RELEASED && i + start < (int)opt.size())
            {
                selected = i + start;
                retEvent = MENU_NOTHING;
            }
            else
                retEvent = MENU_NOTHING;
        }

        track.update(p);

        switch(track.getEvent())
        {
            case TRACK_SWIPE_UP:
                if(start + 7 < (int)opt.size())
                    start++, selected++;
                break;

            case TRACK_SWIPE_DOWN:
                if(start - 1 >= 0)
                    start--, selected--;
                break;
        }
    }

    void menu::draw(const clr& textClr)
    {
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

        int length = 0;
        if((opt.size() - 1) < 7)
            length = opt.size();
        else
            length = start + 7;

        for(int i = start; i < length; i++)
        {
			//Don't draw separator on top and bottom of selected item
			// if(i - 1 != selected && i != selected && separate)
			    // drawRect(frameBuffer, x, y - 1 + ((i - start) * 71), rW, 1, ui::sepClr);
            if(i == selected)
                drawGlowElem(x, y + ((i - start) * 71), rW, 70, clrSh, ui::menuSel, 0);

            drawText(opt[i].c_str(), frameBuffer, shared, x + 15, (y + 26) + ((i - start) * 71), fontSize, textClr);

			// if(i != selected && separate)
			    // drawRect(frameBuffer, x, y - 1 + ((i - start +1) * 71), rW, 1, ui::sepClr);
        }
    }

    void menu::reset()
    {
        opt.clear();
		selected = 0;
		start = 0;
        fc = 0;
    }

    void menu::adjust()
    {
        if(selected > (int)opt.size() - 1)
            selected = opt.size() - 1;

        if(opt.size() < 6)
            start = 0;
        else if(opt.size() > 6 && start + 6 > (int)opt.size() - 1)
            start = opt.size() - 7;
    }
}
