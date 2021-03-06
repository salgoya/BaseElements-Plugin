/*
 BEOpenSSLAES.cpp
 BaseElements Plug-In
 
 Copyright 2014 Goya. All rights reserved.
 For conditions of distribution and use please see the copyright notice in BEPlugin.cpp
 
 http://www.goya.com.au/baseelements/plugin
 
 */


#include "BEOpenSSLAES.h"
#include "BEPluginException.h"


#include <openssl/evp.h>


using namespace std;
using namespace fmx;


void GenerateKeyAndInputVector ( const string password, string& key, vector<char>& input_vector )
{
	const int key_length = 32;
	const int rounds = 5;
	unsigned char key_buffer [ key_length ];
	unsigned char iv [ key_length ];
	
	int key_size = EVP_BytesToKey ( EVP_aes_256_cbc(), EVP_sha1(), NULL, (unsigned char*)password.c_str(), (int)password.size(), rounds, key_buffer , iv );
	if ( key_size == key_length ) {
		
		key.assign ( key_buffer, key_buffer + key_length );
		input_vector.assign ( iv, iv + key_length );
		
	} else {
		throw BEPlugin_Exception ( kKeyEncodingError );
	}
	
} // GenerateKeyAndInputVector


//		http://www.openssl.org/docs/crypto/EVP_EncryptInit.html#


const vector<char> Encrypt_AES ( const string key, const string text, const vector<char> input_vector )
{
	vector<char> result;
	
	EVP_CIPHER_CTX context;
	EVP_CIPHER_CTX_init ( &context );
	int reply = EVP_EncryptInit_ex ( &context, EVP_aes_256_cfb128(), NULL, (unsigned char *)key.c_str(), (unsigned char *)&input_vector[0] );
	
	if ( reply ) {
		
		const size_t input_size = text.size();
		const size_t output_size = input_size + EVP_MAX_BLOCK_LENGTH;
		unsigned char * encrypted_data = new unsigned char [ output_size ]();
		
		int output_length = 0;
		int final_output_length = 0;
		
		reply = EVP_EncryptUpdate ( &context, encrypted_data, &output_length, (unsigned char *)text.c_str(), (int)input_size );
		if ( reply ) {
			EVP_EncryptFinal_ex ( &context, encrypted_data + output_length, &final_output_length ); // reply =			
		} else {
			throw BEPlugin_Exception ( kEncryptionUpdateFailed );
		}
		
		EVP_CIPHER_CTX_cleanup ( &context );
		
		result.insert ( result.end(), encrypted_data, encrypted_data + output_length + final_output_length );
		
		delete[] encrypted_data;
		
	} else {
		throw BEPlugin_Exception ( kEncryptionInitialisationFailed );
	}
	
	return result;
	
} // Encrypt_AES


const vector<char> Decrypt_AES ( const string key, const vector<char> encrypted_data, const vector<char> input_vector )
{
	vector<char> result;
	
	if ( !encrypted_data.empty() ) {
		
		EVP_CIPHER_CTX context;
		EVP_CIPHER_CTX_init ( &context );
		int reply = EVP_DecryptInit_ex ( &context, EVP_aes_256_cfb128(), NULL, (unsigned char *)key.c_str(), (unsigned char *)&input_vector[0] );
		
		if ( reply ) {
			
			const size_t input_size = encrypted_data.size();
			const size_t output_size = input_size + EVP_MAX_BLOCK_LENGTH;
			unsigned char * decrypted_data = new unsigned char [ output_size ]();
			
			int output_length = 0;
			int final_output_length = 0;
			
			reply = EVP_DecryptUpdate ( &context, decrypted_data, &output_length, (unsigned char *)&encrypted_data[0], (int)input_size );
			if ( reply ) {
				
				EVP_DecryptFinal_ex ( &context, decrypted_data + output_length, &final_output_length );
				
			} else {
				throw BEPlugin_Exception ( kDecryptionUpdateFailed );
			}
			
			EVP_CIPHER_CTX_cleanup ( &context );
			
			result.insert ( result.end(), decrypted_data, decrypted_data + output_length + final_output_length );
			
			delete[] decrypted_data;
			
		} else {
			throw BEPlugin_Exception ( kEncryptionInitialisationFailed );
		}
		
	}
	
	return result;
	
} // Decrypt_AES


