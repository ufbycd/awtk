/**
 * File:   iostream_mbedtls.h
 * Author: AWTK Develop Team
 * Brief:  input stream base on mbedtls
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
 * 2021-01-04 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#ifndef TK_IOSTREAM_MBEDTLS_H
#define TK_IOSTREAM_MBEDTLS_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time            time
#define mbedtls_time_t          time_t
#define mbedtls_fprintf         fprintf
#define mbedtls_printf          printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_NET_C) || !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_CERTS_C) || !defined(MBEDTLS_PEM_PARSE_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)
#error "bad config"
#endif
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "tkc/fs.h"
#include "tkc/iostream.h"

BEGIN_C_DECLS

struct _tk_iostream_mbedtls_t;
typedef struct _tk_iostream_mbedtls_t tk_iostream_mbedtls_t;

/**
 * @class tk_iostream_mbedtls_t
 * @parent tk_iostream_t
 *
 * 基于MBEDTLS实现的输入输出流。
 *
 */
struct _tk_iostream_mbedtls_t {
  tk_iostream_t iostream;

  int sock;
  tk_istream_t* istream;
  tk_ostream_t* ostream;
  mbedtls_net_context fd;
  mbedtls_ssl_context ssl;
};

/**
 * @method tk_iostream_mbedtls_create
 *
 * 创建iostream对象。
 *
 * @param {int} sock socket。
 *
 * @return {tk_iostream_t*} 返回iostream对象。
 *
 */
tk_iostream_t* tk_iostream_mbedtls_create(int sock);

#define TK_IOSTREAM_MBEDTLS(obj) ((tk_iostream_mbedtls_t*)(obj))

END_C_DECLS

#endif /*TK_IOSTREAM_MBEDTLS_H*/
