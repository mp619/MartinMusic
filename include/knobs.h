
#include <Arduino.h>
#include <U8g2lib.h>
#include <string>

class Knobs
{
private:
    uint8_t prev_knob = 0;
    uint8_t curr_knob = 0;
    int8_t count_knob = 0;
    uint8_t prev_dir = 0; //-1, then decrease, 0, then cant determine, 1 then decrease
    int num;
    int max_val;
    int min_val = 0;
    // string wave;

public:
    Knobs(int _num)
    {
        num = _num;
        if (_num == 3)
        {
            max_val = 16;
            // set a default volume of 12
            int default_vol = 12;
            count_knob = default_vol;
        }
        else if (_num == 2)
        {
            max_val = 8;
            // set a default volume of 4
            int default_oct = 4;
            count_knob = default_oct;
        }
        else if (_num == 1)
        {
            max_val = 6;
            // set a default volume of 4
            int default_wave = 1;
            count_knob = default_wave;
        }
    }

    uint8_t readKnob(uint8_t key)
    {
        uint8_t val;

        if (num == 1 | num == 3)
        {
            val = key & 3;
        }
        else
        {
            val = key & 12;
            val = val >> 2;
        }
        // access value of 2 LSBs as those are knob3a and knob3b

        return val;
    }

    void decodeKnob(uint8_t key)
    {
        // xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        curr_knob = readKnob(key);

        // xSemaphoreGive(keyArrayMutex);
        uint8_t knob3_state = prev_knob << 2;
        knob3_state += curr_knob;
        switch (knob3_state)
        {
        case 1:
            count_knob++;
            prev_dir = 1;
            break;
        case 2:
            count_knob--;
            prev_dir = -1;
            break;
        case 3: // impossible state
            count_knob += 2 * prev_dir;
            break;
        case 4:
            count_knob--;
            prev_dir = -1;
            break;
        case 6: // impossible state
            count_knob += 2 * prev_dir;
            break;
        case 7:
            count_knob++;
            prev_dir = 1;
            break;
        case 8:
            count_knob++;
            prev_dir = 1;
            break;
        case 9: // impossible state
            count_knob += 2 * prev_dir;
            break;
        case 11:
            count_knob--;
            prev_dir = -1;
            break;
        case 12: // impossible state
            count_knob += 2 * prev_dir;
            break;
        case 13:
            count_knob--;
            prev_dir = -1;
            break;
        case 14:
            count_knob++;
            prev_dir = 1;
            break;
        default: // no change
            prev_dir = 0;
            break;
        }
        prev_knob = curr_knob;
        if (count_knob > max_val)
        {
            count_knob = max_val;
        }
        else if (count_knob < min_val)
        {
            count_knob = min_val;
        }
    }

    int8_t get_count()
    {
        return count_knob;
    }

    int32_t get_wave(int32_t wave_in, int32_t currentStepSize)
    {
        static int32_t phaseAcc = 0;
        static int32_t phaseAcc_new = 0;
        static int32_t up = 1;
        static int32_t Vout = 0;
        // sawtooth
        if (wave_in == 0 |wave_in == 1 | wave_in == 2)
        {
            phaseAcc += currentStepSize;
            Vout = phaseAcc >> 24;
        }
        //  square
        if (wave_in == 3 | wave_in == 4)
        {
            phaseAcc += currentStepSize;
            if (phaseAcc > 1073741824)
            {
                Vout = 127;
            }
            else
            {
                Vout = -128;
            }
        }
        // triangular
        else if (wave_in == 5 | wave_in == 6)
        {
            if (up == 1)
            {
                phaseAcc_new += currentStepSize * 2;
            }
            else if (up == 0)
            {
                phaseAcc_new -= currentStepSize * 2;
            }
            if (phaseAcc_new < phaseAcc & up == 1)
            {
                up = 0;
            }
            else if (phaseAcc_new > phaseAcc & up == 0)
            {
                up = 1;
            }
            if (up == 1)
            {
                phaseAcc += currentStepSize * 2;
            }
            else if (up == 0)
            {
                phaseAcc -= currentStepSize * 2;
            }
            phaseAcc_new = phaseAcc;
            Vout = phaseAcc >> 24;
            // Serial.print("phaseAcc");
        }
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
        return Vout;
    }
};
