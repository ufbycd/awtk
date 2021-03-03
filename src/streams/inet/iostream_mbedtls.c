/**
 * File:   iostream_mbedtls.c
 * Author: AWTK Develop Team
 * Brief:  input stream base on socket
 *
 * Copyright (c) 2019 - 2021  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2019-09-05 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /*WIN32_LEAN_AND_MEAN*/

#include "tkc/mem.h"
#include "tkc/utils.h"
#include "tkc/socket_helper.h"
#include "streams/inet/istream_mbedtls.h"
#include "streams/inet/ostream_mbedtls.h"
#include "streams/inet/iostream_mbedtls.h"

static ret_t tk_iostream_mbedtls_get_prop(object_t* obj, const char* name, value_t* v) {
  tk_iostream_mbedtls_t* iostream_mbedtls = TK_IOSTREAM_MBEDTLS(obj);

  if (tk_str_eq(name, TK_STREAM_PROP_FD)) {
    value_set_int(v, iostream_mbedtls->sock);
    return RET_OK;
  } else if (tk_str_eq(name, TK_STREAM_PROP_IS_OK)) {
    bool_t is_ok1 =
        object_get_prop_bool(OBJECT(iostream_mbedtls->istream), TK_STREAM_PROP_IS_OK, TRUE);
    bool_t is_ok2 =
        object_get_prop_bool(OBJECT(iostream_mbedtls->ostream), TK_STREAM_PROP_IS_OK, TRUE);

    value_set_bool(v, is_ok1 && is_ok2);

    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static ret_t tk_iostream_mbedtls_on_destroy(object_t* obj) {
  tk_iostream_mbedtls_t* iostream_mbedtls = TK_IOSTREAM_MBEDTLS(obj);

  mbedtls_net_free(&(iostream_mbedtls->server_fd));
  mbedtls_x509_crt_free(&(iostream_mbedtls->cacert));
  mbedtls_ssl_free(&(iostream_mbedtls->ssl));
  mbedtls_ssl_config_free(&(iostream_mbedtls->conf));
  mbedtls_ctr_drbg_free(&(iostream_mbedtls->ctr_drbg));
  mbedtls_entropy_free(&(iostream_mbedtls->entropy));
  OBJECT_UNREF(iostream_mbedtls->istream);
  OBJECT_UNREF(iostream_mbedtls->ostream);

  return RET_OK;
}

static const object_vtable_t s_tk_iostream_mbedtls_vtable = {
    .type = "tk_iostream_mbedtls",
    .desc = "tk_iostream_mbedtls",
    .size = sizeof(tk_iostream_mbedtls_t),
    .get_prop = tk_iostream_mbedtls_get_prop,
    .on_destroy = tk_iostream_mbedtls_on_destroy};

static tk_istream_t* tk_iostream_mbedtls_get_istream(tk_iostream_t* stream) {
  tk_iostream_mbedtls_t* iostream_mbedtls = TK_IOSTREAM_MBEDTLS(stream);

  return iostream_mbedtls->istream;
}

static tk_ostream_t* tk_iostream_mbedtls_get_ostream(tk_iostream_t* stream) {
  tk_iostream_mbedtls_t* iostream_mbedtls = TK_IOSTREAM_MBEDTLS(stream);

  return iostream_mbedtls->ostream;
}

static void my_debug(void* ctx, int level, const char* file, int line, const char* str) {
  const char *p, *basename;

  /* Extract basename from file */
  for (p = basename = file; *p != '\0'; p++)
    if (*p == '/' || *p == '\\') basename = p + 1;

  log_debug("%s:%04d: |%d| %s", basename, line, level, str);
}

tk_iostream_t* tk_iostream_mbedtls_connect(const char* host, int sport, const unsigned char* cas_pem,
                                           uint32_t cas_pem_len) {
  char port[32];
  int32_t ret = 0;
  uint32_t flags = 0;
  object_t* obj = NULL;
  const char* pers = "awtk_mbedtls";
  tk_iostream_mbedtls_t* iostream_mbedtls = NULL;
  return_value_if_fail(host != NULL && port > 0, NULL);
  obj = object_create(&s_tk_iostream_mbedtls_vtable);
  iostream_mbedtls = TK_IOSTREAM_MBEDTLS(obj);
  return_value_if_fail(iostream_mbedtls != NULL, NULL);

  tk_snprintf(port, sizeof(port)-1, "%d", sport);
  /*
   * 0. Initialize the RNG and the session data
   */
  mbedtls_net_init(&(iostream_mbedtls->server_fd));
  mbedtls_ssl_init(&(iostream_mbedtls->ssl));
  mbedtls_ssl_config_init(&(iostream_mbedtls->conf));
  mbedtls_x509_crt_init(&(iostream_mbedtls->cacert));
  mbedtls_ctr_drbg_init(&(iostream_mbedtls->ctr_drbg));

  log_debug("\n  . Seeding the random number generator...");

  mbedtls_entropy_init(&(iostream_mbedtls->entropy));
  if ((ret = mbedtls_ctr_drbg_seed(&(iostream_mbedtls->ctr_drbg), mbedtls_entropy_func,
                                   &(iostream_mbedtls->entropy), (const unsigned char*)pers,
                                   strlen(pers))) != 0) {
    log_debug(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
    goto error;
  }

  log_debug(" ok\n");

  /*
     * 0. Initialize certificates
     */
  log_debug("  . Loading the CA root certificate ...");

  if (cas_pem != NULL) {
    ret = mbedtls_x509_crt_parse(&(iostream_mbedtls->cacert), (const unsigned char*)cas_pem,
                                 cas_pem_len);
    if (ret < 0) {
      log_debug(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", (unsigned int)-ret);
      goto error;
    }
  }

  log_debug(" ok (%d skipped)\n", ret);

  /*
   * 1. Start the connection
   */
  log_debug("  . Connecting to tcp/%s/%s...", host, port);

  if ((ret = mbedtls_net_connect(&(iostream_mbedtls->server_fd), host, port,
                                 MBEDTLS_NET_PROTO_TCP)) != 0) {
    log_debug(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
    goto error;
  }

  log_debug(" ok\n");

  /*
   * 2. Setup stuff
   */
  log_debug("  . Setting up the SSL/TLS structure...");

  if ((ret = mbedtls_ssl_config_defaults(&(iostream_mbedtls->conf), MBEDTLS_SSL_IS_CLIENT,
                                         MBEDTLS_SSL_TRANSPORT_STREAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    log_debug(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
    goto error;
  }

  log_debug(" ok\n");
  /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
  if (cas_pem != NULL) {
    mbedtls_ssl_conf_authmode(&(iostream_mbedtls->conf), MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&(iostream_mbedtls->conf), &(iostream_mbedtls->cacert), NULL);
  }

  mbedtls_ssl_conf_rng(&(iostream_mbedtls->conf), mbedtls_ctr_drbg_random,
                       &(iostream_mbedtls->ctr_drbg));
  mbedtls_ssl_conf_dbg(&(iostream_mbedtls->conf), my_debug, NULL);

  if ((ret = mbedtls_ssl_setup(&(iostream_mbedtls->ssl), &(iostream_mbedtls->conf))) != 0) {
    log_debug(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
    goto error;
  }

  if ((ret = mbedtls_ssl_set_hostname(&(iostream_mbedtls->ssl), host)) != 0) {
    log_debug(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
    goto error;
  }

  mbedtls_ssl_set_bio(&(iostream_mbedtls->ssl), &(iostream_mbedtls->server_fd), mbedtls_net_send,
                      mbedtls_net_recv, NULL);

  /*
   * 4. Handshake
   */
  log_debug("  . Performing the SSL/TLS handshake...");

  while ((ret = mbedtls_ssl_handshake(&(iostream_mbedtls->ssl))) != 0) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      log_debug(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int)-ret);
      goto error;
    }
  }

  log_debug(" ok\n");

  /*
     * 5. Verify the server certificate
     */
  if (cas_pem != NULL) {
    log_debug("  . Verifying peer X.509 certificate...");
    if ((flags = mbedtls_ssl_get_verify_result(&(iostream_mbedtls->ssl))) != 0) {
      char vrfy_buf[512];
      log_debug(" failed\n");
      mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);
      log_debug("%s\n", vrfy_buf);
      goto error;
    } else {
      log_debug(" ok\n");
    }
  }

  iostream_mbedtls->sock = iostream_mbedtls->server_fd.fd;
  iostream_mbedtls->istream = tk_istream_mbedtls_create(&(iostream_mbedtls->ssl));
  iostream_mbedtls->ostream = tk_ostream_mbedtls_create(&(iostream_mbedtls->ssl));
  TK_IOSTREAM(obj)->get_istream = tk_iostream_mbedtls_get_istream;
  TK_IOSTREAM(obj)->get_ostream = tk_iostream_mbedtls_get_ostream;

  return TK_IOSTREAM(obj);
error:
  OBJECT_UNREF(obj);

  return NULL;
}
