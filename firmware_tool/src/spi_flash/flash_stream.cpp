#include "spi_flash/winbond_25q256jv.h"

FlashStream::FlashStream(Winbond25Q256JV* memflash) {
    flash = memflash;
    read_data_buf = NULL;
}

FlashStream::~FlashStream(){
    if(read_data_buf != NULL) {
        free(read_data_buf);
    }
}

int FlashStream::read() {
    if(!available()) {
        return -1;
    }
    if(currAddr == load_addr) {
        if(read_data_buf != NULL) {
            free(read_data_buf);
        }
        read_data_buf = flash->read(currAddr, 32000);
        load_addr += 32000;
    }
    uint8_t val = *(read_data_buf + currAddr % 32000);
    currAddr++;
    return val;
}
int FlashStream::peek() {
    Serial.println("using ?");
    if(!available()) {
        return -1;
    }
    uint8_t* read_data = flash->read(currAddr, 1);
    uint8_t val = *read_data;
    free(read_data);
    return val;
}

int FlashStream::available() {
    return flash->totalMemory() - currAddr;
}

void FlashStream::flush() {
}

// write methods


size_t FlashStream::write(const uint8_t *data, size_t size) {
    if(size && data) {
        flash->write(currAddr, data, (uint32_t)size);
        currAddr+=size;
        return size;
    }
    return 0;
}

size_t FlashStream::write(uint8_t data) {
    return 0;
}

String FlashStream::name() {
    return String("flash.bin");
}

uint32_t FlashStream::size() { 
    return flash->totalMemory();
}