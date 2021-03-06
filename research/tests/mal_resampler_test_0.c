#define DR_WAV_IMPLEMENTATION
#include "../../../../dr_libs/dr_wav.h"

#define MAL_DEBUG_OUTPUT
#define MINI_AL_IMPLEMENTATION
#include "../../mini_al.h"
#include "../mal_resampler.h"

#define SAMPLE_RATE_IN  44100
#define SAMPLE_RATE_OUT 44100
#define CHANNELS        1
#define OUTPUT_FILE     "output.wav"

mal_sine_wave sineWave;

mal_uint32 on_read(mal_resampler* pResampler, mal_uint32 frameCount, void** ppFramesOut)
{
    mal_assert(pResampler->config.format == mal_format_f32);

    return (mal_uint32)mal_sine_wave_read_ex(&sineWave, frameCount, pResampler->config.channels, pResampler->config.layout, (float**)ppFramesOut);
}

int main(int argc, char** argv)
{
    mal_result result;
    mal_resampler_config resamplerConfig;
    mal_resampler resampler;

    mal_zero_object(&resamplerConfig);
    resamplerConfig.format = mal_format_f32;
    resamplerConfig.channels = CHANNELS;
    resamplerConfig.sampleRateIn = SAMPLE_RATE_IN;
    resamplerConfig.sampleRateOut = SAMPLE_RATE_OUT;
    resamplerConfig.algorithm = mal_resampler_algorithm_linear;
    resamplerConfig.endOfInputMode = mal_resampler_end_of_input_mode_consume;
    resamplerConfig.layout = mal_stream_layout_interleaved;
    resamplerConfig.onRead = on_read;
    resamplerConfig.pUserData = NULL;

    result = mal_resampler_init(&resamplerConfig, &resampler);
    if (result != MAL_SUCCESS) {
        printf("Failed to initialize resampler.\n");
        return -1;
    }

    mal_sine_wave_init(0.5, 400, resamplerConfig.sampleRateIn, &sineWave);


    // Write to a WAV file. We'll do about 10 seconds worth, making sure we read in chunks to make sure everything is seamless.
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = resampler.config.channels;
    format.sampleRate = resampler.config.sampleRateOut;
    format.bitsPerSample = 32;
    drwav* pWavWriter = drwav_open_file_write(OUTPUT_FILE, &format);

    float buffer[SAMPLE_RATE_IN*CHANNELS];
    float* pBuffer = buffer;
    mal_uint32 iterations = 10;
    mal_uint32 framesToReadPerIteration = mal_countof(buffer)/CHANNELS;
    for (mal_uint32 i = 0; i < iterations; ++i) {
        mal_uint32 framesRead = (mal_uint32)mal_resampler_read(&resampler, framesToReadPerIteration, &pBuffer);
        drwav_write(pWavWriter, framesRead*CHANNELS, buffer);
    }

    //float** test = &buffer;

    drwav_close(pWavWriter);

    (void)argc;
    (void)argv;
    return 0;
}