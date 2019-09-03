#ifndef SND_H
#define SND_H

enum sndName
{
	SND_BACK,
	SND_BING,
	SND_BOUNDS,
	SND_LIST,
	SND_LOADING,
	SND_POPUP,
	SND_SELECT,
	SND_TICK,
	SND_TOUCHOUT,
};

void soundInit();
void soundExit();

void soundPlay(int soundId);

#endif // SND_H
