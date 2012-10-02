//===============================================================================================// SOUND//===============================================================================================#include <stdlib.h>#include <stdio.h>#include <string.h>#include <unistd.h>#include <sys/stat.h>#include <fcntl.h>#include <sys/time.h>#include <sys/types.h>#include <ctype.h>#include <SDL/SDL.h>#include <time.h>#include "snes9x.h"#include "soundux.h"#include "spc700.h"#include "sdlaudio.h"static int Rates[8] ={	0, 8192, 11025, 16000, 22050, 32000, 44100, 48000};static int BufferSizes [8] ={	0, 256, 256, 256, 512, 512, 1024, 1024};static uint8 Buf[MAX_BUFFER_SIZE] __attribute__((aligned(4)));#define FIXED_POINT 0x10000#define FIXED_POINT_SHIFT 16#define FIXED_POINT_REMAINDER 0xffffstatic volatile bool8 block_signal = FALSE;static volatile bool8 block_generate_sound = FALSE;static volatile bool8 pending_signal = FALSE;SDL_mutex *sound_mutex;SDL_cond *sound_cv;static void sdl_audio_callback (void *userdata, Uint8 *stream, int len){	SDL_LockMutex(sound_mutex);	S9xMixSamplesO (stream, len/2, 0);	SDL_CondSignal(sound_cv);	SDL_UnlockMutex(sound_mutex);	return;}bool8_32 S9xOpenSoundDevice (int mode, bool8_32 stereo, int buffer_size) // called from S9xInitSound in ../soundux.cpp{	SDL_AudioSpec	*audiospec;	so.sixteen_bit = TRUE;	so.stereo = stereo;	so.playback_rate = Rates[mode & 0x07];	so.buffer_size = buffer_size;	S9xSetPlaybackRate (so.playback_rate);	SDL_InitSubSystem (SDL_INIT_AUDIO);	audiospec = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));	audiospec->freq = so.playback_rate;	audiospec->channels = so.stereo ? 2 : 1;	audiospec->format = so.sixteen_bit ? AUDIO_S16 : AUDIO_U8;	audiospec->samples = so.buffer_size;	audiospec->callback = sdl_audio_callback;	if (SDL_OpenAudio (audiospec, NULL) < 0)	{		printf ("Failed\n");		free (audiospec);		audiospec = NULL;		return FALSE;	}	sound_mutex = SDL_CreateMutex();	sound_cv = SDL_CreateCond(); 	SDL_PauseAudio (0);	printf ("Rate: %d, Buffer size: %d, 16-bit: %s, Stereo: %s, Encoded: %s\n",		so.playback_rate, so.buffer_size, so.sixteen_bit ? "yes" : "no",		so.stereo ? "yes" : "no", so.encoded ? "yes" : "no");	return (TRUE);}void S9xReinitSound(int new_freq){	SDL_PauseAudio(1);	SDL_CloseAudio();	S9xOpenSoundDevice(Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize);}void S9xGenerateSound (){} 