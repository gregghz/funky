#ifndef _TEXTUAL_HEADER_
#define _TEXTUAL_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int erase_string_contents(char *str);
int erase_string(char *str);
int streq(const char *first, const char *second);
int strch(const char *string, const char character);

int is_whitespace(const char c);
int is_opener(const char c);
int is_closer(const char c);
int is_input_terminator(const char c);
int is_escaper(const char c);
int is_paren(const char c);
int is_quotation(const char c);
int is_symbol_boundary(const char c);
int is_decimal_text(const char *txt);

char parenthetical_compliment(const char c);

#endif 
