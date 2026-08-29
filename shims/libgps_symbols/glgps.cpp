#include <cstring>

#include <openssl/ssl.h>

/*
 * BoringSSL no longer exports the SSLv3 entry points required by the legacy
 * Broadcom GPS daemon. TLSv1_method has the same method layout; replace its
 * low version byte (0x01) with 0x00 to present SSLv3 (0x0300).
 */
extern "C" const SSL_METHOD* SSLv3_method() {
    const SSL_METHOD* const method = TLSv1_method();
    const char ssl_version[2] = {0x00, 0x03};
    std::memcpy(const_cast<SSL_METHOD*>(method), ssl_version, 1);
    return method;
}

extern "C" const SSL_METHOD* SSLv3_client_method() {
    return SSLv3_method();
}
