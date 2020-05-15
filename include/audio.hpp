#ifndef _AUDIO_HPP_
#define _AUDIO_HPP_

#include <iostream>
#include <vector>
#include <string>
#include <al.h>
#include <alc.h>
#include <sndfile.h>

class Audio
{
	public:

		Audio();
		void load_sound(std::string file_path);
		~Audio();
		std::vector<ALuint> sounds;

	private:

		ALCdevice* device;
		ALCcontext* context;
};

class Source
{
	public:

		Source();
		~Source();
		void set_volume(int volume);
		void set_looping(bool loop);
		void stop_sound();
		void play_sound(ALuint sound_buffer);
		bool is_playing() const;
		float get_elapsed_time() const;

	private:

		ALuint source_id;
};

#endif
