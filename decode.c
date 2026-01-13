#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Open stego image file for decoding */
Status open_file(DecodeInfo *decInfo)
{
    decInfo->output_fptr_secret = fopen(decInfo->output_secret_fname, "rb");
    if (decInfo->output_fptr_secret == NULL)
    {
        perror("fopen"); // Print system error
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->output_secret_fname);
        return e_failure;
    }
    return e_success;
}

/* Validate command-line arguments for decoding */
Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if(argc < 3 || argc > 4) // Check valid argument count
        return e_failure;

    char*dot_pos = strchr(argv[2], '.'); // Find extension in filename

    if(dot_pos && strcmp(dot_pos, ".bmp") == 0) 
    {
        decInfo->output_secret_fname = argv[2]; // Store stego image name
    } 
    else 
    {
        printf("ERROR : Source file extension is wrong\n");
        return e_failure;
    }
    /* Set default output filename if not provided */
    if(argv[3] == NULL)
    {
        decInfo->stego_image_fname = (char*)malloc(15); // Allocate memory for output file
        if (decInfo->stego_image_fname == NULL) {
            fprintf(stderr, " ERROR : Memory allocation failed\n");
            return e_failure;
        }
        strcpy(decInfo->stego_image_fname, "decoded_secret"); // Default filename
    }
    else
    {
         /* Output filename should not have extension */
        if(strchr(argv[3], '.') == NULL) 
        {
            decInfo->stego_image_fname = argv[3];
        } 
        else 
        {
            printf(" ERROR : File Extension should not be given\n");
            return e_failure;
        }
    }

    return e_success;
}

/* Decode 8 LSB bits into one byte */
Status decode_lsb_into_byte(char *data, char *image_buffer)
{
    for(int i = 0 ; i < 8 ; i++)
    {
        int bit = image_buffer[i] & 1; // Extract LSB
        *data = (*data << 1) | bit;    // Store bit into data
    }
    return e_success;
}

/* Decode multiple bytes from stego image */
Status decode_image_into_data(char *data, int size, FILE *fptr_stego_image)
{
    char buffer[8];

    for (int i = 0; i < size; i++)
    {
        /* Read 8 bytes from stego image */
        if (fread(buffer, 1, 8, fptr_stego_image) != 8)
        {
            return e_failure;
        }

           /* Decode one character */
        decode_lsb_into_byte(&data[i], buffer);
    }

    return e_success;
}

/* Decode and verify magic string */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    fseek(decInfo->output_fptr_secret, 54L, SEEK_SET); // Skip BMP header
     char buffer[2];

    printf("INFO : Decoding Magic String Signature\n");
    if(decode_image_into_data(buffer, 2, decInfo->output_fptr_secret) == e_failure)
    {
        printf("ERROR : Failed to Decode Magic String\n");
        return e_failure;
    }

    //printf("Decoded magic string -- %s\n",buffer);
    /* Validate magic string */
    if(strcmp(buffer,MAGIC_STRING) != 0)
    {
        printf(" ERROR : Magic string not matched to original \n");
        return e_failure;
    }

    return e_success;
}

/* Decode 32-bit integer from LSBs */
Status decode_lsb_into_size(int *data, char *image_buffer)
{
    for(int i = 0 ; i < 32 ; i++)
    {
        int bit = image_buffer[i] & 1; // Extract LSB
        *data = (*data << 1) | bit;    //store bit
    }

    return e_success;
}

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    printf("INFO : Decoding Secret File Extension Size\n");
    /* Read 32 bytes from stego image */
    if (fread(image_buffer, 1, 32, decInfo->output_fptr_secret) != 32)
    {
        return e_failure;
    }

    /* Decode extension size */
    decode_lsb_into_size(&decInfo->extn_size, image_buffer);

    return e_success;
}

/* Decode secret file extension */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[decInfo->extn_size];
    memset(buffer, 0, decInfo->extn_size); // Clear buffer
    printf("INFO : Decoding Secret File Extension\n");
    if(decode_image_into_data(buffer, decInfo->extn_size, decInfo->output_fptr_secret) == e_failure)
    {
        printf("ERROR : Failed to Decode Secret File Extension\n");
        return e_failure;
    }

    //printf("Decoded Extension -- %s \n",buffer);
    strcpy(decInfo->extn_secret_file, buffer); // Store extension

    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo) 
{
    char buffer[32];

    printf("INFO : Decoding Secret File Size \n");
    if(fread(buffer, sizeof(char), 32, decInfo->output_fptr_secret) != 32)
    {
        printf("ERROR : Failed to read Secret file size data\n");
        return e_failure;
    }
    decode_lsb_into_size((int*)&decInfo->size_secret_file, buffer); // Decode file size
   // printf("Decoded Secret file Size -- %ld\n",decInfo->size_secret_file);
    return e_success;
}

/* Decode actual secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    strcat(decInfo->stego_image_fname, decInfo->extn_secret_file); // Append extension

    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "w"); // Create output file
    printf("INFO : Opening decoded_secret.txt\n");
    if(decInfo->fptr_stego_image == NULL)
    {
        fprintf(stderr, "ERROR : Memory allocation failed\n");
        return e_failure;
    }
    printf("INFO : Done\n");

    char buffer[decInfo->size_secret_file];

    printf("INFO : Decoding Secret File Data \n");
    if(decode_image_into_data(buffer, decInfo->size_secret_file, decInfo->output_fptr_secret) == e_failure) // Write decoded data
    {
        printf("ERROR : Failed To Decode Secret File Data\n");
        return e_failure;
    }

    
    //printf("Decoded Secret Message Data -- %s \n",buffer);

    fwrite(buffer, sizeof(char), decInfo->size_secret_file, decInfo->fptr_stego_image);

    return e_success;
}

/* Perform complete decoding process */
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_file(decInfo) == e_success)
    {
        printf("INFO : Opened %s\n",decInfo->output_secret_fname);
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Error While opening File\n");
        return e_failure;
    }

    if (decode_magic_string(MAGIC_STRING, decInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode Magic String\n");
        return e_failure;
    }

    if(decode_secret_file_extn_size(decInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Decode secret.txt File Extension Size\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo) == e_success)
    {
       printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Extension\n");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Size\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to Encode secret.txt File Data\n");
        return e_failure;
    }

    
    fclose(decInfo -> output_fptr_secret); // Close stego image
    fclose(decInfo -> fptr_stego_image);    // Close output file
    
    return e_success;

}