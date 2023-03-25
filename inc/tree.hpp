#pragma once

#include <list>
#include <set>
#include "item.hpp"

class treenode{
	private:
		int id;
		std::list<item *> sameid;
		treenode * left = NULL;
		treenode * right = NULL;
		treenode * parent = NULL;
	public:
		treenode(int id,treenode * parent,item * firstitem){
			this->id = id;
			this->parent = parent;
			this->sameid.push_back(firstitem);
		}
		~treenode(){
			delete this->left;
			delete this->right;
			sameid.clear();
		}
		void add_to_idtree(int,item*);
		std::list<item*> * search_on_id(int);
		treenode * getleftchild(){
			return this->left;
		};
		void setleftchild(treenode * toset){
			this->left = toset;
		};
		treenode * getrightchild(){
			return this->left;
		};
		void setrightchild(treenode * toset){
			this->right = toset;
		};

		std::list<item*> * get_bucket_elements(std::list<item*> *);

		void print_items();

};

class treehead{
	private:
		treenode * root;
	public:
		treehead(treenode * root){
			this->root = root;
		}
		void add_to_idtree(int id,item* toadd){
			//this->numberofnodes++;
			if(this->root == NULL){
				this->root = new treenode(id,NULL,toadd);
				return;
			}
			this->root->add_to_idtree(id,toadd);
			return;
		}
		std::list<item*> * search_on_id(int id){
			return this->root->search_on_id(id);
		}



		void get_bucket_elements(std::list<item *> *all_elements){
			this->root->get_bucket_elements(all_elements);
		}

		void print_tree(){ if(this->root != NULL) this->root->print_items(); };



		~treehead(){
			delete root;
		}
};
