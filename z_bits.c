#include "z_bits.h"

void z_bits_set(uint8_t* ch,size_t index,int value){
    assert(index >= 0 && index < 8);
    assert(value == 0 || value == 1);

    uint8_t mask = 1 << index;
    ch[0] |= mask;
}

uint8_t z_bits_get(uint8_t ch,size_t index){
    assert(index >= 0 && index < 8);

    uint8_t mask = 1 << index;
    ch = (ch & mask) >> index;
    return ch;
}

uint8_t z_bits_value(uint8_t ch,size_t start,size_t end){
    int len = 8 - (end - start);
    uint8_t mask = (0xFF) >> len;
    ch = ch >> start;
    ch &= mask;
    return ch;
}

void z_bits_copy(uint8_t* dst,size_t index,uint8_t src,size_t from,size_t to){
    assert(dst != NULL);
    assert(index >= 0 && index < 8);
    assert(to > 0 && to <= 8);
    assert(from <= to);

    while (from < to) {
        z_bits_set(dst,index,z_bits_get(src,from));
        ++from;
        ++index;
    }
}
