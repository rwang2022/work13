#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

struct pop_entry
{
    int year;
    int population;
    char boro[15];
};

int main(int argc, char *argv[])
{
    if (argv[1] == NULL)
    {
        printf("No commands were given.\n");
        return 0;
    }
    int i;
    for (i = 1; i < argc; i++)
    {
        // strcmp() returns 0 (false) if same 
        if (!(strcmp(argv[i], "-read_csv")))
        {
            read_csv("nyc_pop.csv");
        }
        else if (!(strcmp(argv[i], "-read_data")))
        {
            read_data();
        }
        else if (!(strcmp(argv[i], "-add_data")))
        {
            add_data();
        }
        else if (!(strcmp(argv[i], "-update_data")))
        {
            update_data();
        }
        else
        {
            printf("Command is not valid.\n");
        }
    }
    return 0;
}

long file_size(char *r)
{
    // returns size of file (duh)
    struct stat st;
    stat(r, &st);
    return st.st_size;
}

void read_csv(char *file_name)
{
    int i;

    // Open file
    int file;
    file = open(file_name, O_RDONLY);
    if (!file)
    {
        printf("Error:\n");
        printf("%s\n", strerror(errno));
        return;
    }

    // Read file
    int filesize = file_size(file_name);
    char data[filesize];
    int read_file;
    read_file = read(file, data, filesize);
    if (!read_file)
    {
        printf("Error:\n");
        printf("%s\n", strerror(errno));
        return;
    }

    // keeps tracks of number of lines
    int num_lines = 0;
    for (i = 0; i < filesize + 1; i++)
    {
        if (data[i] == '\n')
            num_lines++;
    }

    // goes through the lines and prints and updates into a list
    int index = 0, j;
    struct pop_entry pop_entries[5 * num_lines - 5];
    char *boro[5] = {"Manhattan", "Brooklyn", "Queens", "Bronx", "Staten Island"};
    for (i = 0; i < num_lines - 1; i++)
    {
        int lines[6];
        while (data[index] != '\n')
        {
            index++;
        }
        index++;
        sscanf(data + index, "%d, %d, %d, %d, %d, %d\n", &lines[0], &lines[1], &lines[2], &lines[3], &lines[4], &lines[5]);
        for (j = 1; j < 6; j++)
        {
            pop_entries[5 * i + j - 1].year = lines[0];
            strcpy(pop_entries[(5 * i) + j - 1].boro, boro[j - 1]);
            pop_entries[(5 * i) + j - 1].population = lines[j];
        }
    }

    int out = open("nyc_pop.data", O_CREAT | O_WRONLY, 0644);

    if (!out)
    {
        printf("Error:\n");
        printf("%s\n", strerror(errno));
        return;
    }

    read_file = write(out, pop_entries, sizeof(pop_entries));

    if (!out)
    {
        printf("Error:\n");
        printf("%s\n", strerror(errno));
        return;
    }

    printf("reading %s\n", file_name);
    printf("wrote %lld bytes to %s\n", file_size("nyc_pop.data"), file_name);
}

void read_data()
{
    int filesize = file_size("nyc_pop.data");

    // Open file
    int file = open("nyc_pop.data", O_RDONLY);
    if (!file)
    {
        printf("Error: The file doesn't exist. You need to first run with -read_csv.\n");
        return;
    }

    // Read file
    struct pop_entry *pop_entries = malloc(file);
    int read_file = read(file, pop_entries, filesize);
    if (!read_file)
    {
        printf("Error:\n");
        printf("%s\n", strerror(errno));
        return;
    }

    // Print the data
    int i, num_entries = filesize / sizeof(struct pop_entry);
    for (i = 0; i < num_entries; i++)
    {
        printf("%d:\t year: %d\t boro: %s\t pop: %d\n", i, pop_entries[i].year, pop_entries[i].boro, pop_entries[i].population);
    }
    free(pop_entries);
}

void add_data()
{
    struct pop_entry new;
    char input[100];

    printf("Enter year boro pop:\n");
    fgets(input, sizeof(input), stdin);

    // make sure that inputs are inputted correctly
    // new.boro is formatted a little strangely
    // up to 1024 characters 
    // [^0-9] is not digit
    if (sscanf(input, "%d %1024[^0-9] %d", &new.year, new.boro, &new.population) != 3)
    {
        printf("Please enter the data in the format: [int] [string] [int]\n");
        return;
    }

    int file = open("nyc_pop.data", O_WRONLY | O_APPEND);

    if (!file)
    {
        printf("Error: \nData file doesn't exist, run -read_csv first.\n");
        return;
    }

    int err = write(file, &new, sizeof(new));

    if (!err)
    {
        printf("Error");
        return;
    }
}

void update_data()
{
    struct pop_entry new;
    int num_entries = file_size("nyc_pop.data") / sizeof(struct pop_entry);
    char input[100];

    read_data();
    printf("Entry to update: ");

    int index;
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%d", &index);

    if (index >= num_entries || index < 0)
    {
        printf("Not a possible entry\n");
        return;
    }

    int year, pop;
    char *boro;

    printf("Enter year boro pop:\n");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d %1024[^0-9] %d", &new.year, new.boro, &new.population) != 3)
    {
        printf("Please enter the data in the format: [int] [string] [int]\n");
        return;
    }

    int file = open("nyc_pop.data", O_RDWR);

    if (!file)
    {
        printf("Error: \nData file doesn't exist, run -read_csv first.\n");
        return;
    }
    lseek(file, index * sizeof(struct pop_entry), SEEK_SET);

    int err = write(file, &new, sizeof(new));

    if (!err)
    {
        printf("Error:\n");
        return;
    }

    close(file);
}