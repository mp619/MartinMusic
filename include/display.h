#include <Arduino.h>
#include <U8g2lib.h>
#include <string>

// Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Global vaiables
extern volatile uint8_t position;
extern volatile bool _west;
extern volatile bool _east;

class Display
{
    private:
        uint8_t y_1 = 0;
        uint8_t y_2 = 0;
        uint8_t y_3 = 0;

        uint8_t north = 0;
        uint8_t east = 0;
        uint8_t southeast = 0;
        uint8_t west = 0;
        uint8_t southwest = 0;
        uint8_t south = 0;
        uint8_t centre = 0;

        bool print_type;

    public:
        Display(bool type){
            if (type == true){
                y_1 = 10;
                y_2 = 20;
                y_3 = 30;

                north = 45;
                east = 95;
                southeast = 95;
                west = 5;
                southwest  = 5;
                south = 58;
                centre = 60;



                print_type = true;
            } 
            else if (type == false)
            {
                y_1 = 10;
                y_2 = 20;
                y_3 = 30;

                north = 40;
                south = 53;
                centre = 63; 
                west = 5;

                print_type = false;               
            }
        }
        
        void print(const char *key, int _vol, int _oct, int _wave ,bool _CurrentMode)
        {
            if(print_type)
            {
                //Get Centre
                u8g2.setCursor(north, y_1);
                u8g2.print("Receiver");

                //Octave
                u8g2.setCursor(west, y_2);
                u8g2.print("Oct: ");
                u8g2.print(_oct);

                // volume print
                u8g2.setCursor(southwest, y_3);
                u8g2.print("Vol: ");
                u8g2.print(_vol, DEC);
                
                //Key print
                u8g2.setCursor(east, y_2);
                u8g2.print("Key: ");
                u8g2.print(key);

                u8g2.setCursor(centre, y_2);
                u8g2.print(_west ? '-' : 'W');
                u8g2.print(_east ? '-' : 'E');

                //Print Position
                u8g2.setCursor(southeast, y_3);
                u8g2.print("Pos: ");
                u8g2.print(position);

                //Wave print
                u8g2.setCursor(south, y_3);
                if (_wave == 0)
                {
                    u8g2.print("Tri");
                }
                else if (_wave  == 1)
                {
                    u8g2.print("Squ");
                }
                else if (_wave == 2)
                {
                    u8g2.print("Saw");
                }
                else if (_wave > 2)
                {
                    u8g2.print("Sine");
                }

                u8g2.setCursor(west, y_1);
                if(_CurrentMode == true){
                    u8g2.print("Single");
                }
                else{
                    u8g2.print("Multi");
                }
                
            }
            else 
            {
                //Get Centre
                u8g2.setCursor(north, 10);
                u8g2.print("Transmitter");

                u8g2.setCursor(centre, y_2);
                u8g2.print(_west ? '-' : 'W');
                u8g2.print(_east ? '-' : 'E');

                //Print Position
                u8g2.setCursor(south, y_3);
                u8g2.print("Pos: ");
                u8g2.print(position);  

                u8g2.setCursor(west, y_1);
                if(_CurrentMode == true){
                    u8g2.print("Single");
                }
                else{
                    u8g2.print("Multi");
                }              
            }
        }
};