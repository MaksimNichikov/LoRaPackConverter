#ifndef CRC32_H
#define CRC32_H

#include <inttypes.h>

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#endif

uint32_t crc32_fsl(uint32_t crc, const uint8_t *buf, uint32_t len);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif // CRC32_H
