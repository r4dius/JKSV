#include "ui.h"
#include "file.h"
#include "snd.h"

extern "C"
{
	void userAppInit(void)
	{
		appletInitialize();
		romfsInit();
		hidInitialize();
		nsInitialize();
		setsysInitialize();
		accountInitialize();

		void *add = NULL;
		svcSetHeapSize(&add, 0x12000000);
	}

	void userAppExit(void)
	{
		appletExit();
		romfsExit();
		hidExit();
		nsExit();
		setsysExit();
		accountExit();
	}
}

int main(int argc, const char *argv[])
{
	fs::init();
	graphicsInit(1280, 720);
	soundInit();
	data::loadDataInfo();
	ui::init();
	//built with 'make debug CFLAGS:=-D__debug__'
#ifdef __debug__
	socketInitializeDefault();
	nxlinkStdio();
#endif
	while(appletMainLoop())
	{
		hidScanInput();

		uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
		uint64_t held = hidKeysHeld(CONTROLLER_P1_AUTO);

		touchPosition p;
		hidTouchRead(&p, 0);

		if(down & KEY_PLUS || ui::finish)
			break;
		gfxBeginFrame();

		ui::runApp(down, held, p);

		gfxEndFrame(ui::shared);
	}
#ifdef __debug__
	socketExit();
#endif
	ui::exit();
	data::exit();
	soundExit();
	graphicsExit();
}
