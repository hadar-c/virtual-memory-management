//HADAR COHEN - 205372220 - EX5

#include <iostream>
#include "sim_mem.hh"

using namespace std;

char main_memory[MEMORY_SIZE];

int main(){

    char val;
    char ex_name[] = "exec_file";
    char swap_name[] = "swap_file";
    sim_mem* x = new sim_mem (ex_name,swap_name,5,5,5,10,5,5);
    
    char a = x->load (16);  
    x->load (23);
    x->store (3,'a');
    x->store (10,'x');
    x->store (15,'x');
    x->store (20,'x');
    x->store (8,'y');
    x->load (0);
    x->load(10);
    x->store (21,'z');
    x->print_memory();
    x->print_page_table();
    x->print_swap();
    
    delete x;
    return 0;
}