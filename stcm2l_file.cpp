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

#include "stcm2l_file.h"
#include "compress.h"

int stcm2l_file::set_addresses(int startaddr)
{
    int cur_addr = startaddr;
    list<action*>::iterator it;
    for(it=actions.begin(); it!=actions.end(); ++it){
        (*it)->set_addr(cur_addr);
        cur_addr += (*it)->get_length();
    }
    return 0;
}
    
int stcm2l_file::get_actions_length(){
    int ret = 0;
    list<action*>::iterator it;
    for(it=actions.begin(); it!=actions.end(); ++it){
        ret += (*it)->get_length();
    }
    return ret;
}
    
int stcm2l_file::actions_write(FILE* of, int addr)
{
    int cur_addr = addr;
    
    set_addresses(addr);
    
    list<action*>::iterator it;
    for(it=actions.begin(); it!=actions.end(); ++it){
        cur_addr += (*it)->write_to(of);
    }
    return 0;
}

int stcm2l_file::exports_write(FILE *of)
{
    vector<export_data>::iterator it;
    for(it=exports.begin(); it!=exports.end(); ++it){
        (*it).write_to(of);
    }
    return 0;
}

int stcm2l_file::recover_global_calls(int startaddr)
{
    int cur_addr = startaddr;
    global_call_type::iterator gcit;
    vector<parameter*>::iterator paramit;
    list<action*>::iterator actit;
    
    for(actit=actions.begin(); actit!=actions.end(); ++actit){
        gcit = action::global_calls.find(cur_addr);
        if(gcit!=action::global_calls.end()){
            for(paramit=gcit->second.begin(); paramit!=gcit->second.end(); ++paramit){
                (*paramit)->globalp = *actit;
            }
        }
        cur_addr += (*actit)->get_length();
    }
    return 0;
}
    
int stcm2l_file::find_start()
{
    char tmp_buf[2000];
    fread(tmp_buf, 1, 2000, inf);
    
    for(int i=0; i<2000; i+=4){
        if(!strcmp(&tmp_buf[i], "CODE_START_") )
            return i+0x0c;
    }
    return 0;
}
    
stcm2l_file::stcm2l_file()
{
    start = 0;
    start_exports = 0;
    exportcount = 0;
    startdata = NULL;
    inflen = 0;
    current_text = 0;
    cur_line = -1;
    inf = NULL;
}

stcm2l_file::~stcm2l_file()
{
    cleanup();
}

void stcm2l_file::cleanup()
{
    delete startdata;
    
    list<action*>::iterator actit;
    for(actit = actions.begin(); actit!=actions.end(); ++actit){
        delete *actit;
    }
    
    vector<text_entity*>::iterator textit;
    for(textit=texts.begin(); textit!=texts.end(); ++textit){
        delete *textit;
    }
    exports.clear();
    texts.clear();
    actions.clear();

    if(inf!=NULL)
        fclose(inf);
        
    action::global_calls.clear();
        
    start = 0;
    start_exports = 0;
    exportcount = 0;
    startdata = NULL;
    inflen = 0;
    current_text = 0;
    cur_line = -1;
    inf=NULL;
}

int stcm2l_file::load_file(const char* infname)
{
    static char* directory = "/tmp";
    static char* prefix = "tmgs3dit";
    char* tmpfname;

    tmpfname = tempnam(directory, prefix);
    decompress((char*)infname, tmpfname);

    inf = fopen(tmpfname, "rb");
    if(inf==NULL){
        perror("inf:");
        return 1;
    }
    fseek(inf, 0, SEEK_END);
    inflen = ftell(inf);
    fseek(inf, 0, SEEK_SET);
    
    int st = find_start();
    fseek(inf, 0, SEEK_SET);
    if(st==0){
        fprintf(stderr,"Not a valid file\n");
        cleanup();
        return 1;
    }
    start = st;
    printf("Found start at 0x%X\n", start);
    return 0;
}

int stcm2l_file::read_start()
{
    startdata = new uint8_t [start];
    if(startdata == NULL){
        perror("new startdata");
        return 1;
    }
    fread(startdata, 1, start, inf);
    start_exports = *((int*)&startdata[ OFFSET_EXPORT_ADDR ]);
    return 0;
}

int stcm2l_file::read_exports()
{
    int export_len = inflen - start_exports;
    exportcount = export_len / SIZE_EXPORT;

    fseek(inf, start_exports, SEEK_SET);
    for(int i=0; i<exportcount; ++i){
        export_data ex;
        
        fseek(inf, 4, SEEK_CUR);
        fread(ex.name, 32, 1, inf);
        fread(&ex.old_addr, 4, 1, inf);
        
        exports.push_back(ex);
    }
    return exportcount;
}

int stcm2l_file::read_actions()
{
    int action_length;
    int cur_addr=start, max_addr = start_exports - 12;
    int cur_export = 0, exportcount = exports.size();
    fseek(inf, start, SEEK_SET);
    do{
        action* a = new action;

        action_length = a->read_from(inf);
        if(cur_export < exportcount && 
            exports[cur_export].old_addr == cur_addr){
                
            exports[cur_export].exported_action = a;
            cur_export += 1;
        }
            
        cur_addr += action_length;
        actions.push_back(a);
    }while(cur_addr < max_addr);
    
    recover_global_calls(start);
    
    printf("Have %d actions\n", actions.size());
    return 0;
}
    
int stcm2l_file::make_entities()
{
    list<action*>::iterator it;
    for(it=actions.begin(); it!=actions.end(); ++it){
        uint32_t opcode = (*it)->get_opcode();
        if( ( opcode == ACTION_NAME || opcode == ACTION_TEXT ) 
                && (*it)->has_extra_data()){
            text_entity* te = new text_entity;
            if(te==NULL){
                perror("new textentity\n");
            }
            it = te->set_convo(it, &actions);
            texts.push_back(te);
        }else if( opcode == ACTION_CHOICE ){
            text_entity* te = new text_entity;
            it = te->set_answer(it);
            texts.push_back(te);
        }
    }
    printf("got %d texts\n", texts.size());
    return texts.size();
}

int stcm2l_file::fill_liststore()
{
    vector<text_entity*>::iterator it;
    int bla=0;
    GtkTreeIter tree_it;
    
    for(it=texts.begin(); it!=texts.end(); ++it){
        gchar* line = (*it)->get_line_utf8(0);
        
        gtk_list_store_append(liststore, &tree_it);
        gtk_list_store_set(liststore, &tree_it,
                            0, bla++, 1, line, -1);
    }
    printf("filled: %d\n", bla);
    return bla;
}

void stcm2l_file::clear_liststore()
{
    gtk_list_store_clear(liststore);
}

void stcm2l_file::clear_editstore()
{
    gtk_list_store_clear(editstore);
}

void stcm2l_file::selected_text(int index)
{    
    current_text = index;
    cur_line = -1;
    gchar* namestr = texts[index]->get_name_utf8();
    if(namestr!=NULL)
        gtk_entry_set_text(namebox, namestr);
    else
        gtk_entry_set_text(namebox, "");
        
    GtkTreeIter tree_it;

    GtkTreeViewColumn* treecol = gtk_tree_view_get_column(edit_tree, 0);
    GList* cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(treecol));
    if(cells==NULL){
        perror("cells==NULL");
    }
    GtkCellRenderer* cell_rend = (GtkCellRenderer*)cells->data;
    g_list_free(cells);
    if(texts[index]->is_answer){
        g_object_set(G_OBJECT(cell_rend), "foreground", "blue", NULL);
    }else{
       g_object_set(G_OBJECT(cell_rend), "foreground", NULL, NULL);;
    }
    
    
    int linecount = texts[index]->get_linecount();
    
    clear_editstore();
    for(int i=0; i<linecount; ++i){
        int width;
        gchar* line = texts[index]->get_line_utf8(i);
        
        gtk_list_store_append(editstore, &tree_it);
        gtk_list_store_set(editstore, &tree_it, 0, line, -1);
    }
    
}

void stcm2l_file::refresh_editstore()
{
    clear_editstore();
    selected_text(current_text);
}

void stcm2l_file::refresh_liststore()
{
    clear_liststore();
    fill_liststore();
}

void stcm2l_file::set_textline(int index, gchar* new_line)
{
    texts[current_text]->set_line_utf8(index, new_line);
    clear_editstore();
    selected_text(current_text);
}

void stcm2l_file::sync_texts()
{
    vector<text_entity*>::iterator it;
    for(it=texts.begin(); it!=texts.end(); ++it){
        (*it)->reinsert_lines();
    }
}


int stcm2l_file::write(const char* ofname)
{
    int new_export_start;
    static const char* directory = "/tmp";
    static const char* prefix = "hkki";
    char* tmpfname;
    FILE* of;

    tmpfname = tempnam(directory, prefix);
    of = fopen(tmpfname, "wb");
    if(of==NULL){
        perror("opening output:");
        return 1;
    }
        
    new_export_start = start + get_actions_length() + 12;
    *((int*)&startdata[ OFFSET_EXPORT_ADDR ]) = new_export_start;
        
    fwrite(startdata, 1, start, of);
    actions_write(of, start);
    fwrite("EXPORT_DATA", 1, 12, of);
    exports_write(of);
        
    fclose(of);

    compress(tmpfname, (char*)ofname);
    return 0;
}
    
int stcm2l_file::export_plaintext(const char* ofname)
{
    vector<text_entity*>::iterator it;
    int i=0;
    FILE* of = fopen(ofname, "w");
    if(of==NULL){
        perror("exporting texts: ");
        return 1;
    }

    for(it=texts.begin(); it!=texts.end(); ++it){
        fprintf(of, "Box %d\n", ++i);
        (*it)->write_plaintext_to(of);
        fprintf(of,"\n");
    }
    
    fclose(of);
    return 0;
}
