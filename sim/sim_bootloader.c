#include <stdio.h>
#include <stdint.h>

#include "../security/security.h"
#include "ota_metadata.h"
void sim_flash_read(uint32_t addr, uint8_t *out, size_t len);
void sim_watchdog_start(void);
void sim_watchdog_stop(void);
#define SLOT_A_ADDR   0x200
#define SLOT_B_ADDR   0x600
#define SLOT_SIZE     256
#define MAX_ATTEMPTS  3

#define OTA_COMMIT_PENDING 4
#define OTA_BOOT_TEST      5
#define OTA_FAILED         7

static uint32_t slot_addr(uint32_t slot)
{
    return slot ? SLOT_B_ADDR : SLOT_A_ADDR;
}

static int verify_firmware(
    uint32_t slot,
    uint32_t fw_version,
    ota_metadata_t *meta
)
{
    uint8_t image[SLOT_SIZE];
    uint8_t signature[SIGNATURE_MAX_SIZE];

    sim_flash_read(slot_addr(slot), image, SLOT_SIZE);
    sim_flash_read(slot_addr(slot) + SLOT_SIZE,
                   signature,
                   SIGNATURE_MAX_SIZE);

    return security_verify_signature(
        image,
        SLOT_SIZE,
        signature,
        SIGNATURE_MAX_SIZE,
        fw_version,
        meta->min_allowed_version
    ) == SEC_OK;
}

int sim_boot(void)
{
    ota_metadata_block_t meta_block;

    sim_watchdog_start();

    if (!sim_metadata_load(&meta_block)) {
        printf("⚠️ METADATA INVALID → BOOT FACTORY\n");
        sim_watchdog_stop();
        return 0;
    }

    ota_metadata_t meta = meta_block.data;

    if (meta.ota_state == OTA_COMMIT_PENDING ||
        meta.ota_state == OTA_BOOT_TEST) {

        printf("🔎 VERIFYING PENDING SLOT %u\n", meta.pending_slot);

        if (meta.boot_attempts >= MAX_ATTEMPTS ||
            !verify_firmware(meta.pending_slot,
                             meta.pending_fw_version,
                             &meta)) {

            printf("❌ PENDING SLOT INVALID → ROLLBACK\n");

            meta.ota_state = OTA_FAILED;
            meta.boot_attempts = 0;
            meta.pending_fw_version = 0;

            sim_metadata_update(&meta);
            sim_watchdog_stop();
            return 0;
        }

        printf("🚀 BOOTING PENDING SLOT %u (TEST MODE)\n",
               meta.pending_slot);

        meta.ota_state = OTA_BOOT_TEST;
        meta.boot_attempts++;

        sim_metadata_update(&meta);
        sim_watchdog_stop();
        return 1;
    }

    printf("🔎 VERIFYING ACTIVE SLOT %u\n", meta.active_slot);

    if (!verify_firmware(meta.active_slot,
                         meta.active_fw_version,
                         &meta)) {
        printf("❌ ACTIVE INVALID → FACTORY\n");
        sim_watchdog_stop();
        return 0;
    }

    printf("✅ BOOTING ACTIVE SLOT %u\n", meta.active_slot);
    sim_watchdog_stop();
    return 1;
}
