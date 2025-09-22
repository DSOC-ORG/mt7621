#include "spi_flash/winbond_25q256jv.h"

FlashStream::FlashStream(Winbond25Q256JV* memflash) {
    flash = memflash;
    read_data_buf = NULL;
    write_data_buf = NULL;
}

FlashStream::~FlashStream(){
    if(read_data_buf != NULL) {
        free(read_data_buf);
    }
    if(write_data_buf != NULL) {
        free(write_data_buf);
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



// write methods
void FlashStream::flush() {
    if(currWriteAddr != load_write_addr) {
         Serial.println("Flushing");

         flash->write(currWriteAddr, write_data_buf, load_write_addr - currWriteAddr);
    }
}

size_t FlashStream::write(const uint8_t *data, size_t size) {
    if(size && data) {
        if(write_data_buf == NULL) {
            write_data_buf = (uint8_t*) calloc(32001, sizeof(uint8_t));
        }
        uint32_t mod_load_w = load_write_addr % 32000;
        uint8_t has_extra = 0;
        uint32_t offset = 0;

        if(mod_load_w + size <= 32000) {
            memcpy(write_data_buf + mod_load_w, data, size);
            load_write_addr += size;
        }else if(mod_load_w + size > 32000){
            // copy to fit
            offset = size - (mod_load_w + size) % 32000;
            memcpy(write_data_buf + mod_load_w, data, offset);
            has_extra = 1;
            load_write_addr += offset;
        }

        if(load_write_addr % 32000 == 0 && load_write_addr != 0) {
            Serial.printf("\r\n\r\nDispatching write %x \r\n\r\n", currWriteAddr);
            flash->write(currWriteAddr, write_data_buf, 32000);
            currWriteAddr+=32000;
        }

        if(has_extra) {
            Serial.printf("has_extra: %p %p %d %d %d\r\n", write_data_buf, data, offset, size, size-offset);
            memcpy(write_data_buf, data + offset, size - offset);
            load_write_addr+=size-offset;
        }
        Serial.printf("YOOOO");

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