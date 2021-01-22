#pragma once

struct error_table
{
    const char* name;
    unsigned int code;
    const char* message;
};

#define ARRAYLEN(a) (sizeof(a)/sizeof(a[0]))
