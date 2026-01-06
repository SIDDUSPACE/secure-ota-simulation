#ifndef SECURITY_H
#define SECURITY_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    SEC_OK = 0,
    SEC_ERR_INVALID_PARAM,
    SEC_ERR_HASH_MISMATCH,
    SEC_ERR_SIGNATURE_INVALID,
    SEC_ERR_VERSION_ROLLBACK,
    SEC_ERR_INTERNAL
} sec_status_t;

#define SHA256_DIGEST_SIZE 32
#define SIGNATURE_MAX_SIZE 64
sec_status_t security_sha256(
    const uint8_t *data,
    size_t length,
    uint8_t out_hash[SHA256_DIGEST_SIZE]
);

sec_status_t security_verify_signature(
    const uint8_t *image,
    size_t image_len,
    const uint8_t *signature,
    size_t sig_len,
    uint32_t image_version,
    uint32_t min_allowed_version
);

#endif /* SECURITY_H */
