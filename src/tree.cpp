//tree.cpp
#include <iostream>
#include "tree.hpp"

using namespace std;
void treenode::add_to_idtree(int idtocheck,item * toadd){

	if(idtocheck < this->id){
		treenode * temp = this->left;
		if(temp == NULL){
			temp = new treenode(idtocheck,this,toadd);
			this->left = temp;
			temp->sameid.push_back(toadd);
			return;
		}
		this->left->add_to_idtree(idtocheck,toadd);
		return;
	}
	else if(idtocheck > this->id){
		treenode * temp = this->right;
		if(temp == NULL){
			temp = new treenode(idtocheck,this,toadd);
			this->right = temp;
			temp->sameid.push_back(toadd);
			return;
		}
		this->right->add_to_idtree(idtocheck,toadd);
		return;
	}

	this->sameid.push_back(toadd);

	return;

}


std::list<item*> * treenode::search_on_id(int idtocheck){
	if(idtocheck < this->id){
		if(this->left==NULL)
			return NULL;
		return this->left->search_on_id(idtocheck);
	}
	else if(idtocheck > this->id){
		if(this->right==NULL)
			return NULL;
		return this->right->search_on_id(idtocheck);
	}
	return &(this->sameid); 

}



std::list<item * > * treenode::get_bucket_elements(std::list<item*> * all_elements){
	all_elements->insert(all_elements->end(),sameid.begin(),sameid.end());
	if(this->left!=NULL){
		return this->left->get_bucket_elements(all_elements);
	}
	if(this->right!=NULL){
		return this->right->get_bucket_elements(all_elements);
	}

	return all_elements;
}



void treenode::print_items(){

	std::cout << "Node with ID: " << this->id << "contains the following items:" << std::endl;
	for(auto const &i : this->sameid){
		i->printitem();
	}
	std::cout << std::endl << std::endl;

	if(this->left != NULL)
		this->left->print_items();

	if(this->right != NULL)
		this->right->print_items();
}
