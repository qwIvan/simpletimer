#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib.h>

GtkListBox* listbox;
GHashTable* table;
guint timeout_id;

typedef struct {
    GtkBuilder* row_builder;
    int tenths_elapsed;
    int is_running; // 1 is running. 0 is not.
    int is_countdown; // 0 is stopwatch. 1 is countdown.
    char* text;
} RowData;

void shutdown(GtkWindow* window, GdkEvent* event, gpointer user_data) {
    printf("Shutting down!\n");
    g_object_unref(listbox);
    gtk_main_quit();
}

void add_timer(GtkButton* button, gpointer should_be_countdown) {
    GtkBuilder* row_builder = gtk_builder_new_from_file("row.xml");

    GtkWidget* row = GTK_WIDGET(gtk_builder_get_object(row_builder, "row"));

    GtkWidget* label = GTK_WIDGET(gtk_builder_get_object(row_builder, "label"));
    gtk_widget_override_font(label, pango_font_description_from_string("Cantarell 18"));
    gtk_label_set_text(GTK_LABEL(label), "0:00 ");

    gtk_widget_show_all(row);

    RowData* row_data = malloc(sizeof(RowData));
    row_data->row_builder = row_builder;
    row_data->tenths_elapsed = 0;
    row_data->is_running = 1;
    //row_data->is_countdown = 0;
    row_data->is_countdown = *((int*) should_be_countdown);
    row_data->text = malloc(10 * sizeof(char));
    g_hash_table_insert(table, row, row_data);
    gtk_list_box_prepend(listbox, row);
}

void destroy_key(gpointer raw_row) {
}

void destroy_value(gpointer raw_row_data) {
}

void gen_label_text(int tenths, char* text) {
    int seconds = (tenths / 10) % 60;
    int minutes = (tenths / 10) / 60;
    if(seconds < 10) {
        sprintf(text, "%d:0%d ", minutes, seconds);
    } else {
        sprintf(text, "%d:%d ", minutes, seconds);
    }
}

void process_row(gpointer raw_row, gpointer raw_row_data, gpointer user_data) {
    GtkWidget* row = (GtkWidget*) raw_row;
    RowData* row_data = (RowData*) raw_row_data;

    if (row_data->is_running == 1) {
        if (row_data->is_countdown == 0) {
            row_data->tenths_elapsed += 1;
        } else {
            if (row_data->tenths_elapsed > 0) {
                row_data->tenths_elapsed -= 1;
            } else {
                return;
            }
        }
        gen_label_text(row_data->tenths_elapsed, row_data->text);
        gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(row_data->row_builder, "label")), row_data->text);
    }
}

int timeout_callback(gpointer user_data) {
    g_hash_table_foreach(table, process_row, NULL);
    return 1;
}

int main(int argc, char* argv[]) {
    int opt_stopwatch = 0;
    int opt_countdown = 1;
    gtk_init(&argc, &argv);

    GtkBuilder* builder = gtk_builder_new_from_file("main_window.xml");
    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    g_signal_connect(window, "delete-event", G_CALLBACK(shutdown), NULL);
    GtkWidget* add_stopwatch_button = GTK_WIDGET(gtk_builder_get_object(builder, "add_stopwatch_button"));
    g_signal_connect(add_stopwatch_button, "clicked", G_CALLBACK(add_timer), &opt_stopwatch);
    listbox = GTK_LIST_BOX(gtk_builder_get_object(builder, "list"));

    table = g_hash_table_new(NULL, NULL);

    timeout_id = g_timeout_add(100, timeout_callback, NULL);

    gtk_widget_show_all(window);
    
    g_object_unref(window);
    g_object_unref(add_stopwatch_button);

    gtk_main();
}
