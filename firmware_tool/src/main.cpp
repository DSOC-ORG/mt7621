/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#include "Arduino.h"
#include "spi_flash/winbond_25q256jv.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

Winbond25Q256JV* flash;

void setup()
{
    Serial.begin(115200);
    // initialize LED digital pin as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    flash = new Winbond25Q256JV(5, 17);


    uint8_t* data;
    int read_size = 8;

    data = flash->read(0x1FFFF00, read_size);
    for(int i = 0; i < read_size; i++) {
        Serial.printf("%x\r\n", *(data+i));
    }
    free(data);

    Serial.println("Now writing");
    uint8_t arr[] = {0xFF, 0X10, 0xFE, 7, 8, 9, 10, 11, 12}; 
    uint8_t* val = arr;
    flash->write(0x1ffff00, arr, 9);    

    data = flash->read(0x1FFFF00, read_size);

    for(int i = 0; i < read_size; i++) {
        Serial.printf("%x\r\n", *(data+i));
    }
    
    free(data);

}

void loop()
{
    // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED_BUILTIN, HIGH);

    // wait for a second
    delay(1000);

    // turn the LED off by making the voltage LOW
    digitalWrite(LED_BUILTIN, LOW);

    // wait for a second
    delay(1000);
}
