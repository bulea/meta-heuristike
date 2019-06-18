#include <iostream>
#include "lpp.h"
#include <fstream>
#include <exception>
#include <algorithm>
#include <random>

using namespace std;
using namespace std::chrono;

Graf::Graf(int V_,int E_){
	V=V_;
	E=E_;
}

double Graf::find_in_graf(int first,int second){
	//trazimo brid (first,second) u grafu t.d prolazimo po edges i gledamo nalazi li se unutra
    pair<int,int> brid(first,second);
    if(edges.find(brid) != edges.end()) return edges[brid];
    brid.first=second; brid.second=first;
    if(edges.find(brid) != edges.end()) return edges[brid];
    return 0;	
}

void Graf::ucitajgraf(string name){
	int vrh, brid, prvi, drugi;
	double tezina;
	ifstream dat (name);
	if (!dat.is_open())
	{
		cout << "Graf nije otvoren!" << endl;
		throw std::exception();
	}
	dat >> vrh >> brid;
	V=vrh;E=brid;//Graf G(vrh, brid);
	
	pair<int, int> P;
	list<int> L;
	for(int i = 0; i < E; ++i)
	{
		dat >> prvi >> drugi >> tezina;
		P = make_pair(prvi, drugi);
		edges[P] = tezina;

		std::map<int,list<int>>::iterator it;
		it = vertices.find(prvi);
		if(it != vertices.end())
			(vertices[prvi]).push_back(drugi);
		else
		{
			L.clear();
			L.push_back(drugi);
			vertices[prvi] = L;
		}
        
        it = vertices.find(drugi);
		if(it != vertices.end())
			(vertices[drugi]).push_back(prvi);
		else
		{
			L.clear();
			L.push_back(prvi);
			vertices[drugi] = L;
		}
	}
	dat.close();
}

list<int> Graf::find_neighbours(int n){
	list<int> neighbours;
	
	for(list<int>::iterator li=vertices[n].begin(); li != vertices[n].end(); ++li)
	    neighbours.push_back(*li);
	return neighbours;
}

int Graf::number_of_neighbours(int n){
    return vertices[n].size();
}


Algoritam::Algoritam(Graf *graff,int popsize,double mutated){
	
	graf=graff;
	population_size=popsize;
	best=0;
	mutation_rate=mutated;
	best_path=0;
	type=0;
	edge_first_node=0;
	edge_second_node=0;
}

void Algoritam::initialpopulation(){
	for(size_t d=0;d<population_size;++d){
		list<int> individual;
		//random biramo prvi vrh u jedinki
		auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
		mt19937 generator (seed); 
		uniform_int_distribution<int> dis(1,graf->V);
		int n = dis(generator);
		individual.push_back(n);//individual predstavlja jedinku,stavljamo prvi vrh u jedinku
		int first=n;
		int times=0;//sluzi nam da bi kad dodemo do kraja (dosli smo do vrha koji nema susjeda 
		//osim onog iz kojeg smo dosli u njega) isli na drugu stranu
		while(1){
			
            list<int> neighbours = graf->find_neighbours(n);//neighbours=susjedi vrha n
			vector<int> nb;
			int flag1=0,t=0;
			list<int>::iterator it=neighbours.begin();
			while(it!=neighbours.end()){
				
				//prolazimo kroz jedinku i gledamo je li neki vrh vec posjecen
				//trazimo posjecene medu susjedima od n
				
				list<int>::iterator li=individual.begin();
				while(li!=individual.end()){
					if(*li==*it){
						flag1=1;//ako je posjecen, tj. nalazi se vec u jedinki, onda stavljamo flag na 1
						break;
					}
					++li;
				}
				
				if(flag1==1){
					flag1=0;
					neighbours.erase(it++);//brisemo posjecene vrhove iz liste susjeda, ne uzimamo ih u obzir kao one koji bi mogli
					//biti sljedeci vrh u jedinki, simple graph = vrhovi se ne smiju ponavljati
				}
				else{
					nb.push_back(graf->number_of_neighbours(*it));//ako se taj vrh vec ne nalazi u individual-u, stavljamo u polje nb broj susjeda od *it
					++it;
				}
			}
			
			if(neighbours.size()==0 && times==0){
				//dosli smo do vrha s jednim susjedom
				n=first;//idemo natrag na prvog graditi put na drugu stranu ako je to moguce
				times=1;
				continue;
			}
			if(neighbours.size()==0 && times==1) break;//prosli i drugu stranu, gotovo je
			
			//roulette wheel selection cvorova, biramo onaj cvor koji ce biti sljedeci u individual-u	
			
			int sum=0;
			for(size_t i=0;i< nb.size();++i) sum+=nb[i];
			
			for(size_t i=1;i< nb.size();++i) nb[i]=nb[i]+nb[i-1];
			
			auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
			mt19937 generator (seed);
			uniform_int_distribution<int> dis(1,sum);
			int r = dis(generator);

			int flag=0;//roulette wheel 
			for(size_t i=1;i<nb.size();++i){
				if(nb[i]>=r && r>nb[i-1]){
					int cnt=0;
					list<int>::iterator itt=neighbours.begin();
					while(itt!=neighbours.end()){
						if(cnt==i){
							n=*itt;//izabrali n , sljedeci vrh
							if(times==0) individual.push_back(n);
							else individual.push_front(n);
							flag=1;
							break;
						}
						++itt;
						++cnt;
					}
					break;
				}
			}
			if(flag==0){
				list<int>::iterator id=neighbours.begin();
				n=*id;//izabrali n , sljedeci vrh
				if(times==0) individual.push_back(n);
				else individual.push_front(n);
			}
		}
		//provjera duplikata
		if(population.find(individual)!=population.end()){
			--d;//moramo izbaciti tu jedinku,tj ne stavljamo je u populaciju
		}
		else{
			list<int> reverse;
			list<int>::iterator tr=individual.begin();
			while(tr!=individual.end()){
				reverse.push_front(*tr);
				++tr;
			}
			if(population.find(reverse)!=population.end()){
				--d;//moramo izbaciti tu jedinku,tj ne stavljamo je u populaciju jer obrnuta vec postoji, graf nije usmjeren pa su takvi putevi jednaki
			}
		    else population[individual]=0;//dodajemu jedinku u populaciju
		}
	}
}

void Algoritam::print_population(){
	map<list<int>,double>::iterator it=population.begin();
	while(it!=population.end()){
		list<int> lista=it->first;
		list<int>::iterator li=lista.begin();
		while(li!=lista.end()){
			cout<<*li<<"-";
			++li;
		}
		cout<<"--"<<it->second<<endl;
		++it;
	}
}

double Algoritam::assess_fitness(){
	//svim jedinakama u populaciji izracunavamo duzinu i vracamo duzinu trenutacno najduzeg
	double best=0.0;
	for(map<list<int>,double>::iterator it=population.begin();it!=population.end();++it){
		list<int> lista=it->first;
		list<int>::iterator li=lista.begin();
		int previous=*li;
		++li;
		double sum = 0.0;
		for(;li!=lista.end();++li){
			sum=sum+graf->find_in_graf(previous,*li);//vraca duljinu brida;
			previous=*li;
		}
		population[lista]=sum;
		//provjera je li lista najduzi put u generaciji i ukupno
		if(sum>best) best=sum;
		if(sum>best_path){
			//ako je ukupno najduzi put spremamo ga
			best_path=sum;
			best_individual=lista;
		}
	}
	return best;
}

pair<list<int>,list<int>> Algoritam::random_parents(){
	
	//izabiru se dvije jedinke koje ce biti roditelji u krizanju
	
	pair<list<int>,list<int>> return_parents;//izabrani roditelji
	vector<double> value;//sluzi da bi spremili prviremeno duzine puteva roditelja
	for(int i=0;i<2;++i){
		//roulette wheel selection
		map<list<int>,double>::iterator itp=population.begin();
		double sum=0.0;
		while(itp!=population.end()){
			double value=itp->second;
			sum+=value;
			++itp;
		}
		itp=population.begin();
		double preth=0.0;
		while(itp!=population.end()){
			list<int> lista=itp->first;
			double value=itp->second;
			double prop=preth+value;
			proportion[lista]=prop;
			++itp;
			preth=prop;
		}	
		//generira se random broj n
		auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
		mt19937 generator (seed);
		uniform_real_distribution<double> dis(0.0,sum);
		double n = dis(generator);
		
		map<list<int>,double>::iterator it=proportion.begin();
		double prev=it->second;
		++it;
		int nadeno=0,cnt1=1;
		// random generirarni n i gledamo kamo upadne
	
		while(it!=proportion.end()){
			if(it->second >= n && n > prev){
				int cnt2=0;
				map<list<int>,double>::iterator pi=population.begin();
				while(pi!=population.end()){
					if(cnt1==cnt2){
						list<int> lis=pi->first;
						double val=pi->second;
						if(i==1) return_parents.first=lis;//kad smo ga nasli stavljamo u return_parents
						else return_parents.second=lis;
						value.push_back(val);
						population.erase(pi);
						break;
					}
					++pi;
					++cnt2;
				}
				proportion.erase(it);
				nadeno=1;
				break;
			}
			prev=it->second;
			++it;
			++cnt1;
		}//ako je prvi:
		if(nadeno==0){
			map<list<int>,double>::iterator itit=population.begin();
			list<int> listt=itit->first;
			if(i==1) return_parents.first=listt;
			else return_parents.second=listt;
			double val=itit->second;
			value.push_back(val);
			population.erase(itit);
			it=proportion.begin();
			proportion.erase(it);
		}
	}
	population[return_parents.first]=value[0];
	population[return_parents.second]=value[1];
	proportion.clear();
	return return_parents;

}

int Algoritam::check_crossovers(pair<list<int>,list<int>> parents){
	
	//provjeravamo mogu li se jedinke izabrane u random_parents krizati
	
	list<int> parent1=parents.first;
	list<int> parent2=parents.second;
	int lose=0;
	int ok=0;
	list<int>::iterator li=parent1.begin();//provjeravamo za svake dvije jedinke medusobno zadovoljavaju li uvjete
	list<int>::iterator mi=parent2.begin();
	
	//zadovoljava li se 1. tip krizanja
	
	while(li!=parent1.end()){
		mi=parent2.begin();
		while(mi!=parent2.end()){ // treba provjeriti da li nije u parent2 i mi u parent 1, i to provesti do kraja za sve vrhove, i onda izabrati jedan takav brid
			if(graf->find_in_graf(*li,*mi)>0){//jos provjerava imaju li zajednicke bridove i imaju li dva susjedna vrha
				//nasli smo brid koji zadovoljava 1., treba jos provjeriti ovo gore
				list<int>::iterator lt=parent1.begin();
				while(lt!=parent1.end()){
					if(*mi==*lt) lose=1;//nasli smo vrh koji se nalazi u prvoj jedinki, brid je u prvoj jedinki
						++lt;
					}
				if(lose==0){//inace
					list<int>::iterator mt=parent2.begin();
					while(mt!=parent2.end()){
						if(*mt==*li) lose=1;
							++mt;
						}
					if(lose==0){
						ok=1;
					}
				}
				if(lose==1){
					ok=0;
				}
			}
			++mi;
		}
		++li;
	}
	if(ok==1){
		//nasli smo roditelje - prekidamo s trazenjem
		type=1;
		return 1;
	}
	/*2.slucaj
	*gledamo da moze biti i vise takvih bridova; provjeravamo po bridovima u 1.,taj brid ne smije biti prvi ili posljednji u obje jedinke
	*i mora se pojavljivati u drugoj, mogli bi jos ne gledati bridove za koji su jedinke 
	*od pocetka ili kraja do tog brida jednake, npr 2-4-6-7-[8-10]-12-13 i 2-4-6-7-[8-10]-14-15; ako su bridovi u nekom vecem nizu
	*bridova koji su jednaki u obje jedinke, npr 2-4-6-7-[8-10-17-18]-12-13 i 1-5-3-9-[8-10-17-18]-14-15 cijeli taj komad moraju
	*imati oba djeteta
	*/ 
	int pon=0;
	li=parent1.begin();//provjeravamo za svake dvije jedinke medusobno zadovoljavaju li uvjete
	mi=parent2.begin();
	list<int>::iterator lip=parent1.begin();// sluze za provjeru je li to prvi vrh
	list<int>::iterator mip=parent2.begin();
	list<int>::iterator last_li,last_mi,prev_li,prev_mi,next_li,next_mi;//last_li posljednji vrh u parent1
	while(li!=parent1.end()){
		last_li=li;
		++li;
	}
	while(mi!=parent2.end()){
		last_mi=mi;
		++mi;
	}
	li=parent1.begin();
	prev_li=li;
	++li;
	
	/////
	/////
	int con=0,con2=0;
	vector<pair<int,int>> re_edg;//tu stavljamo bridove koji se nalaze u obje jedinke, takve trazimo, ali ne smiju biti na pocetku i kraju jedinki

	while(li!=parent1.end()){
		mi=parent2.begin();
		prev_mi=mi;
		++mi;
		while(mi!=parent2.end()){ 
				
				if((*li==*mi && *prev_li==*prev_mi) || (*li==*prev_mi && *prev_li==*mi)){//provjera nalazi li se brid u obje jedinke
						
						if(prev_li==lip || *prev_li==con){//brid ne smije biti na pocetku 1. jedinke ili u nastavku pocetka
							pon=2;
							con=*li;
							break;
						}
						if(prev_mi==mip || mi==mip || *prev_mi==con2){//isto to za drugu
							pon=2;
							con2=*mi;
							break;
						}
						
						list<int>::iterator pom=li;
						list<int>::iterator pom2=mi;
						
						if(*li==*mi && *prev_li==*prev_mi){
							int flag2=0;
							while(pom!=parent1.end() && pom2!=parent2.end()){//brid ne smije biti na kraju prvog
								if(*pom!=*pom2){
									flag2=1;
									break;
								}
								++pom;
								++pom2;
							}
							if(flag2==0){
								pon=2;
								break;
							}
							
						}
						else{
							int flag2=0;
							--pom2;
							while(pom!=parent1.end() && pom2!=parent2.begin()){//brid ne smije biti na kraju prvog
								if(*pom!=*pom2){
									flag2=1;
									break;
								}
								++pom;
								--pom2;
							}
							if(flag2==0){
								pon=2;
								break;
							}
						}
						
						pom=li;
						pom2=mi;
						if(*li==*prev_mi && *prev_li==*mi){
							--pom;
							int flag2=0;
							while(pom!=parent1.begin() && pom2!=parent2.end()){//brid ne smije biti na kraju drugog
								if(*pom!=*pom2){
									flag2=1;
									break;
								}
								--pom;
								++pom2;
							}
							if(pom==parent1.begin() && pom2!=parent2.end()){
								if(*pom!=*pom2){
									flag2=1;
									break;
								}
							}
							if(flag2==0){
								pon=2;
								break;
							}
							
						}
						
						//ako smo prosli sve uvjete, na takvom bridu se moze napraviti krizanje, stavljamo ga u re_edg iz kojega
						//cemo na slucajan nacin izabrati na kojem bridu ce se napraviti krizanje (zato jer ih moze biti vise takvih)
						
						pair<int,int> par;
						par.first=*prev_li;
						par.second=*li;
						re_edg.push_back(par);
					
				}
				prev_mi=mi;
				++mi;
			}
		
		prev_li=li;
		++li;
	}
	
	if(re_edg.size()!=0){
		//nasli smo roditelje prekidamo s trazenjem, postoji bar jedan takav brid
		auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
		mt19937 generator (seed);
		uniform_int_distribution<int> dis(0,re_edg.size()-1);
		int n = dis(generator);
		edge_first_node=re_edg[n].first;
		edge_second_node=re_edg[n].second;
		type=2;
		return 1;
	}
	
	// 3. slucaj
	pon=0;
	li=parent1.begin();//provjeravamo za svake dvije jedinke medusobno zadovoljavaju li uvjete
	mi=parent2.begin();
	lip=parent1.begin();// sluze za provjeru je li to prvi vrh
	mip=parent2.begin();
	while(li!=parent1.end()){
		last_li=li;
		++li;
	}
	while(mi!=parent2.end()){
		last_mi=mi;
		++mi;
	}
	li=parent1.begin();//gleda se samo jedan takav vrh koji je u oba i ne smije bit na pocetku ili na kraju
	while(li!=parent1.end()){
		mi=parent2.begin();
		while(mi!=parent2.end()){ 
			if(*li==*mi && mi!=last_mi && li!=last_li && li!=lip && mi!=mip){
				//nasli smo vrh koji zadovoljava 3.,cim nademo takav vrh on mora biti zajednicki
				if(pon==0){pon=1; break;}
				else{ pon=2; break;}
			}
			if(*li==*mi && (mi==last_mi || li==last_li || li==lip || mi==mip)){
				pon=2; break;
			}
			++mi;	
		}
		if(pon==2) break;
		++li;
	}
	if(pon==1){
		type=3;
		return 1;
	}
	return 0;
}

pair<list<int>,list<int>> Algoritam::select_parents(){
	
	//biramo roditelje koji ce ici u krizanje, moguce na tri nacina:(prioritet ima 1,pa 2 pa 3, ako nije zadovoljen 1 tek onda mozemo na 2)
	//1. biramo dvije jedinke koji nemaju nista zajednicku a izmedu jednog brida iz 1. i jednog iz 2. postoji brid
	//2. biramo dvije jedinke koje imaju zajednicki brid
	//3. biramo dvije jedinke koje imaju zajednicki vrh
		
		pair<list<int>,list<int>> parents_for_crossover;
		pair<list<int>,list<int>> parents=random_parents();//vrati dvije jedinke koje moramo provjeriti mogu li se krizati
		int flag=check_crossovers(parents);//vrati 1 ako se mogu krizati, 0 ako ne; provjeravamo mogu li se križati
		if(flag==1){
			return parents;
			//break;
		}
	
	return parents_for_crossover;//vraca prazan par ako ih ne nade
}

map<list<int>,double> Algoritam::crossover(list<int> parent_first,list<int> parent_second){
	
	//krizanje, u check_crossovers type je postavljen koji tip krizanja imamo, pretpostavljamo da ce se uci u ovu funkciju ako se mogu krizati
	
	map<list<int>,double> children;
	list<int>::iterator li=parent_first.begin();
	list<int>::iterator mi=parent_second.begin();

	int mark1,mark2,flag=0;

	if(type==1){
	//1. slucaj
	while(li!=parent_first.end()){//1. slucaj
		mi=parent_second.begin();
		while(mi!=parent_second.end()){
			if(graf->find_in_graf(*li,*mi)>0){
				mark1=*li;
				mark2=*mi;
				flag=1;
				break;
			}
			++mi;
		}
		if(flag==1) break;
		++li;
	}
	li=parent_first.begin();
	mi=parent_second.begin();
	list<int> child1;
	list<int> child2;
	list<int> child3;
	list<int> child4;
	while(*li!=mark1){
		child1.push_back(*li);
		child2.push_back(*li);
		++li;
	}
	child1.push_back(mark1);
	child2.push_back(mark1);
	child2.push_back(mark2);
	while(*mi!=mark2){
		child3.push_back(*mi);
		++mi;
	}
	list<int>::iterator ti=mi;
	while(ti!=parent_second.begin()){
		child1.push_back(*ti);
		--ti;
	}
	child1.push_back(*ti);
	++mi;
	++li;
	child3.push_back(mark2);
	child3.push_back(mark1);
	while(mi!=parent_second.end()){
		child2.push_back(*mi);
		child4.push_front(*mi);
		++mi;
	}
	child4.push_back(mark2);
	child4.push_back(mark1);
	while(li!=parent_first.end()){
		child3.push_back(*li);
		child4.push_back(*li);
		++li;
	}
	children[child1]=0;
	children[child2]=0;
	children[child3]=0;
	children[child4]=0;
	

	return children;
	}
	//2. slucaj
	else if(type==2){
		
		mark1=edge_first_node;
		mark2=edge_second_node;
		li=parent_first.begin();
		mi=parent_second.begin();
		list<int>::iterator last_mi;
		while(mi!=parent_second.end()){
			last_mi=mi;
			++mi;
		}
		mi=parent_second.begin();
		list<int> child1;
		list<int> child2;
		while(*li!=mark1 && *li!=mark2){
			child1.push_back(*li);
			++li;
		}

		while(*mi!=mark2 && *mi!=mark1){
			child2.push_back(*mi);
			++mi;
		}

		int value1,value2,value3,value4;
		value1=*li;
		child1.push_back(*li);
		++li;
		value2=*li;
		child1.push_back(*li);
		++li;//li je sada na sljedecom vrhu nakon tog brida koji gledamo
		value3=*mi;
		child2.push_back(*mi);
		++mi;
		value4=*mi;
		child2.push_back(*mi);
		++mi;//mi je sada na sljedecom vrhu nakon tog brida koji gledamo
		
		if(value1==value3 && value2==value4){
			
			while(*li==*mi){
				child1.push_back(*li);
				child2.push_back(*mi);
				++li;
				++mi;
			}

			int endins=0;
			list<int>::iterator pr=child2.begin();
			list<int>::iterator pr1=child1.begin();
			while(mi!=parent_second.end()){
				pr=child1.begin();
				while(pr!=child1.end()){
					if(*mi==*pr){
						endins=1;
						break;
					}
					++pr;
				}
				if(endins==1){ endins=0;break;}
				child1.push_back(*mi);
				++mi;
			}
			endins=0;
			while(li!=parent_first.end()){
				pr1=child2.begin();
				while(pr1!=child2.end()){
					if(*li==*pr1){
						endins=1;
						break;
					}
					++pr1;
				}
				if(endins==1){ endins=0;break;}
				child2.push_back(*li);
				++li;
			}
			children[child1]=0;
			if(children.find(child2)==children.end()) children[child2]=0;
		}
		else{//stvara se 1 dijete na suprotnu stranu
		--li;
		list<int>::iterator it2=last_mi;
		list<int> child_temp;
		while(it2!=parent_second.begin()){
			if(*it2==*li) break;//li je na prvom u bridu od parent2
			child_temp.push_back(*it2);
			--it2;
		}
		child2.clear();
		list<int>::iterator it4=child_temp.begin();
		while(it4!=child_temp.end()){
			child2.push_back(*it4);
			++it4;
		}
		++li;
		child2.push_back(*it2);
		--it2;
		while(*li==*it2){
				child1.push_back(*li);
				child2.push_back(*li);
				++li;
				--it2;
			}

			int endins=0;
			list<int>::iterator it3=li;
			list<int>::iterator pr=child2.begin();
			while(it3!=parent_first.end()){
				pr=child2.begin();
				while(pr!=child2.end()){
					if(*it3==*pr){
						endins=1;
						break;
					}
					++pr;
				}
				if(endins==1){ endins=0;break;}
				child2.push_back(*it3);
				
				++it3;
			}
			list<int>::iterator pr1=child1.begin();
			endins=0;
			while(it2!=parent_second.begin()){
				pr1=child1.begin();
				while(pr1!=child1.end()){
					if(*it2==*pr1){
						endins=1;
						break;
					}
					++pr1;
				}
				if(endins==1){ break;}
				child1.push_back(*it2);
				--it2;
			}
			if(endins==0){
				pr1=child1.begin();
				while(pr1!=child1.end()){
					if(*it2==*pr1){
						endins=1;
						break;
					}
					++pr1;
				}
				if(endins==0) child1.push_back(*it2);
			}
			children[child1]=0;//ako je drugi kopija toga ne stavljamo ga u djecu
			if(children.find(child2)==children.end()) children[child2]=0;
		}

		return children;
		
	}
	else{
		//3. slucaj
		while(li!=parent_first.end()){
			mi=parent_second.begin();
			while(mi!=parent_second.end()){
				if(*li==*mi){//trazimo dva vrha koja su jednaka, iz select parents dobijemo jedinke koje imaju samo jedan takav vrh
					mark1=*li;
					mark2=*mi;
					flag=1;
					break;
				}
				++mi;
			}
			if(flag==1) break;
			++li;
		}
		li=parent_first.begin();
		mi=parent_second.begin();
		list<int> child1;
		list<int> child2;
		list<int> child3;
		list<int> child4;
		while(*li!=mark1){
			child1.push_back(*li);
			child2.push_back(*li);
			++li;
		}

		while(*mi!=mark1){
			child3.push_back(*mi);
			++mi;
		}
		child3.push_back(mark1);
		list<int>::iterator ti=mi;
		while(ti!=parent_second.begin()){
			child1.push_back(*ti);
			--ti;
		}
		child1.push_back(*ti);

		while(mi!=parent_second.end()){
			child2.push_back(*mi);
			child4.push_front(*mi);
			++mi;
		}

		++li;
		while(li!=parent_first.end()){
			child3.push_back(*li);
			child4.push_back(*li);
			++li;
		}
		children[child1]=0;
		children[child2]=0;
		children[child3]=0;
		children[child4]=0;

		return children;
	}
}

void Algoritam::mutate(){

	auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
	mt19937 generator (seed); 
	uniform_real_distribution<double> dis(0.0,100.0);
	double mut = dis(generator);
	map<list<int>,double> new_children;
	//ako je random generirani broj mut manji od mutation_rate onda ulazimo u mutaciju,inace ne
	if(mut<=mutation_rate){
		//trebamo izabrati vrh u djetetu na kojem cemo napraviti mutaciju,to trebamo samo na vrhovima koji imaju stupanj >=3
		//i biramo random vrh u kojem ce se mutirati
		map<list<int>,double>::iterator ittd=children.begin();
		while (ittd != children.end()){//to radimo za svako dijete
			list<int> child1 = ittd -> first;
			list<int>::iterator di = child1.begin();
			vector<int> nbe;
			while(di!=child1.end()){//trazimo vrhove koji su stupnja >= 3
				if(graf->number_of_neighbours(*di)>=3)
					nbe.push_back(*di);

				++di;
				
			}
			if(nbe.size()==0) continue;
			auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
			mt19937 generator (seed); 
			uniform_int_distribution<int> dis(0,nbe.size()-1);
			int mutt = dis(generator);//random biramo koji vrh cemo mutirati
			int mutated_edge=nbe[mutt];//vrh koji cemo mutirati
			di = child1.begin();
			while(*di!=mutated_edge) ++di;
			list<int>::iterator di2=di;
			++di2;
			int next_edge=*di2;//vrh nakon mutiranog, njega pamtimo jer ne zelimo da mutacija opet ide u njega

			int times=0;
			int n=mutated_edge;
			
			//gradimo novi put u jedinki
			while(1){
				
				list<int> neighbours = graf -> find_neighbours (n);//trazimo susjede tog vrha
				vector<int> nb;
				int flag1=0,t=0;
				list<int>::iterator it=neighbours.begin();
				while(it!=neighbours.end()){

					list<int>::iterator li=child1.begin();
					if(times==0){
						if(*it==next_edge){//ako je susjed sljedeci vrh u staroj jedinki, njega preskacemo, provjeravamo samo za prvi put
							neighbours.erase(it++);
							continue;
						}
					}
					while(li!=child1.end()){//trazimo posjecene medu susjedima od n
						if(*li==*it ){
							flag1=1;
							break;
						}
						++li;
					}
					if(flag1==1){
						flag1=0;
						neighbours.erase(it++);//brisemo posjecene vrhove iz liste susjeda
					}
					else{

						nb.push_back(graf->number_of_neighbours(*it));//vraca broj susjeda vrha u grafu
						++it;
					}
				}

				if(neighbours.size()==0 && times==1) break;//dosli do kraja, nemamo vise za dodavati vrhove u jedinku
				if(neighbours.size()==0 && times==0) break; // ako ne postoji vrh osim next_edge na kojeg mozemo ici ne provodimo mutaciju
				if(neighbours.size()>0 && times==0){ // inace izbrisemo sve vrhove nakon izabranog vrha za mutaciju
					while(di2!=child1.end()) child1.erase(di2++);
					times=1;
				
				}
				
				//roulette wheel selection cvorova
				int sum=0;
				for(size_t i=0;i< nb.size();++i) sum+=nb[i];
			
				for(size_t i=1;i< nb.size();++i) nb[i]=nb[i]+nb[i-1];
			
				auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
				mt19937 generator (seed);
				uniform_int_distribution<int> dis(1,sum);
				int r = dis(generator);

				int flag=0;
				for(size_t i=1;i<nb.size();++i){
					if(nb[i]>=r && r>nb[i-1]){
						int cnt=0;
						list<int>::iterator itt=neighbours.begin();
						while(itt!=neighbours.end()){
							if(cnt==i){
								n=*itt;
								child1.push_back(n);
								flag=1;
								break;
							}
							++itt;
							++cnt;
						}
						break;
					}
				}
				if(flag==0){
					list<int>::iterator id=neighbours.begin();
					n=*id;
					child1.push_back(n);
				}
			
			}
			new_children[child1]=0;
			++ittd;
		}
		children.clear();
		map<list<int>,double>::iterator ch=new_children.begin();
		while(ch!=new_children.end()){
			list<int> djete=ch->first;
			children[djete]=0;
			++ch;
		}
	}
}

void Algoritam::children_assess_fitness(){
	
	//kao assess_fitness samo za djecu
	
	for(map<list<int>,double>::iterator it=children.begin();it!=children.end();++it){
		list<int> lista=it->first;
		list<int>::iterator li=lista.begin();
		int previous=*li;
		++li;
		double sum = 0.0;
		for(;li!=lista.end();++li){
			sum=sum+graf->find_in_graf(previous,*li);//vraca duljinu brida;
			previous=*li;
		}
		children[lista]=sum;
	}
}

//Roulette wheel selection jedinki koje ostaju u populaciji
double Algoritam::fitness_proportion(map<list<int>,double>  new_children){

	map<list<int>,double>::iterator it=new_children.begin();
	double sum=0.0;
	while(it!=new_children.end()){
		double value=it->second;
		sum+=value;
		++it;
	}
	it=new_children.begin();
	double preth=0.0;
	while(it!=new_children.end()){
		list<int> lista=it->first;
		double value=it->second;
		double prop=preth+value;
		proportion[lista]=prop;
		++it;
		preth=prop;
	}
	return sum;
}

void Algoritam::newpopulation(pair<list<int>,list<int>> parents){
	
	map<list<int>,double> new_children;
	//dodajemo djecu u populacije,kasnije cemo izbaciti 4(ili 2) iz te vece populacije da bi dobili istu velicinu
	map<list<int>,double>::iterator it=children.begin();
	
	//provjera duplikata: (u populaciju stavljamo samo one koji se ne pojavljuju dosad)
	while(it!=children.end()){
		list<int> child1=it->first;
		if(population.find(child1)!=population.end()){
			++it;
		}
		else{
			list<int> reverse;
			list<int>::iterator tr=child1.begin();
			while(tr!=child1.end()){
				reverse.push_front(*tr);
				++tr;
			}
			if(population.find(reverse)!=population.end()){
				++it;
			}
		    else{
				new_children[child1]=it->second;
				++it;
			}
		}
	}

	if(new_children.empty()){//sva djeca su bila kopije
		children.clear();
		return;	
	}

	map<list<int>,double>::iterator mii=population.begin();
	list<int> parent1;
	list<int> parent2;
	while(mii!=population.end()){
		if(mii==population.find(parents.first)) parent1=mii->first;
		if(mii==population.find(parents.second)) parent2=mii->first;
		++mii;
	}

	new_children[parent1]=population[parent1];
	new_children[parent2]=population[parent2];// u new_children su sada djeca i roditelji
	children.clear();
	population.erase(parent1);
	population.erase(parent2);

	for(size_t i=0;i<2;++i){ 
		double sum;
		sum=fitness_proportion(new_children);//svima izracunamo vjerojatnost da ga izbacimo i sada trebamo izbaciti 4 ili 2 po toj vjerojatnosti

		auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
		mt19937 generator (seed);
		uniform_real_distribution<double> dis(0.0,sum);
		double n = dis(generator);
		map<list<int>,double>::iterator it=proportion.begin();
		double prev=it->second;
		++it;
		int nadeno=0,cnt1=1;
		// random generirarni n i gledamo kamo upadne
	
		while(it!=proportion.end()){
			if(it->second >= n && n > prev){
				int cnt2=0;
				map<list<int>,double>::iterator pi=new_children.begin();
				while(pi!=new_children.end()){
					if(cnt1==cnt2){
						list<int> lis=pi->first;
						children[lis]=0;//dodajemo lis u novu populaciju
						new_children.erase(pi);
						break;
					}
					++pi;
					++cnt2;
				}
				proportion.erase(it);
				nadeno=1;
				break;
			}
			prev=it->second;
			++it;
			++cnt1;
		}
		if(nadeno==0){
			map<list<int>,double>::iterator itit=new_children.begin();
			list<int> listt=itit->first;
			children[listt]=0;//dodajemo listt u novu populaciju
			it=proportion.begin();
			proportion.erase(it);
			it=new_children.begin();
			new_children.erase(itit);
		}

		proportion.clear();

	}

	map<list<int>,double>::iterator itc=children.begin();
	list<int> firstchild=itc->first;
	population[firstchild]=0;
	++itc;
	list<int> secondchild=itc->first;
	population[secondchild]=0;
	itc=children.begin();
	while(itc!=children.end()) children.erase(itc++);
}

void Algoritam::run(){
	
	initialpopulation();
	best=assess_fitness();
	int cnt=0,cnt2=0;

	while(1){ 
		pair<list<int>,list<int>> parents;
		parents=select_parents();//biramo dva roditelja koji ce sudjelovati u krizanju, trebat ce i graf staviti u fju
		list<int> parent_first=parents.first;//prvi roditelj
		list<int> parent_second=parents.second;//drugi roditelj
		if(!parents.first.empty()){//ovo se dogada ako ima krizanja
		children=crossover(parent_first,parent_second);//krizamo roditelje i dobivamo 4 ili 2 djece
		mutate();//mutiramo djecu
		children_assess_fitness();// racunamo vrijednosti duzine puta za djecu
		newpopulation(parents);//stvara se nova populacija 
		best=assess_fitness();
		++cnt2;
		}
		++cnt;
		if(cnt==1000) break;
	}
}
void Graf::ispisi_graf()
{
    cout << "broj vrhova: " << V << ", broj bridova: " << E << endl;
    for (std::map<pair<int,int>,double>::iterator it=edges.begin(); it!=edges.end(); ++it)
    std::cout << it->first.first << " " << it->first.second << " "<< it->second << '\n';
}
