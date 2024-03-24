#include "includes.h"

#include "support.h"

#include <openssl/sha.h>

string sha1(const string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX sha1;
    SHA1_Init(&sha1);
    SHA1_Update(&sha1, input.c_str(), input.size());
    SHA1_Final(hash, &sha1);

    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}
