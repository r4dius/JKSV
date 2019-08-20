#ifndef SOUND_H
#define SOUND_H

#ifdef __cplusplus
extern "C"
{
#endif

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

void sndPlay(int id);

#ifdef __cplusplus
}
#endif

#endif // SOUND_H
