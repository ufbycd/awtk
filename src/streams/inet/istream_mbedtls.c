/**
 * File:   istream_mbedtls.h
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
#include "tkc/socket_helper.h"
#include "streams/inet/istream_mbedtls.h"

static int32_t tk_istream_mbedtls_read(tk_istream_t* stream, uint8_t* buff, uint32_t max_size) {
  int32_t ret = 0;
  tk_istream_mbedtls_t* istream_mbedtls = TK_ISTREAM_MBEDTLS(stream);

  ret = recv(istream_mbedtls->sock, buff, max_size, 0);
  if (ret <= 0) {
    if (errno != EAGAIN) {
      perror("recv");
      istream_mbedtls->is_broken = TRUE;
    }
  }

  return ret;
}

static ret_t tk_istream_mbedtls_wait_for_data(tk_istream_t* stream, uint32_t timeout_ms) {
  tk_istream_mbedtls_t* istream_mbedtls = TK_ISTREAM_MBEDTLS(stream);

  return socket_wait_for_data(istream_mbedtls->sock, timeout_ms);
}

static ret_t tk_istream_mbedtls_get_prop(object_t* obj, const char* name, value_t* v) {
  tk_istream_mbedtls_t* istream_mbedtls = TK_ISTREAM_MBEDTLS(obj);
  if (tk_str_eq(name, TK_STREAM_PROP_FD)) {
    value_set_int(v, istream_mbedtls->sock);
    return RET_OK;
  } else if (tk_str_eq(name, TK_STREAM_PROP_IS_OK)) {
    bool_t is_ok = istream_mbedtls->sock >= 0 && istream_mbedtls->is_broken == FALSE;
    value_set_bool(v, is_ok);
    return RET_OK;
  }

  return RET_NOT_FOUND;
}

static const object_vtable_t s_tk_istream_mbedtls_vtable = {.type = "tk_istream_mbedtls",
                                                        .desc = "tk_istream_mbedtls",
                                                        .size = sizeof(tk_istream_mbedtls_t),
                                                        .get_prop = tk_istream_mbedtls_get_prop};

tk_istream_t* tk_istream_mbedtls_create(int sock) {
  object_t* obj = NULL;
  tk_istream_mbedtls_t* istream_mbedtls = NULL;
  return_value_if_fail(sock >= 0, NULL);

  obj = object_create(&s_tk_istream_mbedtls_vtable);
  istream_mbedtls = TK_ISTREAM_MBEDTLS(obj);
  return_value_if_fail(istream_mbedtls != NULL, NULL);

  istream_mbedtls->sock = sock;
  TK_ISTREAM(obj)->read = tk_istream_mbedtls_read;
  TK_ISTREAM(obj)->wait_for_data = tk_istream_mbedtls_wait_for_data;

  return TK_ISTREAM(obj);
}

tk_istream_mbedtls_t* tk_istream_mbedtls_cast(tk_istream_t* s) {
  return_value_if_fail(s != NULL && OBJECT(s)->vt == &s_tk_istream_mbedtls_vtable, NULL);

  return (tk_istream_mbedtls_t*)s;
}
