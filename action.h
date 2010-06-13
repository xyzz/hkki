/*
Copyright 2010 Matthias Lanzinger. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY MATTHIAS LANZINGER ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS LANZINGER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Matthias Lanzinger.
*/

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
