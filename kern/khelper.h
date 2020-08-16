#ifndef JOSS_KERN_KHELPER_H
#define JOSS_KERN_KHELPER_H

#include <inc/types.h>

uint32_t kern_atoi(char * data);
bool is_hex_string(char * data);
uint32_t parse_base16_string(char * data);
uint32_t parse_base10_string(char * data);             
#endif