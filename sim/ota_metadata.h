#ifndef OTA_METADATA_H
#define OTA_METADATA_H

#include <stdint.h>

typedef struct {
    uint32_t magic;
    uint32_t seq;
    uint32_t active_slot;
    uint32_t pending_slot;
    uint32_t active_fw_version;
    uint32_t pending_fw_version;
    uint32_t min_allowed_version;
    uint32_t ota_state;
    uint32_t boot_attempts;
} ota_metadata_t;

typedef struct {
    ota_metadata_t data;
    uint32_t crc;
} ota_metadata_block_t;

int  sim_metadata_load(ota_metadata_block_t *out);
void sim_metadata_update(const ota_metadata_t *data);

#endif
