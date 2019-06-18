#include <iostream>
#include <list>
#include <set>
#include <time.h> //treba mjeriti vrijeme
#include <utility>
#include <map>
#include <chrono>
#include "lpp.h"
#include <string>
#include <ctime>
#include <gtk/gtk.h>
#include <vector>

using namespace std;
using namespace std::chrono;

//kompajliranje: g++ -std=c++11 lpp.cpp lpp_main.cpp -o program `pkg-config --cflags --libs gtk+-2.0`

std::list<int> best_individual;
double best_path;

int popsize;
double mutationsize;
vector<string> imena={"broughton.txt", "Cumberland_2ndfloor.txt", "example.txt", "fire.txt", "forth_gridok.txt", "gates_first_floor.txt", "1r-5-map.txt", "hamilton_map.txt", "grelha4.txt", "isr_floor_0.txt", "isr_floor_1.txt", "largeexp3.txt", "Map.txt", "Maze.txt", "strongly_connected.txt" }; //imena datoteka koje mogu biti izabrane u padajućem izborniku
string name;
int poz=0;
GtkWidget *hscale;
GtkWidget *hscalepop;
GtkWidget *hscalemut;

/* za prvi prozor, kad je kliknut gumb ok spremi se ime trenutne datoteke */
void my_callback_function(GtkWidget *widget, gpointer   data)
{
    name = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (data)->entry));

    popsize=gtk_range_get_value (GTK_RANGE (hscalepop));
    mutationsize=gtk_range_get_value (GTK_RANGE (hscalemut));
    gtk_main_quit ();
}

/* za rezultat prozor, kad je kliknut gumb quit */
void quit_callback_function(GtkWidget *widget, gpointer   data)
{
    gtk_main_quit ();
}

/* kad je ugaseno sa X */
static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    gtk_main_quit ();
    return FALSE;
}

static GtkWidget *make_box(GtkWidget *window, gint o)
{
    GtkWidget *box;
    if(o==0) box = gtk_hbox_new (FALSE, 0);
    else box = gtk_vbox_new (FALSE, 0);

    /* Put the box into the main window. */
    gtk_container_add (GTK_CONTAINER (window), box);

    return box;
}

static void scale_set_default_values( GtkScale *scale, gint i)
{
    gtk_range_set_update_policy (GTK_RANGE (scale),
                                 GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (scale, i);
    gtk_scale_set_value_pos (scale, GTK_POS_TOP);
    gtk_scale_set_draw_value (scale, TRUE);
}

int prozor(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box;
    GtkWidget *box1;
    GtkWidget *box2;
    GtkWidget *combo;
    GtkWidget *label;
    GtkObject *adj1;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* naziv prozora */
    gtk_window_set_title (GTK_WINDOW (window), "Izbornik");

    /*kada je prozor ugasen s X pozvat će se funkcija delete_event*/
    g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);

    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);

    /*kreiranje vertikalne kutije u koju ce biti smjesteni ostali objekti*/
    box = gtk_vbox_new (FALSE, 0);

    /* box se smjesta u prozor */
    gtk_container_add (GTK_CONTAINER (window), box);

    /*kreiranje vertikalne kutije koja se smjesta u kutiju box pomocu pomocne funkcije*/
    box1=make_box(box,1);

    label = gtk_label_new ("Datoteke:");
    gtk_box_pack_start (GTK_BOX (box1), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    box2=make_box(box1,0);

    //combo - izbornik datoteka
    combo = gtk_combo_new();

    GList *glist = NULL;
    for(int i=0;i<15;i++)
    {
        glist = g_list_append (glist, (gpointer)imena[i].c_str());
    }

    gtk_combo_set_popdown_strings (GTK_COMBO (combo), glist);
    g_list_free(glist);

    gtk_box_pack_start (GTK_BOX(box2), combo, TRUE, TRUE, 0);

    gtk_widget_show (combo);

    gtk_widget_show (box2);

    //parametri
    box2=make_box(box1,0);

    //popsize
    label = gtk_label_new ("Velicina populacije (popsize):");
    gtk_box_pack_start (GTK_BOX (box1), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    adj1 = gtk_adjustment_new (0.0, 0.0, 300, 1.0, 1.0, 1.0);


    hscalepop = gtk_hscale_new (GTK_ADJUSTMENT (adj1));
    scale_set_default_values (GTK_SCALE (hscalepop),0);
    gtk_box_pack_start (GTK_BOX (box1), hscalepop, TRUE, TRUE, 0);
    gtk_widget_show (hscalepop);

    gtk_widget_show (box2);

    //mutationsizesize
    label = gtk_label_new ("Mutacija (mutationsize):");
    gtk_box_pack_start (GTK_BOX (box1), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    adj1 = gtk_adjustment_new (0.0, 0.0, 100, 1.0, 1.0, 1.0);


    hscalemut = gtk_hscale_new (GTK_ADJUSTMENT (adj1));
    scale_set_default_values (GTK_SCALE (hscalemut),2);
    gtk_box_pack_start (GTK_BOX (box1), hscalemut, TRUE, TRUE, 0);
    gtk_widget_show (hscalemut);

    gtk_widget_show (box2);

    gtk_widget_show (box1);

    //ok button
    button = gtk_button_new_with_label ("ok");

    g_signal_connect (button, "clicked", G_CALLBACK (my_callback_function), combo);

    gtk_box_pack_start (GTK_BOX(box), button, FALSE, TRUE, 0);

    gtk_widget_show (button);


    gtk_widget_show (box);

    gtk_widget_show_all(window);

    gtk_widget_show (window);

    gtk_main ();
    return 0;
}

int rezultat(int argc, char *argv[])
{
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *box;
    GtkWidget *box1;
    GtkWidget *box2;
    GtkWidget *combo;
    GtkWidget *label;
    GtkObject *adj1;

    gtk_init (&argc, &argv);

    string string_za_label;


    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* naziv prozora */
    gtk_window_set_title (GTK_WINDOW (window), "Rezultat");

    g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);

    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    gtk_window_set_default_size (GTK_WINDOW (window), 550, 550);

    box = gtk_vbox_new (FALSE, 0);

    gtk_container_add (GTK_CONTAINER (window), box);

    label = gtk_label_new (name.c_str());
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    string_za_label = "Mutacija: " + to_string(mutationsize);
    //buduci da funkcija to_string za double ima preciznost 6 decimala provjerava se jesu li zadnje znamenke 0 i izbacuju se
    while(string_za_label.back() == '0')
        string_za_label.pop_back();
    if(string_za_label.back() == ',') string_za_label.pop_back();
    label = gtk_label_new (string_za_label.c_str());
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    string_za_label = "Populacija: " + to_string(popsize);
    label = gtk_label_new (string_za_label.c_str());
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    label = gtk_label_new ("Najduzi pronadeni put:");
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    string_za_label="";
    list<int>::iterator it=best_individual.begin();
    int i=0;
	while(it!=best_individual.end()){
	    ++i;
		if(i%20 == 0) string_za_label+="\n";
		string_za_label+=to_string(*it)+"-";
		++it;
	}
	string_za_label.pop_back();

    label = gtk_label_new (string_za_label.c_str());
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    string_za_label = "Duljina puta: " + to_string(best_path);
    //buduci da funkcija to_string za double ima preciznost 6 decimala provjerava se jesu li zadnje znamenke 0 i izbacuju se
    while(string_za_label.back() == '0')
        string_za_label.pop_back();
    if(string_za_label.back() == ',') string_za_label.pop_back();
    label = gtk_label_new (string_za_label.c_str());
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_widget_show (label);

    //quit button
    button = gtk_button_new_with_label ("quit");

    g_signal_connect (button, "clicked", G_CALLBACK (quit_callback_function), NULL);

    gtk_box_pack_start (GTK_BOX(box), button, FALSE, TRUE, 0);

    gtk_widget_show (button);


    gtk_widget_show (box);

    gtk_widget_show_all(window);

    gtk_widget_show (window);

    gtk_main ();
    return 0;

}

//the steady-state genetic algorithm
int main(int   argc, char *argv[]){

	//struktura za graf
	Graf G;


	prozor(argc, argv);

	G.ucitajgraf(name);

	Algoritam alg(&G,popsize,mutationsize);

	clock_t begin=clock();
	alg.run();
	clock_t end=clock();

	double elapsed_secs=double(end-begin)/CLOCKS_PER_SEC;
	cout<<"vrijeme: "<<elapsed_secs<<endl;

	best_individual=alg.retbest_individual();
	best_path=alg.retbest_path();

	rezultat(argc, argv);

	return 0;
}
