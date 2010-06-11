
#include <gtk/gtk.h>
#include "stcm2l_file.h"

extern "C" G_MODULE_EXPORT void
on_save_button_clicked(GtkButton*   button,
                       gpointer     user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    GtkWidget *dialog;
    
    if(!file->is_open())
        return;
    
    dialog = gtk_file_chooser_dialog_new ("Save File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        
        file->sync_texts();
        file->write(filename);
        g_free(filename);
    }
    gtk_widget_destroy (dialog);
}


extern "C" G_MODULE_EXPORT void
on_line_edited(GtkCellRendererText *renderer,
               gchar               *path,
               gchar               *new_text,
               gpointer             user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    int index = atoi(path);
    file->set_textline(index, new_text);
    file->refresh_liststore();
}

extern "C" G_MODULE_EXPORT void
on_text_list_row_activated(GtkTreeView       *tree_view,
                           GtkTreePath       *path,
                           GtkTreeViewColumn *column,
                           gpointer           user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    gint* indices = gtk_tree_path_get_indices(path);
    file->selected_text(indices[0]);
}


extern "C" G_MODULE_EXPORT void
on_open_button_clicked(GtkButton* button,
                       gpointer user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    GtkWidget *dialog;
    
    dialog = gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        
        if(file->is_open()){
            file->clear_editstore();
            file->clear_liststore();
            file->cleanup();
        }
        printf("opened %s\n", filename);
        file->load_file(filename, 0x21C);
        g_free (filename);
         
        file->read_start();
        file->read_exports();
        file->read_actions();
        
        file->make_entities();
        file->fill_liststore();
    }
    gtk_widget_destroy (dialog);
}

extern "C" G_MODULE_EXPORT void
on_treeview1_cursor_changed(GtkTreeView* treeview,
                           gpointer user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    if(!file->is_open())
        return;
    
    GtkTreePath* path;
    gtk_tree_view_get_cursor(treeview, &path, NULL);
    if(path==NULL)
        return;
    
    gint* indices = gtk_tree_path_get_indices(path);
    printf("line number: %d\n", indices[0]);    
    file->set_cur_line(indices[0]);
    gtk_tree_path_free(path);
}


extern "C" G_MODULE_EXPORT void
on_add_line_button_clicked(GtkButton* button,
                           gpointer user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    if(!file->is_open())
        return;
    file->add_line();
    file->refresh_editstore();
}

extern "C" G_MODULE_EXPORT void
on_remove_line_clicked(GtkButton* button,
                           gpointer user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    if(!file->is_open())
        return;
    
    file->remove_line();
    file->refresh_editstore();
}

extern "C" G_MODULE_EXPORT void
on_add_button_clicked(GtkButton* button,
                      gpointer user_data)
{
    stcm2l_file* file = (stcm2l_file*)user_data;
    file->clear_liststore();
}

int main(int argc, char** argv)
{
    GtkBuilder* builder;
    GtkWidget* window;
    GtkListStore* ls, *es;
    GtkEntry* nb;
    GError* error = NULL;
    static stcm2l_file file;
    
    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    
    if(!gtk_builder_add_from_file(builder, "hkki.glade", &error))
    {
        g_warning("%s", error->message);
        g_free(error);
        return 1;
    }
    
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    ls = (GtkListStore*)gtk_builder_get_object(builder, "liststore1");
    es = (GtkListStore*)gtk_builder_get_object(builder, "editstore");
    nb = (GtkEntry*)gtk_builder_get_object(builder, "name");
    file.set_liststore(ls);
    file.set_namebox(nb);
    file.set_editstore(es);
    
    gtk_builder_connect_signals(builder, &file);
    g_object_unref( G_OBJECT(builder) );
    gtk_widget_show(window);
    gtk_main();

    return 0;    
}
