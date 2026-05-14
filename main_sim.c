#include <stdio.h>
#include "include/eduos.h"

int main() {

    create_process(1);
    run_threads();
    ipc_send();

    printf("Simulation completed\n");

    return 0;
}
