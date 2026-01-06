#include <stdio.h>
#include <stdint.h>
#include "security/security.h"

static const uint8_t TEST_PUBLIC_KEY_SIM[16] = {
    0xA1, 0xB2, 0xC3, 0xD4,
    0x55, 0x66, 0x77, 0x88,
    0x10, 0x20, 0x30, 0x40,
    0x99, 0xAA, 0xBB, 0xCC
};

int main(void)
{
    uint8_t firmware[] = {
        0x10, 0x20, 0x30, 0x40,
        0x50, 0x60, 0x70, 0x80
    };

    uint32_t image_version = 5;
    uint32_t min_allowed_version = 3;

    uint8_t hash[SHA256_DIGEST_SIZE];
    uint8_t signature[SIGNATURE_MAX_SIZE];
    if (security_sha256(firmware, sizeof(firmware), hash) != SEC_OK) {
        printf("Hash failed\n");
        return -1;
    }
    for (size_t i = 0; i < SIGNATURE_MAX_SIZE; i++) {
        signature[i] =
            hash[i % SHA256_DIGEST_SIZE] ^
            TEST_PUBLIC_KEY_SIM[i % 16];
    }
    sec_status_t result = security_verify_signature(
        firmware,
        sizeof(firmware),
        signature,
        SIGNATURE_MAX_SIZE,
        image_version,
        min_allowed_version
    );
    if (result == SEC_OK) {
        printf("SECURITY CHECK PASSED\n");
    } else {
        printf("SECURITY CHECK FAILED: %d\n", result);
    }

    return 0;
}
