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

#ifndef _STCM2L_FILE_H_
#define _STCM2L_FILE_H_

#include <stdio.h>
#include <stdint.h>
#include <list>
#include <vector>
#include <map>
#include <gtk/gtk.h>

#include "action.h"
#include "export_data.h"
#include "text_entity.h"

using namespace std;

class stcm2l_file
{
  private:
    uint8_t* startdata;
    list<action*> actions;
    vector<export_data> exports;
    vector<text_entity*> texts;
    GtkListStore* liststore, *editstore;
    GtkEntry* namebox;
    FILE* inf;
    int inflen, start, start_exports, exportcount, current_text;
    int cur_line;
    
    
    int set_addresses(int startaddr);
    
    int get_actions_length();
    
    int actions_write(FILE* of, int addr);
    
    int exports_write(FILE *of);
    
    int recover_global_calls(int startaddr);
    
    int find_start();
    
  public:
    stcm2l_file();
    ~stcm2l_file();
    void cleanup();
    
    int load_file(const char* infname);
    int read_start();
    int read_exports();
    int read_actions();
    
    int make_entities();
    int fill_liststore();
    void clear_liststore();
    void clear_editstore();
    void refresh_editstore();
    void refresh_liststore();
    
    void selected_text(int index);
    
    void set_textline(int index, gchar* new_line);
    
    void sync_texts();
    
    int write(const char* ofname);
    
    int is_open(){
        return inf!=NULL;
    }
    
    void set_liststore(GtkListStore* ls){
        liststore=ls;
    }
    void set_namebox(GtkEntry* nb){
        namebox=nb;
    }
    void set_editstore(GtkListStore* es){
        editstore=es;
    }
    void add_line(){
        texts[current_text]->add_line();
    }
    void set_cur_line(int n){
        cur_line = n;
    }
    void remove_line(){
        texts[current_text]->remove(cur_line);
    }
};

#endif /* _STCM2L_FILE_H_ */
