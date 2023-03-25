#pragma once

#include <stdlib.h>
#include <vector>
#include <string>


class item{
    private:
        std::vector<float> * elements;
        int dimensions;
        std::string id; //the id the files
    public:
        item(int dimensions,std::vector<float> * elements, std::string id){
            //elements = (float*)malloc(sizeof(int)*dimensions);
            this->elements = elements;
            this->dimensions = dimensions;
            this->id = id;
        };
        ~item(){
            delete this->elements;
        }
        void change_certain_dimension(int dimension,float change){
            this->elements->at(dimension) = change;
        }
        float get_certain_dimenstion(int dimension){
            return (*this->elements)[dimension];
        }
		float get_length(){
			return this->dimensions;
		}
		std::vector<float> *get_elements(){ return this->elements; };

		std::string get_id(){ return this->id; };
        float item_distance(int,item *); //this could change in order to have any metric we want
        float item_distance_squared(int, item *);
		void printitem();
		std::string printitem(bool clustering_format);
};

