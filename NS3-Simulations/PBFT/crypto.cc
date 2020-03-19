/*
 * Adithya Bhat <bhat24@purdue.edu>
 * 2019
 * */
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

#include "config.h"
#include "crypto.h"

RSA* import_key(const char* filename, bool isPublic)
{
	FILE * fp = fopen(filename,"rb");
	// Check if the file opens
	if(fp == NULL)
	{
		fprintf ( stderr, "Unable to open file %s \n" , filename );
		return NULL;
	}
	RSA *rsa= RSA_new() ;
	if ( isPublic )
	{
		rsa = PEM_read_RSAPublicKey(fp, &rsa, NULL, NULL);
	}
	else
	{
		rsa = PEM_read_RSAPrivateKey(fp, &rsa,NULL, NULL);
	}
	return rsa;
}

bool sign(RSA* rsa, unsigned char* msg, size_t msg_len, unsigned char** out,
		size_t* outlen)
{
	EVP_MD_CTX* m_RSASignCtx = EVP_MD_CTX_create();
	EVP_PKEY* priKey  = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(priKey, rsa);
	if (EVP_DigestSignInit(m_RSASignCtx,NULL, EVP_sha256(), NULL,priKey)<=0) {
		return false;
	}
	if (EVP_DigestSignUpdate(m_RSASignCtx, msg, msg_len) <= 0) {
		return false;
	}
	// Get the number of output bytes
	if (EVP_DigestSignFinal(m_RSASignCtx, NULL, outlen ) <=0 ) {
		return false;
	}
	// Allocate memory for signature
	*out = ( unsigned char* ) malloc ( *outlen );
	if (EVP_DigestSignFinal (m_RSASignCtx, *out, outlen) <= 0) {
		return false;
	}
	EVP_MD_CTX_free(m_RSASignCtx);
	return true;
}

bool verify(RSA* rsa, unsigned char* msg, size_t msg_len, unsigned char* sig,
		size_t sig_len)
{
	EVP_PKEY* pubKey  = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(pubKey, rsa);
	EVP_MD_CTX* m_RSAVerifyCtx = EVP_MD_CTX_create();
	if (EVP_DigestVerifyInit(m_RSAVerifyCtx,NULL, EVP_sha256(),NULL,pubKey) != 1) {
		return false;
	}
	if (EVP_DigestVerifyUpdate(m_RSAVerifyCtx, msg, msg_len) != 1 ) {
		return false;
	}
	int AuthStatus = EVP_DigestVerifyFinal(m_RSAVerifyCtx, sig, sig_len);
	EVP_MD_CTX_free(m_RSAVerifyCtx);
	if ( AuthStatus == 1 ) {
		return true;
	}
	return false;
}

bool mac_verify ( unsigned char* msg , size_t mlen , unsigned char* sig ,
	size_t slen , EVP_PKEY* pkey )
{
	EVP_MD_CTX* ctx = NULL;
	ctx = EVP_MD_CTX_create();
	const EVP_MD* md = EVP_sha256();
	int rc = EVP_DigestInit_ex(ctx, md, NULL);
	rc = EVP_DigestSignInit(ctx, NULL, md, NULL, pkey);
	rc = EVP_DigestSignUpdate(ctx, msg, mlen);
	unsigned char buff [ SHA256_DIGEST_LENGTH ];
	size_t size = sizeof(buff);

	rc = EVP_DigestSignFinal(ctx, buff, &size);
	const size_t m = (slen < size ? slen : size);
	rc = CRYPTO_memcmp(sig, buff, m);
	EVP_MD_CTX_destroy(ctx);
	if ( rc == 0 )
		return true;
	else 
		return false;
}

bool hash_data (
	unsigned char* data,
	size_t in_len,
	unsigned char* hash
)
{
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, data, in_len);
	SHA256_Final(hash, &sha256);
	return true;
}
