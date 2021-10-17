//HADAR COHEN - 205372220 - EX5

#include "sim_mem.hh"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
using namespace std;

sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size){
    if(exe_file_name == NULL || swap_file_name == NULL){ //Checks if the files NULL.
        perror("File not found");
        exit(1);
    }
    program_fd = open(exe_file_name, O_RDONLY, 0);
    if (program_fd == -1){ //Checks if the open command succeed.
        perror("can't open file");
        exit(1);
    }
    swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (swapfile_fd == -1){ //Checks if the open command succeed.
        perror("can't open file");
        exit(1);
    }
    this->counter = 0;
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;

    page_table = new page_descriptor[num_of_pages];
    framesArr = new int[MEMORY_SIZE / page_size];

    for (int i = 0; i<MEMORY_SIZE/page_size; i++){
        framesArr[i] = -1;
    }

    for (int i = 0; i < num_of_pages; i++){
        page_table[i].frame = -1;
        page_table[i].D = 0;
        page_table[i].V = 0;
        if (text_size / page_size > i)
            page_table[i].P = 0;
        else
            page_table[i].P = 1;
    }

    for (int i = 0; i < MEMORY_SIZE; i++){
        main_memory[i] = '0';
    }

    char str[page_size * num_of_pages];
    for (int i = 0; i < page_size * num_of_pages; i++){
        str[i] = '0';
    }
    if (write(swapfile_fd, str, page_size * num_of_pages) == -1){//Checks if the write command succeed.
        perror("Can't write to file");
        exit(1);
    }
}

sim_mem::~sim_mem(){
    int sucCheck;
    sucCheck = close(swapfile_fd);
    if (sucCheck == -1) { //Checks if the close command succeed.
        perror("Can't close file");
        exit(1);
    }
    sucCheck = close(program_fd);
    if (sucCheck == -1) { //Checks if the close command succeed.
        perror("Can't close file");
        exit(1);
    }
    delete [] page_table;
    delete [] framesArr;
}

void sim_mem::pageToMem(int address){
    int sucCheck;
    int page = framesArr[counter];
    if (page != -1){ //this frame in the main memory is alredy allocate.
        page_table[page].V = 0;
        if (page_table[page].D == 1){ //Checks if the page that we need to clear was changed (if he does we will need to write him to the swap)
            char buffer[page_size]; 
            for (int j = 0; j < page_size; j++){ //Puts in to the string "page" the chars we want to write to the swap. 
                buffer[j] = main_memory[counter * page_size + j];
            }
            lseek(swapfile_fd, page_size * page, SEEK_SET);
            sucCheck = write(swapfile_fd, buffer, page_size);
            if (sucCheck == -1) { //Checks if the write command succeed.
                perror("Can't write to file");
                    exit(1);
            }
        }
    }
    if (page_table[address / page_size].D==1){ //If true this page is in the swap.
        lseek(swapfile_fd, page_size * (address/page_size), SEEK_SET);
        char buff[page_size];
        sucCheck = read(swapfile_fd, buff, page_size);
        if (sucCheck == -1) { //Checks if the read command succeed.
            perror("Can't read from file");
            exit(1);
        }
        for(int i = 0; i<page_size; i++){ //load the new page to the memory
            main_memory[counter*page_size + i] = buff[i];
        }
    }
    else if ((address/page_size) >= ((text_size/page_size)+(data_size/page_size))){ //If true this page belong to bss/Heap-stack
        for(int i = 0; i<page_size; i++){ //load the new page to the memory
            main_memory[counter*page_size + i] = '\0';
        }
    }
    else{ //Means that the page didn't changed and it belong to the text/data.
        lseek(program_fd, page_size * (address/page_size), SEEK_SET);
        char buff[page_size];
        sucCheck = read(program_fd, buff, page_size);
        if (sucCheck == -1) { //Checks if the read command succeed.
            perror("Can't read from file");
            exit(1);
        }
        for(int i = 0; i<page_size; i++){ //load the new page to the memory
            main_memory[counter*page_size + i] = buff[i];
        }
    }
    framesArr[counter] = address / page_size;
    page_table[address / page_size].V = 1;
    page_table[address / page_size].frame = counter;
    counter++;
    if (counter == MEMORY_SIZE / page_size){
        counter = 0;
    }
}

char sim_mem::load(int address){
    if (address < 0 || address >= (num_of_pages * page_size)){ //Checks if the address is legal.
        fprintf(stderr, "Address is Illegal\n");
        return '\0';
    }
    if (address > num_of_pages * page_size - heap_stack_size && page_table[address / page_size].D == 0){//Cheks if it's non exist page of heap\stack
        printf("Couldn't load heap/stack\n");
        return '\0';
    }
    if (page_table[address / page_size].V == 0){ //Checks if the page is already in the main memory.
        pageToMem(address);
    }
    return main_memory[page_table[address / page_size].frame*page_size + (address%page_size)];
}

void sim_mem::store(int address, char value){
    if (address < 0 || address >= (num_of_pages * page_size)){ //Checks if the address is legal.
        fprintf(stderr, "Address is Illegal\n");
        return;
    }
    if (page_table[address / page_size].P == 0){
        printf("No permission to write to this page\n");
        return;
    }
    if (page_table[address / page_size].V == 0){ //Checks if the page is already in the main memory.
        pageToMem(address);
    }
    main_memory[page_table[address / page_size].frame*page_size + (address%page_size)] = value;//store the new value in the main memory.
    page_table[address / page_size].D = 1;
}

void sim_mem::print_memory(){
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++){
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap(){
    char *str = (char *)malloc(this->page_size * sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size){
        for (i = 0; i < page_size; i++){
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
}

void sim_mem::print_page_table(){
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame\n");
    for (i = 0; i < num_of_pages; i++){
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[i].V,
               page_table[i].D,
               page_table[i].P,
               page_table[i].frame);
    }
}
