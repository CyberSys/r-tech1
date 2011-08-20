#include <string.h>

#include "audio.h"
#include "debug.h"

namespace Util{

#ifdef USE_SDL1
AudioConverter::AudioConverter(Encoding inputEncoding, int inputChannels, int inputFrequency,
               Encoding outputEncoding, int outputChannels, int outputFrequency){

    SDL_BuildAudioCVT(&conversion, inputEncoding, inputChannels, inputFrequency,
                      outputEncoding, outputChannels, outputFrequency);
}
    
AudioConverter::~AudioConverter(){
}
    
int AudioConverter::convertedLength(int length){
    return length;
}

int AudioConverter::convert(void * input, int length){
    if (conversion.needed){
        conversion.buf = (Uint8*) input;
        conversion.len = length;
        /* then convert to whatever the real output wants */
        SDL_ConvertAudio(&conversion);
        return conversion.len_cvt;
    } else {
        return length;
    }
}
#else
AudioConverter::AudioConverter(Encoding inputEncoding, int inputChannels, int inputFrequency,
               Encoding outputEncoding, int outputChannels, int outputFrequency):
buffer(NULL),
bufferSize(0){

    input.bytes = inputEncoding;
    input.channels = inputChannels;
    input.frequency = inputFrequency;

    output.bytes = outputEncoding;
    output.channels = outputChannels;
    output.frequency = outputFrequency;

    sizeRatio = (double) byteSize(output) * output.frequency / ((double) byteSize(input) * input.frequency);
}
    
int AudioConverter::byteSize(const Format & what){
    return encodingBytes(what.bytes) * what.channels;
}
    
/* how many bytes an encoding takes up */
int AudioConverter::encodingBytes(Encoding what){
    switch (what){
        case Signed16: return 2;
        case Float32: return 4;
    }

    return 1;
}
 
int AudioConverter::convertedLength(int length){
    // return length * sizeRatio;

    int total = length * sizeRatio;

    /* make sure we get an even number of samples */
    if (total % byteSize(output) != 0){
        total -= total % byteSize(output);
    }
    return total;
}

double CubicInterpolate(double y0,double y1,
                        double y2,double y3,
                        double mu){
    double a0,a1,a2,a3,mu2;

    mu2 = mu*mu;
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;

    return (a0*mu*mu2+a1*mu2+a2*mu+a3);
}

template <class Size>
Size clamp(double input){
    Size top = (1 << (sizeof(Size) * 8 - 1)) - 1;
    Size bottom = -(1 << (sizeof(Size) * 8 - 1));
    if (input > top){
        return top;
    }
    if (input < bottom){
        return bottom;
    }
    return input;
}

template <>
float clamp(double input){
    return input;
}

template <class Size>
void doConvertRate(Size * input, int inputSamples, int inputChannels, Size * buffer, int outputSamples, int outputChannels, double ratio){
    for (int sample = 0; sample < outputSamples; sample += 1){
        double inputSample = sample / ratio;

        for (int channel = 0; channel < outputChannels; channel += 1){

            int inputChannel = inputChannels > channel ? channel : inputChannels - 1;

            int sample0 = ((int) inputSample - 1) * inputChannels + inputChannel;
            int sample1 = ((int) inputSample + 0) * inputChannels + inputChannel;
            int sample2 = ((int) inputSample + 1) * inputChannels + inputChannel;
            int sample3 = ((int) inputSample + 2) * inputChannels + inputChannel;

            if (sample0 < 0){
                sample0 = sample1;
            }

            if (sample2 >= inputSamples * inputChannels){
                sample2 = sample1;
            }

            if (sample3 >= inputSamples * inputChannels){
                sample3 = sample2;
            }

            buffer[sample * outputChannels + channel] = clamp<Size>(CubicInterpolate(input[sample0], input[sample1], input[sample2], input[sample3], inputSample - (int) inputSample));

            // Global::debug(0) << "Input[" << sample << "] " << channel << ": " << input[sample1] << " Output: " << buffer[sample * 2 + channel] << std::endl;
        }
    }
}

int AudioConverter::convert(void * input, int length){
    /* no conversion needed */
    if (this->input == this->output){
        return length;
    }

    int total = convertedLength(length);
    /*
    if (total % byteSize(output) != 0){
        total -= total % byteSize(output);
    }
    */

    /* cache the buffer for future use */
    if (total > bufferSize){
        delete[] buffer;
        bufferSize = total;
        buffer = new char[bufferSize];
    }

    double frequencyRatio = (double) output.frequency / (double) this->input.frequency;

    if (this->input.bytes == output.bytes){
        switch (this->input.bytes){
            case Signed16: doConvertRate<signed short>((signed short*) input, length / sizeof(signed short) / this->input.channels, this->input.channels,
                                                       (signed short*) buffer, total / sizeof(signed short) / output.channels, output.channels,
                                                       frequencyRatio); break;
            case Float32: doConvertRate<float>((float*) input, length / sizeof(float) / this->input.channels, this->input.channels,
                                               (float*) buffer, total / sizeof(float) / output.channels, output.channels, frequencyRatio); break;
        }
    }
    
    memcpy(input, buffer, total);

    return total;
}

AudioConverter::~AudioConverter(){
    delete[] buffer;
}
        
bool AudioConverter::Format::operator==(const AudioConverter::Format & him) const {
    return this->bytes == him.bytes &&
           this->channels == him.channels &&
           this->frequency == him.frequency;
}

#endif

}