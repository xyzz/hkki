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
    
  public:
    stcm2l_file();
    ~stcm2l_file();
    void cleanup();
    
    int load_file(const char* infname, int start);
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
    
    void fun();
    
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
