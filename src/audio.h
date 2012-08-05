#ifndef INCLUDE_AUDIO
#define INCLUDE_AUDIO

/* audio.h

   The audio subsystem.
   Atul Varma - 8/2/2003

*/

/* Maximum number of audio samples that can be assigned a
   sample ID. */
#define AUDIO_SAMPLES_MAX 50

/* Data structure for storing audio samples. */
typedef struct AudioSample {
	/* Buffer pointing to the audio sample data. */
	Uint8 *buffer;
	/* Length of the audio sample data. */
	Uint32 length;
} AudioSample;

void audio_init();
void audio_sample_add(const char *filename, int sample_id);
void audio_sample_free(int sample_id);
void audio_sample_play(int sample_id);
void audio_shutdown();
void audio_pause(int pause_flag);

#endif
