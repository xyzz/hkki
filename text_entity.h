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

#ifndef _TEXT_ENTITY_H
#define _TEXT_ENTITY_H

#include <wchar.h>
#include <stdint.h>
#include <list>
#include <vector>
#include <glib.h>

#include "action.h"


typedef list<action*>::iterator action_it;

class text_entity
{
  private:
    gchar* name_utf8;
    action* nameaction;
    vector<gchar*> lines_utf8;
    vector<action*> line_actions;
    int linecount, old_addr;
    action_it end;
    list<action*> *actions;
    
  public:
    bool is_answer, is_highlighted;
    
    
    text_entity();
    ~text_entity();
    
    action_it set_convo(action_it it, list<action*> *actions);
    action_it set_answer(action_it it);
    
    void reinsert_lines();
    int add_line();
    void remove(int n);
    
    
    gchar* get_line_utf8(int n){
        if(n>=linecount){
            return NULL;
        }
        return lines_utf8[n];
    }
    
    gchar* get_name_utf8(){
        return name_utf8;
    }
    
    int get_linecount(){
        return linecount;
    }
    
    void set_line_utf8(int index, gchar* new_line);
};


#endif /* _TEXT_ENTITY_H */
