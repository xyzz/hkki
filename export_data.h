#ifndef _EXPORT_DATA_H
#define _EXPORT_DATA_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "action.h"


class export_data{
  public:
    char name[32];
    int old_addr, addr;
    action* exported_action;
    
    export_data(){
        memset(name,0,32);
        old_addr = 0;
        addr = 0;
        exported_action = NULL;
    }
    
    int write_to(FILE* f)
    {
        int zero = 0;
        
        addr = exported_action->get_addr();
        
        fwrite(&zero, 4, 1, f);
        fwrite(name, 32, 1, f);
        fwrite(&addr, 4, 1, f);
        return 0;
    }
    
};

#endif /* _EXPORT_DATA_H */
