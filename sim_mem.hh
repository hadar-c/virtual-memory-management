//HADAR COHEN - 205372220 - EX5

#define MEMORY_SIZE 15
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor {
unsigned int V; // valid 
unsigned int D; // dirty  
unsigned int P; // permission 
int frame; //the number of a frame if in case it is page-mapped 
} page_descriptor;

class sim_mem{
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int counter;
    int * framesArr; //the index of the array is the frame in the main memory. The value is the page that store in this page
    page_descriptor *page_table; //pointer to page table
    public:
        sim_mem(char[], char[], int, int, int, int, int, int);
        ~sim_mem();
        void pageToMem(int address);//this func gat an address and put the page in this virtual address in the right place at the main memory, if there is outher page in the main memory we will check if to cleer him or to move him to the swap.
        char load(int address);
        void store(int address, char value);
        void print_memory();
        void print_swap ();
        void print_page_table();
};