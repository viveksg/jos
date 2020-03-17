#include "khelper.h"

uint32_t 
kern_atoi(char * data){
    uint32_t result =  is_hex_string(data) ? parse_base16_string(data) : parse_base10_string(data);
    return result;
}

bool 
is_hex_string(char * num){
    int i = 0;
    while(num[i] != '\0' && num[i] != ' '){
        if(num[i++] == 'x')
           return true;
    }
    return false;
}

uint32_t 
parse_base16_string(char * data){
    uint32_t result = 0, int_val = 0, i = 0;
    bool char_x_found = false;
    while (data[i] != '\0' && data[i] != ' ')
    {   
        
        if(data[i] >= '0' && data[i] <= '9')
        {
            int_val = data[i] - '0';
        }
        else if((data[i] >='a' && data[i] < 'g') || (data[i] >= 'A' && data[i] < 'G')){
            int_val = 9 + ((int)data[i] & 31);
        }
        else
        {   
            if(data[i] == 'x' && (!char_x_found)){
                char_x_found = true;
                i++;
                continue;
            }
            result = 0;
            break;
        }
        result = result * 16 + int_val;
        i++;
    }
    return result;
}

uint32_t 
parse_base10_string(char * data){
    uint32_t result = 0, int_val = 0, i = 0;
	while(data[i] != '\0' && data[i] != ' ')
    {
        if(data[i] < '0' || data[i] > '9'){
            result = 0;
            break;
        }
	    int_val = data[i] - '0';
        result = result * 10 + int_val;
        i++;
	}
    return result;
}


