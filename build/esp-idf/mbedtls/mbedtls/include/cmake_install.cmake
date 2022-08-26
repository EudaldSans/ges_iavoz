# Install script for directory: /Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Users/llmerlos/esp/env/v4.2/tools/tools/xtensa-esp32-elf/esp-2020r3-8.4.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "/Users/llmerlos/esp/env/v4.2/esp-idf/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

