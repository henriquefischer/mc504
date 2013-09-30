#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "draw_chars.h"

/*
   ,--.
  ()   \   ZZZzzz...
   /    \
 _/______\_
(__________)
(/  _  _  \)
(`._.()._.´)
(  `-´`-´  )
 \        /
  \,,,,,,/

    +
   / \
  /___\
 ( '7' )
  `---´
  /(_)\
   d b



       {_}
      *-=\
         \\____(
        _|/---\\_
        \        \
*/

void print_elf(int line) {
    switch (line) {
        case 0: printf(TEXT_YELLOW "    +    " TEXT_DEFAULT ""); break;
        case 1: printf(TEXT_RED "   / \\" TEXT_DEFAULT "   "); break;
        case 2: printf(TEXT_RED "  /___\\" TEXT_DEFAULT "  "); break;
        case 3: printf(" ( '7' ) "); break;
        case 4: printf("  `---´  "); break;
        case 5: printf("  /" TEXT_GREEN "(_)" TEXT_DEFAULT "\\  "); break;
        case 6: printf("  " TEXT_RED " d b  " TEXT_DEFAULT " "); break;
    }
}

void print_elf_line(int size) {
    int line, elf;
    for (line = 0; line < HEIGHT_ELF; line++) {
        for (elf = 0; elf < size; elf++) {
            print_elf(line);
        }
        printf("\n");
    }
}

void print_deer(int line, int n) {
    char c[5];
    
    if(n == 0) strcpy (c, TEXT_RED"*"TEXT_DEFAULT);
    else strcpy(c,"*");
    
    switch (line) {
        case 0: printf(" {_}        "); break;
        case 1: printf("%s-=\\        ", c); break;
        case 2: printf("   \\\\____(  "); break;
        case 3: printf("  _|/---\\_  "); break;
        case 4: printf("  \\        \\"); break;
    }
}

void print_deer_line(int size) {
    int line, deer;
    for (line = 0; line < HEIGHT_DEER; line++) {
        for (deer = 0; deer < size; deer++) {
            print_deer(line, deer);
        }
        printf("\n");
    }
}

void print_santa(int line, int sleeping) {
    char z[10] = "ZZZzzz...";
    char c = '_';
    if (sleeping == 2) {
        printf("            ");
        if (line == 1) {
            printf("         ");
        }
        return;
    }
    if (sleeping == 0) {
        strcpy(z, "         ");
        c = '0';
    }
    switch (line) {
        case 0: printf(TEXT_RED"   ,--.     "TEXT_DEFAULT); break;
        case 1: printf(TEXT_WHITE"  ()  "TEXT_RED" \\    "TEXT_DEFAULT"%s", z); break;
        case 2: printf(TEXT_RED"   /    \\   "TEXT_DEFAULT); break;
        case 3: printf(TEXT_WHITE" _"TEXT_RED"/"TEXT_WHITE"______"TEXT_RED"\\"TEXT_WHITE"_ "TEXT_DEFAULT); break;
        case 4: printf(TEXT_RED"(__________)"TEXT_DEFAULT); break;
        case 5: printf(TEXT_WHITE"("TEXT_DEFAULT"/  %c  %c  \\"TEXT_WHITE")"TEXT_DEFAULT, c, c); break;
        case 6: printf(TEXT_WHITE"(`._."TEXT_DEFAULT"()"TEXT_WHITE"._.´)"TEXT_DEFAULT); break;
        case 7: printf(TEXT_WHITE"(  `-´`-´  )"TEXT_DEFAULT); break;
        case 8: printf(TEXT_WHITE" \\        / "TEXT_DEFAULT); break;
        case 9: printf(TEXT_WHITE"  \\,,,,,,/  "TEXT_DEFAULT); break;
    }
}

void print_santa_room(int status, int elves) {
    int line, elf;
    printf("____________________________________________\n");
    for (line = 0; line < HEIGHT_SANTA; line++) {
        printf("|");
        print_santa(line, status);
        if (line >= HEIGHT_SANTA - HEIGHT_ELF) {
            printf("   ");
            for (elf = 0; elf < 3; elf++) {
                if (elf < elves) {
                    print_elf(line - HEIGHT_SANTA + HEIGHT_ELF);
                } else {
                    printf("         ");
                }
            }
        } else if (line == 1) {
            printf("                     ");
        } else {
            printf("                              ");
        }
        printf("|\n");
    }
    printf("|__________________________________________|\n");
    
}

void print_all(int num_elves, int num_reindeers, int num_elves_help, int santa_status){
    int i;
    printf("\033[2J");
    print_deer_line(num_reindeers);
    print_santa_room(santa_status, num_elves_help);
    print_elf_line(num_elves);
    for(i = 0;i<1000000000;i++);
}