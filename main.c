#include "hashtable.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#define BUFFER_SIZE 512
#define STEP_SIZE 100

void start_menu()
{
    sleep(1);
    printf("\n\n\tREPLACE WORDS APP\n\n");
    printf("\\> 1. Replace words from file\n");
    printf("\\> 2. View file content\n");
    printf("\\> 3. Exit program\n\n");
}

/*
    Check if the given file is safe to open
    For other users, change the dir_path name from /home/cypher/ to /home/your_username/
*/
short int file_safe_to_open(char * file_path) 
{
    char dir_path[] = "/home/cypher/";
    char path[256], *p;
    struct stat fs;

    if (strncmp(dir_path, file_path, strlen(dir_path))) {
        printf("[!] \\> Wrong directory\n");
        return -1;
    }
    if (strstr(file_path, "../") != NULL) {
        printf("[!] \\> Security risk: \"../\" detected\n");
        return -1;
    }
    strncpy(path, file_path, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    p = path;

    while (strcmp(p, ".") && strcmp(p, "/")) {
        if (lstat(p, &fs) < 0) {
            printf("[!] \\> Cannot get info about the file\n");
            return -1;
        }
        if (S_ISLNK(fs.st_mode)) {
            printf("[!] \\> Security risk: symlink detected\n");
            return -1;
        }
        p = dirname(p);
    }
    return 1;
}
// Verify if the user input option is correct
short int check_input_option(char * input_option)
{
    if ((strncmp(input_option, "1", 2) == 0) || (strncmp(input_option, "2", 2) == 0) || (strncmp(input_option, "3", 2) == 0))
        return 1;
    return -1;
}

// Function to free the memory occupied by a pointer array
void free_buffer(char ** buffer)
{
    unsigned int line = 0;
    while (buffer[line]) {
        if (buffer[line] != NULL) {
            free(buffer[line]);
            buffer[line] = NULL;
        }
        ++line;
    }
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
}

// Print elements of a pointer array
void print_lines(char ** lines, unsigned int len)
{
    unsigned int line_counter;
    for (line_counter = 0; line_counter < len; ++line_counter) {
        printf("%s", lines[line_counter]);
    }
}

/*
 * Replace all occurrences of a given a word in string.
 */
void replace_words(char *str, const char *old_word, const char *new_word)
{
    char * pos, temp[BUFFER_SIZE];
    unsigned int index = 0;
    unsigned int owlen = strlen(old_word);

    // Repeat till all occurrences are replaced.
    while ((pos = strstr(str, old_word)) != NULL)
    {
        // Backup current line
        strcpy(temp, str);

        // Index of current found word
        index = pos - str;

        // Terminate string after word found index
        str[index] = '\0';

        // Concatenate string with new word
        strcat(str, new_word);

        // Concatenate string with remaining words after
        // old_word found index.
        strcat(str, temp + index + owlen);
    }
}

// Read contents of a given file and put them in memory
char ** load_file(char * file_name, unsigned int * len)
{
    FILE * file_ptr = fopen(file_name, "r");
    if (!file_ptr)
    {
        printf("[!] \\> Can't open for reading...\n");
        return NULL;
    }

    // Allocate memory for contents of the file
    unsigned int ptr_size = STEP_SIZE;
    char ** lines = (char **)malloc(ptr_size * sizeof(char *));
    unsigned int line_counter = 0;
    char buffer[BUFFER_SIZE];

    // Read line by line from file
    while (fgets(buffer, BUFFER_SIZE, file_ptr)) {

        // Check if we need to reallocate memory
        if (line_counter == ptr_size) {

            ptr_size += STEP_SIZE;
            char ** new_lines = realloc(lines, ptr_size * sizeof(char *));

            if (!new_lines) {
                printf("[!] \\> Can't reallocate...\n");
                exit(EXIT_SUCCESS);
            }
            lines = new_lines;
        }

        // Allocate memory for a line
        unsigned int line_len = strlen(buffer);
        char * line = (char *)malloc((line_len + 1) * sizeof(char));
        strcpy(line, buffer);

        lines[line_counter] = line;
        ++line_counter;
    }
    *len = line_counter;
    fclose(file_ptr);
    return lines;
}

int main()
{
    // Declare hashtable
    ht_t *ht = ht_create();
    char * option = (char *)malloc(30 * sizeof(char));

    do {
        // Print menu and read user option
        start_menu();
        printf("\\> Enter option: ");
        scanf(" %s", option);

        if (check_input_option(option) < 0) {
            sleep(1);
            printf("[!] \\> You entered a wrong option\n");
        }
        else if (strncmp(option, "1", 2) == 0){

            char word_key[30], word_value[30];
            
            char * file_path = (char *)malloc(100 * sizeof(char));
            unsigned int num_words;
            short int is_safe;

            // Read file path
            printf("\\> Enter file path: ");
            scanf("%s", file_path);

            is_safe = file_safe_to_open(file_path);

            if (is_safe < 0) {
                printf("[!] \\> File is not safe to open\n");
                sleep(1);
                if (file_path != NULL) {
                    free(file_path);
                    file_path = NULL;
                }
            }
            else {
                
                printf("\\> File safe to open\n\n");
                sleep(1);

                unsigned int length = 0;    // number of lines
                char ** file_text = load_file(file_path, &length);

                // If file does not exist
                if (!file_text) {
                    printf("[!] \\> File reading failed...\n");
                }
                else {

                    // print_lines(file_text, length);
                    printf("\\> Number of lines read: %u\n", length);

                    printf("\\> Enter how many unique words you want to replace: ");
                    scanf("%u", &num_words);

                    if (num_words > 100) {
                        printf("[!] \\> That's way too many to enter by hand, try less words\n");
                    }
                    else {

                        // Open temporary file to write the content of the original file with replaced words
                        FILE * temp_ptr;
                        temp_ptr = fopen("replace.txt", "w");

                        // Read key:value pair; key=word_to_replace, value=word_to_replace_with
                        while (num_words--) {

                            printf("\\> Word you want to replace: ");
                            scanf("%s", word_key);
                            printf("\\> Word to replace by: ");
                            scanf("%s", word_value);
                            
                            // Add key:value in hashtable
                            ht_set(ht, word_key, word_value);
                        }

                        // ht_dump(ht); // uncomment this line if you want to see content of the hashtable
                        printf("\\> Words being replaced...\n");

                        // Parse words line by line, check if they are in the hashtable, if they are, we replace them
                        unsigned int line_counter;
                        for (line_counter = 0; line_counter < length; ++line_counter) {
                            unsigned int line_size = strlen(file_text[line_counter]) + 1;
                            char line[line_size];
                            strcpy(line, file_text[line_counter]);
                            char tmp_line[line_size];
                            strcpy(tmp_line, line);

                            // This is how we parse words from a string in C language
                            // We use strtok function and specify the delimiters
                            // In chr we save a word at a time
                            // This method will destroy the original line, so we had to make a copy of it, tmp_line
                            char * chr = strtok(tmp_line, ".,!?:\'- \"()");
                            while (chr != NULL) {
                                // Check if words are in hashtable
                                if (ht_get(ht, chr)) {
                                    replace_words(line, chr, ht_get(ht, chr));
                                }
                                // Go to next word
                                chr = strtok(NULL, ".,!?:\'- \"()");
                            }
                            // Write in temporary file
                            fprintf(temp_ptr, line);
                        }
                        fclose(temp_ptr);
                        // We remove the original file and replace it with the temporary file
                        remove(file_path);
                        rename("replace.txt", file_path);
                        sleep(1);
                        printf("\\> Done\n");
                    }
                    
                    // Free the pointer array after we are finished
                    free_buffer(file_text);
                }
            }
        }
        else if (strncmp(option, "2", 2) == 0) {

            char * file_path = (char *)malloc(100 * sizeof(char));
            short int is_safe;
            // Read file path
            printf("\\> Enter file path: ");
            scanf("%s", file_path);

            is_safe = file_safe_to_open(file_path);

            if (is_safe < 0) {
                printf("[!] \\> File is not safe to open\n");
                sleep(1);
            }
            else {
                
                printf("\\> File is safe to open\n\n");
                sleep(1);

                unsigned int length = 0;    // number of lines
                char ** file_text = load_file(file_path, &length);

                // If file does not exist
                if (!file_text) {
                    printf("[!] \\> File reading failed...\n");
                }
                else {
                    print_lines(file_text, length);
                }
                if (file_path != NULL) {
                    free(file_path);
                    file_path = NULL;
                }
                free_buffer(file_text);
            }
        }
    } while (strncmp(option, "3", 2) != 0);

    if (option != NULL) {
        free(option);
        option = NULL;
    }

    printf("\\> Exiting program...\n");
    sleep(1);

    return 0;
}
