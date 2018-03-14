#ifndef BITS_UTIL__H
#define BITS_UTIL__H

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"{
#endif

// 把ch[0]中索引为index的比特位设置为value
void z_bits_set(uint8_t* ch,size_t index,int value);

// 返回ch中索引为index的比特位的值
uint8_t z_bits_get(uint8_t ch,size_t index);

// 返回ch中从start到end（不包括）之间的比特位组成的值
uint8_t z_bits_value(uint8_t,size_t start,size_t end);

// 把src[0]中从from 到 to（不包括to）之间的比特复制到dst[0]以index开始的地方
void z_bits_copy(uint8_t* dst,size_t index,uint8_t src,size_t from,size_t to);

#ifdef __cplusplus
}
#endif

#endif // BITS_H
