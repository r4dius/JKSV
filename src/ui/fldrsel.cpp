#include "ui.h"
#include "uiupdate.h"
#include "snd.h"
#include "util.h"

extern std::vector<ui::button> fldNav;
std::vector<ui::button> optButtons;
static std::vector<std::string> opt;

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
		static bool updatemenu = true;

		//Static so they don't get reset every loop
		//Where to start in titles, selected user
		static int start = 0, selected = 0, fontSize = 19, moveSpeed = 0;
		static bool moving = false;

		//Color shift for rect
		static int clrSh = 0;
		//Whether or not we're adding or subtracting from clrShft
		static bool clrAdd = true;

		static ui::touchTrack track;
		unsigned x = 470, y = 131, rW = 720;

		//Selected rectangle X and Y.
		static unsigned selRectX = x, selRectY = y;
		static const char* title;
		static int retEvent = MENU_NOTHING;

		static bool touching = false;

		if(updatemenu)
		{
			optButtons.clear();
			opt.clear();

			for(unsigned i = 0; i < 7; i++)
			{
				//Init + push invisible options buttons
				ui::button newOptButton("", x, y + i * 71, rW, 71);
				optButtons.push_back(newOptButton);
			}

			if(start > 0)
			{
				ui::button newOptButton("", x, y - 42, rW, 42);
				optButtons.push_back(newOptButton);
			}

			util::makeTitleDir(data::curUser, data::curData);
			std::string scanPath = util::getTitleDir(data::curUser, data::curData);

			fs::dirList list(scanPath);
			addOpt("New", rW, fontSize);
			for(unsigned i = 0; i < list.getCount(); i++)
				addOpt(list.getItem(i), rW, fontSize);
		}

		int list_size = opt.size() - 1;
		if(selected > list_size)
			selected = opt.size() - 1;

		if(opt.size() < 6)
			start = 0;
		else if(opt.size() > 6 && start + 6 > list_size)
			start = opt.size() - 7;

		int length = 0;
		if(list_size < 7)
			length = opt.size();
		else
			length = start + 7;

		if(start > 0) drawTextBound(opt[start - 1].c_str(), frameBuffer, shared, x + 15, y + 26 - 71, fontSize, mnutxtClr, 88, 647);

		if(updatemenu)
		{
			updatemenu = false;

			for(int i = start; i < length; i++)
			{
				drawRect(frameBuffer, x, y - 1 + ((i - start) * 71), rW, 1, ui::sepClr);
				
				if((int)i == selected)
				{
					selRectX = x;
					selRectY = y + ((i - start) * 71);

					title = opt[i].c_str();
				}

				drawText(opt[i].c_str(), frameBuffer, shared, x + 15, (y + 26) + ((i - start) * 71), fontSize, mnutxtClr);

				if(i != selected)
					drawRect(frameBuffer, x, y - 1 + ((i - start +1) * 71), rW, 1, ui::sepClr);
			}

			data::curData.icon.drawResize(115, 130, 200, 200);
			drawTextWrap(folderMenuInfo.c_str(), frameBuffer, ui::shared, 76, 359, 19, ui::mnutxtClr, 280);

			fldNav.clear();

			unsigned endX = 1218, butSize = 0;
			std::string butTxt = "Backup";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 17.5), 675.5, 17.5, mnutxtClr);
			texDraw(buttonA, frameBuffer, endX -= 37, 672);
			ui::button fldSel("", endX - 20, 653, butSize + 78, 62);
			fldNav.push_back(fldSel);
			endX -= 41;
			butTxt = "Back";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 17.5), 675.5, 17.5, mnutxtClr);
			texDraw(buttonB, frameBuffer, endX -= 37, 672);
			ui::button fldBck("", endX - 20, 653, butSize + 78, 62);
			fldNav.push_back(fldBck);
			endX -= 41;
			if(selected > 0)
			{
				butTxt = "Delete";
				butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
				drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 17.5), 675.5, 17.5, mnutxtClr);
				texDraw(buttonX, frameBuffer, endX -= 37, 672);
				ui::button fldDel("", endX - 20, 653, butSize + 78, 62);
				fldNav.push_back(fldDel);
				endX -= 41;
				butTxt = "Restore";
				butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
				drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 17.5), 675.5, 17.5, mnutxtClr);
				texDraw(buttonY, frameBuffer, endX -= 37, 672);
				ui::button fldRes("", endX - 20, 653, butSize + 78, 62);
				fldNav.push_back(fldRes);
				endX -= 41;
			}
			butTxt = "Exit";
			butSize = textGetWidth(butTxt.c_str(), shared, 17.5);
			drawText(butTxt.c_str(), frameBuffer, shared, endX -= textGetWidth(butTxt.c_str(), shared, 17.5), 675.5, 17.5, mnutxtClr);
			texDraw(buttonP, frameBuffer, endX -= 37, 672);
			ui::button fldExt("", endX - 20, 653, butSize + 78, 62);
			fldNav.push_back(fldExt);

			drawScrollBar(start, 0, list_size, SCROLL_LIST);

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
			moveSpeed = 0;
			moving = false;
		}

		if(moveSpeed >= 10 && (((held & KEY_UP) && selected != 0) || ((held & KEY_DOWN) && selected != list_size)))
		{
			moving = true;
			moveSpeed = 12;
		}
		else
			moving = false;

		if(hidTouchCount() > 0)
		{
			//Update touchtracking
			track.update(p, 3);
			switch(track.getEvent())
			{
				case TRACK_SWIPE_UP:
					if(start + 7 < (int)opt.size()) {
						start++, selected++;
						return;
					}

					updatemenu = true;

				case TRACK_SWIPE_DOWN:
					if(start - 1 >= 0) {
						start--, selected--;
						return;
					}

					updatemenu = true;
			}

			//Update nav
			for(unsigned i = 0; i < fldNav.size(); i++)
			{
				fldNav[i].update(p);
				if(fldNav[i].getEvent() == BUTTON_PRESSED)
				{
					if(!touching)
						soundPlay(SND_TICK);
					touching = true;
				}
			}

			//Update invisible buttons
			for(int i = 0; i < (list_size < 7 ? list_size + 1 : 7); i++)
			{
				optButtons[i].update(p);
				if(selected == i + start && optButtons[i].getEvent() == BUTTON_RELEASED)
				{
					updatemenu = true;

					retEvent = MENU_DOUBLE_REL;
					return;
				}
				else if(optButtons[i].getEvent() == BUTTON_RELEASED && i + start < (int)opt.size())
				{
					soundPlay(SND_TOUCHOUT);
					selected = i + start;
					selRectY = y + (i * 71);
					title = opt[selected].c_str();
					retEvent = MENU_NOTHING;
					return;
				}
				else if(optButtons[i].getEvent() == BUTTON_PRESSED)
				{
					optButtons[i].draw();
				}
				else
				{
					retEvent = MENU_NOTHING;
				}
			}
		}

		if(start > 0)
		{
			int i = 7;
			optButtons[i].update(p);
			if(optButtons[i].getEvent() == BUTTON_RELEASED)
			{
				soundPlay(SND_TOUCHOUT);
				start--, selected = start;
				
				updatemenu = true;
				return;
			}
			else if(optButtons[i].getEvent() == BUTTON_PRESSED)
			{
				touching = true;
			}
		}

		if(hidTouchCount() <= 0)
			touching = false;

		// draw captured screen so we can overlay "dynamic" stuff
		texDraw(screen, frameBuffer, 0, 0);
		drawGlowButton(selRectX, selRectY, rW, 70, clrSh, BUTTON_LIST, 0);
		drawText(title, frameBuffer, shared, selRectX + 15, selRectY + 26, fontSize, mnutxtClr);

		// if(updatemenu == true)
			// return;

		if((down & KEY_UP) || ((held & KEY_UP) && moving))
		{
			updatemenu = true;

			if(list_size == 0)
				soundPlay(SND_BOUNDS);
			else
				soundPlay(SND_LIST);
			selected--;
			if(selected < 0)
				selected = list_size;

			if((start > selected) && (start > 0))
				start--;
			if(list_size < 7)
				start = 0;
			if((selected - 6) > start)
				start = selected - 6;
		}
		else if((down & KEY_DOWN) || ((held & KEY_DOWN) && moving))
		{
			updatemenu = true;

			if(list_size == 0)
				soundPlay(SND_BOUNDS);
			else
				soundPlay(SND_LIST);
			selected++;
			if(selected > list_size)
				selected = 0;

			if((selected > (start + 6)) && ((start + 6) < list_size))
				start++;
			if(selected == 0)
				start = 0;
		}
		else if(down & KEY_RIGHT || ((held & KEY_RIGHT) && moving))
		{
			updatemenu = true;

			selected += 6;
			if(selected > list_size)
			{
				selected = list_size;
				soundPlay(SND_BOUNDS);
			}
			else
				soundPlay(SND_LIST);

			if((selected - 6) > start)
				start = selected - 6;
		}
		else if(down & KEY_LEFT || ((held & KEY_LEFT) && moving))
		{
			updatemenu = true;

			selected -= 6;
			if(selected < 0)
			{
				selected = 0;
				soundPlay(SND_BOUNDS);
			}
			else
				soundPlay(SND_LIST);

			if(selected < start)
				start = selected;
		}
		else if(down & KEY_A || fldNav[0].getEvent() == BUTTON_RELEASED || retEvent == MENU_DOUBLE_REL)
		{
			updatemenu = true;

			if(selected == 0)
			{
				soundPlay(SND_SELECT);
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
					soundPlay(SND_LOADING);
					std::string path = util::getTitleDir(data::curUser, data::curData) + "/" + folder;
					mkdir(path.c_str(), 777);
					path += "/";

					std::string root = "sv:/";
					fs::copyDirToDir(root, path);
					soundPlay(SND_BING);
				}
			}
			else
			{
				updatemenu = true;

				soundPlay(SND_POPUP);
				std::string scanPath = util::getTitleDir(data::curUser, data::curData);
				fs::dirList list(scanPath);

				std::string folderName = list.getItem(selected - 1);
				if(confirm("Are you sure you want to overwrite \"" + util::cutStr(folderName, 690, 24) + "\"?", "Overwrite"))
				{
					soundPlay(SND_LOADING);
					std::string toPath = util::getTitleDir(data::curUser, data::curData) + folderName + "/";
					//Delete and recreate
					fs::delDir(toPath);
					mkdir(toPath.c_str(), 777);

					std::string root = "sv:/";

					fs::copyDirToDir(root, toPath);
					soundPlay(SND_BING);
				}
			}
		}
		else if(down & KEY_B || fldNav[1].getEvent() == BUTTON_RELEASED)
		{
			updatemenu = true;

			soundPlay(SND_BACK);
			start = 0;
			selected = 0;
			fsdevUnmountDevice("sv");
			mstate = prevState;
		}
		else if(selected > 0 && (down & KEY_X || fldNav[2].getEvent() == BUTTON_RELEASED))
		{
			updatemenu = true;

			soundPlay(SND_POPUP);
			std::string scanPath = util::getTitleDir(data::curUser, data::curData);
			fs::dirList list(scanPath);

			std::string folderName = list.getItem(selected - 1);
			if(confirm("Are you sure you want to delete \"" + util::cutStr(folderName, 690, 24) + "\"?", "Delete"))
			{
				std::string delPath = scanPath + folderName + "/";
				fs::delDir(delPath);
			}
		}
		else if(selected > 0 && (down & KEY_Y || fldNav[3].getEvent() == BUTTON_RELEASED))
		{
			updatemenu = true;

			soundPlay(SND_POPUP);
			if(data::curData.getType() != FsSaveDataType_SystemSaveData)
			{
				std::string scanPath = util::getTitleDir(data::curUser, data::curData);
				fs::dirList list(scanPath);

				std::string folderName = list.getItem(selected - 1);
				if(confirm("Are you sure you want to restore \"" + util::cutStr(folderName, 690, 24) + "\"?", "Restore"))
				{
					soundPlay(SND_LOADING);
					std::string fromPath = util::getTitleDir(data::curUser, data::curData) + folderName + "/";
					std::string root = "sv:/";

					fs::delDir(root);
					fsdevCommitDevice("sv");

					fs::copyDirToDirCommit(fromPath, root, "sv");
					soundPlay(SND_BING);
				}
			}
			else
				ui::showMessage("Writing data to system save data is not allowed currently. It can brick your system.", "Sorry, bro:");
		}
		else if(down & KEY_PLUS || fldNav[fldNav.size() - 1].getEvent() == BUTTON_RELEASED)
		{
			updatemenu = true;
			ui::finish = true;
		}
	}
}
