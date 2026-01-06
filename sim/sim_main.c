#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "../security/security.h"
#include "ota_metadata.h"
void sim_flash_init(void);
void sim_flash_erase(uint32_t addr, size_t len);
void sim_flash_write(uint32_t, const uint8_t*, size_t);
void sim_watchdog_tick(void);
void sim_watchdog_kick(void);
int  sim_boot(void);

#define SLOT_A_ADDR  0x200
#define SLOT_B_ADDR  0x600
#define FACTORY_ADDR 0xA00
#define SLOT_SIZE    256

#define OTA_IDLE           0
#define OTA_COMMIT_PENDING 4

#define FW_GOOD 0
#define FW_BAD  1
#define FW_HANG 2

static const uint8_t KEY[16] = {
    0xA1,0xB2,0xC3,0xD4,
    0x55,0x66,0x77,0x88,
    0x10,0x20,0x30,0x40,
    0x99,0xAA,0xBB,0xCC
};

static void sign_fw(const uint8_t *img, uint32_t ver, uint8_t *sig)
{
    uint8_t buf[SLOT_SIZE + 4], h[SHA256_DIGEST_SIZE];
    memcpy(buf, img, SLOT_SIZE);
    memcpy(buf + SLOT_SIZE, &ver, 4);
    security_sha256(buf, sizeof(buf), h);

    for (size_t i = 0; i < SIGNATURE_MAX_SIZE; i++)
        sig[i] = h[i % SHA256_DIGEST_SIZE] ^ KEY[i % 16];
}

static void program(uint32_t addr, int mode, uint32_t ver)
{
    uint8_t img[SLOT_SIZE], sig[SIGNATURE_MAX_SIZE];

    for (int i = 0; i < SLOT_SIZE; i++)
        img[i] = 0x10 + i;

    if (mode == FW_BAD)
        memset(sig, 0, sizeof(sig));
    else
        sign_fw(img, ver, sig);

    sim_flash_erase(addr, SLOT_SIZE + SIGNATURE_MAX_SIZE);
    sim_flash_write(addr, img, SLOT_SIZE);
    sim_flash_write(addr + SLOT_SIZE, sig, SIGNATURE_MAX_SIZE);
}

static void run_fw(void)
{
    printf("🧠 FIRMWARE RUNNING\n");
    printf("💀 FIRMWARE HUNG\n");
    while (1) sim_watchdog_tick();
}

int main(void)
{
    printf("\n=== OTA FAILURE SIMULATION START ===\n");

    sim_flash_init();

    program(FACTORY_ADDR, FW_GOOD, 1);
    program(SLOT_A_ADDR, FW_GOOD, 5);

    ota_metadata_t meta = {
        .active_slot = 0,
        .pending_slot = 1,
        .active_fw_version = 5,
        .pending_fw_version = 0,
        .min_allowed_version = 3,
        .ota_state = OTA_IDLE,
        .boot_attempts = 0
    };

    sim_metadata_update(&meta);

    printf("\n--- TEST: BAD OTA IMAGE ---\n");
    meta.pending_fw_version = 6;
    program(SLOT_B_ADDR, FW_BAD, 6);
    meta.ota_state = OTA_COMMIT_PENDING;
    sim_metadata_update(&meta);
    sim_boot();

    printf("\n--- TEST: HANGING OTA IMAGE ---\n");
    meta.pending_fw_version = 6;
    program(SLOT_B_ADDR, FW_GOOD, 6);
    meta.ota_state = OTA_COMMIT_PENDING;
    sim_metadata_update(&meta);

    if (sim_boot())
        run_fw();

    return 0;
}
