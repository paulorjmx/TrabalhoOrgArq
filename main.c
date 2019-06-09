/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/


#include "inc/handle_file.h"
#include "inc/handle_index.h"
#include "inc/func_aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    INDEX_DATA *index_data = NULL;
    int option = -1, idServidor = 0, n = 0, r = 0, qt_disk_pages_index = 0, qt_disk_pages_data = 0;
    unsigned int qt_reg_index = 0;
    long int byte_offset = 0;
    double salarioServidor = 0.0;
    char salario[300], cargo[500], nome[200], telefone[15], query_field[20];
    char  csv_file_name[100], data_file_name[100], update_field[20], data_file_name2[100], data_file_name3[100];
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
                scanf(" %500[^\n\r]", nome);
                search_for_nome(data_file_name, nome);
            }
            else
            {
                scanf(" %200[^\n\r]", cargo);
                search_for_cargo(data_file_name, cargo);
            }
            break;

        case 4:
            scanf("%s", data_file_name);
            scanf("%d", &n);
            for(int i = 0; i < n; i++)
            {
                memset(telefone, 0x00, sizeof(telefone));
                memset(nome, 0x00, sizeof(nome));
                memset(cargo, 0x00, sizeof(cargo));
                memset(salario, 0x00, sizeof(salario));
                scanf("%s", query_field);
                if(strcmp(query_field, "idServidor") == 0)
                {
                    scanf("%d", &idServidor);
                    r = remove_by_id(data_file_name, idServidor);
                }
                else if(strcmp(query_field, "salarioServidor") == 0)
                {
                    scanf("%s", salario);
                    if(strcmp(salario, "NULO") == 0)
                    {
                        salarioServidor = -1.0;
                    }
                    else
                    {
                        salarioServidor = strtod(salario, NULL);
                    }
                    r = remove_by_salario(data_file_name, salarioServidor);
                }
                else if(strcmp(query_field, "telefoneServidor") == 0)
                {
                    scan_quote_string(telefone);
                    if(strcmp(telefone, "NULO") == 0)
                    {
                        memset(telefone, 0x00, sizeof(telefone));
                    }
                    r = remove_by_telefone(data_file_name, telefone);
                }
                else if(strcmp(query_field, "nomeServidor") == 0)
                {
                    scan_quote_string(nome);
                    if(strcmp(nome, "NULO") == 0)
                    {
                        memset(nome, 0x00, sizeof(nome));
                    }
                    r = remove_by_nome(data_file_name, nome);
                }
                else if(strcmp(query_field, "cargoServidor") == 0)
                {
                    scan_quote_string(cargo);
                    if(strcmp(cargo, "NULO") == 0)
                    {
                        memset(cargo, 0x00, sizeof(cargo));
                    }
                    r = remove_by_cargo(data_file_name, cargo);
                }
            }
            if(r != -1)
            {
                binarioNaTela2(data_file_name);
            }
            break;

        case 5:
            scanf("%s", data_file_name);
            scanf("%d", &n);
            for(int i = 0; i < n; i++)
            {
                memset(telefone, 0x00, sizeof(telefone));
                memset(nome, 0x00, sizeof(nome));
                memset(cargo, 0x00, sizeof(cargo));
                memset(salario, 0x00, sizeof(salario));
                scanf("%d", &idServidor);
                scanf("%s", salario);
                scan_quote_string(telefone);
                scan_quote_string(nome);
                scan_quote_string(cargo);
                if(strcmp(telefone, "NULO") == 0)
                {
                    memset(telefone, 0x00, sizeof(telefone));
                }
                if(strcmp(nome, "NULO") == 0)
                {
                    memset(nome, 0x00, sizeof(nome));
                }
                if(strcmp(cargo, "NULO") == 0)
                {
                    memset(cargo, 0x00, sizeof(cargo));
                }
                if(strcmp(salario, "NULO") == 0)
                {
                    salarioServidor = -1.0;
                }
                else
                {
                    salarioServidor = strtod(salario, NULL);
                }
                r = insert_bin(data_file_name, idServidor, salarioServidor, telefone, nome, cargo);
            }
            if(r != -1)
            {
                binarioNaTela2(data_file_name);
            }
            break;

        case 6:
            scanf("%s", data_file_name);
            scanf("%d", &n);
            for(int i = 0; i < n; i++)
            {
                memset(telefone, 0x00, sizeof(telefone));
                memset(nome, 0x00, sizeof(nome));
                memset(cargo, 0x00, sizeof(cargo));
                memset(salario, 0x00, sizeof(salario));
                scanf("%s", query_field);
                if(strcmp(query_field, "idServidor") == 0)
                {
                    scanf("%d", &idServidor);
                    scanf("%s", update_field);
                    r = edit_by_id(data_file_name, idServidor, update_field);
                }
                else if(strcmp(query_field, "salarioServidor") == 0)
                {
                    scanf("%s", salario);
                    if(strcmp(salario, "NULO") == 0)
                    {
                        salarioServidor = -1.0;
                    }
                    else
                    {
                        salarioServidor = strtod(salario, NULL);
                    }
                    scanf("%s", update_field);
                    r = edit_by_salario(data_file_name, salarioServidor, update_field);
                }
                else if(strcmp(query_field, "telefoneServidor") == 0)
                {
                    scan_quote_string(telefone);
                    if(strcmp(telefone, "NULO") == 0)
                    {
                        memset(telefone, 0x00, sizeof(telefone));
                    }
                    scanf("%s", update_field);
                    r = edit_by_telefone(data_file_name, telefone, update_field);
                }
                else if(strcmp(query_field, "nomeServidor") == 0)
                {
                    scan_quote_string(nome);
                    if(strcmp(nome, "NULO") == 0)
                    {
                        memset(nome, 0x00, sizeof(nome));
                    }
                    scanf("%s", update_field);
                    r = edit_by_nome(data_file_name, nome, update_field);
                }
                else if(strcmp(query_field, "cargoServidor") == 0)
                {
                    scan_quote_string(cargo);
                    if(strcmp(cargo, "NULO") == 0)
                    {
                        memset(cargo, 0x00, sizeof(cargo));
                    }
                    scanf("%s", update_field);
                    r = edit_by_cargo(data_file_name, cargo, update_field);
                }
            }
            if(r != -1)
            {
                binarioNaTela2(data_file_name);
            }
            break;

        case 7:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            r = sort_data_file(data_file_name, data_file_name2);
            if(r != -1)
            {
                binarioNaTela2(data_file_name2);
            }
            break;

        case 8:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            scanf("%s", data_file_name3);
            r = merging_data_file(data_file_name, data_file_name2, data_file_name3);
            if(r != -1)
            {
                binarioNaTela2(data_file_name3);
            }
            break;

        case 9:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            scanf("%s", data_file_name3);
            r = matching_data_file(data_file_name, data_file_name2, data_file_name3);
            if(r != -1)
            {
                binarioNaTela2(data_file_name3);
            }
            break;

        case 10:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            if(create_index_file(data_file_name2, data_file_name) != -1)
            {
                binarioNaTela2(data_file_name2);
            }
            break;

        case 11:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            scanf("%s", query_field);
            if(strcmp(query_field, "nomeServidor") == 0)
            {
                scanf(" %500[^\n\r]", nome);
                search_for_nome_index(data_file_name, data_file_name2, nome);
            }
            break;

        case 12:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            scanf("%d", &n);
            qt_reg_index = load_index(data_file_name2, &index_data);
            if(qt_reg_index > 0)
            {
                for(int i = 0; i < n; i++)
                {
                    scanf("%s", query_field);
                    if(strcmp(query_field, "nomeServidor") == 0)
                    {
                        scan_quote_string(nome);
                        if(strcmp(nome, "NULO") == 0)
                        {
                            memset(nome, 0x00, sizeof(nome));
                        }
                        r = remove_by_nome(data_file_name, nome);
                        if(r != -1)
                        {
                            r = remove_index_file(data_file_name2, index_data, qt_reg_index, nome);
                            if(r != -1)
                            {
                                binarioNaTela2(data_file_name2);
                            }
                        }
                    }
                }
                write_index_data(data_file_name2, index_data, qt_reg_index);
                free(index_data);
            }
            break;

        case 13:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            scanf("%d", &n);
            qt_reg_index = load_index(data_file_name2, &index_data);
            if(qt_reg_index > 0)
            {
                for(int i = 0; i < n; i++)
                {
                    memset(telefone, 0x00, sizeof(telefone));
                    memset(nome, 0x00, sizeof(nome));
                    memset(cargo, 0x00, sizeof(cargo));
                    memset(salario, 0x00, sizeof(salario));
                    scanf("%d", &idServidor);
                    scanf("%s", salario);
                    scan_quote_string(telefone);
                    scan_quote_string(nome);
                    scan_quote_string(cargo);
                    if(strcmp(telefone, "NULO") == 0)
                    {
                        memset(telefone, 0x00, sizeof(telefone));
                    }
                    if(strcmp(nome, "NULO") == 0)
                    {
                        memset(nome, 0x00, sizeof(nome));
                    }
                    if(strcmp(cargo, "NULO") == 0)
                    {
                        memset(cargo, 0x00, sizeof(cargo));
                    }
                    if(strcmp(salario, "NULO") == 0)
                    {
                        salarioServidor = -1.0;
                    }
                    else
                    {
                        salarioServidor = strtod(salario, NULL);
                    }
                    byte_offset = insert_bin(data_file_name, idServidor, salarioServidor, telefone, nome, cargo);
                    if(byte_offset != -1)
                    {
                        if(nome[0] != '\0')
                        {
                            if(insert_index_file(data_file_name2, &index_data, &qt_reg_index, nome, byte_offset) != -1)
                            {
                                binarioNaTela2(data_file_name2);
                            }
                        }
                    }
                }
                write_index_data(data_file_name2, index_data, qt_reg_index);
                free(index_data);
            }
            break;

        case 14:
            scanf("%s", data_file_name);
            scanf("%s", data_file_name2);
            scanf("%s", query_field);
            if(strcmp(query_field, "nomeServidor") == 0)
            {
                printf("*** Realizando a busca sem o auxílio de índice\n");
                scanf(" %500[^\n\r]", nome);
                qt_disk_pages_data = search_for_nome(data_file_name, nome);
                printf("*** Realizando a busca com o auxílio de um índice secundário fortemente ligado\n");
                qt_disk_pages_index = search_for_nome_index_statistical(data_file_name, data_file_name2, nome);
                printf("\nA diferença no número de páginas de disco acessadas: %d", (qt_disk_pages_data - qt_disk_pages_index));
            }
            break;

        default:
            break;
    }
    return 0;
}
