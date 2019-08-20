#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <inttypes.h>
#include <switch.h>

#include "snd.h"
#include "snd_back_bin.h"
#include "snd_bing_bin.h"
#include "snd_list_bin.h"
#include "snd_loading_bin.h"
#include "snd_popup_bin.h"
#include "snd_select_bin.h"
#include "snd_tick_bin.h"
#include "snd_bounds_bin.h"
#include "snd_touchout_bin.h"

Result audrenRes;
bool initedDriver = false;
bool initedAudren = false;
static AudioDriver drv;
AudioDriverWaveBuf sndBackBuf = {0};
AudioDriverWaveBuf sndBingBuf = {0};
AudioDriverWaveBuf sndBoundsBuf = {0};
AudioDriverWaveBuf sndListBuf = {0};
AudioDriverWaveBuf sndLoadingBuf = {0};
AudioDriverWaveBuf sndPopupBuf = {0};
AudioDriverWaveBuf sndSelectBuf = {0};
AudioDriverWaveBuf sndTickBuf = {0};
AudioDriverWaveBuf sndTouchoutBuf = {0};
int mpid[9];

static const AudioRendererConfig arConfig =
{
	.output_rate     = AudioRendererOutputRate_48kHz,
	.num_voices      = 24,
	.num_effects     = 0,
	.num_sinks       = 1,
	.num_mix_objs    = 1,
	.num_mix_buffers = 2,
};

void soundInit()
{
	void* mempool_ptr[9];
	size_t mempool_size[9];

	int i = 0;
	mempool_size[i] = (snd_back_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_back_bin, snd_back_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_bing_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_bing_bin, snd_bing_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_bounds_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_bounds_bin, snd_bounds_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_list_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_list_bin, snd_list_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_loading_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_loading_bin, snd_loading_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_popup_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_popup_bin, snd_popup_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_select_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_select_bin, snd_select_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_tick_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_tick_bin, snd_tick_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i++;
	mempool_size[i] = (snd_touchout_bin_size + 0xFFF) &~ 0xFFF;
	mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
	memcpy(mempool_ptr[i], snd_touchout_bin, snd_touchout_bin_size);
	armDCacheFlush(mempool_ptr[i], mempool_size[i]);

	i = 0;
	sndBackBuf.data_raw = mempool_ptr[i];
	sndBackBuf.size = snd_back_bin_size;
	sndBackBuf.start_sample_offset = 0;
	sndBackBuf.end_sample_offset = snd_back_bin_size/2;

	i++;
	sndBingBuf.data_raw = mempool_ptr[i];
	sndBingBuf.size = snd_bing_bin_size;
	sndBingBuf.start_sample_offset = 0;
	sndBingBuf.end_sample_offset = snd_bing_bin_size/2;

	i++;
	sndBoundsBuf.data_raw = mempool_ptr[i];
	sndBoundsBuf.size = snd_bounds_bin_size;
	sndBoundsBuf.start_sample_offset = 0;
	sndBoundsBuf.end_sample_offset = snd_bounds_bin_size/2;

	i++;
	sndListBuf.data_raw = mempool_ptr[i];
	sndListBuf.size = snd_list_bin_size;
	sndListBuf.start_sample_offset = 0;
	sndListBuf.end_sample_offset = snd_list_bin_size/2;

	i++;
	sndLoadingBuf.data_raw = mempool_ptr[i];
	sndLoadingBuf.size = snd_loading_bin_size;
	sndLoadingBuf.start_sample_offset = 0;
	sndLoadingBuf.end_sample_offset = snd_loading_bin_size/2;
	sndLoadingBuf.is_looping = true;

	i++;
	sndPopupBuf.data_raw = mempool_ptr[i];
	sndPopupBuf.size = snd_popup_bin_size;
	sndPopupBuf.start_sample_offset = 0;
	sndPopupBuf.end_sample_offset = snd_popup_bin_size/2;

	i++;
	sndSelectBuf.data_raw = mempool_ptr[i];
	sndSelectBuf.size = snd_select_bin_size;
	sndSelectBuf.start_sample_offset = 0;
	sndSelectBuf.end_sample_offset = snd_select_bin_size/2;

	i++;
	sndTickBuf.data_raw = mempool_ptr[i];
	sndTickBuf.size = snd_tick_bin_size;
	sndTickBuf.start_sample_offset = 0;
	sndTickBuf.end_sample_offset = snd_tick_bin_size/2;

	i++;
	sndTouchoutBuf.data_raw = mempool_ptr[i];
	sndTouchoutBuf.size = snd_touchout_bin_size;
	sndTouchoutBuf.start_sample_offset = 0;
	sndTouchoutBuf.end_sample_offset = snd_touchout_bin_size/2;

	audrenRes = audrenInitialize(&arConfig);
	initedDriver = false;
	initedAudren = R_SUCCEEDED(audrenRes);
	if (initedAudren)
	{
		audrenRes = audrvCreate(&drv, &arConfig, 2);
		initedDriver = R_SUCCEEDED(audrenRes);
		if (!R_FAILED(audrenRes))
		{
			for(i = 0; i < 9; i++) 
				mpid[i] = audrvMemPoolAdd(&drv, mempool_ptr[i], mempool_size[i]);

			audrvMemPoolAttach(&drv, mpid[0]);

			static const u8 sink_channels[] = { 0, 1 };
			audrvDeviceSinkAdd(&drv, AUDREN_DEFAULT_DEVICE_NAME, 2, sink_channels);

			audrvUpdate(&drv);
			audrenStartAudioRenderer();
			audrvVoiceInit(&drv, 0, 1, PcmFormat_Int16, 44000);
			audrvVoiceSetDestinationMix(&drv, 0, AUDREN_FINAL_MIX_ID);
			audrvVoiceSetMixFactor(&drv, 0, 1.0f, 0, 0);
			audrvVoiceSetMixFactor(&drv, 0, 1.0f, 0, 1);
			audrvVoiceStart(&drv, 0);
		}
	}
}

void sndPlay(AudioDriverWaveBuf *wavebuf, int id)
{
	audrvMemPoolAttach(&drv, id);
	audrvVoiceStop(&drv, 0);
	audrvVoiceAddWaveBuf(&drv, 0, wavebuf);
	audrvVoiceStart(&drv, 0);
	audrvUpdate(&drv);
}

void sndBack()
{
	if (initedDriver)
		sndPlay(&sndBackBuf, mpid[0]);
}

void sndBing()
{
	if (initedDriver)
		sndPlay(&sndBingBuf, mpid[1]);
}

void sndBounds()
{
	if (initedDriver)
		sndPlay(&sndBoundsBuf, mpid[2]);
}

void sndList()
{
	if (initedDriver)
		sndPlay(&sndListBuf, mpid[3]);
}

void sndLoading()
{
	if (initedDriver)
		sndPlay(&sndLoadingBuf, mpid[4]);
}

void sndPopup()
{
	if (initedDriver)
		sndPlay(&sndPopupBuf, mpid[5]);
}

void sndSelect()
{
	if (initedDriver)
		sndPlay(&sndSelectBuf, mpid[6]);
}

void sndTick()
{
	if (initedDriver)
		sndPlay(&sndTickBuf, mpid[7]);
}

void sndTouchout()
{
	if (initedDriver)
		sndPlay(&sndTouchoutBuf, mpid[8]);
}

void soundExit()
{
	if (initedDriver)
		audrvClose(&drv);
	if (initedAudren)
		audrenExit();
}
