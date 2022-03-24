#include <Arduino.h>
#include <U8g2lib.h>
#include <map>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>
#include <knobs.h>
#include <string>
#define _USE_MATH_DEFINES
#include <cmath>
const int32_t stepSizes[13] = {0, 51076057, 54113197, 57330936, 60740010,
                               64351799, 68178356, 72232452, 76527617,
                               81078186, 85899346, 91007187, 96418756};
const float key_freq_sine[13] = {0, 0.07472, 0.079163, 0.08387, 0.088858, 0.094141, 0.099739,
                                 0.10567, 0.111954, 0.118611, 0.125664, 0.133136, 0.141053};
volatile int32_t currentStepSine = 0;
static int32_t phaseAcc = 0;
static int32_t phaseAcc_new = 0;
static int32_t up = 1;

int main()
{
    while (1)
    {
        static int32_t Vout = 0;
        int32_t current = 51076057;
        phaseAcc += current;
        double x = 2 * 3.14159265358979323846 * phaseAcc; // StepSize;
        float sine = (sin(x))* 255.0;
        Serial.println(sine);
        int32_t sine_int = sine;
        Vout = sine_int >> 24;
        Serial.println(Vout);
    }
};

//*****************sinewave
            // else if (knob1.get_count() == 7 | knob1.get_count() == 8)
            // {
            // static float phaseAcc_sine = 0;
            //     // key_freq_sine[id] = f / f_s * 2 * M_PI;
            //     // phaseAcc += currentStepSize;
            //     // float sine = (sin( currentStepSine + phaseAcc)) * 255.0;
            //     // int32_t current = 51076057;
            //     // phaseAcc += current;
            //     // double x = 2 * 3.14159265358979323846*phaseAcc;//StepSize;
            //     // float sine = (sin(x)) * 255.0;
            //     // Serial.println(sine);
            //     // int32_t sine_int = sine;
            //     // Vout = sine_int >> 24;
            //     // Serial.println(Vout);
            //     // float currentSine=  0.07472;
            //     phaseAcc_sine += (currentStepSine * 255);
            //     // double x = 2 * 3.14159265358979323846 * phaseAcc/180; // StepSize;
            //     // float y= sin(0);
            //     // Serial.println(phaseAcc_sine);
            //     float sine = (sin(phaseAcc_sine)) * 127;
            //     // Serial.println(sine);

            //     int32_t sine_int = sine;
            //     Vout = sine_int;

            //     // Serial.println(sine_int);
            //     //   127*(1+sineLUT)
            //     //   if Vout > 127 Vout = 127
            // }