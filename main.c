#include <stdio.h>
#include "doto.h"

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Argument expected.\n\n");
        print_help();
        return 1;
    }

    return route_command(argc, argv);
}
