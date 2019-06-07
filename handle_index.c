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
int compare_index_function(const void *a, const void *b); // Funcao utilizada em 'qsort' para ordenar os indices
int load_index(const char *file_name, INDEX_DATA **data); // Funcao utilizada para carregar o arquivo de indice para 'data'
int *binary_seach_index(INDEX_DATA *data, unsigned int nitems, int *items_finded, const char *nome); // Funcao utilizada para buscar o nome no indice carregado na memoria primaria. Retorna um conjunto de indices


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

long int *search_name_index(const char *file_name, const char *nome, int *items_finded)
{
    long int *ptr_byte_offsets = NULL;
    int qt_reg = 0, *ptr_indexes = NULL, qt_indexes = 0;
    HEADER_INDEX header;
    INDEX_DATA *index_data = NULL;
    *items_finded = 0;
    if(file_name != NULL)
    {
        qt_reg = load_index(file_name, &index_data);
        if(qt_reg != -1)
        {
            ptr_indexes = binary_seach_index(index_data, qt_reg, &qt_indexes, nome);
            if(ptr_indexes != NULL)
            {
                ptr_byte_offsets = (long int *) malloc(sizeof(long int) * qt_indexes);
                if(ptr_byte_offsets != NULL)
                {
                    for(int i = 0; i < qt_indexes; i++)
                    {
                        ptr_byte_offsets[i] = index_data[ptr_indexes[i]].byteOffset;
                    }
                }
                free(ptr_indexes);
                *items_finded = qt_indexes;
            }
        }
        else
        {
            *items_finded = -1;
        }
        free(index_data);
    }

    return ptr_byte_offsets;
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

int load_index(const char *file_name, INDEX_DATA **data)
{
    char bloat = '@';
    int qt_reg = -1, cluster_size_free = INDEX_CLUSTER_SIZE, ptr = 0;
    HEADER_INDEX header;
    FILE *index_arq = NULL;
    if(file_name != NULL && data != NULL)
    {
        index_arq = fopen(file_name, "rb");
        if(index_arq != NULL)
        {
            fread(&header.status, sizeof(char), 1, index_arq);
            if(header.status == '1')
            {
                fread(&header.nroReg, sizeof(int), 1, index_arq);
                fseek(index_arq, INDEX_CLUSTER_SIZE, SEEK_SET);
                qt_reg = header.nroReg;
                (*data) = (INDEX_DATA *) malloc(sizeof(INDEX_DATA) * qt_reg);
                if((*data) != NULL)
                {
                    for(int i = 0; i < qt_reg; i++)
                    {
                        if((cluster_size_free - 128) < 0)
                        {
                            for(int k = 0; k < cluster_size_free; k++)
                            {
                                fread(&bloat, sizeof(char), 1, index_arq);
                            }
                            cluster_size_free = INDEX_CLUSTER_SIZE;
                        }
                        fread(&( (*data)[ptr].chaveBusca), 120, 1, index_arq);
                        fread(&( (*data)[ptr].byteOffset), sizeof(long int), 1, index_arq);
                        ptr++;
                        cluster_size_free -= 128;
                    }
                }
                else
                {
                    printf("Nao foi possivel alocar memoria.\n");
                }
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
    }
    return qt_reg;
}
 int *binary_seach_index(INDEX_DATA *data, unsigned int nitems, int *items_finded, const char *nome)
 {
    unsigned int sup = nitems, inf = 0, mid = 0, qt_reg = 0;
    int cmp = 0, r = -1, *indexes = NULL, j = 0;
    if(data != NULL)
    {
        mid = (inf + sup) / 2;
        cmp = strcmp(nome, data[mid].chaveBusca);
        while(inf <= sup)
        {
            if(cmp == 0)
            {
                r = mid;
                break;
            }
            else if(cmp < 0)
            {
                sup = (mid - 1);
            }
            else if(cmp > 0)
            {
                inf = (mid + 1);
            }
            mid = (inf + sup) / 2;
            cmp = strcmp(nome, data[mid].chaveBusca);
        }
        if(r != -1)
        {
            sup = (mid - 1);
            while(strcmp(nome, data[sup].chaveBusca) == 0)
            {
                sup++;
                qt_reg++;
            }
            *items_finded = qt_reg;
            indexes = (int *) malloc(sizeof(int) * qt_reg);
            if(indexes != NULL)
            {
                for(int i = (mid - 1); i < sup; i++, j++)
                {
                    indexes[j] = i;
                }
            }
        }
    }
    return indexes;
 }

void remove_index_file(const char *file_name, const char *nome)
{

}
