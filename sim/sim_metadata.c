#include <stdint.h>
#include <string.h>
#include "ota_metadata.h"

/* Flash APIs */
void sim_flash_read(uint32_t addr, uint8_t *out, size_t len);
void sim_flash_write(uint32_t addr, const uint8_t *data, size_t len);
void sim_flash_erase(uint32_t addr, size_t len);

/* Metadata flash layout */
#define META_A_ADDR  0x000
#define META_B_ADDR  0x080
#define META_SIZE    128
#define META_MAGIC   0x4F544131  /* "OTA1" */

static uint32_t crc32(const uint8_t *d, size_t l)
{
    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < l; i++) {
        c ^= d[i];
        c *= 0x01000193;
    }
    return c;
}

static int read_block(uint32_t addr, ota_metadata_block_t *b)
{
    sim_flash_read(addr, (uint8_t *)b, sizeof(*b));
    if (b->data.magic != META_MAGIC) return 0;
    return crc32((uint8_t *)&b->data, sizeof(b->data)) == b->crc;
}

int sim_metadata_load(ota_metadata_block_t *out)
{
    ota_metadata_block_t a, b;
    int va = read_block(META_A_ADDR, &a);
    int vb = read_block(META_B_ADDR, &b);

    if (!va && !vb) return 0;
    *out = (va && (!vb || a.data.seq >= b.data.seq)) ? a : b;
    return 1;
}

void sim_metadata_update(const ota_metadata_t *data)
{
    ota_metadata_block_t cur, next;
    uint32_t addr;

    if (sim_metadata_load(&cur)) {
        next.data = *data;
        next.data.seq = cur.data.seq + 1;
        addr = (cur.data.seq & 1) ? META_A_ADDR : META_B_ADDR;
    } else {
        next.data = *data;
        next.data.seq = 1;
        addr = META_A_ADDR;
    }

    next.data.magic = META_MAGIC;
    next.crc = crc32((uint8_t *)&next.data, sizeof(next.data));

    sim_flash_erase(addr, META_SIZE);
    sim_flash_write(addr, (uint8_t *)&next, sizeof(next));
}
