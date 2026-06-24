#include <stdio.h>
#include "doto.h"

int main(int argc, char *argv[]){
    if (argc < 2){
        printf("Argument expected.\n");
        return 1;
    } else if (argc > 4){
        printf("Too much arguments.");
        return 1;
    }

    return route_command(argc, argv);
}
