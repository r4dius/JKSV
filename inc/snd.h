#ifndef SOUND_H
#define SOUND_H

#ifdef __cplusplus
extern "C"
{
#endif

void soundInit();
void soundExit();

void sndPlay(AudioDriverWaveBuf *wavebuf, int id);

void sndBack();
void sndBing();
void sndBounds();
void sndList();
void sndPopup();
void sndLoading();
void sndSelect();
void sndTick();
void sndTouchout();

#ifdef __cplusplus
}
#endif

#endif // SOUND_H
