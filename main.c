#include <stdio.h>
#include <stdlib.h>
#include "inc/handle_file.h"

int main(int argc, char const *argv[])
{
    create_bin_file("data.cdbf", "data_.csv");
    return 0;
}
