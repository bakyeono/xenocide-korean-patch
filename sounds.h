#include <string>
#include <map>

using namespace std;

struct FMUSIC_MODULE;
struct FSOUND_SAMPLE;

class SOUNDS {
private:
	string current_song;
	bool initialized;
	bool music_is_playing;
	bool heartbeat_is_playing;
	FMUSIC_MODULE *current_music;
	map < string, FMUSIC_MODULE *> songs;
	map < string, FSOUND_SAMPLE *> samples;

	FMUSIC_MODULE *LoadMusic(const string &songname);
	FSOUND_SAMPLE *LoadSound(const string &songname);
public:
	SOUNDS();
	~SOUNDS();
	bool PlayMusic(const string &songname); // plays in loop (single)
	bool PlaySoundOnce(const string &soundname, int volume=100); // plays once (many at the same time)
	bool PlayHeartBeat();
	bool StopHeartBeat();
};

