#ifndef TK_MBEDTLS_CLIENT_H
#define TK_MBEDTLS_CLIENT_H

#include "mbedtls_helper.h"

typedef struct _mbedtls_client_t {
  mbedtls_ctx_t ctx;
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_net_context server_fd;
  mbedtls_ssl_config conf;
  mbedtls_x509_crt cacert;
} mbedtls_client_t;

mbedtls_ctx_t* mbedtls_client_create(const char* host, const char* port, const uint8_t* cas_pem, uint32_t cas_len);

#endif /* TK_MBEDTLS_CLIENT_H */
