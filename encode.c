#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }
    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Validate encode command line arguments */
Status read_and_validate_encode_args(int argc, char *argv[], EncodeInfo *encInfo)
{
    // Check argument count (program + 3 or 4 arguments)
    if (argc < 4 || argc > 5)
    {
        return e_failure;
    }

    //Source image (.bmp) or not
    int len = strlen(argv[2]);
    if (len < 4 || strcmp(&argv[2][len - 4], ".bmp") != 0)
    {
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];
    

    //Secret file  check
    if (strchr(argv[3], '.') == NULL)
    {
        return e_failure;
    }
    encInfo->secret_fname = argv[3];
    strncpy(encInfo->extn_secret_file,strrchr(argv[3], '.'),MAX_FILE_SUFFIX - 1);
    encInfo->extn_secret_file[MAX_FILE_SUFFIX - 1] = '\0';

    //Stego image is there or not
    if (argc == 4)
    {
        encInfo->stego_image_fname = "stego.bmp";
        printf("INFO : Output File Not Mentioned. Creating stego.bmp as default\n");
    }
    else
    {
        int len_out = strlen(argv[4]);
        if (len_out < 4 || strcmp(&argv[4][len_out - 4], ".bmp") != 0)
        {
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4]; 
    }
    return e_success;
}

/* Get size of secret file */
uint get_file_size(FILE *fptr)
{
    printf("INFO : Checking for secret.txt size\n");
    fseek(fptr, 0, SEEK_END);    // Move to end of file
    long int size = ftell(fptr); // Get file size
    printf("INFO : Done. NOT Empty\n");
    return size;

}

/* Check if image has enough capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo -> image_capacity = get_image_size_for_bmp(encInfo -> fptr_src_image);

    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);
    /*Check if image can store secret data */
    printf("INFO : Checking for beautiful.bmp capacity to handle secret.txt\n");
    if(encInfo->image_capacity > 8*(encInfo->size_secret_file + strlen(MAGIC_STRING) + 4 + strlen(encInfo->extn_secret_file) + 4))
    {
        return e_success;
    }
    else
    {
        printf("INFO : Done. Found not ok\n");
        return e_failure;
    }
}

/* Copy BMP header from source to stego image */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf("INFO : Copying Image Header\n");
    char buffer[54];
    rewind(fptr_src_image);                             // Move to file start
    // Read 54 bytes from source image
    if (fread(buffer, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }

    // Write 54 bytes to destination image
    if (fwrite(buffer,1,54, fptr_dest_image) != 54)
    {
        return e_failure;
    }

    return e_success;
}

/* Encode one byte into LSB of 8 image bytes */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Failure check
    if (image_buffer == NULL)
    {
        return e_failure;
    }

    for (int i = 0; i < 8; i++)
    {
        // Get bits from MSB to LSB
        int bit = (data >> (7 - i)) & 1;

        // Clear LSB and set new bit
        image_buffer[i] = (image_buffer[i] & ~1) | bit;
    }

    return e_success;
}

/* Encode data bytes into image */
Status encode_data_to_image(char *data, int size,FILE *fptr_src_image,FILE *fptr_stego_image)
{
    char buffer[8];
    for (int i = 0; i < size; i++)
    {
        // Read 8 bytes from source image
        if (fread(buffer, 1, 8, fptr_src_image) != 8)
        {
            return e_failure;
        }

        // Encode one byte into LSB
        encode_byte_to_lsb(data[i], buffer);

        // Write 8 bytes into stego image
        if (fwrite(buffer,1,8, fptr_stego_image) != 8)
        {
            return e_failure;
        }
    }

    return e_success;
}

/* Encode magic string */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("INFO : Encoding Magic String Signature\n");
    if(encode_data_to_image(MAGIC_STRING,strlen(magic_string), encInfo -> fptr_src_image,encInfo -> fptr_stego_image) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Encode integer size into 32 LSBs */
Status encode_size_to_lsb(int data, char *image_buffer)
{
    if (image_buffer == NULL)
    {
        return e_failure;
    }

    for (int i = 0; i < 32; i++)
    {
        int bit = (data >> (31 - i)) & 1; // get MSB first
        image_buffer[i] = (image_buffer[i] & ~1) | bit; //encode it lsb
    }

    return e_success;
}

/* Encode secret file extension size */
Status encode_secret_file_extn_size(long file_size, EncodeInfo *encInfo)
{
    // Validate EncodeInfo pointer
    if (encInfo == NULL)
    {
        printf("ERROR : EncodeInfo is NULL\n");
        return e_failure;
    }

    // Validate file pointers
    if (encInfo->fptr_src_image == NULL || encInfo->fptr_stego_image == NULL)
    {
        printf("ERROR : File pointer is NULL\n");
        return e_failure;
    }

    printf("INFO : Encoding secret.txt File Extension Size\n");

    char buffer[32];

    // Read 32 bytes from source image
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        printf("ERROR : Failed to read image data\n");
        return e_failure;
    }

    // Encode extension size into LSBs
    if (encode_size_to_lsb(file_size, buffer) == e_failure)
    {
        printf("ERROR : Failed to encode extension size\n");
        return e_failure;
    }

    // Write 32 bytes into stego image
    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        printf("ERROR : Failed to write stego image data\n");
        return e_failure;
    }

    return e_success;
}

/* Encode secret file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    printf("INFO : Encoding secret.txt File Extension\n");
    if(encode_data_to_image(encInfo -> extn_secret_file,strlen(encInfo -> extn_secret_file), encInfo -> fptr_src_image,encInfo -> fptr_stego_image) == e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }

}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    printf("INFO : Encoding secret.txt File Size\n");
    char image_buffer[32];

    // Read 32 bytes from source image
    if (fread(image_buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        return e_failure;
    }

    // Encode 32-bit file_size into LSBs
    encode_size_to_lsb(file_size, image_buffer);

    // Write 32 bytes to stego image
    if (fwrite(image_buffer, 1, 32, encInfo->fptr_stego_image) != 32)
    {
        return e_failure;
    }

    return e_success;
}

/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    rewind(encInfo->fptr_secret); //keep fptr pointing to start of file 
    printf("INFO : Encoding secret.txt File Data\n");

    char* secret_data = (char*)malloc(encInfo->size_secret_file); //dynamic allocation of memory 

    if (!secret_data)
    {
        printf("ERROR : Memory allocation failed\n"); 
        return e_failure;
    }
    
    // Read the secret file content
    if(fread(secret_data, 1, encInfo->size_secret_file, encInfo->fptr_secret) != encInfo->size_secret_file)
    {
        printf("ERROR : Reading Secret file failed\n");
        return 0;
    }
    if(encode_data_to_image(secret_data, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        return e_failure;
    }


    return e_success;
}

/* Copy remaining image data */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    printf("INFO : Copying remaining image data\n");

    // Read and check in the loop condition itself
    while(fread(&ch, 1, 1, fptr_src) == 1)
    {
        if(fwrite(&ch, 1, 1, fptr_dest) != 1)
        {
            printf("ERROR : Failed to write\n");
            return e_failure;
        }
    }

    // Check if we stopped due to EOF or error
    if(ferror(fptr_src))
    {
        printf("ERROR : Read error during copy\n");
        return e_failure;
    }
    return e_success;
}

/* Complete encoding procedure */
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("INFO : Opened %s\n",encInfo->stego_image_fname);
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Error While opening Files\n");
        return e_failure;
    }
    printf("INFO : ## Encoding Procedure Started ##\n");
    if(check_capacity(encInfo) == e_success)
    {
        printf("INFO : Done. Found Ok\n");
    }
    else
    {
        printf("ERROR : Secret File Size is Greater than Source File!\n");
        return e_failure;
    }

    if(copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Copy Header\n");
        return e_failure;
    }
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode Magic String\n");
        return e_failure;
    }
    if(encode_secret_file_extn_size(strlen( encInfo -> extn_secret_file),encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Extension Size\n");
        return e_failure;
    }
    if(encode_secret_file_extn( encInfo -> extn_secret_file,encInfo) == e_success)
    {
       printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Extension\n");
        return e_failure;
    }
    if(encode_secret_file_size( encInfo -> size_secret_file, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Size\n");
        return e_failure;
    }
    if(encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Data\n");
        return e_failure;
    }

    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Copy Remaining  Data\n");
        return e_failure;
    }
    
    /* Closing all files*/
    fclose(encInfo -> fptr_src_image); 
    fclose(encInfo -> fptr_secret);
    fclose(encInfo -> fptr_stego_image);
    return e_success;

}