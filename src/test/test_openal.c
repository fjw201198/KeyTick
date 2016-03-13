#include <stdio.h>
#include <unistd.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#define VOICE_FILE "../../sound/tick.wav"

int main(int argc, char* argv[])
{
    alutInit(&argc, argv);
    ALCdevice* device = alcOpenDevice(NULL);
    if (!device)
        return -1;
    ALCcontext* context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    if (alGetError() != AL_NO_ERROR)
    {
        return -1;
    }
    ALuint buffer = alutCreateBufferFromFile(VOICE_FILE);
    if (alGetError() != AL_NO_ERROR)
    {
        return -2;
    }
    ALuint source;
    alGenSources(1, &source);
    if (alGetError() != AL_NO_ERROR)
    {
        return -3;
    }
    alSourcei(source, AL_BUFFER, buffer);
    if (alGetError() != AL_NO_ERROR)
    {
        return -4;
    }
    alSourcePlay(source);
    if (alGetError() != AL_NO_ERROR)
    {
        return -5;
    }
    sleep(10);
    context = alcGetCurrentContext();
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
    alutExit();
    return 0;
}
