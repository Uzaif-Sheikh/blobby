#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

// the first byte of every blobette has this value
#define BLOBETTE_MAGIC_NUMBER          0x42

// number of bytes in fixed-length blobette fields
#define BLOBETTE_MAGIC_NUMBER_BYTES    1
#define BLOBETTE_MODE_LENGTH_BYTES     3
#define BLOBETTE_PATHNAME_LENGTH_BYTES 2
#define BLOBETTE_CONTENT_LENGTH_BYTES  6
#define BLOBETTE_HASH_BYTES            1

// maximum number of bytes in variable-length blobette fields
#define BLOBETTE_MAX_PATHNAME_LENGTH   65535
#define BLOBETTE_MAX_CONTENT_LENGTH    281474976710655


// ADD YOUR #defines HERE


typedef enum action {
    a_invalid,
    a_list,
    a_extract,
    a_create
} action_t;


void usage(char *myname);
action_t process_arguments(int argc, char *argv[], char **blob_pathname,
                           char ***pathnames, int *compress_blob);

void list_blob(char *blob_pathname);
void extract_blob(char *blob_pathname);
void create_blob(char *blob_pathname, char *pathnames[], int compress_blob);

uint8_t blobby_hash(uint8_t hash, uint8_t byte);

// ADD YOUR FUNCTION PROTOTYPES HERE

void dir_traverse(FILE* fp,char* pathname);
uint64_t get_content_length(FILE* fp);
uint16_t get_path_length(FILE* fp);
uint32_t get_mode(FILE* fp);
void get_path_name(uint16_t length,char* pathname,FILE* fp);
void extract_file(FILE* fp,char* file_name,int content_length,uint32_t mode);
void extract_dir(char* pathname,uint32_t mode);
void write_blob_file(FILE* fp,char* file_name);
void write_blob_dir(FILE* fp,char* pathname);
uint8_t check_hash(FILE* fp,int size);




// YOU SHOULD NOT NEED TO CHANGE main, usage or process_arguments

int main(int argc, char *argv[]) {
    char *blob_pathname = NULL;
    char **pathnames = NULL;
    int compress_blob = 0;
    action_t action = process_arguments(argc, argv, &blob_pathname, &pathnames,
                                        &compress_blob);

    switch (action) {
    case a_list:
        list_blob(blob_pathname);
        break;

    case a_extract:
        extract_blob(blob_pathname);
        break;

    case a_create:
        create_blob(blob_pathname, pathnames, compress_blob);
        break;

    default:
        usage(argv[0]);
    }

    return 0;
}

// print a usage message and exit

void usage(char *myname) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s -l <blob-file>\n", myname);
    fprintf(stderr, "\t%s -x <blob-file>\n", myname);
    fprintf(stderr, "\t%s [-z] -c <blob-file> pathnames [...]\n", myname);
    exit(1);
}

// process command-line arguments
// check we have a valid set of arguments
// and return appropriate action
// *blob_pathname set to pathname for blobfile
// *pathname and *compress_blob set for create action

action_t process_arguments(int argc, char *argv[], char **blob_pathname,
                           char ***pathnames, int *compress_blob) {
    extern char *optarg;
    extern int optind, optopt;
    int create_blob_flag = 0;
    int extract_blob_flag = 0;
    int list_blob_flag = 0;
    int opt;
    while ((opt = getopt(argc, argv, ":l:c:x:z")) != -1) {
        switch (opt) {
        case 'c':
            create_blob_flag++;
            *blob_pathname = optarg;
            break;

        case 'x':
            extract_blob_flag++;
            *blob_pathname = optarg;
            break;

        case 'l':
            list_blob_flag++;
            *blob_pathname = optarg;
            break;

        case 'z':
            (*compress_blob)++;
            break;

        default:
            return a_invalid;
        }
    }

    if (create_blob_flag + extract_blob_flag + list_blob_flag != 1) {
        return a_invalid;
    }

    if (list_blob_flag && argv[optind] == NULL) {
        return a_list;
    } else if (extract_blob_flag && argv[optind] == NULL) {
        return a_extract;
    } else if (create_blob_flag && argv[optind] != NULL) {
        *pathnames = &argv[optind];
        return a_create;
    }

    return a_invalid;
}


// list the contents of blob_pathname

void list_blob(char *blob_pathname) {

    // REPLACE WITH YOUR CODE FOR -l

    //printf("list_blob called to list '%s'\n", blob_pathname);

    FILE* fd = fopen(blob_pathname,"r");

    if(fd == NULL){

        perror("ERROR");
        exit(1);

    }


    int magic_number;

    while((magic_number = fgetc(fd)) != EOF){


        if(magic_number == BLOBETTE_MAGIC_NUMBER){      // checking for the magic_number and then start taking out information.

            uint32_t mode = get_mode(fd);           // this function gets the mode of the blobette
            
            uint16_t length = get_path_length(fd);      // this function gets the path_length of the blobette
            
            uint64_t content_length = get_content_length(fd);       // this function gets the content_length of the blobette


            char pathname[length+1];            // Creating a string of size length+1 
            get_path_name(length,pathname,fd);          // and getting the string in the char array pathname.

            // The function used here fseek() is adopted from the lecture notes

            fseek(fd,content_length+1,SEEK_CUR);            // Using fseek() to take the file pointer to the next blobette. 

            printf("%06lo %5lu %s\n",(uint64_t)mode, content_length, pathname);            


        }                                   // if magic number is not found then raise an error.                
        else{                                                           
            fprintf(stderr,"ERROR: Magic byte of blobette incorrect\n");
            exit(1);
        }



    }

    fclose(fd);

    // HINT: you'll need a printf like:
    // printf("%06lo %5lu %s\n", mode, content_length, pathname);
}


// extract the contents of blob_pathname

void extract_blob(char *blob_pathname) {

    // REPLACE WITH YOUR CODE FOR -x

    //printf("extract_blob called to extract '%s'\n", blob_pathname);

    FILE* fd = fopen(blob_pathname,"r");        // opening the blob file to read from

    if(fd == NULL){                 // Checking for the error.
        perror("ERROR");
        exit(1);
    }

    int magic_number = 0;

    while((magic_number = fgetc(fd))!= EOF){
        
        if(magic_number == BLOBETTE_MAGIC_NUMBER){          // start to read after the magic number

            uint32_t mode = get_mode(fd);           // gets the mode of the blobette we are trying to exract.
            
            uint16_t length = get_path_length(fd);          //gets the path length.
            
            uint64_t content_length = get_content_length(fd);       // gets content length. 

            char pathname[length+1]; 
            get_path_name(length,pathname,fd);      // gets the path_name using the path length.

            if(mode & __S_IFDIR){               // check the mode if it's a dir then creating a dir

                printf("Creating directory: %s\n",pathname);

                extract_dir(pathname,mode);

            }
            else{                                       // else if it's a file then creating the file.

                printf("Extracting: %s\n",pathname);

                extract_file(fd,pathname,content_length,mode);    

            }

            int size = content_length+length+6+2+3;         // need the size to shift the file pointer back to calculate the hash.

            fseek(fd,-1 * (size),SEEK_CUR);             // shift the file pointer back.

            uint8_t hash = check_hash(fd,size);     // getting the hash of the blobette
            
            int c = fgetc(fd);

            if(hash != c){                                          // Checking the hash don't match then raise an error.
                fprintf(stderr,"ERROR: blob hash incorrect\n");
                exit(1);
            }
        }

        else{                       // if the magic number is not correct then raise an error.

            fprintf(stderr,"ERROR: Magic byte of blobette incorrect\n");
            exit(1);

        }


    }

    fclose(fd);                 // then lastly closing the blob file.
    
}

// create blob_pathname from NULL-terminated array pathnames
// compress with xz if compress_blob non-zero (subset 4)

void create_blob(char *blob_pathname, char *pathnames[], int compress_blob) {

    // REPLACE WITH YOUR CODE FOR -c

    /*printf("create_blob called to create %s blob '%s' containing:\n",
           compress_blob ? "compressed" : "non-compressed", blob_pathname);*/

    char buffer[100];               // make a buffer string of 100.

    struct stat sz;

    FILE* fd = fopen(blob_pathname,"w");            // opening the blob file to write bytes into it. 

    for (int p = 0; pathnames[p]; p++) {

        int c = 0;

        int len_path = strlen(pathnames[p]);
        for(int i = 0;pathnames[p][i] != '\0';i++){         // looping to the string pathnames[p].

            if(pathnames[p][i] == '/'){         // checking if it have "/" as a char then it's a dir and calling the write_dir function.

                if(i == len_path-1) break;

            
                buffer[c] = '\0';

                fputc(BLOBETTE_MAGIC_NUMBER,fd);

                printf("Adding: %s \n",buffer);
                write_blob_dir(fd,buffer);

                buffer[c] = '/';
                
            }
            buffer[c++] = pathnames[p][i]; 

        }

        int check = stat(pathnames[p],&sz);

        if(check != 0){             // Checking for error.
            perror("ERROR");
            exit(0);
        }        

        if(sz.st_mode & __S_IFDIR){         // if the given pathnames[p] is dir then call the dir_traverse to add all it's contain to blob file.

            //make function for write in dir.
            dir_traverse(fd,pathnames[p]);

        }
        else{

            printf("Adding: %s\n", pathnames[p]);       // if it's only a file then just add the file into blob file.
            fputc(BLOBETTE_MAGIC_NUMBER,fd);
            write_blob_file(fd,pathnames[p]);

        }

    }

    fclose(fd);         // at last closin the blob file.

}

    // # ########################################################### #
    // #                   HELPER   FUNCTION                         #
    // # ########################################################### #


    // FUNCTIONs Like mkdir(), chmod(), opendir() and readdir() are adopted from the lecture 
    // notes courtesy of Andrew Taylor. 



// ADD YOUR FUNCTIONS HERE

    // This function takes in a file pointer and read till
    // 3 bytes  using fgetc() and convert big-endian to little-endian format 

uint32_t get_mode(FILE* fp){

    uint32_t read_mode = 0;
    uint32_t mode = 0;                 // mode is 3 bytes so gonna store it in 4 bytes.
    int i = 0;

    while(i < 3){

        read_mode = fgetc(fp);

        if((int32_t)read_mode == EOF){      //checking for error.
            perror("ERROR");
            exit(1);
        }

        if(i == 0){                      // For the first byte read storing it the little-endian fortmat.

            mode = (read_mode << 16) | mode;

        }
        else if(i == 1){                // Doing the same for 2nd byte.

            mode = (read_mode << 8) | mode;

        }
        else if(i == 2){            // And the 3th byte.

            mode = read_mode | mode;

        }   

        i++;
    }

    return mode;

}

    // This function gets the path_length and takes in FILE pointer.
    // and read till 2 bytes and storing it in uint16_t in little-endain format.

uint16_t get_path_length(FILE* fp){

    int i = 0;

    uint16_t path_length = 0;
    uint16_t length = 0;

    while (i < 2){

        path_length = fgetc(fp);
                                                
        if((int16_t)path_length == EOF){           // checking for the error

            perror("ERROR");
            exit(1);

        }

        if(i == 0){                                 // storing the first byte into uint16_t length.

            length = (path_length << 8) | length;

        }
        else if(i == 1){                           // Doing the same for the 2nd byte.

            length = path_length | length;

        }

        i++;

    }

    return length;

}

    // This function takes in length, a char array and a FILE pointer
    // it gives out the name of the file stored in blob file.

void get_path_name(uint16_t length,char* pathname,FILE* fp){

    int i = 0;
    int c = 0;

    while(i < length){              // here length is given by the pervious fuction.

        c = fgetc(fp);              

        if(c == EOF){                 // Checking for the error.

            perror("ERROR");
            exit(1);

        }

        pathname[i] = c;            // Storing each char in pathname array.

        i++;

    }

    pathname[i] = '\0';

}

    // This function takes in FILE pointer and returns the 
    // length of the content of the file.

uint64_t get_content_length(FILE* fp){

    int i = 0;

    uint64_t content = 0;
    uint64_t content_length = 0;

    while(i < 6){                   // As we know content length is stored in 6 bytes.

        content = fgetc(fp);

        if((int64_t)i == EOF){      // Checking for error.

            perror("ERROR");
            exit(1);

        }

        // Here for every byte we are storing it in little-endain format using bit-wise operation.

        if(i == 0){

            content_length = (content << 40) | content_length;

        }
        else if(i == 1){

            content_length = (content  << 32) | content_length;

        }
        else if(i == 2){

            content_length = (content << 24) | content_length;

        }
        else if(i == 3){
            
            content_length = (content << 16) | content_length;

        }
        else if(i == 4){
            
            content_length = (content << 8) | content_length;

        }
        else if(i == 5){
            
            content_length = content | content_length;

        }

        i++;

    }

    return content_length;

}

    // This function helps in extracting the given file, the function takes in
    // 4 arg file pointer , file name , length of it's content and, mode of the file.

void extract_file(FILE* fp,char* file_name,int content_length,uint32_t mode){

    FILE* file = fopen(file_name,"w");    // Open the file I want to extract from the blob file.

    if(file == NULL){               // Checking for error.
        perror("ERROR");
        exit(1);
    }

    // The function used here chmod() is adopted from the lecture notes.

    int check_mode = chmod(file_name,mode);     // THis function set the mode of the file by the given mode.

    if(check_mode != 0){            // Checking for error.
        perror("ERROR");
        exit(1);
    }    

    int i = 0;
    int c = 0;

    while(i < content_length){      // As the content length is given runing the loop till lenght and writing into file.

        c = fgetc(fp);
        if(c != EOF){               // writing all the bytes and checking for end of the file.
            fputc(c,file);
        }

        i++;
    }

    fclose(file);           // Closing the file when the function is gonna end.
}

    // This function is for make the given dir and it's mode.

void extract_dir(char* pathname,uint32_t mode){

    // the function used here mkdir() is adopted from the lecture notes.

    int check = mkdir(pathname,mode);

    if(check != 0){         //checking for the error.

        perror("ERROR");
        exit(1);

    }    

}

    // This function is used when a blob file had to be created.
    // it takes in file pointer and the file_name to write it's content.

void write_blob_file(FILE* fp,char* file_name){

    struct stat st;

    // the function used here mkdir() is adopted from the lecture notes.    

    int check = stat(file_name,&st);

    if(check == -1){            // Checking for error.
        perror("ERROR");
        exit(1);
    }
    
    uint8_t hash = 0xde;           // the blob file should even content the hash of the file.

    uint32_t mode = st.st_mode;     // gets the mode of the file.

    uint8_t c = 0;

    int i = 0;

    while(i < 3){                  // Writing the mode in big-endian format. 

        if(i == 0){
            c = (mode >> 16) & 0xff;
        }
        else if(i == 1){
            c = (mode >> 8) & 0xff;
        }
        else if(i == 2){
            c = mode & 0xff;
        }


        hash = blobby_hash(hash,c);     // For each character written in the blob file calculating 
                                        // the hash with the help of pervious hash.

        fputc(c,fp);

        i++;

    }

    i = 0;

    uint16_t path_length = strlen(file_name);           // Getting the file length 

    c = (path_length >> 8) & 0xff;          // writing it in the blob file in big-endain.


    hash = blobby_hash(hash,c);

    fputc(c,fp);

    c = path_length & 0xff;

    hash = blobby_hash(hash,c);

    fputc(c,fp);

    uint64_t content_length = st.st_size;           // getting the size of content. 

    while(i < 6){                   // And content_length can be stored in 6 bytes in big-endian format.
        
        if(i == 0){
            c = (content_length >> 40) & 0xff;
        }
        else if(i == 1){
            c = (content_length >> 32) & 0xff;
        }
        else if(i == 2){
            c = (content_length >> 24) & 0xff;
        }
        else if(i == 3){
            c = (content_length >> 16) & 0xff;
        }
        else if(i == 4){
            c = (content_length >> 8) & 0xff;
        }
        else if(i == 5){
            c = content_length & 0xff;
        }


        hash = blobby_hash(hash,c);         // Calculating the hash at every step.

        fputc(c,fp);

        i++;
    }

    i = 0;

    while(i < (int)path_length){            // Now writting down the name of the file int blob file, using the file length.

        hash = blobby_hash(hash,file_name[i]);          // calculating the hash using the char of file name.

        fputc(file_name[i],fp);
        i++;
    }

    FILE* file = fopen(file_name,"r");              // To write the content of the file we need to open the file in read mode.


    if(file == NULL){               // Checking for error.

        perror("ERROR");
        exit(1);

    }

    int s = 0;

    while((s = fgetc(file)) != EOF){        // writting the content of the file in blob so reading the file till EOF.
        
        hash = blobby_hash(hash,s);         // Calculating the hash for every char of content.

        fputc(s,fp);

    }

    fputc(hash,fp);         // last writting down the hash into the blob file.

    fclose(file);           // and then closing the file.
    

}

    // THis function helps to write the dir into blob file,
    // it takes the arg file pointer and the dir pathname.

void write_blob_dir(FILE* fp,char* pathname){

    struct stat sz;

    stat(pathname,&sz);

    uint8_t hash = 0xde;            

    uint32_t mode = sz.st_mode;             // Get the mode of dir.

    uint8_t c = 0;

    int i = 0;

    while(i < 3){               // writting the mode in big-endian format into blob file.

        if(i == 0){
            c = (mode >> 16) & 0xff;
        }
        else if(i == 1){
            c = (mode >> 8) & 0xff;
        }
        else if(i == 2){
            c = mode & 0xff;
        }

        hash = blobby_hash(hash,c);        // even calculating the hash.

        fputc(c,fp);

        i++;

    }

    i = 0;

    uint16_t path_length = strlen(pathname);     // Now similar to write file take the length of the dir pathname.

    c = (path_length >> 8) & 0xff;

    hash = blobby_hash(hash,c);

    fputc(c,fp);

    c = path_length & 0xff;

    hash = blobby_hash(hash,c);

    fputc(c,fp);

    while(i < 6){               // The content of dir in the blob file is always zero.

        c = 0x00;

        hash = blobby_hash(hash,c);

        fputc(c,fp);

        i++;

    }

    i = 0;

    while(i < (int)path_length){            // Here writing down the path_name.

        c = pathname[i];

        hash  = blobby_hash(hash,c);

        fputc(c,fp);

        i++;

    }

    fputc(hash,fp);             // And lastly writing the hash into the blob file. 

}


    // This function helps to traverse all the files and sub-dir of that dir.
    // the function uses recrusion to traverse as the dir is just a tree of files and sub-dir.  

    // the function used here opendir() and readdir() is adopted from the lecture notes.

void dir_traverse(FILE* fp,char* pathname){

    char dir_name[100];                     // allocating the char array for a sub-dir of 100
    char file_name[100];            // allocating the char array for a file in dir of 100

    struct stat sz;
    DIR* dir = opendir(pathname);           // opening the dir inorder to read it's content.
    if(dir == NULL){
        return;                         // if the path given is not a dir than the function returns. 
    }
    else{
        printf("Adding: %s \n",pathname);           // else the path given is a dir so we
        if(pathname[strlen(pathname)-1] == '/'){
           pathname[strlen(pathname)-1] = '\0'; 
        }  
        fputc(BLOBETTE_MAGIC_NUMBER,fp);            // call the above function to write dir into blob file,   
        write_blob_dir(fp,pathname);                // giving the file pointer of blob file.
    }


    strcpy(dir_name,pathname);                  // copy the pathname into dir_name string.
    struct dirent* dir_r;

    while((dir_r = readdir(dir)) != NULL){       // Now reading all the content of the dir. 

        if(strcmp(".",dir_r->d_name) != 0 && strcmp("..",dir_r->d_name) != 0){      // while reading it was giving out "." and ".." so only taking the content which not "." and "..".

            
            strcat(dir_name,"/");                   // now concatinating the string dir_name with "/" to read the file or sub-dir in it.
            strcat(dir_name,dir_r->d_name);             // Then concatinating the read content with dir_name/
            stat(dir_name,&sz);
            if(sz.st_mode & __S_IFDIR){          // checking if the dir_name is dir or not 

                dir_traverse(fp,dir_name);          // if it's dir then calling the traverse function again.
                strcpy(dir_name,pathname);      // and when it backtrack copy the pathname back into the dir_name string.
                
            }
            else{                                 // else if the dir_name is not dir then it's a file.                  
                strcpy(file_name,pathname);             // so copying the pathname into file_name.
                strcat(file_name,"/");                           // And concatinating it will "/"       
                strcat(file_name,dir_r->d_name);           // Then concatinaing the path with file name so it will be something like "dir/dir1/text.txt". 

                printf("Adding: %s \n",file_name);

                fputc(BLOBETTE_MAGIC_NUMBER,fp);

                write_blob_file(fp,file_name);              // Calling the function to write file into the blob file.
                strcpy(dir_name,pathname);             // And lastly copying the pathname back to the dir_name string. 
            }
            

        }

    }

    
}

    // This function is used to check the hash in subset 2 and compare with the file hash.

uint8_t check_hash(FILE* fp,int size){

    int i = 0;
    int c = 0;
    uint8_t hash = 0xde;

    //printf("blobby_hash(0,42) = %x\n",hash);      FOR DEBUGGING

    while(i < size){

        c = fgetc(fp);

        //printf("blobby_hash(%x,%x) =",hash,c);   FOR DEBUGGING

        if(c == EOF){
            fprintf(stderr,"Error\n");
            exit(1); 
        }


        hash = blobby_hash(hash,(uint8_t)c);        // just calling the function given in the start code blobby_hash().

        //printf("%x\n",hash);      FOR DEBUGGING

        i++;
    }

    return hash;

}

// YOU SHOULD NOT CHANGE CODE BELOW HERE

// Lookup table for a simple Pearson hash
// https://en.wikipedia.org/wiki/Pearson_hashing
// This table contains an arbitrary permutation of integers 0..255

const uint8_t blobby_hash_table[256] = {
    241, 18,  181, 164, 92,  237, 100, 216, 183, 107, 2,   12,  43,  246, 90,
    143, 251, 49,  228, 134, 215, 20,  193, 172, 140, 227, 148, 118, 57,  72,
    119, 174, 78,  14,  97,  3,   208, 252, 11,  195, 31,  28,  121, 206, 149,
    23,  83,  154, 223, 109, 89,  10,  178, 243, 42,  194, 221, 131, 212, 94,
    205, 240, 161, 7,   62,  214, 222, 219, 1,   84,  95,  58,  103, 60,  33,
    111, 188, 218, 186, 166, 146, 189, 201, 155, 68,  145, 44,  163, 69,  196,
    115, 231, 61,  157, 165, 213, 139, 112, 173, 191, 142, 88,  106, 250, 8,
    127, 26,  126, 0,   96,  52,  182, 113, 38,  242, 48,  204, 160, 15,  54,
    158, 192, 81,  125, 245, 239, 101, 17,  136, 110, 24,  53,  132, 117, 102,
    153, 226, 4,   203, 199, 16,  249, 211, 167, 55,  255, 254, 116, 122, 13,
    236, 93,  144, 86,  59,  76,  150, 162, 207, 77,  176, 32,  124, 171, 29,
    45,  30,  67,  184, 51,  22,  105, 170, 253, 180, 187, 130, 156, 98,  159,
    220, 40,  133, 135, 114, 147, 75,  73,  210, 21,  129, 39,  138, 91,  41,
    235, 47,  185, 9,   82,  64,  87,  244, 50,  74,  233, 175, 247, 120, 6,
    169, 85,  66,  104, 80,  71,  230, 152, 225, 34,  248, 198, 63,  168, 179,
    141, 137, 5,   19,  79,  232, 128, 202, 46,  70,  37,  209, 217, 123, 27,
    177, 25,  56,  65,  229, 36,  197, 234, 108, 35,  151, 238, 200, 224, 99,
    190
};

// Given the current hash value and a byte
// blobby_hash returns the new hash value
//
// Call repeatedly to hash a sequence of bytes, e.g.:
// uint8_t hash = 0;
// hash = blobby_hash(hash, byte0);
// hash = blobby_hash(hash, byte1);
// hash = blobby_hash(hash, byte2);
// ...

uint8_t blobby_hash(uint8_t hash, uint8_t byte) {
    return blobby_hash_table[hash ^ byte];
}
