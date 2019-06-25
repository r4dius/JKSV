#ifndef UIUPDATE_H
#define UIUPDATE_H

//Contains declarations of ui updating functions

namespace ui
{
	void updateUserMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p);
	void updateTitleMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p);
	void updateFolderMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p);
	void updateBlacklistMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p);
}

#endif // USRSEL_H
