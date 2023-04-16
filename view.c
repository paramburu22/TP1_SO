#include "information.h"

int main(int argc, char * argv[]) {
    // Error case -> no parameters where given and it is not connected to a pipe
    char * shared_memory_name;
    char * semaphore_name;
    if(argc > 1) { // We receive arguments as parameters
        if(argc != 3){
            error_call("You must send exactly 2 arguments\n",1);
        }
        shared_memory_name = malloc(sizeof(argv[1])+1);
        semaphore_name = malloc(sizeof(argv[2])+1);

        strcpy(shared_memory_name, argv[1]);
        strcpy(semaphore_name,argv[2]);
        
    } else { // We receive arguments from pipe or we wait for the user to send them
        printf("AISNDJAB\n");
        shared_memory_name = NULL;
        semaphore_name = NULL;
        size_t len = 0;
        // Getting shared memory name
        getline(&shared_memory_name, &len, stdin);
        shared_memory_name[strcspn(shared_memory_name, "\n")] = '\0'; // Remove the trailing newline character
        len = 0;
        // Getting semaphore name
        getline(&semaphore_name, &len, stdin);
        semaphore_name[strcspn(semaphore_name, "\n")] = '\0';
    }

    // Open shared memory
    printf("%s\n", shared_memory_name);
    int shm_fd = shm_open(shared_memory_name, O_RDWR, 0666);
        
    Response * pointer_to_shm = (Response *) mmap(NULL, MAX_FILES * sizeof(Response), PROT_READ, MAP_SHARED, shm_fd, 0);
    Response * next_response = pointer_to_shm;
    // Open semaphore
    sem_t * semaphore = sem_open(semaphore_name, O_EXCL);
    int auxi;
    sem_getvalue(semaphore, &auxi);
    printf("%d\n",auxi);

    sem_wait(semaphore);
    while((*next_response).pid > 0) {
        // Waiting for semaphore -> if it is 0, wait until it is 1
        // Semaphore is 1 -> we can read from shared memory
        print_process_information(*next_response);
        sem_wait(semaphore);
        next_response ++;
    }
    printf("HOLA\n");
    // Closing semaphore
    sem_close(semaphore);
    // Closing semaphore
    sem_destroy(semaphore);
    // Closing shared memory
    close_shared_memory(pointer_to_shm, MAX_FILES * sizeof(Response), shared_memory_name, shm_fd);
    
    // Freeing memory
    free(shared_memory_name);
    free(semaphore_name);
    return 0;
}