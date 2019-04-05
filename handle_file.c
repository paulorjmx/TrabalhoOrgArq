#include "inc/handle_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct t_file_header
{
    int topo_lista;
    char desc_campo1[40], desc_campo2[40], desc_campo3[40], desc_campo4[40], desc_campo5[40];
    char status, tag_campo1, tag_campo2, tag_campo3, tag_campo4, tag_campo5;
} FILE_HEADER;

#define SIZE_FILE_HEADER 210
#define CLUSTER_SIZE 32000 // Tamanho do cluster em bytes

void init_file_header(FILE_HEADER *header)
{
    if(header != NULL)
    {
        header->topo_lista = -1;
        header->status = '1';
        strcpy(header->desc_campo1, "numero de identificacao do servidor");
        strcpy(header->desc_campo2, "salario do servidor");
        strcpy(header->desc_campo3, "telefone celular do servidor");
        strcpy(header->desc_campo4, "nome do servidor");
        strcpy(header->desc_campo5, "cargo do servidor");
        header->tag_campo1 = '^';
        header->tag_campo2 = '#';
        header->tag_campo3 = '$';
        header->tag_campo4 = '%';
        header->tag_campo5 = '!';
    }
}

void create_bin_file(const char *file_name)
{
    if(access(file_name, F_OK) != 0)
    {
        int i = 0;
        char bloat = '@';
        FILE_HEADER fh;
        FILE *fp = fopen(file_name, "wb");
        if(fp != NULL)
        {
            init_file_header(&fh);
            fwrite(&fh.status, sizeof(char), 1, fp);
            fwrite(&fh.topo_lista, sizeof(int), 1, fp);
            fwrite(&fh.tag_campo1, sizeof(char), 1, fp);
            fwrite(&fh.desc_campo1, sizeof(fh.desc_campo1), 1, fp);
            fwrite(&fh.tag_campo2, sizeof(char), 1, fp);
            fwrite(&fh.desc_campo2, sizeof(fh.desc_campo2), 1, fp);
            fwrite(&fh.tag_campo3, sizeof(char), 1, fp);
            fwrite(&fh.desc_campo3, sizeof(fh.desc_campo3), 1, fp);
            fwrite(&fh.tag_campo4, sizeof(char), 1, fp);
            fwrite(&fh.desc_campo4, sizeof(fh.desc_campo4), 1, fp);
            fwrite(&fh.tag_campo5, sizeof(char), 1, fp);
            fwrite(&fh.desc_campo5, sizeof(fh.desc_campo5), 1, fp);
            while(i < (CLUSTER_SIZE - 210))
            {
                fwrite(&bloat, sizeof(char), 1, fp);
                i++;
            }
        }
        else
        {
            printf("Falha ao criar o arquivo!\n");
        }
        fclose(fp);
    }
    else
    {
        printf("O arquivo ja existe.\n");
    }
}

void read_file(const char *file_name)
{
    FILE *arq_csv = NULL;
    char line_readed[1000], *token = NULL;

    if(file_name != NULL)
    {
        arq_csv = fopen(file_name, "r+b");
        fgets(line_readed, sizeof(line_readed), arq_csv);
        if(arq_csv != NULL)
        {
            while(1)
            {
                if(feof(arq_csv) != 0)
                {
                    break;
                }
                else
                {
                    fgets(line_readed, sizeof(line_readed), arq_csv);
                    token = line_readed;

                }
            }
        }
        else
        {
            printf("Falha no carregamento do arquivo.\n");
        }
    }
    else
    {

    }
}

void print_file_header(FILE_HEADER header)
{
    // if(header != NULL)
    // {
        printf("%d\n", header.topo_lista);
        printf("%s\n", header.desc_campo1);
        printf("%s\n", header.desc_campo2);
        printf("%s\n", header.desc_campo3);
        printf("%s\n", header.desc_campo4);
        printf("%s\n", header.desc_campo5);
        printf("%c\n", header.status);
        printf("%c\n", header.tag_campo1);
        printf("%c\n", header.tag_campo2);
        printf("%c\n", header.tag_campo3);
        printf("%c\n", header.tag_campo4);
        printf("%c\n", header.tag_campo5);
    // }
}
