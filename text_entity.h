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
    text_entity();
    ~text_entity();
    
    action_it set(action_it it, list<action*> *actions);
    
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
