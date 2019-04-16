#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/handle_file.h"

int main(int argc, char const *argv[])
{
    int option = -1, idServidor = 0;
    double salarioServidor = 0.0;
    char cargo[200], nome[500], telefone[15], query_field[500], csv_file_name[100], data_file_name[100];
    scanf("%d", &option);
    switch(option)
    {
        case 1:
            scanf("%s", csv_file_name);
            create_bin_file("arquivoTrab1.bin", csv_file_name);
            break;

        case 2:
            scanf("%s", data_file_name);
            get_all_data_file(data_file_name);
            break;

        case 3:
            scanf("%s", data_file_name);
            scanf("%s", query_field);
            if(strcmp(query_field, "idServidor") == 0)
            {
                scanf("%d", &idServidor);
                search_for_id(data_file_name, idServidor);
            }
            else if(strcmp(query_field, "salarioServidor") == 0)
            {
                scanf("%lf", &salarioServidor);
                search_for_salario(data_file_name, salarioServidor);
            }
            else if(strcmp(query_field, "telefoneServidor") == 0)
            {
                scanf("%s", telefone);
                search_for_telefone(data_file_name, telefone);
            }
            else if(strcmp(query_field, "nomeServidor") == 0)
            {
                fgets(nome, sizeof(nome), stdin);
                search_for_nome(query_field, nome);
            }
            else
            {
                fgets(cargo, sizeof(cargo), stdin);
                search_for_cargo(query_field, cargo);
            }
            break;

        default:
            break;
    }
    // create_bin_file("data.cdbf", "data_.csv");
    // // get_all_data_file("data.cdbf");
    // // search_for_id("data.cdbf", 6106893);
    // // search_for_salario("data.cdbf", 0);
    // // search_for_telefone("data.cdbf", "");
    // // search_for_nome("data.cdbf", "");
    // search_for_cargo("data.cdbf", "AUXILIAR OPERACIONAL EM AGROPECUARIA");
    return 0;
}
