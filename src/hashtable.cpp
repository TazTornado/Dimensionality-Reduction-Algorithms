#include <iostream>
#include <string>
#include <climits>
#include <stdio.h>
#include <cmath>
#include <numeric>
#include <map>
#include <set>
#include <fstream>
#include "tree.hpp"
#include "item.hpp"
#include "error.hpp"
#include "hashtable.hpp"
#include "utilities.hpp"


HashTable::HashTable(int size, int k,int w) : size(size){
	int i;

	this->window = w;

	for(i = 0; i < k; i++){

		float random = uniform_random_generator(DEFAULT_WINDOW);
		this->t.push_back(random);

	}

	buckets = (treehead **)malloc(size * sizeof(treehead *));

	treehead ** alloc_buckets = (treehead **)buckets;

	for (i = 0; i < size; i++)
		alloc_buckets[i] = new treehead(NULL);
}



HashTable::~HashTable(){

	if(this->f == NULL){
		treehead ** delete_buckets = (treehead **)buckets;

		for(unsigned int i = 0; i < this->size; i++){
			delete(delete_buckets[i]);
		}

		free((treehead **)buckets);

	} else {
		map<string, vector<item *> *> *cube_buckets = (map<string, vector<item *> *> *) this->buckets;

		for(auto const &i : *cube_buckets){
			delete i.second;
		}
		delete cube_buckets;

		for(auto const &i : *(this->f))
			delete i;
		delete this->f;
	}

	for(long unsigned int i = 0; i < this->v.size(); i++){
		delete(v[i]);
	}
}



void HashTable::print_table(){
	treehead ** print_buckets = (treehead **)(this->buckets);

	for(unsigned int i = 0; i < this->size; i++){
		cout << "Items in bucket " << i << ":" << endl;
		cout << "-------------------" << endl;
		print_buckets[i]->print_tree();
		cout << endl << "-------------------------------------------------------------------" << endl;
	}
}

void HashTable::print_table_cube(){
	map<string,vector<item*> *> *  print_buckets = (map<string,vector<item*> *> *) (this->buckets);

	for(auto const &i : *print_buckets){
		cout << "Items in bucket " << i.first << ":" << endl;
		cout << "---------------------------------------------" << endl;

		for(auto const &j : *(i.second)){
			j->printitem();
		}

		cout << endl << "-------------------------------------------------------------------" << endl;
	}
}



int HashTable::H(item *p, int i){

	float p_dot_v = std::inner_product(p->get_elements()->begin(), p->get_elements()->end(), this->v[i]->get_elements()->begin(), 0);
	int result = floor((p_dot_v + this->t[i]) / this->window);

	return result;
}


int HashTable::ID(item *p, int k, std::vector<int> *r){

	int ID = 0;

	for (int i = 0; i < k; i++){

		int hi = euclidean_modulo(this->H(p, i), M);
		ID += euclidean_modulo(r->at(i) * hi, M);

	}

	return ID;
}


void HashTable::initialize_v(unsigned int v_length, int k){

	if(this->v.empty()){
		for (int i = 0; i < k; i++){
			std::vector<float> *vi = new std::vector<float>();

			for (unsigned int j = 0; j < v_length; j++){
				vi->push_back(gaussian_random_generator());
			}
			this->v.push_back(new item(v_length, vi, ""));
		}
	}
}

void HashTable::insert_element(item *p, int k, std::vector<int> *r){

	int id = this->ID(p, k, r);
	int bucket_index = euclidean_modulo(id, this->size);
	treehead ** temp  = (treehead **)buckets;
	temp[bucket_index]->add_to_idtree(id, p);
}


void HashTable::range_search_items_lsh(item * query,int k,std::vector<int> *r,std::set<item *> *return_set){
	int IDquery;
	IDquery = this->ID(query, k, r);
	int index = euclidean_modulo(IDquery,this->size);
	treehead ** lsh_buckets = (treehead **)buckets;
	std::list<item *> *all_in_bucket = new std::list<item *>();
	lsh_buckets[index]->get_bucket_elements(all_in_bucket); //all_in_bucket now contains all the elements in buckets
	for (auto const &i : *(all_in_bucket)){
		return_set->insert(i);
	}

}


void HashTable::range_search_lsh(item * query, int k, std::vector<int> *r, int ldistance,float radius,std::set<item *> * neighbors_in_range){ // ,std::set<item *> * neighbors_in_range
	int IDquery;
	IDquery = this->ID(query, k, r);
	int index = euclidean_modulo(IDquery,this->size);
	treehead ** lsh_buckets = (treehead **)buckets;
	std::list<item *> *all_in_bucket = new std::list<item *>();
	lsh_buckets[index]->get_bucket_elements(all_in_bucket); //all_in_bucket now contains all the elements in buckets


	for (auto const &i : *(all_in_bucket)){

		float current_dist = i->item_distance(ldistance, query);
		if(current_dist < radius)
			neighbors_in_range->insert(i);


	}
}


void HashTable::bucket_ready_range_search(item * q, int ldistance,float radius,std::set<item *> * in_range , std::list<item *> * bucket){
		for (auto const &i : *(bucket)){
			float current_dist = i->item_distance(ldistance, q);
			if(current_dist < radius)
				in_range->insert(i);
		}
}


void HashTable::execute_query(int N, int k, std::vector<int> *r, int ldistance, float radius, item * query, std::multimap <float,item*> * nearest_neighbors,std::set<std::string> * avoid_duplicates_in_nn ,std::set<item *> *neighbors_in_range, double *nn_time){

	int IDquery;
	IDquery = this->ID(query, k, r);
	int index = euclidean_modulo(IDquery,this->size);
	treehead ** lsh_buckets = (treehead **)buckets;
	std::list<item *> *sameid = lsh_buckets[index]->search_on_id(IDquery);



	clock_t nn_start, nn_end;

	std::list<item *> *all_in_bucket = new std::list<item *>();
	lsh_buckets[index]->get_bucket_elements(all_in_bucket);
	this->bucket_ready_range_search(query,ldistance,radius,neighbors_in_range,all_in_bucket);
	if(sameid == NULL){

		nn_start = clock();
		for (auto const &i : *(all_in_bucket)){


			float current_dist = i->item_distance(ldistance, query);


			if(sameid == NULL){


				if(nearest_neighbors->size() < (unsigned long int) N){
					nearest_neighbors->insert(std::pair<float, item *>(current_dist, i));
					avoid_duplicates_in_nn->insert(i->get_id());


				}
				else if (current_dist <= nearest_neighbors->rbegin()->first &&
						 avoid_duplicates_in_nn->find(i->get_id()) == avoid_duplicates_in_nn->end()){

					nearest_neighbors->insert(std::pair<float, item *>(current_dist, i));
					avoid_duplicates_in_nn->insert(i->get_id());

					avoid_duplicates_in_nn->erase(nearest_neighbors->rbegin()->second->get_id());
					nearest_neighbors->erase(nearest_neighbors->rbegin()->first);
				}
			}
		}

		nn_end = clock();
	}


	delete all_in_bucket;



	if(sameid != NULL){
		nn_start = clock();


		for (auto const &i : *(sameid)){


				float current_dist = i->item_distance(ldistance, query);

			if(nearest_neighbors->size() < (unsigned long int) N){


				nearest_neighbors->insert(std::pair<float, item *>(current_dist, i));
				avoid_duplicates_in_nn->insert(i->get_id());

			} else if (current_dist <= nearest_neighbors->rbegin()->first){

				if(avoid_duplicates_in_nn->find(i->get_id()) == avoid_duplicates_in_nn->end()){

					nearest_neighbors->insert(std::pair<float, item *>(current_dist, i));
					avoid_duplicates_in_nn->insert(i->get_id());

					avoid_duplicates_in_nn->erase(nearest_neighbors->rbegin()->second->get_id());
					nearest_neighbors->erase(nearest_neighbors->rbegin()->first);

				}
			}
		}

		nn_end = clock();
	}

	*nn_time = (double) (nn_end - nn_start) / CLOCKS_PER_SEC;
}

//////////////////////HYPERCUBE////////////////////////////

HashTable::HashTable(int size,int w){
	int i;

	this->window = w;
	this->size = size;
	for(i = 0; i < size; i++){


		this->t.push_back(uniform_random_generator(DEFAULT_WINDOW));
	}

	this->buckets = new std::map<std::string,std::vector<item*> *>();

	//each h has each own map
	this->f = new std::vector<std::map<int, unsigned char> *>();
	std::map<int, unsigned char> * temp;
	for (i = 0; i < size; i++){
		temp = new std::map<int, unsigned char>();
		f->push_back(temp);
	}
}




void HashTable::insert_element(item *p, int k){
	int hi;
	unsigned char hvalue;
	std::map<int, unsigned char>::iterator fvalue;
	std::string position_on_hypercube = "";
	std::map<std::string,std::vector<item*>*> * cube_buckets = (std::map<std::string,std::vector<item*>*> *)buckets;

	for(int i = 0;i < k; i++){
		hi = H(p,i);
		fvalue = (this->f->at(i))->find(hi);

		if(fvalue == (this->f->at(i))->end()){
			hvalue = rand()%2;
			(this->f->at(i))->insert(std::pair<int,unsigned char>(hi,hvalue));
			position_on_hypercube.push_back(hvalue + '0');
		}
		else{
			position_on_hypercube.push_back(fvalue->second + '0');
		}
	}
	//position on hypercube is complete
	std::map<std::string,std::vector<item*> *>::iterator same_position;

	same_position = cube_buckets->find(position_on_hypercube);
	if(same_position == cube_buckets->end()){
		std::vector<item *> * new_vector = new  std::vector<item *>();
		new_vector->push_back(p);
		cube_buckets->insert(std::pair<std::string,std::vector<item*> *>(position_on_hypercube,new_vector));

	}
	else
		same_position->second->push_back(p);
}


std::set<item *> * HashTable::range_search_items(item * query,int k, int m, int probes){
	srand(time(NULL));
	int limit_of_total_checks = m;
	int limit_of_probe_check = probes;
	std::set<std::string> checked_positions;
	std::map<std::string,std::vector<item*>*> * cube_buckets = (std::map<std::string,std::vector<item*>*> *)buckets;
	std::set<item *> * return_set = new std::set<item *>();

	int hi;
	unsigned char hvalue;
	std::map<int, unsigned char>::iterator fvalue;
	std::string position_on_hypercube = "";
	for(int i = 0;i < k; i++){
		hi = H(query,i);
		fvalue = (f->at(i))->find(hi);
		if(fvalue == (f->at(i))->end()){
			hvalue = rand()%2;
			(f->at(i))->insert(std::pair<int,unsigned char>(hi,hvalue));
			position_on_hypercube.push_back(hvalue + '0');
		}
		else{
			position_on_hypercube.push_back(fvalue->second + '0');
		}
	}

	std::map<std::string,std::vector<item*> *>::iterator same_position;

	same_position = cube_buckets->find(position_on_hypercube);

	if(same_position != cube_buckets->end()){
		std::vector<item*>::iterator temp;

		temp = same_position->second->begin();

		while(limit_of_total_checks > 0 && temp != same_position->second->end()){

			return_set->insert(*temp);
			temp++;
			limit_of_total_checks--;
		}
	}

	unsigned int random_neighbor;
	std::map<std::string,std::vector<item*> *>::iterator next_position;
	std::string next_neighbor;
	std::set<std::string> already_checked;
	std::set<std::string>::iterator not_checked;
	int hamming_distance = 1;
	int checked_neighbors = 1;
	std::set<unsigned int> positions_done;
	int j;

	while(limit_of_total_checks>0 && limit_of_probe_check>0 && hamming_distance<=k){
		next_neighbor = position_on_hypercube;
		j = 0;
		while(j < hamming_distance){
			random_neighbor = uniform_random_generator((int) (this->size-1));
			if(positions_done.find(random_neighbor) == positions_done.end() ){
				positions_done.insert(random_neighbor);
				j++;
			}
			else
				continue;
			if(next_neighbor[random_neighbor] == '0')
				next_neighbor[random_neighbor] = '1';
			else
				next_neighbor[random_neighbor] = '0';
		}

		positions_done.clear();
		not_checked = already_checked.find(next_neighbor);
		if(not_checked == already_checked.end()){ //not_checked is true!
			next_position = cube_buckets->find(next_neighbor);
			if(next_position != cube_buckets->end()){
				for(auto const &i : *(next_position->second)){

						return_set->insert(i);
					limit_of_total_checks--;
				}
			}
			checked_neighbors++;
			limit_of_probe_check--;
		}
		else
			continue;
		if((unsigned int) checked_neighbors == this->size){
			already_checked.clear();
			hamming_distance++;
			checked_neighbors = 1;
		}

	}

	// cout<<return_set->size()<<std::endl;
	return return_set;

}


std::set<item *> * HashTable::range_search(item * query,int N, int k, int m, int probes,int radius){
	srand(time(NULL));
	int limit_of_total_checks = m;
	int limit_of_probe_check = probes;
	std::set<std::string> checked_positions;
	std::map<std::string,std::vector<item*>*> * cube_buckets = (std::map<std::string,std::vector<item*>*> *)buckets;

	std::set<item *> * neighbors_in_range = new std::set<item *>() ;


	int hi;
	unsigned char hvalue;
	std::map<int, unsigned char>::iterator fvalue;
	std::string position_on_hypercube = "";
	for(int i = 0;i < k; i++){
		hi = H(query,i);
		fvalue = (f->at(i))->find(hi);
		if(fvalue == (f->at(i))->end()){
			hvalue = rand()%2;
			(f->at(i))->insert(std::pair<int,unsigned char>(hi,hvalue));
			position_on_hypercube.push_back(hvalue + '0');
		}
		else{
			position_on_hypercube.push_back(fvalue->second + '0');
		}
	}
	//position on hypercube is complete
	std::map<std::string,std::vector<item*> *>::iterator same_position;

	same_position = cube_buckets->find(position_on_hypercube);
	if(same_position != cube_buckets->end()){
		std::vector<item*>::iterator temp;

		temp = same_position->second->begin();

		while(limit_of_total_checks > 0 && temp != same_position->second->end()){
			float current_dist = (*temp)->item_distance(2, query); //this could be generic with ldistance
			if(current_dist < radius)
				neighbors_in_range->insert((*temp));
			temp++;
			limit_of_total_checks--;
		}
	}
	unsigned int random_neighbor;
	std::map<std::string,std::vector<item*> *>::iterator next_position;
	float current_dist;
	std::string next_neighbor;
	std::set<std::string> already_checked;
	std::set<std::string>::iterator not_checked;
	int hamming_distance = 1;
	int checked_neighbors = 1;
	std::set<unsigned int> positions_done;
	int j;

	while(limit_of_total_checks>0 && limit_of_probe_check>0 && hamming_distance<=k){
		next_neighbor = position_on_hypercube;
		j = 0;
		while(j < hamming_distance){
			random_neighbor = uniform_random_generator((int) (this->size-1));
			if(positions_done.find(random_neighbor) == positions_done.end() ){
				positions_done.insert(random_neighbor);
				j++;
			}
			else
				continue;
			if(next_neighbor[random_neighbor] == '0')
				next_neighbor[random_neighbor] = '1';
			else
				next_neighbor[random_neighbor] = '0';
		}

		positions_done.clear();
		not_checked = already_checked.find(next_neighbor);
		if(not_checked == already_checked.end()){ //not_checked is true!
			next_position = cube_buckets->find(next_neighbor);
			if(next_position != cube_buckets->end()){
				for(auto const &i : *(next_position->second)){
					current_dist = query->item_distance(2,i);
					if(current_dist < radius)
						neighbors_in_range->insert(i);
					limit_of_total_checks--;
				}
			}
			checked_neighbors++;
			limit_of_probe_check--;
		}
		else
			continue;
		if((unsigned int) checked_neighbors == this->size){
			already_checked.clear();
			hamming_distance++;
			checked_neighbors = 1;
		}

	}
	if(neighbors_in_range->empty())
		cout<< "no neighbor in range was found " << std::endl;
	return neighbors_in_range;

}

std::set<item *> * HashTable::positions_checked_range_search(item * q,int m,int radius,std::set<std::string> * checked_positions){
	int limit_of_total_checks = m;
	std::map<std::string,std::vector<item*>*> * cube_buckets = (std::map<std::string,std::vector<item*>*> *)buckets;
	std::set<item *> * in_range = new std::set<item *>();
	float current_dist;
	for (auto const &i :(*checked_positions)) {
		if(cube_buckets->find(i) != cube_buckets->end()){
			std::vector<item*>::iterator temp;

			temp = cube_buckets->find(i)->second->begin();

			while(limit_of_total_checks > 0 && temp != cube_buckets->find(i)->second->end()){
				current_dist = (*temp)->item_distance(2, q); //this could be generic with ldistance
				if(current_dist<radius){
					in_range->insert(*temp);
				}

				temp++;
				limit_of_total_checks--;
			}
		}
		if(limit_of_total_checks<=0)
			break;
	}
	return in_range;
}


void HashTable::execute_query(int N, int k, int m, int probes, float radius, item *query, std::multimap <float,item*> * nearest_neighbors ,
	std::set<std::string> * checked_positions, double * nn_time){
	clock_t start = clock();
	int limit_of_total_checks = m;
	int limit_of_probe_check = probes;
	std::map<std::string,std::vector<item*>*> * cube_buckets = (std::map<std::string,std::vector<item*>*> *)buckets;


	int hi;
	unsigned char hvalue;
	std::map<int, unsigned char>::iterator fvalue;
	std::string position_on_hypercube = "";
	for(int i = 0;i < k; i++){
		hi = H(query,i);
		fvalue = (f->at(i))->find(hi);
		if(fvalue == (f->at(i))->end()){
			hvalue = rand()%2;
			(f->at(i))->insert(std::pair<int,unsigned char>(hi,hvalue)); //should this exist?
			position_on_hypercube.push_back(hvalue + '0');
		}
		else{
			position_on_hypercube.push_back(fvalue->second + '0');
		}
	}
	std::map<std::string,std::vector<item*> *>::iterator same_position;

	same_position = cube_buckets->find(position_on_hypercube);
	checked_positions->insert(position_on_hypercube);
	if(same_position != cube_buckets->end()){
		std::vector<item*>::iterator temp;

		temp = same_position->second->begin();

		while(limit_of_total_checks > 0 && temp != same_position->second->end()){
			float current_dist = (*temp)->item_distance(2, query); //this could be generic with ldistance
			if(nearest_neighbors->size() < (unsigned long int) N){
				nearest_neighbors->insert(std::pair<float, item *>(current_dist,*temp));

			}
			else if (current_dist <= nearest_neighbors->rbegin()->first ){

				nearest_neighbors->insert(std::pair<float, item *>(current_dist, *temp));
				nearest_neighbors->erase(nearest_neighbors->rbegin()->first);

			}
			temp++;
			limit_of_total_checks--;
		}
	}
	unsigned int random_neighbor;
	std::map<std::string,std::vector<item*> *>::iterator next_position;
	float current_dist;
	std::string next_neighbor;
	std::set<std::string> already_checked;
	std::set<std::string>::iterator not_checked;
	int hamming_distance = 1;
	int checked_neighbors = 1;
	std::set<unsigned int> positions_done;
	int j;

	while(limit_of_total_checks>0 && limit_of_probe_check>0 && hamming_distance<=k){
		next_neighbor = position_on_hypercube;
		j = 0;
		while(j < hamming_distance){
			random_neighbor = uniform_random_generator((int) (this->size-1));
			if(positions_done.find(random_neighbor) == positions_done.end() ){
				positions_done.insert(random_neighbor);
				j++;
			}
			else
				continue;
			if(next_neighbor[random_neighbor] == '0')
				next_neighbor[random_neighbor] = '1';
			else
				next_neighbor[random_neighbor] = '0';
		}

		positions_done.clear();
		checked_positions->insert(next_neighbor);
		not_checked = already_checked.find(next_neighbor);
		if(not_checked == already_checked.end()){ //not_checked is true!
			next_position = cube_buckets->find(next_neighbor);
			if(next_position != cube_buckets->end()){
				for(auto const &i : *(next_position->second)){
					current_dist = query->item_distance(2,i);
					if(nearest_neighbors->size() < (unsigned long int) N){
						nearest_neighbors->insert(std::pair<float, item *>(current_dist,i));

					}
					else if (current_dist <= nearest_neighbors->rbegin()->first ){

						nearest_neighbors->insert(std::pair<float, item *>(current_dist, i));
						nearest_neighbors->erase(nearest_neighbors->rbegin()->first);

					}
					limit_of_total_checks--;
				}
			}
			checked_neighbors++;
			limit_of_probe_check--;
		}
		else
			continue;
		if((unsigned int) checked_neighbors == this->size){
			already_checked.clear();
			hamming_distance++;
			checked_neighbors = 1;
		}

	}
	if(nearest_neighbors->empty())
		cout<< "no neighbor was found " << std::endl;
	clock_t end = clock();
	* nn_time = (double)(end - start) / CLOCKS_PER_SEC;

}
