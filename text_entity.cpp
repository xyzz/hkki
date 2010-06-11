
#include "text_entity.h"

text_entity::text_entity()
{
    name_utf8 = NULL;
    nameaction = NULL;
    linecount = 0;
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

action_it text_entity::set(action_it it, list<action*> *actions)
{
    this->actions = actions;
    uint32_t opcode = (*it)->get_opcode();
    old_addr = (*it)->get_old_addr();
    while(opcode == 0xd2 || opcode == 0xd4){
        
        if(opcode==0xd4){
            if(name_utf8==NULL){
                wchar_t* name;
                int len = (*it)->get_string_from_param(&name, 0);
                name_utf8 = g_convert((char*)name, len, "UTF-8",
                                      "shift_JIS", NULL, NULL, NULL);
                nameaction = *it;
            }
        }else{
            wchar_t* tmp;
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
        wchar_t* name = (wchar_t*)g_convert(name_utf8, len, "shift_JIS", 
                        "UTF-8", NULL, NULL, NULL);
        nameaction->set_string(name, 0);
    }
    
    for(int i=0; i<linecount; ++i){
        wchar_t* line;
        int len = strlen(lines_utf8[i]);
        line = (wchar_t*)g_convert(lines_utf8[i], len, "shift_JIS", "UTF-8",
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
