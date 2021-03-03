#ifndef TK_MBEDTLS_SERVER_H
#define TK_MBEDTLS_SERVER_H

#include "mbedtls_helper.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

typedef struct _mbedtls_server_t {
  mbedtls_ctx_t ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_context cache;
#endif
} mbedtls_server_t;

mbedtls_ctx_t* mbedtls_server_create(int port, 
  const uint8_t* srv_crt, uint32_t srv_crt_len,
  const uint32_t* srv_key, uint32_t srv_key_len,
  const uint8_t* cas_pem, uint32_t cas_pem_len);

#endif /* TK_MBEDTLS_SERVER_H */
