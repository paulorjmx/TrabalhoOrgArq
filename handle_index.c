/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inc/handle_index.h"

#define INDEX_CLUSTER_SIZE 32000

typedef struct header_index_t
{
    int nroReg;
    char status;
} HEADER_INDEX;

typedef struct index_data_t
{
    long int byteOffset;
    char chaveBusca[120];
} INDEX_DATA;

void init_index_header(HEADER_INDEX *h); // Funcao para inicialiar o registro de cabecalho de um arquivo de indice
void write_index_header(const char *file_name, HEADER_INDEX *h); // Funcao utilizada para escrever o arquivo de cabecalho no arquivo de indice, criando-o tambem
int compare_index_function(const void *a, const void *b);


void init_index_header(HEADER_INDEX *h)
{
    if(h != NULL)
    {
        h->nroReg = 0;
        h->status = '1';
    }
}

void write_index_header(const char *file_name, HEADER_INDEX *h)
{
    FILE *arq = NULL;
    char bloat = '@';
    if(file_name != NULL)
    {
        arq = fopen(file_name, "wb");
        if(arq != NULL)
        {
            fwrite(&(h->status), sizeof(char), 1, arq);
            fwrite(&(h->nroReg), sizeof(int), 1, arq);
            for(int i = 5; i < INDEX_CLUSTER_SIZE; i++)
            {
                fwrite(&bloat, sizeof(char), 1, arq);
            }
        }
        fclose(arq);
    }
}

void create_index_file(const char *file_name, const char *data_file_name)
{
    int idServidor;
    int cluster_size_free = INDEX_CLUSTER_SIZE, reg_size = 0, register_bytes_readed = 0, var_field_size, ptr = 0;
    char nome[120], cargo[200], telefone[15];
    char removido_token = '-', status_arq = '0', bloat = '@', tag_campo = '*', tag_campo4 = '#', tag_campo5 = '$';
    FILE *index_arq = NULL, *arq = NULL;
    long int bo = 0, el = 0;
    double salario;
    HEADER_INDEX header;
    INDEX_DATA index_data[6000];
    if((file_name != NULL) && (data_file_name != NULL))
    {
        arq = fopen(data_file_name, "rb");
        if(arq != NULL)
        {
            fread(&status_arq, sizeof(char), 1, arq);
            if(status_arq == '1')
            {
                fseek(arq, 131, SEEK_CUR);
                fread(&tag_campo4, sizeof(char), 1, arq);
                fseek(arq, 40, SEEK_CUR);
                fread(&tag_campo5, sizeof(char), 1, arq);
                fseek(arq, INDEX_CLUSTER_SIZE, SEEK_SET);
                while (1)
                {
                    memset(&index_data[ptr].chaveBusca, '@', sizeof(index_data[ptr].chaveBusca));
                    register_bytes_readed = 34;
                    var_field_size = 0;
                    bo = ftell(arq);
                    fread(&removido_token, sizeof(char), 1, arq);
                    if(feof(arq) != 0)
                    {
                        break;
                    }
                    else
                    {
                        fread(&reg_size, sizeof(int), 1, arq);
                        if(removido_token == '-')
                        {
                            fseek(arq, 34, SEEK_CUR); // Pula para o comeco dos campos variaveis
                            while(register_bytes_readed < reg_size)
                            {
                                fread(&bloat, sizeof(char), 1, arq);
                                if(bloat == '@')
                                {
                                    register_bytes_readed++;
                                }
                                else
                                {
                                    fseek(arq, -1, SEEK_CUR);
                                    fread(&var_field_size, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&tag_campo, sizeof(char), 1, arq);
                                    register_bytes_readed += sizeof(char);
                                    if(tag_campo == tag_campo4)
                                    {
                                        index_data[ptr].byteOffset = bo;
                                        fread(nome, (var_field_size - 1), 1, arq);
                                        strcpy(index_data[ptr].chaveBusca, nome);
                                        ptr++;
                                    }
                                    else if(tag_campo == tag_campo5)
                                    {
                                        fread(cargo, (var_field_size - 1), 1, arq);
                                    }
                                    register_bytes_readed += (var_field_size - 1);
                                }
                            }
                        }
                        else if(removido_token == '@')
                        {
                            fseek(arq, reg_size, SEEK_CUR);
                        }
                    }
                }
                header.status = '0';
                header.nroReg = ptr;
                write_index_header(file_name, &header);
                index_arq = fopen(file_name, "r+b");
                if(index_arq != NULL)
                {
                    fseek(index_arq, INDEX_CLUSTER_SIZE, SEEK_SET);
                    qsort(index_data, ptr, sizeof(INDEX_DATA), compare_index_function);
                    for(int i = 0; i < ptr; i++)
                    {
                        if((cluster_size_free - 128) < 0)
                        {
                            for(int j = 0; j < cluster_size_free; j++)
                            {
                                fwrite(&bloat, sizeof(char), 1, index_arq);
                            }
                            cluster_size_free = INDEX_CLUSTER_SIZE;
                        }
                        fwrite(&index_data[i].chaveBusca, sizeof(index_data[i].chaveBusca), 1, index_arq);
                        fwrite(&index_data[i].byteOffset, sizeof(long int), 1, index_arq);
                        cluster_size_free -= 128;
                    }
                }
                header.status = '1';
                rewind(index_arq);
                fwrite(&header.status, sizeof(char), 1, index_arq);
            }
            else
            {
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            printf("Falha no processamento do arquivo.\n");
        }
        fclose(index_arq);
        fclose(arq);
    }
}


int compare_index_function(const void *a, const void *b)
{
    int r = strcmp(((INDEX_DATA *) a)->chaveBusca, ((INDEX_DATA *) b)->chaveBusca);
    if(r == 0)
    {
        r = ((INDEX_DATA *) a)->byteOffset - ((INDEX_DATA *) b)->byteOffset;
    }
    return r;
}
