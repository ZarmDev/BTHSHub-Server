#include <sodium.h>
#include <iostream>
#include <string>
#include "hash.h"

using namespace std;

    // if (sodium_init() < 0) {
    //     cerr << "Failed to initialize libsodium" << endl;
    //     return 1;
    // }
namespace Hash {
string hashPassword(const string& password) {
    char hashed_password[crypto_pwhash_STRBYTES];

    // Hash the password
    if (crypto_pwhash_str(
            hashed_password,
            password.c_str(),
            password.length(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        cerr << "Password hashing failed" << endl;
        return "";
    }

    cout << "Hashed password: " << hashed_password << endl;
    return string(hashed_password);
}

bool verifyPassword(const string& inputPassword, const string& hash) {
    if (crypto_pwhash_str_verify(
            hash.c_str(),
            inputPassword.c_str(),
            inputPassword.length()) == 0) {
        cout << "Password is correct!" << endl;
    } else {
        cout << "Password is incorrect!" << endl;
    }

    return 0;
}
}