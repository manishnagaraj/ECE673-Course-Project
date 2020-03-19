/*
 * Adithya Bhat <bhat24@purdue.edu>
 * 2019
 * */
#include <openssl/rsa.h>

/*
 * This function is used to import the RSA structure from the PEM key file
 * input: 
 * - filename: filename string for the key
 * - isPublic: boolean to specify whether the key is a public key or a private
 *   key
 * */
RSA* import_key ( 
	const char* filename, 
	bool isPublic
);

/*
 * This function takes a byte array and writes out a RSA signature in out buffer
 * */
bool sign ( 
	RSA* key, 
	unsigned char* data, 
	size_t in_len, 
	unsigned char** out,
	size_t* outlen
);

/*
 * This function takes a byte array and verifies the RSA signature in data buffer
 * */
bool verify (
	RSA* rsa, 
	unsigned char* msg, 
	size_t msg_len, 
	unsigned char* sig,
	size_t sig_len
);

bool mac_verify ( 
	unsigned char* msg , 
	size_t mlen , 
	unsigned char* sig ,
	size_t slen , 
	EVP_PKEY* pkey 
);

bool hash_data (
	unsigned char* data,
	size_t in_len,
	unsigned char* hash
);
