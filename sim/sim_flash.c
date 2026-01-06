#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SIM_FLASH_SIZE 4096

static uint8_t sim_flash[SIM_FLASH_SIZE];

/*
 * simulating the sudden power loss.
 */
static int32_t power_cut_at = -1;

/* Initialing flash to erased state (0xFF) */
void sim_flash_init(void)
{
    memset(sim_flash, 0xFF, sizeof(sim_flash));
    power_cut_at = -1;
}
void sim_flash_set_power_cut(int32_t addr)
{
    power_cut_at = addr;
}
void sim_flash_read(uint32_t addr, uint8_t *out, size_t len)
{
    if ((addr + len) > SIM_FLASH_SIZE) {
        printf("❌ FLASH READ OOB\n");
        exit(1);
    }

    memcpy(out, &sim_flash[addr], len);
}
void sim_flash_write(uint32_t addr, const uint8_t *data, size_t len)
{
    if ((addr + len) > SIM_FLASH_SIZE) {
        printf("❌ FLASH WRITE OOB\n");
        exit(1);
    }

    for (size_t i = 0; i < len; i++) {
        uint32_t current_addr = addr + i;
        if ((int32_t)current_addr == power_cut_at) {
            printf("💥 POWER LOSS at flash address 0x%04X\n", current_addr);
            exit(0); 
        }
        sim_flash[current_addr] &= data[i];
    }
}
void sim_flash_erase(uint32_t addr, size_t len)
{
    if ((addr + len) > SIM_FLASH_SIZE) {
        printf("❌ FLASH ERASE OOB\n");
        exit(1);
    }

    memset(&sim_flash[addr], 0xFF, len);
}

void sim_flash_dump(uint32_t addr, size_t len)
{
    if ((addr + len) > SIM_FLASH_SIZE) {
        printf("❌ FLASH DUMP OOB\n");
        exit(1);
    }

    for (size_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            printf("\n0x%04X: ", addr + (uint32_t)i);
        }
        printf("%02X ", sim_flash[addr + i]);
    }
    printf("\n");
}
