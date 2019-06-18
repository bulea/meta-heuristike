#ifndef LPP_H
#define LPP_H

#include <stdio.h>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <stdlib.h>
#include <chrono>
//using namespace std;
class Graf{
	
	private:
	
	int V;
	int E;
	double find_in_graf(int first,int second);//vraca 0 ako nisu povezani bridom, inace duljinu
	std::map<std::pair<int,int>,double> edges;//pamti koja dva vrha su krajevi brida i njihovu duzinu
	std::map<int,std::list<int>> vertices;//pamti vrhove i sve njihove susjedne vrhove
	std::list<int> find_neighbours(int n);// pronalazi susjede cvora n
	int number_of_neighbours(int n);// vraca broj susjeda cvora n
	
	public:
	
	Graf(int V_,int E_);
	/**/Graf() {}
	void ucitajgraf(std::string name);
	
	friend class Algoritam;
	
	void ispisi_graf();
};

class Algoritam{
	
	private:
	Graf* graf;
	int population_size;
	double mutation_rate;
	double best;//duzina trenutno najduljeg, u toj generaciji
	double old_best;//duzina najduljeg u prethodnoj generaciji
	double best_path;//duzina ukupno najduljeg put 
	int type;//tip krizanja
	int edge_first_node;
	int edge_second_node;
	std::list<int> best_individual;//najdulja jedinka
	std::map<std::list<int>,double> proportion;//za roulette wheel selection
	std::map<std::list<int>,double> population;
	std::map<std::list<int>,double> children;
	void initialpopulation();
	double assess_fitness();
	std::pair<std::list<int>,std::list<int>> select_parents();//bira roditelje koji idu u krizanje
	std::pair<std::list<int>,std::list<int>> random_parents();
	int check_crossovers(std::pair<std::list<int>,std::list<int>> parents);
	std::map<std::list<int>,double> crossover(std::list<int> parent_first,std::list<int> parent_second);
	void mutate();
	void newpopulation(std::pair<std::list<int>,std::list<int>> parents);
	//void add_children();
	double fitness_proportion(std::map<std::list<int>,double>  new_children);
	void print_population();
	void children_assess_fitness();
	public:
	Algoritam(Graf *graff,int popsize,double mutated);
	void run();
	
	//funkcije vracaju privatne varijable best_individual i best_path da bi se ispisale u grafičkom sučelju
	std::list<int> retbest_individual() {return best_individual;}
	double retbest_path() {return best_path;}
};

#endif
