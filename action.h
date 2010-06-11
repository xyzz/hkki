#ifndef _ACTION_H
#define _ACTION_H

#include <map>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <string.h>

#include "parameter.h"


#define global_call_type  map<int,vector<parameter*> > 
#define OFFSET_EXPORT_ADDR      0x20
#define SIZE_EXPORT             0x28

using namespace std;


class action
{
  private:
    int length, paramcount, old_addr, addr;
    uint32_t opcode, is_local_call;
    uint8_t* extra_data, extra_data_len;
    vector<parameter*> params;
    
  public:    
    static global_call_type global_calls;
    
    action();
    ~action();
    
    int write_to(FILE* f);
    int read_from(FILE* f);

    void init_op(int special, int newopcode, int newparamcount);
    int set_string(wchar_t* str, int param);
    int get_string_from_param(wchar_t** target, int param);
    
    int get_length(){
        return length;
    }
    uint32_t get_opcode(){
        return opcode;
    }
    int get_old_addr(){
        return old_addr;
    }
    int get_addr(){
        return addr;
    }
    void set_addr(int newaddr){
        addr = newaddr;
    }
};

#endif /*_ACTION_H */
