
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <threads.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>


//condition node
typedef struct cnd_node { 
    int condition; 
    struct cnd_node *next;
}cnd_node;

//fifo condition queue
typedef struct cnd_queue { 
    cnd_node *head ;
    cnd_node *tail ;
}cnd_queue;

//directory node
typedef struct dir_node {
    char *dir_path;
    struct dir_node *next;
}dir_node;

//fifo directories queue
typedef struct dir_queue { 
    dir_node *head;
    dir_node *tail;
}dir_queue;


int thrds_created = 0; //counts how many threads have been created
int num_of_thrds; // number of threads in the program(not including main)
dir_queue *program_dir_queue;
cnd_queue *program_cnd_queue;
char *search_term;
int count_contains_T = 0; // number of files that contain the search term
int count_threads_waiting_or_exit = 0; //number of threads that are waiting(or exited)
int flag_all_done = 0; // =1 if all threads have been created
int err_in_thrd_func = 0; //=1 if error accured 

mtx_t count_contains_T_lock;
mtx_t lockq;
mtx_t create_all_thrds;
cnd_t all_created;
cnd_t start_together;
cnd_t *cnd_array;
int cnd_array_pointer = 0;
cnd_t wait_for_wakening;
int currently_wakening; // num of threads that needed to be awake

//all functions
void add_cnd(int cnd); 
int remove_cnd();
int add_dir(char *dir_to_add);
int remove_dir();
int is_empty_cnd();
int is_empty_dir();
int search();

//add condition to program_cnd_queue
void add_cnd(int cnd)
{
    cnd_node *cnd_node_to_add = (cnd_node*)malloc(sizeof(cnd_node));
    if(cnd_node_to_add == NULL)
    {
        err_in_thrd_func = 1;
        fprintf(stderr, "malloc cnd_node for cnd queue failed: %s\n", strerror(errno));
        //problem we cant continue with
        exit(1);
    }
    cnd_node_to_add->condition = cnd;
    if(is_empty_cnd(program_cnd_queue))
    {
        program_cnd_queue->head = cnd_node_to_add;
        program_cnd_queue->tail = cnd_node_to_add;
    }
    else
    {
        program_cnd_queue->tail->next = cnd_node_to_add;
        program_cnd_queue->tail = cnd_node_to_add;
    }
    program_cnd_queue->tail->next = NULL;
}

//remove and return the next condition from program_cnd_queue
int remove_cnd()
{
    if(program_cnd_queue->head == NULL)
    {
        return -1;
    }
    cnd_node *to_free = program_cnd_queue->head;
    int cnd_to_return = program_cnd_queue->head->condition;
    if(program_cnd_queue->head->next == NULL)
    {
        program_cnd_queue->head = NULL;
        program_cnd_queue->tail = NULL;
    }
    else
    {
        program_cnd_queue->head = program_cnd_queue->head->next;
    }
    free(to_free);
    return cnd_to_return;
}

//add directory to program_dir_queue
//returns 1 on success, 0 otherwise
int add_dir(char *dir_path_to_add)
{
    dir_node *dir_node_to_add = (dir_node*)malloc(sizeof(dir_node));
    dir_node_to_add->dir_path = (char*) malloc(PATH_MAX*sizeof(char));
    if(dir_node_to_add == NULL || dir_node_to_add->dir_path == NULL) //souldnt happen
    {
        fprintf(stderr, "malloc dir_node for dir queue failed: %s\n", strerror(errno));
        err_in_thrd_func = 1;
        return 0;
    }
    
    strcpy(dir_node_to_add->dir_path,dir_path_to_add);
    if(is_empty_dir())
    {
        program_dir_queue->head = dir_node_to_add;
        program_dir_queue->tail = dir_node_to_add;
    }
    else
    {
        program_dir_queue->tail->next = dir_node_to_add;
        program_dir_queue->tail = dir_node_to_add;

    }
    program_dir_queue->tail->next = NULL;
    return 1;
}


//remove and put in dir_to_return the next directory from program_dir_queue
//returns 0 if program_dir_queue is empty(shouldnt happen) and 1 on success
int remove_dir(char *dir_to_return)
{
   
    if(is_empty_dir()){ //souldnt happen
        return 0;
        }
    strcpy(dir_to_return,program_dir_queue->head->dir_path);
    dir_node *to_free = program_dir_queue->head;
    if(program_dir_queue->head->next == NULL)
    {
        program_dir_queue->head = NULL;
        program_dir_queue->tail = NULL;
    }
    else
    {
        program_dir_queue->head = program_dir_queue->head->next;
    }
    free(to_free->dir_path);
    free(to_free);
    return 1;
}

//returns 1 if program_cnd_queue empty, 0 otherwise
int is_empty_cnd()
{
    if(program_cnd_queue->head == NULL) {return 1;}
    return 0;
}

//returns 1 if program_dir_queue empty, 0 otherwise
int is_empty_dir()
{
    if(program_dir_queue->head == NULL) {return 1;}
    return 0;
}


//threads function
int search()
{
    //wait for all the threads to be created
    mtx_lock(&create_all_thrds);
    thrds_created++;
    if (thrds_created == num_of_thrds) {
      cnd_broadcast(&all_created);
    }
    cnd_wait(&start_together,&create_all_thrds);
    mtx_unlock(&create_all_thrds);

    //the program after all threds
    while(1)
    {
        mtx_lock(&lockq);
        if(currently_wakening > 0) // wait for threads that have been awake
        {
            cnd_wait(&wait_for_wakening,&lockq);       
        }
        if(is_empty_dir())
        {
            if(count_threads_waiting_or_exit == num_of_thrds - 1) //are we done?
            {
                flag_all_done = 1; //we are done
                while(!is_empty_cnd())
                {
                    int removed_cnd = remove_cnd();
                    cnd_signal(&cnd_array[removed_cnd]);
                }
                mtx_unlock(&lockq);
                thrd_exit(thrd_success);
            }

            int curr_cnd;
            curr_cnd = cnd_array_pointer;
            add_cnd(curr_cnd);
            cnd_array_pointer++;
            cnd_array_pointer = cnd_array_pointer%num_of_thrds;

            count_threads_waiting_or_exit ++;
            cnd_wait(&cnd_array[curr_cnd],&lockq);
            currently_wakening --;
            count_threads_waiting_or_exit --;

            if(currently_wakening == 0)
            {
            cnd_broadcast(&wait_for_wakening);
            }
            if(flag_all_done)
            {
                mtx_unlock(&lockq);
                thrd_exit(thrd_success);
            }
        }

        char curr_dir_path[PATH_MAX];
        int try_remove = remove_dir(curr_dir_path); // program_dir_queue isnt empty

        if(try_remove == 0){ //Shouldn't happen
            if(currently_wakening>0)
            {
                cnd_wait(&wait_for_wakening,&lockq);       
            }
            mtx_unlock(&lockq);
            continue;
            }
        mtx_unlock(&lockq);
         
        struct dirent *curr_entry;
        DIR *curr_dir;
        curr_dir = opendir(curr_dir_path);
        if(curr_dir == NULL) //Shouldn't happen
        {
            mtx_lock(&lockq);
            if(currently_wakening > 0)
            {
                cnd_wait(&wait_for_wakening,&lockq);       
            }
            mtx_unlock(&lockq);
            continue;
        }
        
        //check evrything in the folder
        while((curr_entry=readdir(curr_dir)) != NULL)
        {
            
            char curr_entry_name[PATH_MAX]; 
            strcpy(curr_entry_name,curr_entry->d_name);
            // if the entry name is "." or ".." ignore it
            if(strcmp(curr_entry_name,".") == 0 || strcmp(curr_entry_name,"..") == 0) { continue;}
            
            char full_path[PATH_MAX];
            strcpy(full_path,curr_dir_path);
            strcat(full_path, "/");
            strcat(full_path, curr_entry_name);

            struct stat st;
            int result = stat(full_path,&st);
            if(result == -1)
            {
                err_in_thrd_func = 1;
                continue;
            }
            // if its a directory
            if(S_ISDIR(st.st_mode))  
            {
                //the directory cant be searched
                if(access(full_path,  R_OK | X_OK) == -1) 
                {
                    printf("Directory %s: Permission denied.\n",full_path);
                    continue;
                }

                //else- the directory can be searched- add to the queue
                mtx_lock(&lockq);
                result = add_dir(full_path);
                if((!is_empty_cnd()) && (result != 0))
                {
                    int removed_cnd = remove_cnd();
                    cnd_signal(&cnd_array[removed_cnd]);
                    currently_wakening ++;
                }

                mtx_unlock(&lockq);
            }
            //its a file, check if its name fits the search term
            else if(strstr(curr_entry_name,search_term)) 
            {

                mtx_lock(&count_contains_T_lock);
                count_contains_T++;
                mtx_unlock(&count_contains_T_lock);
                printf("%s\n",full_path);
            }
        }
        closedir(curr_dir);
    } 
}


int main(int argc, char *argv[]) {

    //validate that the correct number of command line arguments is passed  
    if(argc != 4)
    {
        fprintf(stderr, "wrong number of arguments");
        exit(1);
    }

   
    char* rootD = argv[1];
    search_term = argv[2];
    num_of_thrds = atoi(argv[3]);
    int i;
    int check;

    //validate that the directory specified in argv[1] can be searched
    if(access(rootD,  R_OK | X_OK) != 0)
    {
        fprintf(stderr, "Directory %s: Permission denied.\n",rootD);
        exit(1);
    }

    //Create a FIFO queue that holds directories.
    program_dir_queue = (dir_queue*)malloc(sizeof(dir_queue));
    if(program_dir_queue == NULL)
    {
        fprintf(stderr, "malloc failed: %s\n", strerror(errno));
        exit(1);
    }

    //Put the search root directory
    int result = add_dir(rootD);
    if(result == 0)
    {
        exit(1);
    }

    //Create a FIFO queue that holds conditions.
    program_cnd_queue = (cnd_queue*)malloc(sizeof(cnd_queue));
    if(program_cnd_queue == NULL)
    {
        fprintf(stderr, "malloc failed: %s\n", strerror(errno));
        exit(1);
    }

    //init cnd and mtx
    check = cnd_init(&all_created);
    if(check != thrd_success)
    {
        fprintf(stderr, "cnd_init failed: %s\n", strerror(errno));
        exit(1);
    }
    check = mtx_init(&create_all_thrds,mtx_plain );
    if(check != thrd_success)
    {
        fprintf(stderr, "mtx_init failed: %s\n", strerror(errno));
        exit(1);
    }
    check = mtx_init(&lockq,mtx_plain);
    if(check != thrd_success)
    {
        fprintf(stderr, "mtx_init failed: %s\n", strerror(errno));
        exit(1);
    }
    check = mtx_init(&count_contains_T_lock, mtx_plain);
    if(check != thrd_success)
    {
        fprintf(stderr, "mtx_init failed: %s\n", strerror(errno));
        exit(1);
    }

    check = cnd_init(&start_together);
    if(check != thrd_success)
    {
        fprintf(stderr, "cnd_init failed: %s\n", strerror(errno));
        exit(1);
    }
    check = cnd_init(&wait_for_wakening);
    if(check != thrd_success)
    {
        fprintf(stderr, "cnd_init failed: %s\n", strerror(errno));
        exit(1);
    }
    
    //create condition array
    cnd_array = (cnd_t*)malloc(num_of_thrds*sizeof(cnd_t));
    for(i = 0; i < num_of_thrds; i++)
    {
        check = cnd_init(&cnd_array[i]);
        if(check != thrd_success)
        {
            fprintf(stderr, "cnd_init failed: %s\n", strerror(errno));
            exit(1);
        }
    }

    //Create n searching threads
    thrd_t threads[num_of_thrds];
    mtx_lock(&create_all_thrds);
    for (i = 0; i < num_of_thrds; i++) 
    {
       check = thrd_create(&threads[i], search, NULL);
       if (check != thrd_success) {
            fprintf(stderr, "ERROR in thrd_create(): %s\n", strerror(errno));
            exit(1);
        }
    }

    cnd_wait(&all_created,&create_all_thrds);
    cnd_broadcast (&start_together);
    mtx_unlock(&create_all_thrds);
    
    // Wait for all threads to complete
    for (int i = 0; i < num_of_thrds; i++) 
    {
        check = thrd_join(threads[i], NULL);
        if (check != thrd_success) {
            fprintf(stderr, "ERROR in thrd_join(): %s\n", strerror(errno));
            printf("Done searching, found %d files\n",count_contains_T);
            exit(1);
        }
    }
    printf("Done searching, found %d files\n",count_contains_T);

    mtx_destroy(&create_all_thrds);
    mtx_destroy(&lockq);
    mtx_destroy(&count_contains_T_lock);
    cnd_destroy(&all_created);
    cnd_destroy(&start_together);
    cnd_destroy(&wait_for_wakening);
    for(i = 0; i < num_of_thrds; i++)
    {
        cnd_destroy(&cnd_array[i]);
    }
    free(cnd_array);
    free(program_dir_queue);
    free(program_cnd_queue);
    

    if(!err_in_thrd_func){exit(0);}
    else{exit(1);}

}
//done :)








