#include "sounds.h"

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__)
#include <windows.h>
#else
#include "FMOD/inc/wincompat.h"
#endif

#include "FMOD/inc/fmod.h"
#include "FMOD/inc/fmod_errors.h"    /* optional */

#define CHANNEL_MUSIC 1
#define CHANNEL_HEARTBEAT 2

#define HEARTBEAT_PATH "data/sounds/other/heartbeat.wav"

SOUNDS::SOUNDS()
{
	music_is_playing=false;
	heartbeat_is_playing=false;
	initialized=false;
	current_music=NULL;

	if (FSOUND_GetVersion() < FMOD_VERSION)
		return;

	if (!FSOUND_Init(44100, 64, 0))
		return;

	initialized=true;
}

SOUNDS::~SOUNDS()
{
	if (initialized)
		FSOUND_Close();
}


bool SOUNDS::PlayMusic(const string &songname)
{
	if (!initialized)
		return false;

	if (songname==current_song)
		return true;

	if (music_is_playing)
	{
		if (current_music!=NULL)
		{
			FMUSIC_StopSong(current_music);				
			current_music=NULL;
		}
		else
			FSOUND_StopSound(CHANNEL_MUSIC);
	}

	// check if we have this song
	map < string, FMUSIC_MODULE *>::iterator m;
	map < string, FSOUND_SAMPLE *>::iterator s;

	m = songs.find(songname);
	if (m!=songs.end()) // yes, play it
	{
		current_music=(*m).second;
		FMUSIC_PlaySong(current_music);   
		current_song = songname;
		return true;
	}

	s=samples.find(songname);
	if (s!=samples.end())
	{
		FSOUND_Sample_SetMode((*s).second, FSOUND_LOOP_NORMAL);
		FSOUND_PlaySound(CHANNEL_MUSIC, (*s).second);
		current_song = songname;
		return true;
	}

	// Not in samples and not in music
	current_music = LoadMusic(songname);
	if (current_music!=NULL)
	{
		FMUSIC_PlaySong(current_music);   	
		current_song = songname;
		return true;
	}

	// Music not loaded
	FSOUND_SAMPLE *sample = LoadSound(songname);
	if (sample!=NULL)
	{
		FSOUND_Sample_SetMode(sample, FSOUND_LOOP_NORMAL);
		FSOUND_PlaySound(CHANNEL_MUSIC, sample);  	
		current_song = songname;
		return true;
	}
	return false;
}

bool SOUNDS::PlaySoundOnce(const string &soundname, int volume)
{
	if (!initialized)
		return false;

	// check if we have this sound
	FSOUND_SAMPLE *sample=NULL;
	map < string, FSOUND_SAMPLE *>::iterator s;
	s=samples.find(soundname);
	if (s!=samples.end())
		sample = (*s).second;
	else
		sample = LoadSound(soundname);

	if (sample!=NULL)
	{
		FSOUND_Sample_SetMode(sample, FSOUND_LOOP_OFF);
		int channel = FSOUND_PlaySound(FSOUND_FREE, sample);
		FSOUND_SetVolume(channel,volume);
	}
	return true;
}

FMUSIC_MODULE *SOUNDS::LoadMusic(const string &songname)
{
	if (!initialized)
		return false;

	FMUSIC_MODULE *mod = FMUSIC_LoadSong(songname.c_str());
	return mod;
}

FSOUND_SAMPLE *SOUNDS::LoadSound(const string &songname)
{
	if (!initialized)
		return false;

	FSOUND_SAMPLE *sample = FSOUND_Sample_Load(FSOUND_UNMANAGED,songname.c_str(),FSOUND_NORMAL | FSOUND_2D | FSOUND_MPEGACCURATE | FSOUND_NONBLOCKING, 0, 0);
	return sample;
}

bool SOUNDS::PlayHeartBeat()
{
	if (!initialized)
		return false;

	if (heartbeat_is_playing)
		return true;

	FSOUND_StopSound(CHANNEL_HEARTBEAT);

	// check if we have this song
	map < string, FSOUND_SAMPLE *>::iterator s;
	
	s=samples.find(HEARTBEAT_PATH);
	if (s!=samples.end())
	{
		FSOUND_Sample_SetMode((*s).second, FSOUND_LOOP_NORMAL);
		FSOUND_PlaySound(CHANNEL_HEARTBEAT, (*s).second);
		return true;
	}
	FSOUND_SAMPLE *sample = LoadSound(HEARTBEAT_PATH);
	if (sample!=NULL)
	{
		FSOUND_Sample_SetMode(sample, FSOUND_LOOP_NORMAL);
		FSOUND_PlaySound(CHANNEL_HEARTBEAT, sample);  	
		heartbeat_is_playing=true;
		return true;
	}
	return false;
}

bool SOUNDS::StopHeartBeat()
{
	if (!initialized)
		return false;

	if (!heartbeat_is_playing)
		return true;

	FSOUND_StopSound(CHANNEL_HEARTBEAT);
	heartbeat_is_playing=false;
	return true;
}