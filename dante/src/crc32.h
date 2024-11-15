/*
 * File     : $RCSfile$
 * Created  : 8 Apr 2011 11:20:11
 * Updated  : $Date$
 * Author   : Jerry Kim
 * Synopsis : crc32 header
 *
 * This software is copyright (c) 2004-2011 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>


int foo(int n);

unsigned long int crc32(unsigned long int crc, const uint8_t *buf, int32_t len);

#define ENV_SIZE (1024*64-4)

// 65535
typedef struct env_s
{
    uint32_t crc;
    char data[ENV_SIZE+4];
}env_t;

#endif /* CONFIGD_H */
