#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4  // ".txt" + '\0'


typedef struct _DecodeInfo
{
    /* Secret File Info */ // output of encode
    char *output_secret_fname;
    FILE *output_fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX]; //merge in decode extension
    long size_secret_file;
    int extn_size;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;


/* Decoding function prototype */

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(int argc,char *argv[], DecodeInfo *decInfo); //done

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo); //doing

/* Get File pointers for i/p and o/p files */
Status open_file(DecodeInfo *decInfo); //done //open one file only //optional do in decoding

/* skip_Copy bmp image header */
Status skip_bmp_header(FILE *fptr_stego_image); //done move fptr to 55th position

/* Read Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo); //done //use macro & compare it with macro.

/* Decode secret file extension size */
Status decode_secret_file_extn_size(DecodeInfo *decInfo); //done

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo); //done

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo); //done

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo); //doing

/* Decode function, which does the real encoding */
Status decode_image_into_data(char *data, int size,FILE *fptr_stego_image); //done

/* Decode a byte into LSB of image data array */
Status decode_lsb_into_byte(char *data, char *image_buffer); //done //return data  //&data[i] // here take (*)or return char and store in data[i]
//then change to char* data type
/* Decode a byte into LSB of image data array */
Status decode_lsb_into_size(int *data, char *image_buffer); //done //return address  e





#endif
