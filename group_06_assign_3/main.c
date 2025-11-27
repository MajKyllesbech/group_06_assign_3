#include "io.h"
#include "mm.h"
#include <stdlib.h>
#include <unistd.h>


int main() {
    int counter = 0;
    int capacity = 4;
    int size = 0;


    int *collection = simple_malloc(capacity * sizeof(int));

    if (!collection) return 1;

    int c;
    while ((c = read_char()) != -1) {
        if (c == 'a') {

            if (size == capacity) {

                int old_capacity = capacity;
                capacity *= 2;



                int *tmp = simple_malloc(capacity * sizeof(int));

                if (!tmp) {
                    simple_free(collection);
                    return 1;
                }


                for (int i = 0; i < old_capacity; i++) {
                    tmp[i] = collection[i];
                }


                simple_free(collection);

                collection = tmp;

            }
            collection[size++] = counter;
            counter++;
        } else if (c == 'b') {

            counter++;
        } else if (c == 'c') {

            if (size > 0) size--;
            counter++;
        } else {

            break;
        }
    }


    if (size == 0) {

        write_char(';');
        write_char('\n');
    } else {

        if (size == 1 && collection[0] == 0) {
             write_char('0');
        } else {
            for (int i = 0; i < size; i++) {

                if (collection[i] == 0) {
                    write_char('0');
                } else {
                    write_int(collection[i]);
                }

                if (i < size - 1) {
                    write_char(',');
                }
            }
        }
        write_char(';');
        write_char('\n');
    }


    simple_free(collection);
    return 0;
}