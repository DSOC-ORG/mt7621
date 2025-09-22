/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */
#include "Arduino.h"
#include "WiFi.h"
#include "WebServer.h"
#include "spi_flash/winbond_25q256jv.h"
#include "wifi_creds.h"

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const char *networkName = WIFI_AP_NAME;
const char *networkPswd = WIFI_AP_PASSWORD;

Winbond25Q256JV *flash;

WebServer server(80);

void printAsMatrix(uint8_t *bytes, uint32_t len, uint32_t width);


FlashStream* writeStream = NULL;

void setup()
{
    Serial.setRxBufferSize(2000);
    Serial.begin(115200);
    Serial.setTimeout(5000);

    pinMode(LED_BUILTIN, OUTPUT);

    flash = new Winbond25Q256JV(5, 17);

    if (!flash->isOK())
    {
        Serial.println("Nothing good here");
        return;
    }

    // initialize LED digital pin as an output.

    WiFi.begin(networkName, networkPswd);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.printf("Local IP: %s\r\n", WiFi.localIP().toString());

    server.on("/", []()
              { server.send(200, "text/html", "Hello world <br/><a href=\"/dump\">Dump firmware from flash</a>"); });
    server.on("/dump", [&]()
              {
        FlashStream* fs = flash->getFlashStream();
        server.streamFile(*fs, String("application/octet-stream"));
        delete fs; });
    server.on("/upload", HTTP_POST, [&]()
              { server.send(200, "application/html", "Upload complete"); }, [&]()
              {
                  HTTPUpload &upload = server.upload();
                  if (upload.status == UPLOAD_FILE_START)
                  {
                      writeStream = flash->getFlashStream();
                      if(server.hasArg("chipErase")){
                        flash->chipErase();
                      }
                      Serial.println("Upload file start");
                  }
                  if (upload.status == UPLOAD_FILE_WRITE)
                  {
                      writeStream->write(upload.buf, upload.currentSize);
                      Serial.printf("%d %d\r\n",upload.totalSize, upload.currentSize);
                  }
                  if (upload.status == UPLOAD_FILE_END)
                  {
                      writeStream->flush();
                      delete writeStream;
                      writeStream = NULL;
                      Serial.println("Upload file end");
                  } });
    server.onNotFound([]()
                      { server.send(404, "text/plain", "not found"); });
    server.begin();

    digitalWrite(LED_BUILTIN, 1);
}

void emptyReadBuf()
{
    // empty read buff
    while (Serial.available())
    {
        Serial.read();
    }
}

uint32_t getUIntFromSerial(uint8_t base = 10)
{

    char serial_input[10] = {0};
    // wait for new line
    uint8_t curr_c_idx = 0;
    char curr_c;
    while (true)
    {
        if (Serial.available())
        {
            curr_c = Serial.read();
            serial_input[curr_c_idx] = curr_c;
            curr_c_idx++;
            if (curr_c == '\r' || curr_c == '\n' || curr_c_idx == 9)
            {
                serial_input[curr_c_idx] = '\0';
                break;
            }
        }
        else
        {
            delay(2);
        }
    }
    emptyReadBuf();

    return (uint32_t)strtol(serial_input, 0, base);
}

void printAsMatrix(uint8_t *bytes, uint32_t len, uint32_t width)
{
    for (uint32_t i = 0; i < len; i++)
    {
        Serial.printf("%x ", *(bytes + i));
        if ((i + 1) % width == 0)
        {
            Serial.println();
        }
    }
    Serial.println();
}

void readDataFromSerialHex(uint8_t *data, uint32_t len)
{
    char x[len * 2];
    char *pos = x;
    while (Serial.available() < len * 2 + 1)
    {
        delay(2);
    }
    // we probably have all chars with terminator
    Serial.readBytesUntil('\n', x, len * 2);

    for (size_t count = 0; count < len; count++)
    {
        sscanf(pos, "%2hhx", data + count);
        pos += 2;
    }

    emptyReadBuf();
}

void loop()
{
    server.handleClient();

    delay(2); // allow cpu to handle other tasks.

    // uint32_t addr, len, width;
    // uint8_t* data;
    // Serial.print("Enter cmd: ");
    // switch(getUIntFromSerial()) {
    //     case 1:
    //         Serial.print("Enter adress: ");
    //         addr = getUIntFromSerial(16);
    //         Serial.print("Enter length to write: ");
    //         len = getUIntFromSerial();
    //         data = (uint8_t*)calloc(len, sizeof(uint8_t));
    //         Serial.print("Enter data in hex: ");
    //         readDataFromSerialHex(data, len);
    //         printAsMatrix(data, len, 8);
    //         flash->write(addr, data, len);
    //         break;
    //     case 2:
    //         Serial.print("Enter adress (read ): ");
    //         addr = getUIntFromSerial(16);
    //         Serial.print("Enter length to read:");
    //         len = getUIntFromSerial();
    //         Serial.print("Enter num cols: ");
    //         width = getUIntFromSerial();
    //         data = flash->read(addr, len);
    //         printAsMatrix(data, len, width);
    //         free(data);
    //         break;
    //     case 3:
    //         break;
    //     default:
    //         Serial.println("Command unknown");
    //         Serial.println("1 - write");
    //         Serial.println("2 - read");
    //         Serial.println("3 - erase");
    // }
}
