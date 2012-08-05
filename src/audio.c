#include "globals.h"

#include <stdlib.h>

#include "SDL.h"

#include "debug.h"
#include "audio.h"

/* The audio sample slots.  Each array index signifies a
   sample ID. */
static AudioSample g_samples[AUDIO_SAMPLES_MAX];

/* The audio spec that is currently being used by the
   audio subsystem. */
static SDL_AudioSpec g_audiospec;

/* Pointer to the current audio buffer being played. */
static Uint8 *g_audio_buffer;
/* Pointer to the current position of the audio buffer being
   played.  This will be g_audio_buffer + (some number). */
static Uint8 *g_audio_buffer_pos;
/* The remaining length to the end of the audio buffer, relative to
   the current position of the audio buffer. */
static int g_audio_buffer_len;

/* Clears the given sample ID slot.  This is used internally and does
   NOT deallocate any memory taken up by the sample.  For that, use
   audio_sample_free(). */
void audio_sample_clear(int sample_id)
{
	g_samples[sample_id].buffer = NULL;
	g_samples[sample_id].length = 0;
}

/* Returns whether the given sample ID slot has been assigned to
   a sample or not. */
int audio_sample_is_empty(int sample_id)
{
	return (g_samples[sample_id].buffer == NULL);
}

/* The audio function callback takes the following parameters:
       stream:  A pointer to the audio buffer to be filled
       len:     The length (in bytes) of the audio buffer

   Some code taken and modified from SDL audio examples
   at \docs\html\guideaudioexamples.html .
	   
*/
void audio_fill_buffer(void *udata, Uint8 *stream, int len)
{
    /* Only play if we have data left */
    if ( g_audio_buffer_len == 0 )
        return;

    /* Mix as much data as possible */
    len = ( len > g_audio_buffer_len ? g_audio_buffer_len : len );
    //SDL_MixAudio(stream, g_audio_buffer_pos, len, SDL_MIX_MAXVOLUME);
	memcpy(stream, g_audio_buffer_pos, len);
    g_audio_buffer_pos += len;
    g_audio_buffer_len -= len;
}

/* Clears all the sample IDs in the audio subsystem, marking them
   as empty.  Used internally by audio_init(). */
void audio_clear_all_samples()
{
	int i;

	for (i = 0; i < AUDIO_SAMPLES_MAX; i++) {
		audio_sample_clear(i);
	}
}

/* Some code taken and modified from SDL audio examples
   at \docs\html\guideaudioexamples.html . */
void audio_init()
{
#ifdef USE_AUDIO
    /* Set the audio format */
    g_audiospec.freq = 22050;
    g_audiospec.format = AUDIO_S16;
    g_audiospec.channels = 1;    /* 1 = mono, 2 = stereo */
    g_audiospec.samples = 1024;  /* Good low-latency value for callback */
    g_audiospec.callback = audio_fill_buffer;
    g_audiospec.userdata = NULL;

	g_audio_buffer_len = 0;

    /* Open the audio device, forcing the desired format */
    if ( SDL_OpenAudio(&g_audiospec, NULL) < 0 ) {
        err("Couldn't open audio\n", 1);
    }

#endif

	/* Clear all the audio sample ID slots, marking them as empty. */
	audio_clear_all_samples();
}

/* Adds the given sample, assigning it to the given sample ID slot. */
void audio_sample_add(const char *filename, int sample_id)
{
#ifdef USE_AUDIO
	SDL_AudioSpec wav_audiospec;
	Uint8 *buf;
	Uint32 length;

	/* If the given sample ID is already occupied, don't do anything. */
	if (!audio_sample_is_empty(sample_id)) return;

	if (!SDL_LoadWAV(filename, &wav_audiospec, &buf, &length)) {
		err("Couldn't load WAV file.\n", 1);
	}

	g_samples[sample_id].buffer = buf;
	g_samples[sample_id].length = length;
#endif
}

/* Frees the memory allocated by the given sample ID and clears the
   sample ID slot. */
void audio_sample_free(int sample_id)
{
	if (audio_sample_is_empty(sample_id)) return;
	SDL_FreeWAV(g_samples[sample_id].buffer);
	audio_sample_clear(sample_id);
}

/* Plays the given sample ID. */
void audio_sample_play(int sample_id)
{
	if (audio_sample_is_empty(sample_id)) return;
	SDL_LockAudio();
	g_audio_buffer = g_samples[sample_id].buffer;
	g_audio_buffer_pos = g_audio_buffer;
	g_audio_buffer_len = g_samples[sample_id].length;
	SDL_UnlockAudio();
}

/* Shuts down the audio subsystem. */
void audio_shutdown()
{
#ifdef USE_AUDIO
	SDL_CloseAudio();
#endif
}

/* Pauses/unpauses playback of audio. */
void audio_pause(int pause_flag)
{
#ifdef USE_AUDIO
	SDL_PauseAudio(pause_flag);
#endif
}
