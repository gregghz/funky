#include "texty.h"

int erase_string_contents(char *str) {
    char *cur=str;
    if(!str)
        return 1;
    while(*cur) {
        *cur='\0';
        ++cur;
    }
    return 0;
}

int erase_string(char *str) {
    if(erase_string_contents(str)!=0)
        return 1;
    free(str);
    return 0;
}

int streq(const char *first, const char *second) {
    return (first && second) ? strcmp(first, second)==0 : 0;
}

int is_whitespace(const char c) {
    return c==' ' || c=='\t' || c=='\n' || c=='\r' || c==',';
}

int is_input_terminator(const char c) {
    return c=='\0' || c==EOF;
}

int is_opener(const char c) {
    return c=='(' || c=='[' || c=='{';
}

int is_closer(const char c) {
    return c==')' || c==']' || c=='}';
}

int is_paren(const char c) {
    return is_opener(c) || is_closer(c);
}

int is_escaper(const char c) {
    return c=='\\';
}

int is_quotation(const char c) {
    return c=='"' || c=='\'';
}

int is_symbol_boundary(const char c) {
    return c==EOF || is_whitespace(c) || is_paren(c) || is_quotation(c);
}

char parenthetical_compliment(const char c) {
    switch (c) {
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        case '{': return '}';
        case '}': return '{';
    }
    return c;
}

static int is_digit(const char c) {
    switch(c) {
        case '0': case '1': case '2': case '3': 
        case '4': case '5': case '6': case '7': 
        case '8': case '9': case 'E': case '.':
            return 1;
    }
    return 0;
}

static int is_valid_numeric(const char c) {
    return c=='-' || is_digit(c);
}

int is_decimal_text(const char *txt) {
    unsigned long i=0;
    unsigned long l=strlen(txt);
    if(strlen(txt)==1) {
        if(*txt=='-')
            return 0;
        if(*txt=='.')
            return 0;
    }
    if(!is_valid_numeric(txt[i]))
        return 0;
    for(i=1;i<l;++i)
        if(!is_digit(txt[i]))
            return 0;
    return 1;
}
