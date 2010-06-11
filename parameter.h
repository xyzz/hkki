#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <stdint.h>
#include "action.h"

class action;

enum param_type{
    PTYPE_LOCALP,
    PTYPE_GLOBALP,
    PTYPE_VALUE
};


struct parameter{
    uint32_t val1, val2, val3;
    int relative_addr;
    action* globalp;
    param_type type;
};

#endif /* _PARAMETER_H */
