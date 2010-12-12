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

#include "text_entity.h"

text_entity::text_entity()
{
    name_utf8 = NULL;
    nameaction = NULL;
    linecount = 0;
    is_answer = false;
}

text_entity::~text_entity()
{
    g_free(name_utf8);
    for(int i=0; i<linecount;++i){
        g_free( lines_utf8[i] );
    }
    lines_utf8.clear();
    line_actions.clear();
}

action_it text_entity::set_convo(action_it it, list<action*> *actions)
{
    this->actions = actions;
    uint32_t opcode = (*it)->get_opcode();
    old_addr = (*it)->get_old_addr();
    while( opcode == ACTION_NAME || opcode == ACTION_TEXT ){
        
        if( opcode==ACTION_TEXT ){
            if(name_utf8==NULL){
                char* name;
                int len = (*it)->get_string_from_param(&name, 0);
                name_utf8 = g_convert((char*)name, len, "UTF-8",
                                      "shift_JIS", NULL, NULL, NULL);
                nameaction = *it;
            }
        }else{
            char* tmp;
            gchar* tmp_utf8;
            int len = (*it)->get_string_from_param(&tmp,0);            
            tmp_utf8 = g_convert((char*)tmp, len, "UTF-8", "shift_JIS",
                                NULL, NULL, NULL);
            lines_utf8.push_back(tmp_utf8);
            
            line_actions.push_back(*it);
            
            linecount++;
        }
        
        ++it;  end = it;
        if(it==actions->end())
            break;
        opcode = (*it)->get_opcode();
    }
    it--;
    return it;
}


action_it text_entity::set_answer(action_it it)
{
    char *tmp;
    gchar* tmp_utf8;
    int len = (*it)->get_string_from_param(&tmp, 0);
    tmp_utf8 = g_convert((char*)tmp, len, "UTF-8", "shift_JIS",
                                NULL, NULL, NULL);
    lines_utf8.push_back(tmp_utf8);
    line_actions.push_back(*it);
    linecount = 1;
    is_answer = true;

    return it;
}


void text_entity::set_line_utf8(int index, gchar* new_line)
{
    int new_len = strlen(new_line)+1;
    gchar* line = lines_utf8[index];
    g_free(line);
    line = (gchar*)g_malloc(new_len);
    if(line==NULL){
        perror("error on line malloc");
    }
    memcpy(line, new_line, new_len);
    lines_utf8[index] = line;
}

void text_entity::reinsert_lines()
{
    if(name_utf8!=NULL){
        int len = strlen(name_utf8);
        char* name = (char*)g_convert(name_utf8, len, "shift_JIS", 
                        "UTF-8", NULL, NULL, NULL);
        nameaction->set_string(name, 0);
    }
    
    for(int i=0; i<linecount; ++i){
        char* line;
        int len = strlen(lines_utf8[i]);
        line = (char*)g_convert(lines_utf8[i], len, "shift_JIS", "UTF-8",
                         NULL, NULL, NULL);
        line_actions[i]->set_string(line,0);
    }
}

int text_entity::add_line()
{
    gchar* newline = NULL;
    action* newaction = new action;
    
    newaction->init_op(0, 0xd2, 1);
    
    lines_utf8.push_back(newline);
    line_actions.push_back(newaction);
    actions->insert(end,newaction);
    
    linecount += 1;
    return linecount;
}

void text_entity::remove(int n)
{
    printf("called with n= %d\n", n);
    if(linecount == 1 || n>=linecount || n<0)
        return;
    vector<action*>::iterator lineactit = line_actions.begin();
    vector<gchar* >::iterator linesit   = lines_utf8.begin();
    action* to_remove = NULL;
    
    for(int i=0; i<n; ++i){
        lineactit++;
        linesit++;
    }
    to_remove = *lineactit;    
    delete to_remove;
    
    line_actions.erase(lineactit);
    lines_utf8.erase(linesit);
    linecount -= 1;
    
    actions->remove(to_remove);
}

void text_entity::write_plaintext_to(FILE* f)
{
    if(name_utf8!=NULL){
        fprintf(f,"said by: %s\n",name_utf8);
    }
    for(int i=0; i<linecount; ++i){
        fprintf(f, "%s\n", lines_utf8[i]);
    }
}

