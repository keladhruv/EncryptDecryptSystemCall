#include <asm/unistd.h>
#include </usr/include/stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include <dirent.h>
#ifndef __NR_cpenc
#error cpenc system call not defined
#endif

struct cpenc_parameters{
	
	char *infile;    		/* File to be Encrypted */
	char *outfile;   		/* File after Encrytiopm */
	void *keybuf;    		/* Buffer holding the cipher key*/
	unsigned int keylen; 	/* Length of the Buffer */
	unsigned int flag;		/* Flag to hold whether you are encrypting or decrypting */
	
};
/*
	Checks if filepath is NULL and for user permissions.
*/
void fileValidity(struct cpenc_parameters *param){
	
	if(param->infile==NULL){
		fprintf(stderr,  "[ERROR] Please mention the Input File. \n");
		exit(EXIT_FAILURE);
	}
	if(param->outfile==NULL){
		fprintf(stderr,  "[ERROR] Please mention the Output File. \n");
		exit(EXIT_FAILURE);
	}
	
/* 	This snippet of code which checks for user permissions has been referred from the follwoing link and modified according to use case.
	https://stackoverflow.com/questions/20238042/is-there-a-c-function-to-get-permissions-of-a-file */
	struct stat statRes;
	stat(param->infile, &statRes);
	mode_t bits = statRes.st_mode;
	if((bits & S_IRUSR) == 0){
		fprintf(stderr,  "[ERROR] User doesn't have read prividges. \n");
		exit(EXIT_FAILURE);
	}	
}

/*	
	Source - https://www.openssl.org/docs/man1.1.0/man3/PKCS5_PBKDF2_HMAC_SHA1.html
    RFC 2898 Suggestion of 1000 iterations.
	
	Checks if password is valid or not, depending on copy/encryption/decryption option chosen.
*/
 void passwordValidity(void *keybuf, char *password, int flag_copy){
	 if(password!=NULL){ 
		int error_no=0;
		if(password[strlen(password)-1]=='\n'){
			password[strlen(password)-1] = '\0';
		}
		if(strlen(password)<6){
			printf("[ERROR] PASSWORD SHOULD BE AT LEAST 6 CHARACTERS LONG\n");
			exit(EXIT_FAILURE);
		} 
		else{
			error_no=PKCS5_PBKDF2_HMAC_SHA1(password, strlen(password),NULL,0,1000, 16,  keybuf);
			if(error_no!=1){
				printf("[ERROR] FAILIURE IN GENERATION OF KEY FROM PASSWORD\n");
				exit(EXIT_FAILURE);
			} 
		}
	 }
	 else{
		if(flag_copy!=1){
		fprintf(stderr,  "[ERROR] Please mention the password {-p PASSWORD}  \n");
		exit(EXIT_FAILURE);
		}
		else{
			keybuf=NULL;
		}
	 }		 
} 

/*
	Checks if at least one flag is given.
*/
void flagValidity(struct cpenc_parameters *param, int flag_copy,int flag_encrypt, int flag_decrypt){
	
	if(flag_copy==0 && flag_encrypt==0 && flag_decrypt==0){
		fprintf(stderr,  "[ERROR] Please mention if you want to Copy/Encrypt/Decrypty {-e|-d|-c} \n");
		exit(EXIT_FAILURE);
	}
	else{
		if(flag_copy==1)
			param->flag=0x4;
		else if(flag_encrypt==1)
			param->flag=0x1;
		else if(flag_decrypt==1)
			param->flag=0x2;
			
	}	
}
/*
	All error numbers have been taken from here http://www-numi.fnal.gov/offline_software/srt_public_context/WebDocs/Errors/unix_system_errors.html
*/

int main(int argc, char* const argv[]){
	/* Variable Decleration */
	int rc;
	struct cpenc_parameters *param=malloc(sizeof(*param));
	void *buf=malloc(16*sizeof(char));
	char *password=NULL;
	(*param).infile=NULL;
	(*param).outfile=NULL;
	int c=0;
	int flag_encrypt=0, flag_decrypt=0, flag_copy=0,flag_password=0;
	
	
	
	
	while ((c = getopt(argc,  argv, "p:ecdh")) != -1) {

		switch (c) {
		
			case 'p':
					if (flag_password == 0) {
						password = optarg;
						flag_password=1;
					} 
					else {
						fprintf(stderr,  "[ERROR] %s: -p cannot be used more than once. \n",  argv[0]);
						exit(EXIT_FAILURE);
					}
					break;
			case ':':
					fprintf(stderr,  "[ERROR] %s: -p  option should be followed by the password.\n",  argv[0]);
					exit(EXIT_FAILURE);
					break;
			case 'd':
					if(flag_decrypt==0 && flag_encrypt==0 && flag_copy==0){
						flag_decrypt=1;
					}
					else if(flag_encrypt==1 || flag_copy==1){
						fprintf(stderr,"[ERROR] Please chose one of {-e|-d|-c}. \n");
						exit(EXIT_FAILURE);
					}
					else if(flag_decrypt==1){
						fprintf(stderr,"[ERROR] -d cannot be used more than once. \n");
						exit(EXIT_FAILURE);
					}	
					break;
			case 'e':
					if(flag_decrypt==0 && flag_encrypt==0 && flag_copy==0){
						flag_encrypt=1;
					}
					else if(flag_decrypt==1 || flag_copy==1){
						fprintf(stderr,"[ERROR] Please chose one of {-e|-d|-c}. \n");
						exit(EXIT_FAILURE);
					}
					else if(flag_encrypt==1){
						fprintf(stderr,"[ERROR] -e cannot be used more than once. \n");
						exit(EXIT_FAILURE);
					}
					break;
			case 'c':
					if(flag_decrypt==0 && flag_encrypt==0 && flag_copy==0){
						flag_copy=1;
					}
					else if(flag_decrypt==1 || flag_encrypt==1){
						fprintf(stderr,"[ERROR] Please chose one of {-e|-d|-c}. \n");
						exit(EXIT_FAILURE);
					}
					else if(flag_copy==1){
						fprintf(stderr,"[ERROR] -c cannot be used more than once. \n");
						exit(EXIT_FAILURE);
					}
					break;
			case 'h':
					fprintf(stdout,"Usage: %s {-p PASSWORD} {-e|-d|-c} [-h HELP] inputFile outputFile\n", argv[0]);
					fprintf(stdout,"-p : Password (Minimum length of Password - 6 characters)\n");
					fprintf(stdout,"-e : Encrypt Input File\n");
					fprintf(stdout,"-d : Decrypt Input File\n");
					fprintf(stdout,"-c : Copy Input File\n");
					fprintf(stdout,"-h : Help \n");
					fprintf(stdout,"inputFile: Input File\n");
					fprintf(stdout,"outputFile: Output File\n");
					exit(EXIT_FAILURE);
					break;
			case '?':
					fprintf(stderr,  "[ERROR] Please check %s -h\n",  argv[0]);
					exit(EXIT_FAILURE);
					break;
		}
	}
	
	
	flagValidity(param,flag_copy,flag_encrypt,flag_decrypt);
	passwordValidity(buf, password, flag_copy);
	

	(*param).infile=argv[optind];
	(*param).outfile=argv[++optind];	
	(*param).keybuf=buf;
	(*param).keylen=16;
	
	fileValidity(param);
	
	
  	rc = syscall(__NR_cpenc, param);
	if (rc == 0){
		if(flag_copy==1){
			printf("[INFO] Copying Sucessfull\n");
		}
		if(flag_encrypt==1){
			printf("[INFO] Encryption Sucessfull\n");
		}
		if(flag_decrypt==1){
			printf("[INFO] Decryption Successfull\n");
		}
	}	
	else{
		
		if(flag_copy==1){
			fprintf(stderr,"[ERROR] Error in copying, with %d (errno=%d)\n",rc, errno);
		}
		if(flag_encrypt==1){
			fprintf(stderr,"[ERROR] Error in Encryption, with %d (errno=%d)\n",rc, errno);
		}
		if(flag_decrypt==1){
			fprintf(stderr,"[ERROR] Error in Decryption, with %d (errno=%d)\n",rc, errno);
		}
		
		switch(errno)
		{
			case 1: 
				printf("[ERROR] EPERM: Incorrect Password given\n");
				break;

			case 2: 
				printf("[ERROR] ENOENT: File not found\n");
				break;

			case 9: 
				printf("[ERROR] EBADF: Bad File\n");
				break;

			case 12: 
				printf("[ERROR] ENOMEM: Out of memory\n");
				break;
			
			case 13: 
				printf("[ERROR] EACCES: Permissions not given to operate on file\n");
				break;
				
			case 14: 
				printf("[ERROR] EFAULT: Bad Address\n");
				break;

			case 21: 
				printf("[ERROR] EISDIR: Directory given in place of file.\n");
				break;
				
			case 22: 
				printf("[ERROR] EINVAL: Invalid argument\n");
				break;
		}
	}
	free(param);
	free(buf);
	exit(rc);
}
