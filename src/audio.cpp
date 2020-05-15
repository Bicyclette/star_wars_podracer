#include "audio.hpp"

Audio::Audio()
{
	device = alcOpenDevice(nullptr);
	if(device == nullptr)
	{
		std::cerr << "Error: failed initializing audio device." << std::endl;
	}

	context = alcCreateContext(device, nullptr);
	if(context == nullptr)
	{
		std::cerr << "Error: failed creating audio context." << std::endl;
	}

	if(!alcMakeContextCurrent(context))
	{
		std::cerr << "Error: failed activating context." << std::endl;
	}
}

Audio::~Audio()
{
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);
	alDeleteBuffers(sounds.size(), sounds.data());
}

void Audio::load_sound(std::string file_path)
{
	SF_INFO fileInfos;
	SNDFILE* file = sf_open(file_path.c_str(), SFM_READ, &fileInfos);
	if(file == nullptr)
	{
		std::cerr << "Error: failed reading sound file." << std::endl;
	}

	ALsizei samples_count = static_cast<ALsizei>(fileInfos.channels * fileInfos.frames);
	ALsizei sample_rate = static_cast<ALsizei>(fileInfos.samplerate);
	std::vector<ALshort> samples(samples_count);

	if(sf_read_short(file, samples.data(), samples_count) < samples_count)
	{
		std::cerr << "Error: samples read from file doesn't match the actual samples count." << std::endl;
	}

	sf_close(file);

	// query file format
	ALenum format;
	if(fileInfos.channels == 1)
		format = AL_FORMAT_MONO16;
	else if(fileInfos.channels == 2)
		format = AL_FORMAT_STEREO16;
	else
		std::cerr << "Error: unknown sound file format." << std::endl;

	// create sound buffer
	ALuint sound_buffer;
	alGenBuffers(1, &sound_buffer);

	alBufferData(sound_buffer, format, samples.data(), samples_count * sizeof(ALushort), sample_rate);

	if(alGetError() != AL_NO_ERROR)
		std::cerr << "Error: failed filling sound buffer." << std::endl;

	// append it to the buffers array
	sounds.push_back(sound_buffer);
}

// ##################################################
// ##################################################
// ##################################################

Source::Source()
{
	alGenSources(1, &source_id);
}

Source::~Source()
{
	alSourcei(source_id, AL_BUFFER, 0);
	alDeleteSources(1, &source_id);
}

void Source::set_volume(int volume)
{
	alSourcef(source_id, AL_GAIN, static_cast<float>(volume) / 100.0f);
}

void Source::set_looping(bool loop)
{
	if(loop)
		alSourcei(source_id, AL_LOOPING, AL_TRUE);
	else
		alSourcei(source_id, AL_LOOPING, AL_FALSE);
}

void Source::stop_sound()
{
	alSourceStop(source_id);
}

void Source::play_sound(ALuint sound_buffer)
{
	alSourcei(source_id, AL_BUFFER, sound_buffer);
	alSourcePlay(source_id);
}

bool Source::is_playing() const
{
	ALint status;
	alGetSourcei(source_id, AL_SOURCE_STATE, &status);
	return status == AL_PLAYING;
}

float Source::get_elapsed_time() const
{
	ALfloat elapsed_time = 0.0f;
	alGetSourcef(source_id, AL_SEC_OFFSET, &elapsed_time);
	return static_cast<float>(elapsed_time);
}
