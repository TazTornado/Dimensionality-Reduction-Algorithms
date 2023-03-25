#pragma once
#include <string>
#include <vector>
#include <map>
#include "hashtable.hpp"
#include "lsh.hpp"
#include "hypercube.hpp"
// #include "kmeans.hpp"

using namespace std;
/* Prototypes of helper functions */

extern double distance_max, distance_mean;
extern string max_dist_query;

extern double time_max, time_mean;
extern string max_time_query;

extern int bad_dist_counter, bad_time_counter;

void error_handler(int error_code, const string &message);
int euclidean_modulo(int num1, int num2);
float gaussian_random_generator();
float uniform_random_generator(int upper_bound);
float uniform_random_generator(float upper_bound);
void read_configuration(map<string, string> *parameters, string file_to_read);
map<string, string> *get_program_parameters(int argc, char *argv[], string behavior);
int count_lines_in_file(const string &filepath);
void parse_file(string file_to_read, string file_to_write, void *problem, string problem_type);