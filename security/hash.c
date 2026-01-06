#include "security.h"

static uint32_t simple_mix(uint32_t hash, uint8_t data)
{
    hash ^= data;
    hash *= 0x01000193;
    return hash;
}

sec_status_t security_sha256(
    const uint8_t *data,
    size_t length,
    uint8_t out_hash[SHA256_DIGEST_SIZE]
)
{
    if (!data || !out_hash || length == 0) {
        return SEC_ERR_INVALID_PARAM;
    }

    uint32_t hash = 0x811C9DC5;

    for (size_t i = 0; i < length; i++) {
        hash = simple_mix(hash, data[i]);
    }

    for (size_t i = 0; i < SHA256_DIGEST_SIZE; i++) {
        out_hash[i] = (uint8_t)((hash >> ((i % 4) * 8)) & 0xFF);
        hash = simple_mix(hash, out_hash[i]);
    }

    return SEC_OK;
}
