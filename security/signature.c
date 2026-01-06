#include "security.h"
#include <string.h>

static const uint8_t PUBLIC_KEY_SIM[16] = {
    0xA1, 0xB2, 0xC3, 0xD4,
    0x55, 0x66, 0x77, 0x88,
    0x10, 0x20, 0x30, 0x40,
    0x99, 0xAA, 0xBB, 0xCC
};

sec_status_t security_verify_signature(
    const uint8_t *image,
    size_t image_len,
    const uint8_t *signature,
    size_t sig_len,
    uint32_t image_version,
    uint32_t min_allowed_version
)
{
    if (!image || !signature || image_len == 0 || sig_len == 0)
        return SEC_ERR_INVALID_PARAM;

    if (image_version < min_allowed_version)
        return SEC_ERR_VERSION_ROLLBACK;

    uint8_t buffer[image_len + sizeof(uint32_t)];
    uint8_t hash[SHA256_DIGEST_SIZE];

    memcpy(buffer, image, image_len);
    memcpy(buffer + image_len, &image_version, sizeof(uint32_t));

    if (security_sha256(buffer, sizeof(buffer), hash) != SEC_OK)
        return SEC_ERR_INTERNAL;

    for (size_t i = 0; i < sig_len; i++) {
        uint8_t expected =
            hash[i % SHA256_DIGEST_SIZE] ^
            PUBLIC_KEY_SIM[i % sizeof(PUBLIC_KEY_SIM)];

        if (signature[i] != expected)
            return SEC_ERR_SIGNATURE_INVALID;
    }

    return SEC_OK;
}
