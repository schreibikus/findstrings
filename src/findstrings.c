/*
* Copyright (C) 2018 Schreibikus https://github.com/schreibikus
* License: http://www.gnu.org/licenses/gpl.html GPL version 3 or higher
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define is_valid(x) (((x) >= 0x20) && ((x) <= 0x7E))
#define MIN_STRING_LEN 4

static bool mapfile(const char *filename, unsigned char **data, unsigned long *size)
{
    bool result = false;
    unsigned long fsize;
    int fd;

    if(filename && data && size)
    {
        fd = open(filename, O_RDONLY);
        if(fd != -1)
        {
            fsize = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            if(fsize > 0)
            {
                void *maddr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
                if(maddr != MAP_FAILED)
                {
                    *data = maddr;
                    *size = fsize;
                    result = true;
                }
                else
                {
                    fprintf(stderr, "%s %d: Failed(%s) mmap file %s\n", __FUNCTION__, __LINE__, strerror(errno), filename);
                }
            }
            else
            {
                fprintf(stderr, "%s %d: File %s is empty\n", __FUNCTION__, __LINE__, filename);
            }
            close(fd);
        }
        else
        {
            fprintf(stderr, "%s %d: Failed(%s) open file %s\n", __FUNCTION__, __LINE__, strerror(errno), filename);
        }
    }
    else
    {
        fprintf(stderr, "%s %d: invalid arguments %p %p %p\n", __FUNCTION__, __LINE__, filename, data, size);
    }

    return result;
}

static bool printStrings(const unsigned char *data, unsigned long size)
{
    bool result = false;
    unsigned long index;
    bool stringValid = false;
    const unsigned char *startptr = 0;

    if(data && size)
    {
        for(index = 0; index < size; index ++)
        {
            if(stringValid)
            {
                if(data[index])
                {
                    if(is_valid(data[index]))
                    {
                    }
                    else if(((data[index] == '\n') && (index + 1 < size) && !data[index + 1]) ||
                            ((data[index] == '\r') && (index + 1 < size) && !data[index + 1]) ||
                            ((data[index] == '\n') && (index + 2 < size) && (data[index + 1] == '\r') && !data[index + 2]) ||
                            ((data[index] == '\r') && (index + 2 < size) && (data[index + 1] == '\n') && !data[index + 2]))
                    {
                        if(&data[index] - startptr >= MIN_STRING_LEN)
                        {
                            printf("0x%08lX: %s", (unsigned long)(startptr - data), startptr);
                            result = true;
                        }
                        stringValid = false;
                    }
                    else
                    {
                        stringValid = false;
                    }
                }
                else
                {
                    if(&data[index] - startptr >= MIN_STRING_LEN)
                    {
                        printf("0x%08lX: %s\n", (unsigned long)(startptr - data), startptr);
                        result = true;
                    }
                    stringValid = false;
                }
            }
            else
            {
                if(is_valid(data[index]))
                {
                    startptr = &data[index];
                    stringValid = true;
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "%s %d: invalid arguments %p %lu\n", __FUNCTION__, __LINE__, data, size);
    }

    return result;
}

int main(int argc, char *argv[])
{
    int result = 1;

    while(1)
    {
        static struct option long_options[] =
                {
                    {"help", no_argument, 0, 'h'},
                    {0, 0, 0, 0}
                };
        /* getopt_long stores the option index here. */
        int option_index = 0;
        int c = getopt_long(argc, argv, "h", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 'h':
                printf("Usage: findstrings file\n");
                printf("Options:\n");
                printf("\t--help\t\t\t\tdisplay this help and exit\n");
                printf("\tfile\t\t\t\tfile in which you want to do strings search\n\n");
                printf("findstrings Copyright (C) 2018 Schreibikus\n");
                return 0;
            default:
                fprintf(stderr, "%s %d: invalid option %d\n", __FUNCTION__, __LINE__, c);
                return 1;
        }
    }

    if (optind >= argc)
    {
        printf("Try to use: findstrings --help\n");
    }
    else
    {
        unsigned long sdatasize = 0;
        unsigned char *sdata = 0;

        if(mapfile(argv[optind], &sdata, &sdatasize))
        {
            if(printStrings(sdata, sdatasize))
                result = 0;
            else
                fprintf(stderr, "String not found\n");

            if(munmap(sdata, sdatasize) != 0)
                fprintf(stderr, "%s %d: Failed(%s) unmap file: %s\n", __FUNCTION__, __LINE__, strerror(errno), argv[optind]);
        }
        else
        {
            fprintf(stderr, "%s %d: stop!\n", __FUNCTION__, __LINE__);
        }
    }

    return result;
}
