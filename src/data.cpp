#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include <switch.h>

#include "data.h"
#include "file.h"
#include "util.h"

//Sorts titles sort-of alphabetically
static struct
{
    bool operator()(data::titledata& a, data::titledata& b)
    {
        uint32_t tmpA, tmpB;
        for(unsigned i = 0; i < a.getTitle().length(); )
        {
            ssize_t uCnt = decode_utf8(&tmpA, (const uint8_t *)&a.getTitle().data()[i]);
            decode_utf8(&tmpB, (const uint8_t *)&b.getTitle().data()[i]);
            tmpA = tolower(tmpA);
            tmpB = tolower(tmpB);
            if(tmpA != tmpB)
                return tmpA < tmpB;

            i += uCnt;
        }

        return false;
    }
} sortTitles;

//Returns -1 for new
static int getUserIndex(const u128& id)
{
    for(unsigned i = 0; i < data::users.size(); i++)
    {
        if(data::users[i].getUID() == id)
            return i;
    }

    return -1;
}

static std::vector<uint64_t> blacklist;

static bool blacklisted(const uint64_t& id)
{
    for(unsigned i = 0; i < blacklist.size(); i++)
    {
        if(id == blacklist[i])
            return true;
    }

    return false;
}

namespace data
{
    titledata curData;
    user      curUser;
    std::vector<icn> icons;
    std::vector<user> users;
    bool sysSave = false, forceMount = true;

    void loadDataInfo()
    {
        //Clear titles + users just in case
        for(unsigned i = 0; i < users.size(); i++) {
            users[i].titles.clear();
            users[i].blktitles.clear();
		}

        users.clear();

        loadBlacklist();

        icn defIcon;
        defIcon.load(0, "romfs:/img/icn/icnDefault.png");
        icons.push_back(defIcon);

        FsSaveDataIterator saveIt;
        size_t total = 0;
        FsSaveDataInfo info;

        if(R_FAILED(fsOpenSaveDataIterator(&saveIt, FsSaveDataSpaceId_All)))
        {
            printf("SaveDataIterator Failed\n");
            return;
        }

        while(true)
        {
            if(R_FAILED(fsSaveDataIteratorRead(&saveIt, &info, 1, &total)) || total == 0)
                break;

            //If save data, not black listed or just ignore
            if(info.SaveDataType == FsSaveDataType_SaveData || sysSave)
            {
                int u = getUserIndex(info.userID);
                if(u == -1)
                {
                    user newUser;
                    if(newUser.init(info.userID) || (sysSave && newUser.initNoChk(info.userID)))
                    {
                        users.push_back(newUser);

                        u = getUserIndex(info.userID);
                        titledata newData;
                        newData.init(info);
                        if(newData.isMountable(newUser.getUID()) || !forceMount) {
                            if(!blacklisted(info.titleID))
								users[u].titles.push_back(newData);
							else
								users[u].blktitles.push_back(newData);
						}
                    }
                }
                else
                {
                    titledata newData;
                    newData.init(info);
                    if(newData.isMountable(users[u].getUID()) || !forceMount) {
						if(!blacklisted(info.titleID))
							users[u].titles.push_back(newData);
						else
							users[u].blktitles.push_back(newData);
					}
                }
            }
        }

        fsSaveDataIteratorClose(&saveIt);

        for(unsigned i = 0; i < users.size(); i++) {
            std::sort(users[i].titles.begin(), users[i].titles.end(), sortTitles);
            std::sort(users[i].blktitles.begin(), users[i].blktitles.end(), sortTitles);
		}

        curUser = users[0];
    }

    void exit()
    {
        for(unsigned i = 0; i < users.size(); i++)
            users[i].delIcon();

        for(unsigned i = 0; i < icons.size(); i++)
            icons[i].deleteData();
    }

    void icn::load(const uint64_t& _id, const uint8_t *jpegData, const size_t& jpegSize)
    {
        titleID = _id;
        iconTex = texLoadJPEGMem(jpegData, jpegSize);
    }

    void icn::load(const uint64_t& _id, const std::string& _png)
    {
        titleID = _id;
        iconTex = texLoadPNGFile(_png.c_str());
    }

    int findIcnIndex(const uint64_t& titleID)
    {
        for(unsigned i = 0; i < icons.size(); i++)
        {
            if(icons[i].getTitleID() == titleID)
                return i;
        }

        return -1;
    }

    void titledata::init(const FsSaveDataInfo& inf)
    {
        NsApplicationControlData *dat = new NsApplicationControlData;
        std::memset(dat, 0, sizeof(NsApplicationControlData));
        NacpLanguageEntry *ent = NULL;

        if(inf.SaveDataType == FsSaveDataType_SaveData)
            id = inf.titleID;
        else if(inf.SaveDataType == FsSaveDataType_SystemSaveData)
            id = inf.saveID;

        uID = inf.userID;
        type = (FsSaveDataType)inf.SaveDataType;
        size_t outSz = 0;

        if(R_SUCCEEDED(nsGetApplicationControlData(1, id, dat, sizeof(NsApplicationControlData), &outSz)) && outSz >= sizeof(dat->nacp) \
                && R_SUCCEEDED(nacpGetLanguageEntry(&dat->nacp, &ent)) && ent != NULL)
        {
            title.assign(ent->name);
            titleSafe.assign(util::safeString(title));
            if(titleSafe.empty())
            {
                char tmp[18];
                sprintf(tmp, "%016lX", id);
                titleSafe.assign(tmp);
            }

            int icnInd = findIcnIndex(id);
            if(icnInd == -1)
            {
                size_t icnSize = outSz - sizeof(dat->nacp);
                icn newIcn;
                newIcn.load(id, dat->icon, icnSize);
                icons.push_back(newIcn);

                icon = icons[findIcnIndex(id)];
            }
            else
                icon = icons[icnInd];
        }
        else
        {
            char tmp[18];
            sprintf(tmp, "%016lX", id);
            title.assign(tmp);
            titleSafe.assign(tmp);
            icon = icons[0];
        }
        delete dat;
    }

    bool titledata::isMountable(const u128& uID)
    {
        data::user tmpUser;
        tmpUser.initNoChk(uID);
        if(fs::mountSave(tmpUser, *this))
        {
            fsdevUnmountDevice("sv");
            return true;
        }
        return false;
    }

    bool user::init(const u128& _id)
    {
        userID = _id;

        AccountProfile prof;
        AccountProfileBase base;

        if(R_FAILED(accountGetProfile(&prof, userID)))
            return false;

        if(R_FAILED(accountProfileGet(&prof, NULL, &base)))
            return false;

        username.assign(base.username);
        if(username.empty())
            username = "Unknown";
        userSafe = util::safeString(username);

        size_t sz = 0;
        accountProfileGetImageSize(&prof, &sz);
        uint8_t *profJpeg = new uint8_t[sz];

        accountProfileLoadImage(&prof, profJpeg, sz, &sz);
        userIcon = texLoadJPEGMem(profJpeg, sz);

        delete[] profJpeg;

        accountProfileClose(&prof);

        return true;
    }

    bool user::initNoChk(const u128& _id)
    {
        userID = _id;

        AccountProfile prof;
        AccountProfileBase base;

        if(R_SUCCEEDED(accountGetProfile(&prof, userID)) && R_SUCCEEDED(accountProfileGet(&prof, NULL, &base)))
        {
            username.assign(base.username);
            userSafe = util::safeString(username);
            accountProfileClose(&prof);
        }
        else
        {
            username = "Unknown";
            userSafe = "Unknown";
            //This shouldn't happen too much
            userIcon = texLoadPNGFile("romfs:/img/icn/icnDefault.png");
        }

        return true;
    }

    void loadBlacklist()
    {
        blacklist.clear();
        if(fs::fileExists(fs::getWorkDir() + "blacklist.txt"))
        {
            std::string line;
            std::fstream bl(fs::getWorkDir() + "blacklist.txt", std::ios::in);

            while(std::getline(bl, line))
            {
                if(line[0] == '#' || line[0] == '\n')
                    continue;

                blacklist.push_back(std::strtoull(line.substr(0,18).c_str(), NULL, 16));
            }
            bl.close();
        }
    }

    void blacklistAdd(user& u, titledata& t)
    {
        std::fstream bl(fs::getWorkDir() + "blacklist.txt", std::ios::app);

        std::string titleLine = "#" + t.getTitle() + "\n";
        char idLine[255];
        sprintf(idLine, "0x%016lX %s\n", t.getID(), titleLine.c_str());

        //bl.write(titleLine.c_str(), titleLine.length());
        bl.write(idLine, std::strlen(idLine));
        bl.close();

        //Remove it from every user
        for(unsigned i = 0; i < users.size(); i++)
        {
            for(unsigned j = 0; j < users[i].titles.size(); j++)
            {
                if(users[i].titles[j].getID() == t.getID()) {
                    users[i].blktitles.push_back(users[i].titles[j]);
					std::sort(users[i].blktitles.begin(), users[i].blktitles.end(), sortTitles);
                    users[i].titles.erase(users[i].titles.begin() + j);
				}
            }
        }

        int uInd = getUserIndex(u.getUID());
        u = users[uInd];
    }
	
	void blacklistRemove(user& u, titledata& t)
    {
        // std::fstream bl(fs::getWorkDir() + "blacklist.txt", std::ios::app);

        // std::string titleLine = "#" + t.getTitle() + "\n";
        // char idLine[32];
        // sprintf(idLine, "0x%016lX\n", t.getID());

        // bl.write(titleLine.c_str(), titleLine.length());
        // bl.write(idLine, std::strlen(idLine));
        // bl.close();

        //Remove it from every user
        for(unsigned i = 0; i < users.size(); i++)
        {
            for(unsigned j = 0; j < users[i].blktitles.size(); j++)
            {
                if(users[i].blktitles[j].getID() == t.getID()) {
                    users[i].titles.push_back(users[i].blktitles[j]);
					std::sort(users[i].titles.begin(), users[i].titles.end(), sortTitles);
                    users[i].blktitles.erase(users[i].blktitles.begin() + j);
				}
            }
        }

        int uInd = getUserIndex(u.getUID());
        u = users[uInd];
    }
}
