#include "inc/handle_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void write_file_header(const char *file_name, FILE_HEADER *header);

// typedef struct t_file_header
// {
//     long int topo_lista;
//     char desc_campo1[40], desc_campo2[40], desc_campo3[40], desc_campo4[40], desc_campo5[40];
//     char status, tag_campo1, tag_campo2, tag_campo3, tag_campo4, tag_campo5;
// } FILE_HEADER;

#define SIZE_FILE_HEADER 214 // Tamanho do cabecalho
#define CLUSTER_SIZE 32000 // Tamanho do cluster em bytes


void init_file_header(FILE_HEADER *header)
{
    if(header != NULL)
    {
        header->topo_lista = -1;
        header->status = '0';
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

void write_file_header(const char *file_name, FILE_HEADER *header)
{
    if(access(file_name, F_OK) != 0)
    {
        int i = 0;
        char bloat = '@';
        FILE *fp = NULL;
        fp = fopen(file_name, "wb");
        if(fp != NULL)
        {
            init_file_header(header);
            fwrite(&(header->topo_lista), sizeof(long int), 1, fp);
            fwrite(&(header->status), sizeof(char), 1, fp);
            fwrite(&(header->tag_campo1), sizeof(char), 1, fp);
            fwrite(&(header->desc_campo1), sizeof(header->desc_campo1), 1, fp);
            fwrite(&(header->tag_campo2), sizeof(char), 1, fp);
            fwrite(&(header->desc_campo2), sizeof(header->desc_campo2), 1, fp);
            fwrite(&(header->tag_campo3), sizeof(char), 1, fp);
            fwrite(&(header->desc_campo3), sizeof(header->desc_campo3), 1, fp);
            fwrite(&(header->tag_campo4), sizeof(char), 1, fp);
            fwrite(&(header->desc_campo4), sizeof(header->desc_campo4), 1, fp);
            fwrite(&(header->tag_campo5), sizeof(char), 1, fp);
            fwrite(&(header->desc_campo5), sizeof(header->desc_campo5), 1, fp);
            while(i < (CLUSTER_SIZE - 214))
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

void create_bin_file(const char *data_file_name, const char *csv_file_name)
{
    FILE_HEADER header;
    FILE *arq = NULL, *arq_csv = NULL;
    char line_readed[1000], telefone_servidor[15], nome_servidor[500], cargo_servidor[200], bloat = '@', removido_token = '-', *token = NULL;
    int id_servidor = 0, reg_size = 39, cluster_size_free = CLUSTER_SIZE, nome_servidor_size = 0, cargo_servidor_size = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;

    if(csv_file_name != NULL)
    {
        if(access(csv_file_name, F_OK) == 0)
        {
            write_file_header(data_file_name, &header);
            // print_file_header(header);
            arq_csv = fopen(csv_file_name, "r");
            arq = fopen(data_file_name, "r+b");
            if(arq != NULL && arq_csv != NULL)
            {
                fgets(line_readed, sizeof(line_readed), arq_csv); // Pega a primeira linha do arquivo csv que nao sera inserida no arquivo de dados.
                fseek(arq, 32000, SEEK_SET); // Pula o cabecalho
                while(1)
                {
                    fgets(line_readed, sizeof(line_readed), arq_csv);
                    token = line_readed;
                    if(feof(arq_csv) != 0)
                    {
                        break;
                    }
                    else
                    {
                        reg_size = 39;
                        nome_servidor_size = 0;
                        cargo_servidor_size = 0;
                        memset(&nome_servidor, 0, sizeof(nome_servidor));
                        memset(&cargo_servidor, 0, sizeof(cargo_servidor));
                        sscanf(token, "%d", &id_servidor);
                        token = strchr(token, ',');
                        token++;
                        if(token[0] == ',')
                        {
                            strcpy(telefone_servidor, "\0@@@@@@@@@@@@@");
                        }
                        else
                        {
                            sscanf(token, " %14[^,]", telefone_servidor);
                        }
                        token = strchr(token, ',');
                        token++;
                        if(token[0] == '0')
                        {
                            salario_servidor = 0.0;
                        }
                        else
                        {
                            sscanf(token, "%lf", &salario_servidor);
                        }
                        token = strchr(token, ',');
                        token++;
                        if(token[0] == ',')
                        {
                            // Profissao e nula.
                        }
                        else
                        {
                            sscanf(token, " %200[^,]", cargo_servidor);
                            cargo_servidor_size = strlen(cargo_servidor) + 1;
                            reg_size += 5 + cargo_servidor_size;
                        }
                        token = strchr(token, ',');
                        token++;
                        if(token[0] == '\n' || token[0] == '\r')
                        {
                            // Nome nulo.
                        }
                        else
                        {
                            sscanf(token, " %500[^\r\n]", nome_servidor);
                            nome_servidor_size = strlen(nome_servidor) + 1;
                            reg_size += 5 + nome_servidor_size;
                        }
                        if((cluster_size_free - reg_size) < 0)
                        {
                            for(int i = 0; i < cluster_size_free; i++)
                            {
                                fwrite(&bloat, sizeof(char), 1, arq);
                            }
                            cluster_size_free = CLUSTER_SIZE;
                        }
                        fwrite(&removido_token, sizeof(char), 1, arq);
                        fwrite(&reg_size, sizeof(int), 1, arq);
                        fwrite(&encadeamento_lista, sizeof(long int), 1, arq);
                        fwrite(&id_servidor, sizeof(int), 1, arq);
                        fwrite(&salario_servidor, sizeof(double), 1, arq);
                        fwrite(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                        if(nome_servidor_size > 0)
                        {
                            fwrite(&nome_servidor_size, sizeof(int), 1, arq);
                            fwrite(&header.tag_campo4, sizeof(char), 1, arq);
                            fwrite(&nome_servidor, nome_servidor_size, 1, arq);
                        }
                        if(cargo_servidor_size > 0)
                        {
                            fwrite(&cargo_servidor_size, sizeof(int), 1, arq);
                            fwrite(&header.tag_campo5, sizeof(char), 1, arq);
                            fwrite(&cargo_servidor, cargo_servidor_size, 1, arq);
                        }
                        cluster_size_free -= reg_size;
                        // printf("%d\n", reg_size);
                        // printf("%d\n", cluster_size_free);
                        // printf("%d\n", id_servidor);
                        // printf("%s\n", telefone_servidor);
                        // printf("%lf\n", salario_servidor);
                        // printf("%s\n", cargo_servidor);
                        // printf("%s\n", nome_servidor);
                    }
                }
                for(int i = 0; i < cluster_size_free; i++)
                {
                    fwrite(&bloat, sizeof(char), 1, arq);
                }
            }
            else
            {
                printf("Falha ao criar o arquivo!\n");
            }
            fseek(arq, 0, SEEK_SET);
            header.status = '1';
            fwrite(&header.status, sizeof(char), 1, arq);
            fclose(arq);
            fclose(arq_csv);
        }
        else
        {
            printf("O arquivo csv nao existe.\n");
        }
    }
    else
    {
        printf("Digite um nome do arquivo csv.\n");
    }
}


void print_file_header(FILE_HEADER header)
{
    // if(header != NULL)
    // {
        printf("%c\n", header.status);
        printf("%d\n", header.topo_lista);
        printf("%c\n", header.tag_campo1);
        printf("%s\n", header.desc_campo1);
        printf("%c\n", header.tag_campo2);
        printf("%s\n", header.desc_campo2);
        printf("%c\n", header.tag_campo3);
        printf("%s\n", header.desc_campo3);
        printf("%c\n", header.tag_campo4);
        printf("%s\n", header.desc_campo4);
        printf("%c\n", header.tag_campo5);
        printf("%s\n", header.desc_campo5);
    // }
}
