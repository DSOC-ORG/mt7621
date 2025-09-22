#ifndef winbond_25q256jv
#define windbond_25q256jv

#include <FS.h>
#include "Arduino.h"
#include "SPI.h"
#include <functional>

class Winbond25Q256JV;

class FlashStream: public Stream {
    protected: 
        uint32_t currAddr = 0;
        uint32_t currWriteAddr = 0;
        uint32_t load_addr = 0;
        uint32_t load_write_addr = 0;
        uint8_t* read_data_buf;
        uint8_t* write_data_buf;
        Winbond25Q256JV* flash;
    public:
        FlashStream(Winbond25Q256JV* flash);
        ~FlashStream();
        int available() override;
        int read() override;
        int peek() override;
        void flush() override;

        size_t write(const uint8_t *buffer, size_t size) override;
        size_t write(uint8_t data) override;

        String name();
        uint32_t size();
       
};


class Winbond25Q256JV {
    protected:
        uint8_t cs_pin;
        uint8_t hold_pin;
        bool is_ok = false;
        SPISettings spi_settings = SPISettings(20000000, MSBFIRST, SPI_MODE0);
        template <typename T>
        T transaction(std::function<T(uint8_t* err)> func);
        uint8_t readRegister(uint8_t reg_nr);
        void writeEnable();
        void writeDisable();
    public:
        Winbond25Q256JV(uint8_t flash_cs_pin, uint8_t flash_hold_pin);
        void doResetAndSetup(); // reset the device, put 4 byte addressing mode
        uint32_t getVersion();
        bool isBusy();
        bool isWLE();
        bool addrMode();
        bool isOK();
        uint8_t* read(uint32_t addr, uint32_t length);
        void write(uint32_t addr, const uint8_t* data, uint32_t length);
        void chipErase();
        uint32_t totalMemory();
        FlashStream* getFlashStream();
};



#endif

