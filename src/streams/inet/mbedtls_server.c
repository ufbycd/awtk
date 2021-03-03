#include "mbedtls_server.h"

static ret_t mbedtls_server_destroy(mbedtls_ctx_t* ctx) {
  mbedtls_server_t* c = (mbedtls_server_t*)ctx;

  return RET_OK;
}

mbedtls_ctx_t* mbedtls_server_create(int port, 
  const uint8_t* srv_crt, uint32_t srv_crt_len,
  const uint32_t* srv_key, uint32_t srv_key_len,
  const uint8_t* cas_pem, uint32_t cas_pem_len) {
  return NULL;
}
