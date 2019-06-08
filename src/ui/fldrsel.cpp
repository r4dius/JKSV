#include <string>
#include <vector>
#include <sys/stat.h>

#include "ui.h"
#include "uiupdate.h"
#include "util.h"

extern std::vector<ui::button> fldNav;
std::vector<ui::button> optButtons;
std::vector<std::string> opt;

namespace ui
{
    void addOpt(const std::string& add, const unsigned& w, const int& fontSize)
    {
        if(textGetWidth(add.c_str(), ui::shared, fontSize) < w - 60 || w == 0)
            opt.push_back(add);
        else
        {
			opt.push_back(util::cutStr(add, w - 60, fontSize));
        }
    }

    void updateFolderMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
    {
        static int start = 0, selected = 0, movespeed = 0;
		static bool move = false;
		unsigned x = 470, y = 131, rW = 720;
        static unsigned selRectX = x, selRectY = y;
		static int fontSize = 19, retEvent = MENU_NOTHING;

char char_arr[200];
		//Color shift for rect
        static int clrSh = 0;
        //Whether or not we're adding or subtracting from clrShft
        static bool clrAdd = true;

        static ui::touchTrack track;
		static const char* title;

        optButtons.clear();

        for(unsigned i = 0; i < 7; i++)
        {
            //Init + push invisible options buttons
            ui::button newOptButton("", x, y + i * 71, rW, 71);
            optButtons.push_back(newOptButton);
        }

        opt.clear();
		fldNav.clear();

        util::makeTitleDir(data::curUser, data::curData);
        std::string scanPath = util::getTitleDir(data::curUser, data::curData);

        fs::dirList list(scanPath);
        addOpt("New", rW, fontSize);
        for(unsigned i = 0; i < list.getCount(); i++)
            addOpt(list.getItem(i), rW, fontSize);

        if(selected > (int)opt.size() - 1)
            selected = opt.size() - 1;

        if(opt.size() < 6)
            start = 0;
        else if(opt.size() > 6 && start + 6 > (int)opt.size() - 1)
            start = opt.size() - 7;


		int length = 0;
		if((opt.size() - 1) < 7)
			length = opt.size();
		else
			length = start + 7;

		for(int i = start; i < length; i++)
		{
			//Don't draw separator on top and bottom of selected item
			//if(i - 1 != selected && i != selected)
			drawRect(frameBuffer, x, y - 1 + ((i - start) * 71), rW, 1, ui::sepClr);
			
			if((int)i == selected)
			{
				selRectX = x;
				selRectY = y + ((i - start) * 71);

				title = opt[i].c_str();
			}

			drawText(opt[i].c_str(), frameBuffer, shared, x + 15, (y + 26) + ((i - start) * 71), fontSize, mnuTxt);

			if(i != selected)
				drawRect(frameBuffer, x, y - 1 + ((i - start +1) * 71), rW, 1, ui::sepClr);
		}

		data::curData.icon.drawResize(115, 130, 200, 200);
		drawTextWrap(folderMenuInfo.c_str(), frameBuffer, ui::shared, 76, 359, 19, ui::mnuTxt, 280);

		unsigned endX = 1218, butSize = 0;
		std::string butTxt = "Backup";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 18), 675, 18, mnuTxt);
		texDraw(buttonA, frameBuffer, endX -= 38, 672);
		ui::button fldSel("", endX, 656, butSize + 38, 64);
		fldNav.push_back(fldSel);
		endX -= 41;
		butTxt = "Back";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 18), 675, 18, mnuTxt);
		texDraw(buttonB, frameBuffer, endX -= 38, 672);
		ui::button fldBck("", endX, 656, butSize + 38, 64);
		fldNav.push_back(fldBck);
		endX -= 41;
		if(selected > 0)
		{
			butTxt = "Delete";
			butSize = textGetWidth(butTxt.c_str(), shared, 18);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 18), 675, 18, mnuTxt);
			texDraw(buttonX, frameBuffer, endX -= 38, 672);
			ui::button fldDel("", endX, 656, butSize + 38, 64);
			fldNav.push_back(fldDel);
			endX -= 41;
			butTxt = "Restore";
			butSize = textGetWidth(butTxt.c_str(), shared, 18);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 18), 675, 18, mnuTxt);
			texDraw(buttonY, frameBuffer, endX -= 38, 672);
			ui::button fldRes("", endX, 656, butSize + 38, 64);
			fldNav.push_back(fldRes);
			endX -= 41;
		}
		butTxt = "Exit";
		butSize = textGetWidth(butTxt.c_str(), shared, 18);
		drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 18), 675, 18, mnuTxt);
		texDraw(buttonP, frameBuffer, endX -= 38, 672);
		ui::button fldExt("", endX, 656, butSize + 38, 64);
		fldNav.push_back(fldExt);

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

			//Update nav
			for(unsigned i = 0; i < fldNav.size(); i++)
				fldNav[i].update(p);

			if((held & KEY_UP) || (held & KEY_DOWN))
				movespeed++;
			else {
				movespeed = 0;
				move = false;
			}

			if(movespeed >= 15) {
				move = true;
				movespeed = 12;
			} else move = false;

			int size = opt.size() - 1;
			if((down & KEY_UP) || ((held & KEY_UP) && move))
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
				break;
			}
			else if((down & KEY_DOWN) || ((held & KEY_DOWN) && move))
			{
				selected++;
				if(selected > size)
					selected = 0;

				if((selected > (start + 6)) && ((start + 6) < size))
					start++;
				if(selected == 0)
					start = 0;
				break;
			}
			else if(down & KEY_RIGHT)
			{
				selected += 7;
				if(selected > size)
					selected = size;
				if((selected - 6) > start)
					start = selected - 6;
				break;
			}
			else if(down & KEY_LEFT)
			{
				selected -= 7;
				if(selected < 0)
					selected = 0;
				if(selected < start)
					start = selected;
				break;
			}

			track.update(p);
			switch(track.getEvent())
			{
				case TRACK_SWIPE_UP:
					if(start + 7 < (int)opt.size()) {
						start++, selected++;
						return;
					}
					break;

				case TRACK_SWIPE_DOWN:
					if(start - 1 >= 0) {
						start--, selected--;
						return;
					}
					break;
			}
			
			//New touch shit
			for(int i = 0; i < 7; i++)
			{
				optButtons[i].update(p);
				if(selected  == i + start && optButtons[i].getEvent() == BUTTON_RELEASED)
				{
					retEvent = MENU_DOUBLE_REL;
					// selRectY = y + (i * 71);
					// title = opt[i + start].c_str();

sprintf(char_arr, "touch 1 %d", i);
drawText(char_arr, frameBuffer, ui::shared, 800, 10, 14, txtClr);
					break;
				}
				else if(optButtons[i].getEvent() == BUTTON_RELEASED && i + start < (int)opt.size())
				{
					selected = i + start;
					selRectY = y + (i * 71);
					title = opt[i + start].c_str();
					retEvent = MENU_NOTHING;

sprintf(char_arr, "touch 2 %d", i);
drawText("touch 2", frameBuffer, ui::shared, 800, 30, 14, txtClr);
				}
				else {
					retEvent = MENU_NOTHING;

sprintf(char_arr, "touch 3 %d", i);
drawText("touch 3", frameBuffer, ui::shared, 800, 40, 14, txtClr);
				}
			}

			gfxBeginFrame();
			texDraw(screen, frameBuffer, 0, 0);
			drawGlowElem(selRectX, selRectY, rW, 70, clrSh, ui::menuSel, 0);
			drawText(title, frameBuffer, shared, selRectX + 15, selRectY + 26, fontSize, mnuTxt);

sprintf(char_arr, "fldNav %d", fldNav.size());
drawText(char_arr, frameBuffer, ui::shared, 500, 10, 14, txtClr);
sprintf(char_arr, "start %d", start);
drawText(char_arr, frameBuffer, ui::shared, 500, 40, 14, txtClr);
			gfxEndFrame();
			
			if(down & KEY_A || fldNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
			{
				if(selected == 0)
				{
					std::string folder;
					//Add back 3DS shortcut thing
					if(held & KEY_R)
						folder = data::curUser.getUsernameSafe() + " - " + util::getDateTime(util::DATE_FMT_YMD);
					else if(held & KEY_L)
						folder = data::curUser.getUsernameSafe() + " - " + util::getDateTime(util::DATE_FMT_YDM);
					else if(held & KEY_ZL)
						folder = data::curUser.getUsernameSafe() + " - " + util::getDateTime(util::DATE_FMT_HOYSTE);
					else
					{
						const std::string dict[] =
						{
							util::getDateTime(util::DATE_FMT_YMD).c_str(),
							util::getDateTime(util::DATE_FMT_YDM).c_str(),
							util::getDateTime(util::DATE_FMT_HOYSTE).c_str(),
							data::curUser.getUsernameSafe().c_str(),
							data::curData.getTitle().length() < 24 ? data::curData.getTitleSafe() : util::generateAbbrev(data::curData)
						};
						folder = util::getStringInput(data::curUser.getUsernameSafe() + " - " + util::getDateTime(util::DATE_FMT_YDM), "Enter a folder name", 64, 5, dict);
					}
					if(!folder.empty())
					{
						std::string path = util::getTitleDir(data::curUser, data::curData) + "/" + folder;
						mkdir(path.c_str(), 777);
						path += "/";

						std::string root = "sv:/";
						fs::copyDirToDir(root, path);
					}
				}
				else
				{
					std::string scanPath = util::getTitleDir(data::curUser, data::curData);
					fs::dirList list(scanPath);

					std::string folderName = list.getItem(selected - 1);
					if(confirm("Are you sure you want to overwrite \"" + util::cutStr(folderName, 690, 24) + "\"?", "Overwrite"))
					{
						std::string toPath = util::getTitleDir(data::curUser, data::curData) + folderName + "/";
						//Delete and recreate
						fs::delDir(toPath);
						mkdir(toPath.c_str(), 777);

						std::string root = "sv:/";

						fs::copyDirToDir(root, toPath);
					}
				}
				break;
			}
			else if(down & KEY_B || fldNav[1].getEvent() == BUTTON_RELEASED)
			{
				start = 0;
				selected = 0;
				fsdevUnmountDevice("sv");
				mstate = prevState;
				return;
			}
			else if(selected > 0 && (down & KEY_X || fldNav[2].getEvent() == BUTTON_RELEASED))
			{
				std::string scanPath = util::getTitleDir(data::curUser, data::curData);
				fs::dirList list(scanPath);

				std::string folderName = list.getItem(selected - 1);
				if(confirm("Are you sure you want to delete \"" + util::cutStr(folderName, 690, 24) + "\"?", "Delete"))
				{
					std::string delPath = scanPath + folderName + "/";
					fs::delDir(delPath);
				}
				break;
			}
			else if(down & KEY_Y || fldNav[3].getEvent() == BUTTON_RELEASED)
			{
				if(selected > 0 && data::curData.getType() != FsSaveDataType_SystemSaveData)
				{
					std::string scanPath = util::getTitleDir(data::curUser, data::curData);
					fs::dirList list(scanPath);

					std::string folderName = list.getItem(selected - 1);
					if(confirm("Are you sure you want to restore \"" + util::cutStr(folderName, 690, 24) + "\"?", "Restore"))
					{
						std::string fromPath = util::getTitleDir(data::curUser, data::curData) + folderName + "/";
						std::string root = "sv:/";

						fs::delDir(root);
						fsdevCommitDevice("sv");

						fs::copyDirToDirCommit(fromPath, root, "sv");
					}
				}
				else
					ui::showMessage("Writing data to system save data is not allowed currently. It CAN brick your system.", "Sorry, bro:");
				break;
			}
			else if(down & KEY_PLUS || fldNav[fldNav.size() - 1].getEvent() == BUTTON_RELEASED)
			{
				ui::finish = true;
				break;
			}
		}
    }
}
