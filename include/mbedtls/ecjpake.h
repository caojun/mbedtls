/**
 * \file ecjpake.h
 *
 * \brief Elliptic curve J-PAKE
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */
#ifndef MBEDTLS_ECJPAKE_H
#define MBEDTLS_ECJPAKE_H

/*
 * Implementation based on Chapter 7.4 of the Thread v1.0 Specification,
 * available from the Thread Group http://threadgroup.org/
 *
 * This file implements the EC J-PAKE algorithm, with payload serializations
 * suitable for use in TLS, but the result could be used outside TLS.
 */

#include "ecp.h"
#include "md.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Roles in the EC J-PAKE exchange
 */
typedef enum {
    MBEDTLS_ECJPAKE_CLIENT = 0,         /**< Client                         */
    MBEDTLS_ECJPAKE_SERVER,             /**< Server                         */
} mbedtls_ecjpake_role;

/**
 * EC J-PAKE context structure.
 *
 * J-PAKE is a symmetric protocol, except for the identifiers used in
 * Zero-Knowledge Proofs, and the serialization of the second message
 * (KeyExchange) as defined by the Thread spec.
 *
 * In order to benefit from this symmetry, we choose a different naming
 * convetion from the Thread v1.0 spec. Correspondance is indicated in the
 * description as a pair C: <client name>, S: <server name>
 */
typedef struct
{
    const mbedtls_md_info_t *md_info;   /**< Hash to use                    */
    mbedtls_ecp_group grp;              /**< Elliptic curve                 */
    mbedtls_ecjpake_role role;          /**< Are we client or server?       */

    mbedtls_ecp_point Xm1;              /**< My public key 1   C: X1, S: X3 */
    mbedtls_ecp_point Xm2;              /**< My public key 2   C: X2, S: X4 */
    mbedtls_ecp_point Xp1;              /**< Peer public key 1 C: X3, S: X1 */
    mbedtls_ecp_point Xp2;              /**< Peer public key 2 C: X4, S: X2 */
    mbedtls_ecp_point Xp;               /**< Peer public key   C: Xs, S: Xc */

    mbedtls_mpi xm1;                    /**< My private key 1  C: x1, S: x3 */
    mbedtls_mpi xm2;                    /**< My private key 2  C: x2, S: x4 */

    mbedtls_mpi s;                      /**< Pre-shared secret (passphrase) */
} mbedtls_ecjpake_context;

/*
 * \brief           Initialize a context
 *                  (just makes it ready for setup() or free()).
 *
 * \param ctx       context to initialize
 */
void mbedtls_ecjpake_init( mbedtls_ecjpake_context *ctx );

/*
 * \brief           Set up a context for use
 *
 * \note            Currently the only values for hash/curve allowed by the
 *                  standard are MBEDTLS_MD_SHA256/MBEDTLS_ECP_DP_SECP256R1.
 *
 * \param ctx       context to set up
 * \param role      Our role: client or server
 * \param hash      hash function to use (MBEDTLS_MD_XXX)
 * \param curve     elliptic curve identifier (MBEDTLS_ECP_DP_XXX)
 * \param secret    pre-shared secret (passphrase)
 * \param len       length of the shared secret
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_setup( mbedtls_ecjpake_context *ctx,
                           mbedtls_ecjpake_role role,
                           mbedtls_md_type_t hash,
                           mbedtls_ecp_group_id curve,
                           const unsigned char *secret,
                           size_t len );

/*
 * \brief           Generate and write contents of ClientHello extension
 *                  (excluding extension type and length bytes)
 *
 * \param ctx       Context to use
 * \param buf       Buffer to write the contents to
 * \param len       Buffer size
 * \param olen      Will be updated with the number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_write_client_ext( mbedtls_ecjpake_context *ctx,
                            unsigned char *buf, size_t len, size_t *olen,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng );
/*
 * \brief           Read and process contents of the ClientHello extension
 *                  (excluding extension type and length bytes)
 *
 * \param ctx       Context to use
 * \param buf       Pointer to extension contents
 * \param len       Extension length
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_read_client_ext( mbedtls_ecjpake_context *ctx,
                                         const unsigned char *buf,
                                         size_t len );

/*
 * \brief           Generate and write contents of ServerHello extension
 *                  (excluding extension type and length bytes)
 *
 * \param ctx       Context to use
 * \param buf       Buffer to write the contents to
 * \param len       Buffer size
 * \param olen      Will be updated with the number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_write_server_ext( mbedtls_ecjpake_context *ctx,
                            unsigned char *buf, size_t len, size_t *olen,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng );

/*
 * \brief           Read and process contents of the ServerHello extension
 *                  (excluding extension type and length bytes)
 *
 * \param ctx       Context to use
 * \param buf       Pointer to extension contents
 * \param len       Extension length
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_read_server_ext( mbedtls_ecjpake_context *ctx,
                                         const unsigned char *buf,
                                         size_t len );

/*
 * \brief           Generate and write ServerECJPAKEParams
 *                  (the contents for the ServerKeyExchange)
 *
 * \param ctx       Context to use
 * \param buf       Buffer to write the contents to
 * \param len       Buffer size
 * \param olen      Will be updated with the number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_write_server_params( mbedtls_ecjpake_context *ctx,
                            unsigned char *buf, size_t len, size_t *olen,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng );

/*
 * \brief           Read and process ServerECJPAKEParams
 *                  (the contents for the ServerKeyExchange)
 *
 * \param ctx       Context to use
 * \param buf       Pointer to the message
 * \param len       Message length
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_read_server_params( mbedtls_ecjpake_context *ctx,
                                            const unsigned char *buf,
                                            size_t len );

/*
 * \brief           Generate and write ClientECJPAKEParams
 *                  (the contents for the ClientKeyExchange)
 *
 * \param ctx       Context to use
 * \param buf       Buffer to write the contents to
 * \param len       Buffer size
 * \param olen      Will be updated with the number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_write_client_params( mbedtls_ecjpake_context *ctx,
                            unsigned char *buf, size_t len, size_t *olen,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng );

/*
 * \brief           Read and process ClientECJPAKEParams
 *                  (the contents for the ClientKeyExchange)
 *
 * \param ctx       Context to use
 * \param buf       Pointer to the message
 * \param len       Message length
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_read_client_params( mbedtls_ecjpake_context *ctx,
                                            const unsigned char *buf,
                                            size_t len );

/*
 * \brief           Derive the Pre-Master Secret used by TLS
 *
 * \param ctx
 * \param buf       Buffer to write the contents to
 * \param len       Buffer size
 * \param olen      Will be updated with the number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 if successfull,
 *                  a negative error code otherwise
 */
int mbedtls_ecjpake_tls_derive_pms( mbedtls_ecjpake_context *ctx,
                            unsigned char *buf, size_t len, size_t *olen,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng );

/*
 * \brief           Free a context's content
 *
 * \param ctx       context to free
 */
void mbedtls_ecjpake_free( mbedtls_ecjpake_context *ctx );

#if defined(MBEDTLS_SELF_TEST)
/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if a test failed
 */
int mbedtls_ecjpake_self_test( int verbose );
#endif

#ifdef __cplusplus
}
#endif

#endif /* ecjpake.h */
