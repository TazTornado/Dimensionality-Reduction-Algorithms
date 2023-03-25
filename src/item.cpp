#include <iostream>
#include <vector>
#include <math.h>
#include "item.hpp"
#include <iostream>
using namespace std;

float item:: item_distance(int l,item * tocompare){
    float sum = 0.0 ;

    for(int i = 0;i < this->dimensions;i++){
        sum = sum + pow(tocompare->elements->at(i) - this->elements->at(i),l);
    }

    return pow(sum,1.0/l);
}

float item::item_distance_squared(int l, item * tocompare){
	float sum = 0.0 ;
	for(int i = 0;i < this->dimensions;i++){
		sum = sum + pow(tocompare->elements->at(i) - this->elements->at(i),l);
	}
	return sum;
}


void item::printitem(){
	cout << "Item with ID:" << this->id << endl << "<";

    for(int i = 0;i < this->dimensions;i++){
        cout<<this->elements->at(i);
		if(i < this->dimensions - 1)
			cout << ", ";
    }
    cout<< ">" <<endl;
}


string item::printitem(bool clustering_format){
	string to_return = "<";

	 for(int i = 0;i < this->dimensions;i++){
        to_return += to_string(this->elements->at(i));
		if(i < this->dimensions - 1)
			to_return += ", ";
    }

	to_return += ">";

	return to_return;
}
