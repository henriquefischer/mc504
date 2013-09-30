#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_DEFAULT "\e[m"
#define TEXT_GREEN "\e[32m"
#define TEXT_RED "\e[31m"
#define TEXT_WHITE "\e[37m"

#define HEIGHT_DEER 5
#define HEIGHT_ELF 7
#define HEIGHT_SANTA 10

void print_elf(int line);
void print_elf_line(int size);
void print_deer(int line, int n);
void print_deer_line(int size);
void print_santa(int line, int sleeping);
void print_santa_room(int status, int elves);
void print_all(int num_elves, int num_reindeers, int num_elves_room, int santa_status);