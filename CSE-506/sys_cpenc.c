#include <linux/linkage.h>
#include <linux/moduleloader.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/errno.h>
#include <linux/scatterlist.h>
#include <linux/crypto.h>
#include <crypto/hash.h>
#include <crypto/skcipher.h>
#include <linux/err.h>
#define SHA_DIGEST_SIZE 20

struct cpenc_parameters {
	char *infile;	/* File to be Encrypted */
	char *outfile;	/* File after Encrytiopm */
	void *keybuf;	/* Buffer holding the cipher key*/
	unsigned int keylen;	/* Length ff the Buffer */
	unsigned int flag;	/* Flag to hold encrypt/decrypt/copy */
} cpenc_parameters;
/*
*The  struct skcipher_def and function encrypt_decrypt have been taken and altered according to the usecase from the example given in
*Crypto API Kernel Document - (EG-Code Example For Symmetric Key Cipher Operation)
https://kernel.readthedocs.io/en/sphinx-samples/crypto-API.html#code-examples
*/
struct skcipher_def {
	struct scatterlist sg;
	struct crypto_skcipher *tfm;
	struct skcipher_request *req;
} skcipher_def;

static int encrypt_decrypt(char *buffer, void *keyhash, int bytes, int enc)
{
	struct skcipher_def sk;
	struct crypto_skcipher *skcipher = NULL;
	struct skcipher_request *req = NULL;
	char *ivda = "abcdefghijklmnop";
	char *ivdata = NULL;
	char *scratchpad = NULL;
	int error_no = 0;

	skcipher = crypto_alloc_skcipher("ctr-aes-aesni", 0, 0);
	if (IS_ERR(skcipher)) {
		error_no = -ENOMEM;
		pr_alert(KERN_ALERT"ERROR: ERROR  COULD NOT ALLOCATE SKCIPHER HANDLE\n");
		return PTR_ERR(skcipher);
	}
	req = skcipher_request_alloc(skcipher, GFP_KERNEL);
	if (!req) {
		pr_alert(KERN_ALERT"ERROR: ERROR IN SKCIPHER REQUEST\n");
		error_no = -ENOMEM;
		goto out;
	}
	if (crypto_skcipher_setkey(skcipher, keyhash, 16)) {
		pr_alert(KERN_ALERT"ERROR: KEY COULD NOT BE SET\n");
		error_no = -EAGAIN;
		goto out;
	}
	sk.tfm = skcipher;
	sk.req = req;
	scratchpad = kmalloc(bytes, GFP_KERNEL);
    if (!scratchpad) {
		error_no = -ENOMEM;
		pr_alert(KERN_ALERT"ERROR: COULD NOT ALLOCATE SPACE\n");
		goto out;
    }
	(void)memcpy(scratchpad, buffer, bytes);
	ivdata = kmalloc(16, GFP_KERNEL);
	if (!ivdata) {
		error_no = -ENOMEM;
		pr_alert(KERN_ALERT"ERROR: COULD NOT ALLOCATE SPACE\n");
		goto out;
    }
	(void)memcpy(ivdata, ivda, 16);
	sg_init_one(&sk.sg, scratchpad, bytes);
    (void)skcipher_request_set_crypt(req, &sk.sg, &sk.sg, bytes, ivdata);
    if (enc == 1)
		error_no = crypto_skcipher_encrypt(req);
    else if (enc == 2)
		error_no = crypto_skcipher_decrypt(req);
	if (error_no != 0) {
		if (enc == 1)
			pr_alert("ERROR: ERROR IN ENCRYPTION.\n");
		if (enc == 2)
			pr_alert("ERROR: ERROR IN DECRYPTION.\n");
		goto out;
	}
	(void)memcpy(buffer, scratchpad, bytes);
	goto out;
out:
    if (skcipher)
		crypto_free_skcipher(skcipher);
    if (req)
		skcipher_request_free(req);
	if (ivdata)
		kfree(ivdata);
    if (scratchpad)
		kfree(scratchpad);
    return error_no;
}
/*
	THIS FUNCTION IS BASED ON THE EXAMPLE GIVEN IN THE HW1 DOCUMENT AVAILABLE AT-https://www3.cs.stonybrook.edu/~ezk/cse506-s19/hw1.txt
	UNDER READING FILES FROM INSIDE THE KERNEL.
	IT HAS BEEN MODIFIED TO USE 'vfs_read'
*/
int readFile(struct file *fileRead, void *buffer, int size)
{
	int dataRead;
	mm_segment_t old_fs;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	dataRead = vfs_read(fileRead, buffer, size, &(fileRead->f_pos));
	set_fs(old_fs);
	return dataRead;
}
int writeFile(struct file *fileWrite, void *buffer, int size)
{
	int dataWrite;
	mm_segment_t old_fs;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	dataWrite = vfs_write(fileWrite, buffer, size, &(fileWrite->f_pos));
	set_fs(old_fs);
	return dataWrite;
}
/*
	Referred https://elixir.bootlin.com/linux/latest/source/include/linux/fs.h#L3473 for dentry and inode.
	preEncryptDecrypt function does the following-
	1)  Decides whether the operation is Encryption/Decryption/Copy.
	2)	Check if input file can be opened.
	2)	Check if output file exists.
	3)	Check if output file can be opened if it exists otherwise it creates output file.
	4)	Calls readFile(),writeFile() and encrypt_decrypt() function.
	5)	In case of failiure removes any temp file created.
*/

long preEncryptDecrypt(struct cpenc_parameters *parameters, void *keyhash)
{
	int error_no = 0, iterator = 0, leftBytes = 0, sizeOfFile = 0, iterationCount = 0, dataRead = 0, dataWrite = 0, flag_option = 0, flag_create = 0;
	struct file *fileIn, *fileTemp, *fileOut;
	struct filename *nameOfTheInFile, *nameOfTheOutFile;
	struct dentry *fileTemp_dentry, *fileOut_dentry;
	void *buffer = kmalloc(sizeof(char)*PAGE_SIZE, GFP_KERNEL);
	/*SETTING ENCRYPTION/DECRYPTION/COPYING FLAG*/
	if (parameters->flag & 0x1)
		flag_option = 1;
	else if (parameters->flag & 0x2)
		flag_option = 2;
	else if (parameters->flag & 0x4)
		flag_option = 0;
	/*Opening Files that are needed*/
	nameOfTheInFile = getname(parameters->infile);
	if (IS_ERR(nameOfTheInFile)) {
		pr_alert("ERROR: GETNAME FOR INPUT FILE FAILED\n");
		error_no = PTR_ERR(nameOfTheInFile);
		goto ERROR;
	}
	fileIn  = filp_open(nameOfTheInFile->name, O_RDONLY, 0);
	if (!(fileIn) || IS_ERR(fileIn)) {
		pr_alert("ERROR: INPUT FILE COULD NOT OPEN\n");
		error_no = PTR_ERR(fileIn);
		goto ERROR;
	}
	nameOfTheOutFile = getname(parameters->outfile);
	if (IS_ERR(nameOfTheOutFile)) {
		pr_alert("Error in file input getname\n");
		error_no = PTR_ERR(nameOfTheOutFile);
		goto ERROR;
	}
	/* Check if output file needs to be create */
	fileOut = filp_open(nameOfTheOutFile->name, O_RDONLY, 0);
	if (!(fileOut) || IS_ERR(fileOut)) {
		flag_create = 1;
		fileOut  = filp_open(nameOfTheOutFile->name, O_WRONLY|O_CREAT, fileIn->f_mode);
		if (!(fileOut) || IS_ERR(fileOut)) {
			pr_alert("ERROR: OUTPUT FILE COULD NOT BE CREATED\n");
			error_no = PTR_ERR(fileOut);
			goto ERROR;
		}
	}
	fileTemp = filp_open("temp.tmp", O_WRONLY|O_CREAT|O_TRUNC, fileIn->f_mode);
	if (!(fileTemp) || IS_ERR(fileTemp)) {
		pr_alert("ERROR: OUTPUT FILE COULD NOT OPEN\n");
		error_no = PTR_ERR(fileTemp);
		goto ERROR;
	} else {
		fileOut_dentry = fileOut->f_path.dentry;
		fileTemp_dentry = fileTemp->f_path.dentry;
		fileTemp->f_path.dentry->d_inode->i_opflags = fileIn->f_path.dentry->d_inode->i_opflags;
		fileTemp->f_path.dentry->d_inode->i_mode = fileIn->f_path.dentry->d_inode->i_mode;
		fileTemp->f_path.dentry->d_inode->i_flags = fileIn->f_path.dentry->d_inode->i_flags;
		fileTemp->f_path.dentry->d_inode->i_uid = fileIn->f_path.dentry->d_inode->i_uid;
		fileTemp->f_path.dentry->d_inode->i_gid = fileIn->f_path.dentry->d_inode->i_gid;
	}
	/* Calculating file byte read details */
	if (flag_option == 2) {
		sizeOfFile = fileIn->f_inode->i_size;
		iterationCount = (sizeOfFile-parameters->keylen)/PAGE_SIZE;
		leftBytes = (sizeOfFile-parameters->keylen)%PAGE_SIZE;
	} else {
		sizeOfFile = fileIn->f_inode->i_size;
		iterationCount = (sizeOfFile)/PAGE_SIZE;
		leftBytes = (sizeOfFile)%PAGE_SIZE;
	}
	/* Setting/Reading Preamble */
	if (flag_option == 1) {
		(void)memcpy(buffer, parameters->keybuf, parameters->keylen);
		dataWrite = writeFile(fileTemp, buffer, parameters->keylen);
		if (dataWrite != parameters->keylen) {
			pr_alert("ERROR: IN WRITING FILE.\n");
			goto REMOVE;
		}
	}
	if (flag_option == 2) {
		dataRead = readFile(fileIn, buffer, parameters->keylen);
		if (dataRead != parameters->keylen) {
			pr_alert("ERROR: IN READING FILE.\n");
			goto REMOVE;
		}
		if (memcmp(buffer, parameters->keybuf, parameters->keylen) != 0) {
			pr_alert("ERROR: INCORRECT PASSWORD\n");
			error_no = -EPERM;
			goto ERROR;
		}
	}
	/* Reading/Writing PAGE_SIZE size blocks */
	while (iterator < iterationCount) {
		dataRead = readFile(fileIn, buffer, PAGE_SIZE);
		if (dataRead != PAGE_SIZE) {
			pr_alert("ERROR: IN READING FILE.\n");
			goto REMOVE;
		}
		if (flag_option != 0) {
			error_no = encrypt_decrypt(buffer, keyhash, PAGE_SIZE, flag_option);
			if (error_no != 0) {
				if (flag_option == 1)
					pr_alert("ERROR: ERROR IN ENCRYPTING FILE.\n");
				else
					pr_alert("ERROR: ERROR IN DECRYPTING FILE.\n");
				goto REMOVE;
			}
		}
		dataWrite = writeFile(fileTemp, buffer, PAGE_SIZE);
		if (dataWrite != PAGE_SIZE) {
			pr_alert("ERROR: IN WRITING FILE.\n");
			goto REMOVE;
		}
		iterator += 1;
	}
	/* Reading/Writing bytes less than PAGE_SIZE blocks */
	if (leftBytes != 0) {
		dataRead = readFile(fileIn, buffer, leftBytes);
		if (dataRead != leftBytes) {
			pr_alert("ERROR: IN READING FILE.\n");
			goto REMOVE;
		}
		if (flag_option != 0) {
			encrypt_decrypt(buffer, keyhash, leftBytes, flag_option);
			if (error_no != 0) {
				if (flag_option == 1)
					pr_alert("ERROR: ERROR IN ENCRYPTING FILE.\n");
				else
					pr_alert("ERROR: ERROR IN DECRYPTING FILE.\n");
				goto REMOVE;
			}
		}
		dataWrite = writeFile(fileTemp, buffer, leftBytes);
		if (dataWrite != leftBytes) {
			pr_alert("ERROR: IN WRITING FILE.\n");
			goto REMOVE;
		}
	}
	/* moving temp file to ouputfile location*/
	error_no = vfs_rename(fileTemp_dentry->d_parent->d_inode, fileTemp_dentry, fileOut_dentry->d_parent->d_inode, fileOut_dentry, NULL, 0);
	if (error_no) {
		pr_alert("ERROR: ERROR IN OUTPUT FILE GENERATION\n");
		goto REMOVE;
	}
	if (flag_option == 0)
		pr_info("INFO: SUCCESSFULLY COPIED INPUT FILE TO OUTPUT FILE\n");
	if (flag_option == 1)
		pr_info("INFO: SUCCESSFULLY ENCRYPTED INPUT FILE TO OUTPUT FILE\n");
	if (flag_option == 2)
		pr_info("INFO: SUCCESSFULLY DECRYPTED INPUT FILE TO OUTPUT FILE\n");
	goto ERROR;

REMOVE:
	vfs_unlink(fileTemp_dentry->d_parent->d_inode, fileTemp_dentry, NULL);
	if (flag_create == 1)
		vfs_unlink(fileOut_dentry->d_parent->d_inode, fileOut_dentry, NULL);
	goto ERROR;
ERROR:
	if (nameOfTheInFile)
		kfree(nameOfTheInFile);
	if (nameOfTheOutFile)
		kfree(nameOfTheOutFile);
	if (fileIn)
		filp_close(fileIn, NULL);
	if (fileTemp)
		filp_close(fileTemp, NULL);
	if (fileOut)
		filp_close(fileOut, NULL);
	kfree(buffer);
	return error_no;
}
/*
	NOTE-OUTPUT FILE IS NULL CONDITION IS ALREADY CHECKED IN BASIC VALIDITY.
	File validation steps have been reffered from -
	(Linux doc) https://elixir.bootlin.com/linux/v4.5/source/drivers/md/md.c#L6280 and
	(Linux Core Utils 'cp' function) https://github.com/coreutils/coreutils/blob/master/src/cp.c
	validateOutputFile takes two parameters-cpenc_parameters for outfile data and ksIn which holds information about the input file. This is used to check if the input file and the output file are the same or not by checking their inode number.
*/
int validateOutputFile(struct cpenc_parameters *parameters, struct kstat ksIn)
{
	mm_segment_t old_fs;
	int error_no = 0;
	struct kstat ks;
	struct filename *nameOfTheFile;
	nameOfTheFile = getname(parameters->outfile);
	if (IS_ERR(nameOfTheFile)) {
		pr_alert("ERROR: IN GETNAME FOR OUTPUT FILE\n");
		error_no = PTR_ERR(nameOfTheFile);
		goto ERROR;
	}
	old_fs = get_fs();
	(void)set_fs(KERNEL_DS);
	error_no = vfs_stat(nameOfTheFile->name, &ks);
	(void)set_fs(old_fs);
	if (!error_no) {
		if (!S_ISREG(ks.mode)) {
			pr_alert("ERROR: OUTPUT FILE NOT REGULAR\n");
			error_no = -EBADF;
			goto ERROR;
		} else if (!(ks.mode & S_IWUSR)) {
			pr_alert("ERROR: OUTPUT FILE HAS NO WRITE ACCESS\n");
			error_no = -EACCES;
			goto ERROR;
		}
	} else {
		if (S_ISDIR(ks.mode)) {
			pr_alert("ERROR: PATH GIVEN FOR OUTPUT FILE IS A DIRECTORY\n");
			return -EISDIR;
		}
		error_no = 0;
	}
	if (ks.ino == ksIn.ino) {
		pr_alert("ERROR: INPUT FILE AND OUTPUT FILE CANNOT BE SAME\n");
		error_no = -EINVAL;
		goto ERROR;
	}
	return error_no;
ERROR:
	if (nameOfTheFile)
		kfree(nameOfTheFile);
	return error_no;
}
/*
	NOTE-INPUT FILE IS NULL CONDITION IS ALREADY CHECKED IN BASIC VALIDITY.
	File validation steps have been reffered from -
	(Linux doc) https://elixir.bootlin.com/linux/v4.5/source/drivers/md/md.c#L6280 and
	(Linux Core Utils 'cp' function) https://github.com/coreutils/coreutils/blob/master/src/cp.c
	The following function checks for input file validation and calls the validateOutputFile() function to check if the output file is valid and not equal to the input file.
*/
int validateInputFile(struct cpenc_parameters *parameters)
{
	mm_segment_t old_fs;
	int error_no = 0;
	struct kstat ks;
	struct filename *nameOfTheFile;
	nameOfTheFile = getname(parameters->infile);
	if (IS_ERR(nameOfTheFile)) {
		pr_alert("ERROR: IN GETNAME FOR INPUT FILE\n");
		error_no = PTR_ERR(nameOfTheFile);
		goto ERROR;
	}
	old_fs = get_fs();
	(void)set_fs(KERNEL_DS);
	error_no = vfs_stat(nameOfTheFile->name, &ks);
	(void)set_fs(old_fs);
	if (error_no) {
		pr_alert("ERROR: INCORRECT INPUT FILE\n");
		error_no = -EBADF;
		goto ERROR;
	} else if (S_ISDIR(ks.mode)) {
		pr_alert("ERROR: INPUT FILE IS A DIRECTORY\n");
		error_no = -EISDIR;
		goto ERROR;
	} else if (!(ks.mode & S_IRUSR)) {
		pr_alert("ERROR: PERMISSION TO READ INPUT FILE\n");
		error_no = -EACCES;
		goto ERROR;
	} else if (!(S_ISREG(ks.mode))) {
		pr_alert("ERROR: INPUT FILE NOT REGULAR\n");
		error_no = -EBADF;
		goto ERROR;
	}
	error_no = validateOutputFile(parameters, ks);
	if (error_no != 0)
		goto ERROR;
	return error_no;

ERROR:
	if (nameOfTheFile)
		kfree(nameOfTheFile);
	return error_no;
}
/*
	This function is based of the given link in the Linux Kernel Document. It is modified according to the usecase.
	https://elixir.bootlin.com/linux/v4.5/source/crypto/tcrypt.c#L958
	The generateHash function creates a 'sha1' hash based on the password.
*/
int generateHash(struct cpenc_parameters *parameters, void *keyhash)
{
	struct scatterlist sg[1];
	struct crypto_ahash *tfm;
	struct ahash_request *req;
	int error_no;
	tfm = crypto_alloc_ahash("sha1", CRYPTO_ALG_TYPE_SKCIPHER, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)) {
		pr_alert("ERROR:FAILED TO LOAD TRANSFORM FOR 'sha1'\n");
		return PTR_ERR(tfm);
	}
	req = ahash_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		pr_alert("ERROR:FAILED TO ALLOCATE AHASH REQUEST\n");
		return -ENOMEM;
	}
	(void)ahash_request_set_callback(req, 0, NULL, NULL);
	(void)sg_init_one(&sg[0], parameters->keybuf, parameters->keylen);
	(void)ahash_request_set_crypt(req, sg, keyhash, parameters->keylen);
	error_no = crypto_ahash_digest(req);
	if (error_no) {
		pr_alert("ERROR:FAILED TO CREATE HASH\n");
		return error_no;
	}
	(void)ahash_request_free(req);
	return error_no;
}
/* passwordValidity function checks validity of the password for 'encrypt' and 'decrypt' options. */
int passwordValidity(struct cpenc_parameters *parameters)
{
	if (!(parameters->flag & 0x4)) {
		if (parameters->keybuf == NULL) {
			pr_alert("ERROR: NO PASSWORD GIVEN.\n");
			return -EINVAL;
		}
		if (parameters->keylen < 6) {
			pr_alert("ERROR: PASSWORD LESS THAN 6 CHARACTERS.\n");
			return -EINVAL;
		}
		if (strlen(parameters->keybuf) < 6) {
			pr_alert("ERROR: PASSWORD LESS THAN 6 CHARACTERS.\n");
			return -EINVAL;
		}
	}
	return 0;
}
/*flagValidity function checks for if the correct flag is specified by the user.*/
int flagValidity(struct cpenc_parameters *parameters)
{	
	int counter=0;
	if(parameters->flag & 0x1)
		counter+=1;
	if(parameters->flag & 0x2) 
		counter+=1;
	if(parameters->flag & 0x4)
		counter+=1;
	
	if(counter != 1) {
		pr_alert(" INCORRECT FLAG OPTION\n");
		return -EINVAL;
	}
	
	return 0;
}
/*The basicValidity function checkf if the parameters were assigned values from the values passed from the user or not. We do not check for password and keylen here as the user might want to chose the 'copy' functionlity which does not require password or keylen. For the 'encrypt/decrypt' functionality the password and keylen is checked in the passwordValidity functionality. */
int basicValidity(struct cpenc_parameters *parameters)
{
	if (parameters->infile == NULL) {
		pr_alert("ERROR: NO INPUT FILE NAME GIVEN.\n");
		return -ENOENT;
	}
	if (parameters->outfile == NULL) {
		pr_alert("ERROR: NO OUTPUT FILE NAME GIVEN.\n");
		return -ENOENT;
	}
	if (parameters->flag == -1) {
		pr_alert("ERROR: NO FLAG FOR ENCRYPTION/DECRYPTION/COPY GIVEN.\n");
		return -EINVAL;
	}
	return 0;
}
/* This function intializes the values of the sruct cpenc_parameters to NULL and -1 according to the data type. */
void initialize(struct cpenc_parameters *parameters)
{
		parameters->infile = NULL;
		parameters->outfile = NULL;
		parameters->keybuf = NULL;
		parameters->keylen =  -1;
		parameters->flag =  -1;
}
/**
	This function controls the flow of the whole program.
	@arg: The abstract data type which received from the user holding the values of
	1) Input File
	2) Output File
	3) Password
	4) Length of Password
	5) Flags to encrypt/decrypt/copy
**/
asmlinkage extern long (*sysptr)(void *arg);
asmlinkage long cpenc(void *arg)
{	struct cpenc_parameters *parameters;
	int error_no = 0;
	void *keyhash;
	/* Basic Check */
	if (arg == NULL) {
		pr_alert("ERROR: NO ARGUMENTS WERE PASSED.\n");
		error_no = -EINVAL;
		return error_no;
	}
	parameters = kmalloc(sizeof(struct cpenc_parameters), GFP_KERNEL);
	if (!parameters) {
		pr_alert("ERROR: Memory not allocated for Structure.\n");
		error_no = -ENOMEM;
		goto ERROR;
	}
	(void)initialize(parameters);
	error_no = copy_from_user(parameters, arg, sizeof(struct cpenc_parameters));
	if (error_no != 0) {
		pr_alert("ERROR: COULD NOT COPY THE ARGUMENTS FROM USER.\n");
		error_no = -EFAULT;
		goto ERROR;
	}
	error_no = basicValidity(parameters);
	if (error_no != 0)
		goto ERROR;
	error_no = flagValidity(parameters);
	if (error_no != 0)
		goto ERROR;
	error_no = passwordValidity(parameters);
	if (error_no != 0)
		goto ERROR;
	if (!(parameters->flag & 0x4)) {
	keyhash = kmalloc(SHA_DIGEST_SIZE, GFP_KERNEL);
	error_no = generateHash(parameters, keyhash);
	}
	if (error_no != 0)
		goto ERROR;
	parameters->keybuf = keyhash;
	error_no = validateInputFile(parameters);
	if (error_no != 0)
		goto ERROR;
	/* Encrypt File */
	error_no = preEncryptDecrypt(parameters, keyhash);
	if (error_no != 0)
		goto ERROR;
ERROR:
	if (parameters)
		kfree(parameters);
	if (keyhash)
		kfree(keyhash);
	return error_no;
}
/*
The init and exit function below tells the kernel which function to load and unload.
These functions were given as a part of the program template.
*/
static int __init init_sys_cpenc(void)
{
	pr_info("installed new sys_cpenc module\n");
	if (sysptr == NULL)
		sysptr = cpenc;
	return 0;
}
static void  __exit exit_sys_cpenc(void)
{
	if (sysptr != NULL)
		sysptr = NULL;
	pr_info("removed sys_cpenc module\n");
}
module_init(init_sys_cpenc);
module_exit(exit_sys_cpenc);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DHRUV");
MODULE_VERSION("0.01");
MODULE_DESCRIPTION("ENCRYPTION/DECRYPTION");
