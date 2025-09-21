#include "spi_flash/winbond_25q256jv.h"

#define CMD_ENABLE_RESET 0x66
#define CMD_RESET 0x99
#define CMD_ENTER_4BYTE_MODE 0xB7
#define CMD_JDEC_ID 0x9F
#define CMD_STATUS_REGISTER_1 0x05
#define CMD_STATUS_REGISTER_2 0x35
#define CMD_STATUS_REGISTER_3 0x15
#define CMD_WRITE_ENABLE 0x06
#define CMD_WRITE_DISABLE 0x04
#define CMD_READ_DATA_4BYTE_ADDR 0x13
#define CMD_WRITE_PAGE_4BYTE_ADDR 0x12
#define CMD_CHIP_ERASE 0x60

void spiCommand(uint8_t* err, uint8_t c) { 
    uint8_t res = SPI.transfer(c);
    if(res == 0) {
        return;
    }
    *err = 1;
    if(Serial) {
        Serial.printf("Not good, byte command (%x) yielded results (%x)\r\n", c, res);
    }
}



void spiCommand32(uint8_t* err, uint32_t c) { 
    uint32_t res = SPI.transfer32(c);
    if(res == 0) {
        return;
    }
    *err = 1;
    if(Serial) {
        Serial.printf("Not good, int command (%x) yielded results (%x)\r\n", c, res);
    }

}

uint8_t spiGetByte() { 
    return (uint8_t)SPI.transfer(0x00);
}


uint8_t getManufacturerId(uint32_t version) {
    return version >> 16 & 0xFF;
}

uint8_t getDevIdSubtype(uint32_t version) {
    return version >> 8 & 0xFF;
}

uint8_t getDevIdMemory(uint32_t version) {
    return version & 0xFF;
}

Winbond25Q256JV::Winbond25Q256JV(uint8_t flash_cs_pin, uint8_t flash_hold_pin) {
    cs_pin = flash_cs_pin;
    hold_pin = flash_hold_pin;
    pinMode(cs_pin, OUTPUT);
    pinMode(hold_pin, OUTPUT);
    digitalWrite(cs_pin, 1);
    digitalWrite(hold_pin, 1);
    SPI.begin();
    delay(50);

    uint32_t version = getVersion();

    Serial.printf("Version: %x\r\n", version);

    if(getManufacturerId(version) != 0xef) {
        return;
    }

    if(getDevIdSubtype(version) != 0x40) {
        return;
    }
    
    if(Serial) { 
        Serial.println("The device matches the specs.");
    }
    
    doResetAndSetup();

        
    if(Serial) { 
        Serial.println("The addr mode is");
        Serial.println(addrMode());
    }

    
    
    // we want to be sure write is disabled for start
    if(isWLE()) {
        writeDisable();
    }
    
    is_ok = true;
}

// protected API
template <typename T>
T Winbond25Q256JV::transaction(std::function<T(uint8_t* err)> func) {
    T res = 0;
    uint8_t err;
    digitalWrite(cs_pin, 1);
    do { 
        err = 0;
        SPI.beginTransaction(spi_settings);
        digitalWrite(cs_pin, 0);
        res = func(&err);
        digitalWrite(cs_pin, 1);
        SPI.endTransaction();
    } while(err);

    return res;
}
uint8_t Winbond25Q256JV::readRegister(uint8_t reg_nr)
{

    uint8_t res = transaction<uint8_t>([reg_nr](uint8_t* err) {

        uint8_t res = 0;

        switch (reg_nr)
        {
            case 1:
                spiCommand(err, CMD_STATUS_REGISTER_1);
                break;
            case 2:
                spiCommand(err, CMD_STATUS_REGISTER_2);
                break;
            case 3:
                spiCommand(err, CMD_STATUS_REGISTER_3);
                break;
            default:
                return res;
        }

        res = spiGetByte();

        return res;
    });


    return res;
}

void Winbond25Q256JV::writeEnable()
{
    transaction<uint8_t>([&](uint8_t* err) {
        spiCommand(err, CMD_WRITE_ENABLE);
        return 0;
    });
    
    while(!isWLE()) {
        Serial.print(".");
    }
}

void Winbond25Q256JV::writeDisable()
{
    transaction<uint8_t>([&](uint8_t* err) {
        spiCommand(err, CMD_WRITE_DISABLE);
        return 0;
    });
}

// public API


void Winbond25Q256JV::doResetAndSetup() {
    transaction<uint8_t>([&](uint8_t* err) { 
        spiCommand(err, CMD_ENABLE_RESET);

        return 0;
    }); 
    transaction<uint8_t>([&](uint8_t* err) { 
        spiCommand(err, CMD_RESET);

        return 0;
    }); 
    delayMicroseconds(50);

    transaction<uint8_t>([&](uint8_t* err) { 
        // holdUpDown(this->hold_pin);
        spiCommand(err, CMD_ENTER_4BYTE_MODE);

        return 0;
    }); 
      
}


uint32_t Winbond25Q256JV::getVersion() { 
    return transaction<uint32_t>([&](uint8_t* err) {
        spiCommand(err, CMD_JDEC_ID);
        uint32_t res_internal = 0;
        res_internal |= spiGetByte() << 16;
        res_internal |= spiGetByte() << 8;
        res_internal |= spiGetByte();

        return res_internal;
    });
}

bool Winbond25Q256JV::isBusy()
{
    return readRegister(1) & 0x01;
}

bool Winbond25Q256JV::isWLE()
{
    return (readRegister(1) >> 1) & 0x01;
}

bool Winbond25Q256JV::addrMode()
{
    return readRegister(3) & 0x01;
}

bool Winbond25Q256JV::isOK()
{
    return is_ok;
}

uint8_t *Winbond25Q256JV::read(uint32_t addr, uint32_t length)
{
    while(isBusy()) {
        Serial.print(".");
    }
    return transaction<uint8_t*>([&](uint8_t* err) { 
        uint8_t* bytes = (uint8_t*) calloc(length, sizeof(uint8_t));
        
        if(bytes == NULL) {
            return (uint8_t*)NULL;
        }

        spiCommand(err, CMD_READ_DATA_4BYTE_ADDR);
        spiCommand32(err, addr);

        for(uint32_t i = 0; i < length; i++) {
            uint8_t byte = spiGetByte();
            *(bytes+i) = byte;
        }

        return bytes;
    });
}

void Winbond25Q256JV::write(uint32_t addr, const uint8_t *data, uint32_t length)
{
    Serial.printf("Writing to %x, len %d\r\n", addr, length);

    while(isBusy()) {
        Serial.print(".");
    }
    uint32_t page_addr = addr & 0xFFFFFF00; // just trim the last byte
    // find offset from start
    uint8_t offset = addr - page_addr;
    Serial.print("Offset");
    Serial.print(offset);
    Serial.println();
    // to not overwrite ( we have to read out )
    uint8_t* data_in_offset = read(page_addr, offset);
    // we want to keep track of written bytes
    uint32_t bytes_written = 0;
    // enable writing

    while(bytes_written != length) {   
        writeEnable();

        // page write
        uint8_t bw = transaction<uint8_t>([&](uint8_t* err){
            int bytesw = 0;

            spiCommand(err, CMD_WRITE_PAGE_4BYTE_ADDR);
            spiCommand32(err, page_addr);
            for(uint32_t i = 0; i < offset; i++) {
                spiCommand(err, *(data_in_offset + i));
            }
            // we need to write the page
            for(uint32_t i = 0; i < std::min((uint32_t)(256 - offset), length - bytes_written); i++ ){   
                //Serial.print("Writing byte:");
                //Serial.println(*(data+i));
                spiCommand(err, *(data+i));
                bytesw++;
            }
            return bytesw;
        });
        
        while(isBusy()) { 
            Serial.print(".");
        }
        // move to other page
        bytes_written += bw;
        offset = 0;
        page_addr += 256;
    }
  
    writeDisable();
}

void Winbond25Q256JV::chipErase()
{
    writeEnable();
    transaction<uint8_t>([&](uint8_t* err){
        spiCommand(err, CMD_CHIP_ERASE);
        return 0;
    });

    while(isBusy()) { 
        Serial.print(".");
    }
}
uint32_t Winbond25Q256JV::totalMemory()
{
    return 33554432;
}

FlashStream* Winbond25Q256JV::getFlashStream() {
    return new FlashStream(this);
}