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

#include "action.h"

global_call_type action::global_calls;
    
action::action(){
    length=0;
    paramcount=0;
    opcode = 0;
    is_local_call = 0;
    extra_data_len = 0;
    addr = 0;
    extra_data = NULL;
    localpcount=0;
}

action::~action(){
    free(extra_data);
    vector<parameter*>::iterator it;
    for(it=params.begin();it!=params.end();++it){
        delete *it;
    }
    params.clear();
}

int action::write_to(FILE* f)
{
    fwrite(&is_local_call, 4, 1, f);
    fwrite(&opcode, 4, 1, f);
    fwrite(&paramcount, 4, 1, f);
    fwrite(&length, 4, 1, f);
    
    for(int i=0; i<paramcount; ++i){
        parameter *p = params[i];
        if(p->type == PTYPE_LOCALP){
            int localaddr = p->relative_addr + addr;
            fwrite(&localaddr, 4, 1, f);
        }else{
            fwrite(&p->val1, 4, 1, f);
        }
        
        if(p->type == PTYPE_GLOBALP){
            if(p->globalp == NULL){
                printf("RAGGEE 0x%X\n", p->val2);
            }
            int globaladdr = p->globalp->get_addr();
            fwrite(&globaladdr, 4, 1, f);
        }else{
            fwrite(&p->val2, 4, 1, f);
        }
        fwrite(&p->val3, 4, 1, f);
    }
    
    fwrite(extra_data, 1, extra_data_len, f);
    return length;
}

int action::read_from(FILE* f)
{
    old_addr = ftell(f);
    fread(&is_local_call, 4, 1, f);
    fread(&opcode, 4, 1, f);
    fread(&paramcount, 4, 1, f);
    fread(&length, 4, 1, f);
    
    for(int i=0; i<paramcount; ++i){
        parameter* p = new parameter;
        fread(&p->val1, 4, 1, f);
        fread(&p->val2, 4, 1, f);
        fread(&p->val3, 4, 1, f);
        
        if( ((p->val1>>24) & 0xff)!=0xff && 
              p->val1 > old_addr && p->val1 < old_addr+length){
            p->type = PTYPE_LOCALP;
            localpcount++;
            p->relative_addr = p->val1 - old_addr;
        }else if(p->val1 == 0xffffff41){
            global_calls[p->val2].push_back(p);
            p->globalp = NULL;
            p->type = PTYPE_GLOBALP;
        }else{
            p->type = PTYPE_VALUE;
        }
        
        params.push_back(p);
    }

    extra_data_len = length - 16 - paramcount*12;
    if(extra_data_len>0){
        extra_data = (uint8_t*)malloc(extra_data_len);
        fread(extra_data, 1, extra_data_len, f);
    }
    
    return length;
}

void action::init_op(int special, int newopcode, int newparamcount)
{
    is_local_call = special;
    opcode = newopcode;
    paramcount = newparamcount;
    
    length = 0x10 + paramcount*12;
    
    for(int i=0; i<paramcount; ++i){
        parameter* p = new parameter;
        p->val1 = 0xff000000;
        p->val2 = 0xff000000;
        p->val3 = 0xff000000;
        p->type = PTYPE_VALUE;
        params.push_back(p);
    }
}

int action::set_string(char* str, int param)
{
    if(param>paramcount) return 1;
    int reallen = strlen((char*)str)+1, len, *intdata;
    int new_extra_start_idx;
    uint8_t* new_extra_start;
    len = reallen + (4-reallen%4);
    if(len==0x1a)
        printf("reallen: 0x%X\n", reallen);
    
    
    if(param==0){
        params[param]->relative_addr = 16+paramcount*12;;
    }else{
        fprintf(stderr,"set_string: param!=0 not yet implemented\n");
        return 1;
    }
    if(localpcount>1){
        fprintf(stderr,"set_string: localpcount > 1 not yet implemented\n");
        return 1;
    }
    
    if(params[param]->type == PTYPE_LOCALP){
        /* Delete and set new */
        
        free(extra_data);
        extra_data = NULL;
        length -= extra_data_len;
        extra_data_len = len+16;
        new_extra_start_idx = 0;
    }else{
        params[param]->type = PTYPE_LOCALP;
        localpcount++;
        new_extra_start_idx = extra_data_len;
        extra_data_len += len+16;
    }
    length += len+16;
    extra_data = (uint8_t*)realloc(extra_data, extra_data_len);
        
    new_extra_start = &extra_data[new_extra_start_idx];
    intdata = (int*)new_extra_start;
    memset(new_extra_start, 0, len+16);
    memcpy(&new_extra_start[16], str, reallen);
    
    *intdata++ = 0;
    *intdata++ = len/4;
    *intdata++ = 1;
    *intdata = len;

    return 0;
}

int action::get_string_from_param(char** target, int param)
{
    int offset = params[param]->relative_addr - 16-paramcount*12;
    uint8_t* stringdata = &extra_data[offset];
    int* intdata = (int*)stringdata;
    
    int len = intdata[3];
    
    *target = (char*)malloc(len);
    if(*target==NULL){
        perror("get_string_from_param malloc failed");
    }
    
    memcpy(*target, stringdata+16, len);
    return len;
}


