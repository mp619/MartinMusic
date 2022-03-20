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