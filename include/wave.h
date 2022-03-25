#include <Arduino.h>
#include <U8g2lib.h>
#include <string>
#include <vector>
#include <math.h>
#define _USE_MATH_DEFINES

const int32_t freq[13] = {0, 262,	277,	294,	311,	330,	349,	367,	392,	415,	440,	466,	494};

const int LUT_SIZE = 1024;
const int Fs = 22000;

class Wave
{   
    private:

    public:
        int32_t LUT[LUT_SIZE];
        uint16_t diff[13];
};

class Sine : public Wave
{
    public:
        Sine()
        {
            for(int i = 0; i < LUT_SIZE; i++)
            {
                LUT[i] = (int32_t)(127*sinf(2.0*M_PI*(float)i/LUT_SIZE)) + 128;
            }
            float diff_float[13];
            for(int i = 0; i < 13; i++)
            {
                diff_float[i] = (freq[i]*LUT_SIZE)/Fs;
                diff[i] = diff_float[i];
            }
        }

        int32_t makeVout(int key, int octave)
        {
            static u_int16_t idx = 0;
            int shift = octave - 4;

            if (shift == 0)
            {
                idx += diff[key];
            }
            else if (shift > 0)
            {
                idx += diff[key] << shift;
            }
            else if (shift < 0)
            {
                idx += diff[key] >> -shift;
            }
            
            if(idx > LUT_SIZE-1){
                idx -= LUT_SIZE;
            }
            return LUT[idx];
        }
};

class Saw : public Wave
{
    public:
        Saw()
        {
            for(int i = 0; i < LUT_SIZE; i++)
            {
                LUT[i] = (int32_t)(255*(float)i/LUT_SIZE);
            }
            float diff_float[13];
            for(int i = 0; i < 13; i++)
            {
                diff_float[i] = (freq[i]*LUT_SIZE)/Fs;
                diff[i] = diff_float[i];
            }
        }

        int32_t makeVout(int key, int octave)
        {
            static u_int16_t idx = 0;
            int shift = octave - 4;

            if (shift == 0)
            {
                idx += diff[key];
            }
            else if (shift > 0)
            {
                idx += diff[key] << shift;
            }
            else if (shift < 0)
            {
                idx += diff[key] >> -shift;
            }
            
            if(idx > LUT_SIZE-1){
                idx -= LUT_SIZE;
            }
            return LUT[idx];
        }
};

class Tri : public Wave
{
    public:
        Tri()
        {
            for(int i = 0; i < LUT_SIZE; i++)
            {
                if(i < LUT_SIZE/2)
                {
                    LUT[i] = (int32_t)(255*2*(float)i/LUT_SIZE);
                } 
                else
                {
                    LUT[i] = (int32_t)(255*(float)LUT_SIZE/(2*i));      
                }
            }
            float diff_float[13];
            for(int i = 0; i < 13; i++)
            {
                diff_float[i] = (freq[i]*LUT_SIZE)/Fs;
                diff[i] = diff_float[i];
            }
        }

        int32_t makeVout(int key, int octave)
        {
            static u_int16_t idx = 0;
            int shift = octave - 4;

            if (shift == 0)
            {
                idx += diff[key];
            }
            else if (shift > 0)
            {
                idx += diff[key] << shift;
            }
            else if (shift < 0)
            {
                idx += diff[key] >> -shift;
            }
            
            if(idx > LUT_SIZE-1){
                idx -= LUT_SIZE;
            }
            return LUT[idx];
        }
};

class Squ : public Wave
{
    public:
        Squ()
        {
            for(int i = 0; i < LUT_SIZE; i++)
            {
                if(i < LUT_SIZE/2)
                {
                    LUT[i] = (int32_t)255;
                } 
                else
                {
                    LUT[i] = (int32_t)0;      
                }
            }
            float diff_float[13];
            for(int i = 0; i < 13; i++)
            {
                diff_float[i] = (freq[i]*LUT_SIZE)/Fs;
                diff[i] = diff_float[i];
            }
        }

        int32_t makeVout(int key, int octave)
        {
            static u_int16_t idx = 0;
            int shift = octave - 4;

            if (shift == 0)
            {
                idx += diff[key];
            }
            else if (shift > 0)
            {
                idx += diff[key] << shift;
            }
            else if (shift < 0)
            {
                idx += diff[key] >> -shift;
            }
            
            if(idx > LUT_SIZE-1){
                idx -= LUT_SIZE;
            }
            return LUT[idx];
        }
};