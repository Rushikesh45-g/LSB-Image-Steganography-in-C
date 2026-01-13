#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

/* Check whether encode or decode operation is requested */
OperationType check_operation_type(char *argv[])
{
    // Compare argument with encode flag
    if(strcmp(argv[1],"-e") == 0) 
        return e_encode;
    else if(strcmp(argv[1],"-d") == 0) // Compare argument with decode flag
        return e_decode;
    else
        e_unsupported;  // Unsupported operation
}

int main(int argc, char*argv[])
{
    EncodeInfo encInfo; // Structure for encoding information
    DecodeInfo decInfo; // Structure for decoding information
    
     /* Perform encoding operation */
    if(check_operation_type(argv) == e_encode)
    {
        // Validate encode arguments
        if(read_and_validate_encode_args(argc, argv, &encInfo) == e_success)
        {
            printf("INFO : Opening Required Files\n");
            printf("INFO : Opened Beautiful.bmp\n");
            printf("INFO : Opened Secret.txt\n");
            // Start encoding process
            if(do_encoding(&encInfo) == e_success)
            {
                printf("INFO : ## Encoding Done Successfully ##\n");
            }
            else  
            { 
                printf("ERROR : Encoding Failed !\n"); 
            }
        }
        else  
        { 
            printf("ERROR : Failed Arguments are not correct\n");
            return e_failure;
        }
    }

    /* Perform decoding operation */
    if(check_operation_type(argv) == e_decode)
    {
        printf("INFO : ## Decoding Procedure Started ##\n");
        printf("INFO : Opening Required Files\n");
        // Validate decode arguments
        if(read_and_validate_decode_args(argc, argv, &decInfo) == e_success)
        {
            // Start decoding process
            if(do_decoding(&decInfo) == e_success)
            {
                printf("INFO : ## Decoding Done Successfully ##\n");
            }
            else
            {
                printf("ERROR : Decoding Failed");
            }

        }
        else  
        { 
            printf("ERROR : Failed ,Arguments are not correct\n");
            return e_failure;
        }
    }

    return 0;  // Program execution successful
} 
