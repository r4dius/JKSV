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

AudioDriverWaveBuf wavebuf[9] = {0};
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

	const u8 *snd_bin[9] =
	{
		snd_back_bin,
		snd_bing_bin,
		snd_bounds_bin,
		snd_list_bin,
		snd_loading_bin,
		snd_popup_bin,
		snd_select_bin,
		snd_bounds_bin,
		snd_bounds_bin,
	};
	
	u32 snd_bin_size[9] =
	{
		snd_back_bin_size,
		snd_bing_bin_size,
		snd_bounds_bin_size,
		snd_list_bin_size,
		snd_loading_bin_size,
		snd_popup_bin_size,
		snd_select_bin_size,
		snd_bounds_bin_size,
		snd_bounds_bin_size,
	};

	for(int i = 0; i < 9; i++) {
		mempool_size[i] = (snd_bin_size[i] + 0xFFF) &~ 0xFFF;
		mempool_ptr[i] = memalign(0x1000, mempool_size[i]);
		memcpy(mempool_ptr[i], snd_bin[i], snd_bin_size[i]);
		armDCacheFlush(mempool_ptr[i], mempool_size[i]);

		wavebuf[i].data_raw = mempool_ptr[i];
		wavebuf[i].size = snd_bin_size[i];
		wavebuf[i].start_sample_offset = 0;
		wavebuf[i].end_sample_offset = snd_bin_size[i]/2;

		// looping sound(s)
		if(i == 4)
			wavebuf[i].is_looping = true;
	}

	audrenRes = audrenInitialize(&arConfig);
	initedDriver = false;
	initedAudren = R_SUCCEEDED(audrenRes);
	if (initedAudren)
	{
		audrenRes = audrvCreate(&drv, &arConfig, 2);
		initedDriver = R_SUCCEEDED(audrenRes);
		if (!R_FAILED(audrenRes))
		{
			for(int i = 0; i < 9; i++) 
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

void sndPlay(int id)
{
	audrvMemPoolAttach(&drv, id);
	audrvVoiceStop(&drv, 0);
	audrvVoiceAddWaveBuf(&drv, 0, &wavebuf[id]);
	audrvVoiceStart(&drv, 0);
	audrvUpdate(&drv);
}

void soundExit()
{
	if (initedDriver)
		audrvClose(&drv);
	if (initedAudren)
		audrenExit();
}
