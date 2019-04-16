#include "inc/handle_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Registro de cabecalho
typedef struct t_file_header
{
    long int topo_lista;
    char desc_campo1[40], desc_campo2[40], desc_campo3[40], desc_campo4[40], desc_campo5[40];
    char status, tag_campo1, tag_campo2, tag_campo3, tag_campo4, tag_campo5;
} FILE_HEADER;


void write_file_header(const char *file_name, FILE_HEADER *header); // Funcao que escreve o arquivo de cabecalho no arquivo de dados binario
void init_file_header(FILE_HEADER *header, char *desc); // Funcao que inicializa a estrutura de dados FILE_HEADER
void print_file_header(FILE_HEADER header); // Funcao utilidade que mostra na tela todos os campos do registro de cabecalho


#define SIZE_FILE_HEADER 214 // Tamanho do cabecalho
#define CLUSTER_SIZE 32000 // Tamanho do cluster em bytes


void init_file_header(FILE_HEADER *header, char *desc)
{
    int i = 0;
    // token eh usada para percorrer a primeira linha lida no arquivo .csv
    // d_campoX eh usada para armazenar os metadados do arquivo de dados binario. Estes serao inseridos no arquivo de dados binario.
    // bloat eh usada para preencher com lixo o resto das strings.
    char *token = NULL, d_campo1[40], d_campo2[40], d_campo3[40], d_campo4[40], d_campo5[40], bloat = '@';
    if(header != NULL)
    {
        if(desc != NULL)
        {
            token = desc;

            sscanf(token, "%40[^,]", d_campo1); // Recupera a descricao do campo 1 do arquivo .csv
            for(i = (strlen(d_campo1) + 1); i < sizeof(d_campo1); i++) // Preenche o resto da string da descricao do campo 1 com lixo.
            {
                d_campo1[i] = bloat;
            }
            token = strchr(token, ',');
            token++;

            sscanf(token, "%40[^,]", d_campo2);
            for(i = (strlen(d_campo2) + 1); i < sizeof(d_campo2); i++)
            {
                d_campo2[i] = bloat;
            }
            token = strchr(token, ',');
            token++;

            sscanf(token, "%40[^,]", d_campo3);
            for(i = (strlen(d_campo3) + 1); i < sizeof(d_campo3); i++)
            {
                d_campo3[i] = bloat;
            }
            token = strchr(token, ',');
            token++;

            sscanf(token, "%40[^,]", d_campo4);
            for(i = (strlen(d_campo4) + 1); i < sizeof(d_campo4); i++)
            {
                d_campo4[i] = bloat;
            }
            token = strchr(token, ',');
            token++;

            sscanf(token, "%40[^\n\r]", d_campo5);
            for(i = (strlen(d_campo5) + 1); i < sizeof(d_campo5); i++)
            {
                d_campo5[i] = bloat;
            }

            header->status = '0';
            header->topo_lista = -1;
            memcpy(header->desc_campo1, d_campo1, 40);
            memcpy(header->desc_campo2, d_campo2, 40);
            memcpy(header->desc_campo3, d_campo3, 40);
            memcpy(header->desc_campo4, d_campo4, 40);
            memcpy(header->desc_campo5, d_campo5, 40);
            header->tag_campo1 = 'i';
            header->tag_campo2 = 's';
            header->tag_campo3 = 't';
            header->tag_campo4 = 'n';
            header->tag_campo5 = 'c';
        }
    }
}

void write_file_header(const char *file_name, FILE_HEADER *header)
{
    int i = 0;
    char bloat = '@'; // Variavel utilizada para inserir o lixo no arquivo.
    FILE *fp = NULL;
    fp = fopen(file_name, "wb");
    if(fp != NULL)
    {
        // Escreve os dados do cabecalho no arquivo de dados binario a ser criado.
        fwrite(&(header->status), sizeof(char), 1, fp);
        fwrite(&(header->topo_lista), sizeof(long int), 1, fp);
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
        while(i < (CLUSTER_SIZE - 214)) // Preenche o resto da pagina de disco com lixo.
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

void create_bin_file(const char *data_file_name, const char *csv_file_name)
{
    FILE_HEADER header; // Variavel utilizada para recuperar o arquivo de cabecalho do arquivo de dados binario
    FILE *arq = NULL, *arq_csv = NULL; // Ponteiros utilizados para o arquivo de dados binario a ser criado e o arquivo .csv, respectivamente
    // Line readed eh utilizada para ler as linhas do arquivo .csv
    // telefone_servidor, nome_servidor e cargo_servidor sao usadas para guardar temporariamente os valores lidos do arquivo .csv
    // token eh usada para percorrer as linhas lidas do arquivo .csv
    char line_readed[1000], telefone_servidor[15], nome_servidor[500], cargo_servidor[200], *token = NULL;
    // bloat eh usada para preencher com lixo o restante do espaco livre da pagina de disco
    // removido_token eh usada para inserir a flag de removido no registro
    char bloat = '@', removido_token = '-';
    // id_servidor eh usada para guardar temporariamente o valore lido do arquivo .csv
    // reg_size eh usada para armazenar os valores dos registros
    // last_registry_size eh usada para armazenar o tamanho do ultimo registro armazenado no arquivo de dados
    // cluster_size_free eh usada para guardar a informacao da quantidade de bytes ainda estao livres na pagina de disco
    // nome_servidor_size e cargo_servidor_size servem para guardar o tamanho referentes ao nome do servidor e ao cargo do servidor, respectivamente
    int id_servidor = 0, reg_size = 34, last_registry_size = 0, cluster_size_free = CLUSTER_SIZE, nome_servidor_size = 0, cargo_servidor_size = 0;
    // encadeamento_lista eh usada para inserir o valor dos "ponteiros" da lista no arquivo de dados binario
    // last_registry_inserted eh usada como "ponteiro" para o ultimo registro armazenado no arquivo de dados
    long int encadeamento_lista = -1, last_registry_inserted = -1;
    // salario_servidor eh usada para guardar temporariamente o valor lido do arquivo .csv
    double salario_servidor = 0.0;

    if(csv_file_name != NULL)
    {
        if(access(csv_file_name, F_OK) == 0) // Checa a existencia do arquivo .csv (Se existe)
        {
            arq_csv = fopen(csv_file_name, "r");
            if(arq_csv != NULL)
            {
                fgets(line_readed, sizeof(line_readed), arq_csv); // Recupera a primeira linha do arquivo csv.
                init_file_header(&header, line_readed);
                write_file_header(data_file_name, &header);
                arq = fopen(data_file_name, "r+b");
                fseek(arq, CLUSTER_SIZE, SEEK_SET); // Pula o cabecalho
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
                        reg_size = 34;
                        nome_servidor_size = 0;
                        cargo_servidor_size = 0;
                        memset(&nome_servidor, 0, sizeof(nome_servidor));
                        memset(&cargo_servidor, 0, sizeof(cargo_servidor));
                        sscanf(token, "%d", &id_servidor);
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
                            telefone_servidor[0] = '\0';
                            for(int i = 1; i < sizeof(telefone_servidor); i++)
                            {
                                telefone_servidor[i] = '@';
                            }
                        }
                        else
                        {
                            sscanf(token, " %14[^,]", telefone_servidor);
                        }
                        token = strchr(token, ',');
                        token++;
                        if(token[0] == ',')
                        {
                            // Profissao e nula.
                        }
                        else
                        {
                            sscanf(token, " %500[^,]", nome_servidor);
                            nome_servidor_size = strlen(nome_servidor) + 2; // Tamanho da string + o caractere '\0' + tag do campo
                            reg_size += 4 + nome_servidor_size;
                        }
                        token = strchr(token, ',');
                        token++;
                        if(token[0] == '\n' || token[0] == '\r')
                        {
                            // Nome nulo.
                        }
                        else
                        {
                            sscanf(token, " %200[^\r\n]", cargo_servidor);
                            cargo_servidor_size = strlen(cargo_servidor) + 2; // Tamanho da string + o caractere '\0' + tag do campo
                            reg_size += 4 + cargo_servidor_size;
                        }
                        if((cluster_size_free - (reg_size + 5)) < 0)
                        {
                            for(int i = 0; i < cluster_size_free; i++)
                            {
                                fwrite(&bloat, sizeof(char), 1, arq);
                            }
                            fseek(arq, (last_registry_inserted + 1), SEEK_SET);
                            fread(&last_registry_size, sizeof(int), 1, arq);
                            last_registry_size += cluster_size_free;
                            fseek(arq, -4, SEEK_CUR);
                            fwrite(&last_registry_size, sizeof(int), 1, arq);
                            fseek(arq, last_registry_size, SEEK_CUR);
                            cluster_size_free = CLUSTER_SIZE;
                        }
                        last_registry_inserted = ftell(arq);
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
                            fwrite(&nome_servidor, (strlen(nome_servidor) + 1), 1, arq);
                        }
                        if(cargo_servidor_size > 0)
                        {
                            fwrite(&cargo_servidor_size, sizeof(int), 1, arq);
                            fwrite(&header.tag_campo5, sizeof(char), 1, arq);
                            fwrite(&cargo_servidor, (strlen(cargo_servidor) + 1), 1, arq);
                        }
                        cluster_size_free -= (reg_size + 5); // Calcula o espaço livre na pagina de disco (tamanho do registro, mais a flag de removido).
                    }
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
            printf("%s\n", data_file_name);
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

void get_all_data_file(const char *file_name)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@';
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) == 0)
        {
            arq = fopen(file_name, "r+b");
            if(arq != NULL)
            {
                fread(&header.status, sizeof(header.status), 1, arq);
                if(header.status == '1')
                {
                    fread(&header.topo_lista, sizeof(header.topo_lista), 1, arq);
                    fread(&header.tag_campo1, sizeof(header.tag_campo1), 1, arq);
                    fread(&header.desc_campo1, sizeof(header.desc_campo1), 1, arq);
                    fread(&header.tag_campo2, sizeof(header.tag_campo2), 1, arq);
                    fread(&header.desc_campo2, sizeof(header.desc_campo2), 1, arq);
                    fread(&header.tag_campo3, sizeof(header.tag_campo3),1, arq);
                    fread(&header.desc_campo3, sizeof(header.desc_campo3), 1, arq);
                    fread(&header.tag_campo4, sizeof(header.tag_campo4),1, arq);
                    fread(&header.desc_campo4, sizeof(header.desc_campo4), 1, arq);
                    fread(&header.tag_campo5, sizeof(header.tag_campo5), 1, arq);
                    fread(&header.desc_campo5, sizeof(header.desc_campo5), 1, arq);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '0';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    while(1)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                fread(&removido_token, sizeof(char), 1, arq);
                                total_bytes_readed += sizeof(char);
                                if(feof(arq) != 0)
                                {
                                    break;
                                }
                                else if(removido_token == '-')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&salario_servidor, sizeof(double), 1, arq);
                                    register_bytes_readed += sizeof(double);
                                    fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                                    register_bytes_readed += sizeof(telefone_servidor) - 1;
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
                                            if(tag_campo == header.tag_campo4)
                                            {
                                                nome_servidor_size = var_field_size;
                                                fread(&nome_servidor, (nome_servidor_size - 1) , 1, arq);
                                                register_bytes_readed += nome_servidor_size;
                                            }
                                            else if(tag_campo == header.tag_campo5)
                                            {

                                                cargo_servidor_size = var_field_size;
                                                fread(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                                                register_bytes_readed += cargo_servidor_size;
                                            }
                                        }
                                    }
                                    // printf("BYTES READED: %d\n", register_bytes_readed);
                                    printf("%d ", id_servidor);
                                    if(salario_servidor < 0.0)
                                    {
                                        printf("         ");
                                    }
                                    else
                                    {
                                        printf("%.2lf ", salario_servidor);
                                    }
                                    if(telefone_servidor[0] == '\0')
                                    {
                                        printf("              ");
                                    }
                                    else
                                    {
                                        printf("%s", telefone_servidor);
                                    }
                                    if(nome_servidor_size != 0)
                                    {
                                        printf(" %zd", strlen(nome_servidor));
                                        printf(" %s", nome_servidor);
                                    }
                                    if(cargo_servidor_size != 0)
                                    {
                                        printf(" %zd", strlen(cargo_servidor));
                                        printf(" %s", cargo_servidor);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                    printf("\n");
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                printf("Numero de paginas de disco acessadas: %d\n", disk_pages);
                fclose(arq);
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
    }
}

void search_for_id(const char *file_name, int id)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@', flag_found = 0x01;
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) == 0)
        {
            arq = fopen(file_name, "r+b");
            if(arq != NULL)
            {
                fread(&header.status, sizeof(header.status), 1, arq);
                if(header.status == '1')
                {
                    fread(&header.topo_lista, sizeof(header.topo_lista), 1, arq);
                    fread(&header.tag_campo1, sizeof(header.tag_campo1), 1, arq);
                    fread(&header.desc_campo1, sizeof(header.desc_campo1), 1, arq);
                    fread(&header.tag_campo2, sizeof(header.tag_campo2),1, arq);
                    fread(&header.desc_campo2, sizeof(header.desc_campo2), 1, arq);
                    fread(&header.tag_campo3, sizeof(header.tag_campo3),1, arq);
                    fread(&header.desc_campo3, sizeof(header.desc_campo3), 1, arq);
                    fread(&header.tag_campo4, sizeof(header.tag_campo4),1, arq);
                    fread(&header.desc_campo4, sizeof(header.desc_campo4), 1, arq);
                    fread(&header.tag_campo5, sizeof(header.tag_campo5), 1, arq);
                    fread(&header.desc_campo5, sizeof(header.desc_campo5), 1, arq);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '0';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    while(flag_found != 0x00)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                fread(&removido_token, sizeof(char), 1, arq);
                                total_bytes_readed += sizeof(char);
                                if(feof(arq) != 0)
                                {
                                    break;
                                }
                                else if(removido_token == '-')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    if(id_servidor == id) //  Se o valor do campo for igual ao valor a ser buscado, leia o resto do registro.
                                    {
                                        fread(&salario_servidor, sizeof(double), 1, arq);
                                        register_bytes_readed += sizeof(double);
                                        fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                                        register_bytes_readed += sizeof(telefone_servidor) - 1;
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
                                                if(tag_campo == header.tag_campo4)
                                                {
                                                    nome_servidor_size = var_field_size;
                                                    fread(&nome_servidor, (nome_servidor_size - 1) , 1, arq);
                                                    register_bytes_readed += nome_servidor_size;
                                                }
                                                else if(tag_campo == header.tag_campo5)
                                                {

                                                    cargo_servidor_size = var_field_size;
                                                    fread(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                                                    register_bytes_readed += cargo_servidor_size;
                                                }
                                            }
                                        }
                                        printf("%s: %d\n", header.desc_campo1, id_servidor);
                                        printf("%s: ", header.desc_campo2);
                                        if(salario_servidor <= 0.0)
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%.2lf\n", salario_servidor);
                                        }
                                        printf("%s: ", header.desc_campo3);
                                        if(telefone_servidor[0] == '\0')
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%s\n", telefone_servidor);
                                        }
                                        printf("%s: ", header.desc_campo4);
                                        if(nome_servidor_size != 0)
                                        {
                                            printf("%s\n", nome_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("%s: ", header.desc_campo5);
                                        if(cargo_servidor_size != 0)
                                        {
                                            printf("%s\n", cargo_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        flag_found = 0x00;
                                        break;
                                    }
                                    else
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                if(flag_found != 0x00)
                {
                    printf("Registro inexistente.\n");
                }
                printf("\nNumero de paginas de disco acessadas: %d\n", disk_pages);
                fclose(arq);
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
    }
}

void search_for_salario(const char *file_name, double salario)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@', flag_found = 0x01;
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) == 0)
        {
            arq = fopen(file_name, "r+b");
            if(arq != NULL)
            {
                fread(&header.status, sizeof(header.status), 1, arq);
                if(header.status == '1')
                {
                    fread(&header.topo_lista, sizeof(header.topo_lista), 1, arq);
                    fread(&header.tag_campo1, sizeof(header.tag_campo1), 1, arq);
                    fread(&header.desc_campo1, sizeof(header.desc_campo1), 1, arq);
                    fread(&header.tag_campo2, sizeof(header.tag_campo2),1, arq);
                    fread(&header.desc_campo2, sizeof(header.desc_campo2), 1, arq);
                    fread(&header.tag_campo3, sizeof(header.tag_campo3),1, arq);
                    fread(&header.desc_campo3, sizeof(header.desc_campo3), 1, arq);
                    fread(&header.tag_campo4, sizeof(header.tag_campo4),1, arq);
                    fread(&header.desc_campo4, sizeof(header.desc_campo4), 1, arq);
                    fread(&header.tag_campo5, sizeof(header.tag_campo5), 1, arq);
                    fread(&header.desc_campo5, sizeof(header.desc_campo5), 1, arq);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '0';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    while(1)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                fread(&removido_token, sizeof(char), 1, arq);
                                total_bytes_readed += sizeof(char);
                                if(feof(arq) != 0)
                                {
                                    break;
                                }
                                else if(removido_token == '-')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&salario_servidor, sizeof(double), 1, arq);
                                    register_bytes_readed += sizeof(double);
                                    if(salario_servidor == salario) //  Se o valor do campo for igual ao valor a ser buscado, leia o resto do registro.
                                    {
                                        flag_found = 0x00;
                                        fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                                        register_bytes_readed += sizeof(telefone_servidor) - 1;
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
                                                if(tag_campo == header.tag_campo4)
                                                {
                                                    nome_servidor_size = var_field_size;
                                                    fread(&nome_servidor, (nome_servidor_size - 1) , 1, arq);
                                                    register_bytes_readed += nome_servidor_size;
                                                }
                                                else if(tag_campo == header.tag_campo5)
                                                {

                                                    cargo_servidor_size = var_field_size;
                                                    fread(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                                                    register_bytes_readed += cargo_servidor_size;
                                                }
                                            }
                                        }
                                        printf("%s: %d\n", header.desc_campo1, id_servidor);
                                        printf("%s: ", header.desc_campo2);
                                        if(salario_servidor <= 0.0)
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%.2lf\n", salario_servidor);
                                        }
                                        printf("%s: ", header.desc_campo3);
                                        if(telefone_servidor[0] == '\0')
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%s\n", telefone_servidor);
                                        }
                                        printf("%s: ", header.desc_campo4);
                                        if(nome_servidor_size != 0)
                                        {
                                            printf("%s\n", nome_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("%s: ", header.desc_campo5);
                                        if(cargo_servidor_size != 0)
                                        {
                                            printf("%s\n", cargo_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("\n");
                                    }
                                    else
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                if(flag_found != 0x00)
                {
                    printf("Registro inexistente.\n");
                }
                printf("Numero de paginas de disco acessadas: %d\n", disk_pages);
                fclose(arq);
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
    }
}

void search_for_telefone(const char *file_name, const char *telefone)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@', flag_found = 0x01;
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) == 0)
        {
            arq = fopen(file_name, "r+b");
            if(arq != NULL)
            {
                fread(&header.status, sizeof(header.status), 1, arq);
                if(header.status == '1')
                {
                    fread(&header.topo_lista, sizeof(header.topo_lista), 1, arq);
                    fread(&header.tag_campo1, sizeof(header.tag_campo1), 1, arq);
                    fread(&header.desc_campo1, sizeof(header.desc_campo1), 1, arq);
                    fread(&header.tag_campo2, sizeof(header.tag_campo2),1, arq);
                    fread(&header.desc_campo2, sizeof(header.desc_campo2), 1, arq);
                    fread(&header.tag_campo3, sizeof(header.tag_campo3),1, arq);
                    fread(&header.desc_campo3, sizeof(header.desc_campo3), 1, arq);
                    fread(&header.tag_campo4, sizeof(header.tag_campo4),1, arq);
                    fread(&header.desc_campo4, sizeof(header.desc_campo4), 1, arq);
                    fread(&header.tag_campo5, sizeof(header.tag_campo5), 1, arq);
                    fread(&header.desc_campo5, sizeof(header.desc_campo5), 1, arq);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '0';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    while(1)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                fread(&removido_token, sizeof(char), 1, arq);
                                total_bytes_readed += sizeof(char);
                                if(feof(arq) != 0)
                                {
                                    break;
                                }
                                else if(removido_token == '-')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&salario_servidor, sizeof(double), 1, arq);
                                    register_bytes_readed += sizeof(double);
                                    fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                                    register_bytes_readed += sizeof(telefone_servidor) - 1;
                                    if(strcmp(telefone_servidor, telefone) == 0) //  Se o valor do campo for igual ao valor a ser buscado, leia o resto do registro.
                                    {
                                        flag_found = 0x00;
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
                                                if(tag_campo == header.tag_campo4)
                                                {
                                                    nome_servidor_size = var_field_size;
                                                    fread(&nome_servidor, (nome_servidor_size - 1) , 1, arq);
                                                    register_bytes_readed += nome_servidor_size;
                                                }
                                                else if(tag_campo == header.tag_campo5)
                                                {

                                                    cargo_servidor_size = var_field_size;
                                                    fread(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                                                    register_bytes_readed += cargo_servidor_size;
                                                }
                                            }
                                        }
                                        printf("%s: %d\n", header.desc_campo1, id_servidor);
                                        printf("%s: ", header.desc_campo2);
                                        if(salario_servidor <= 0.0)
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%.2lf\n", salario_servidor);
                                        }
                                        printf("%s: ", header.desc_campo3);
                                        if(telefone_servidor[0] == '\0')
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%s\n", telefone_servidor);
                                        }
                                        printf("%s: ", header.desc_campo4);
                                        if(nome_servidor_size != 0)
                                        {
                                            printf("%s\n", nome_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("%s: ", header.desc_campo5);
                                        if(cargo_servidor_size != 0)
                                        {
                                            printf("%s\n", cargo_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("\n");
                                    }
                                    else
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                if(flag_found != 0x00)
                {
                    printf("Registro inexistente.\n");
                }
                printf("Numero de paginas de disco acessadas: %d\n", disk_pages);
                fclose(arq);
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
    }
}

void search_for_nome(const char *file_name, const char *nome)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@', flag_found = 0x01;
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) == 0)
        {
            arq = fopen(file_name, "r+b");
            if(arq != NULL)
            {
                fread(&header.status, sizeof(header.status), 1, arq);
                if(header.status == '1')
                {
                    fread(&header.topo_lista, sizeof(header.topo_lista), 1, arq);
                    fread(&header.tag_campo1, sizeof(header.tag_campo1), 1, arq);
                    fread(&header.desc_campo1, sizeof(header.desc_campo1), 1, arq);
                    fread(&header.tag_campo2, sizeof(header.tag_campo2),1, arq);
                    fread(&header.desc_campo2, sizeof(header.desc_campo2), 1, arq);
                    fread(&header.tag_campo3, sizeof(header.tag_campo3),1, arq);
                    fread(&header.desc_campo3, sizeof(header.desc_campo3), 1, arq);
                    fread(&header.tag_campo4, sizeof(header.tag_campo4),1, arq);
                    fread(&header.desc_campo4, sizeof(header.desc_campo4), 1, arq);
                    fread(&header.tag_campo5, sizeof(header.tag_campo5), 1, arq);
                    fread(&header.desc_campo5, sizeof(header.desc_campo5), 1, arq);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '0';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    while(1)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                fread(&removido_token, sizeof(char), 1, arq);
                                total_bytes_readed += sizeof(char);
                                if(feof(arq) != 0)
                                {
                                    break;
                                }
                                else if(removido_token == '-')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&salario_servidor, sizeof(double), 1, arq);
                                    register_bytes_readed += sizeof(double);
                                    fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                                    register_bytes_readed += sizeof(telefone_servidor) - 1;
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
                                            if(tag_campo == header.tag_campo4)
                                            {
                                                nome_servidor_size = var_field_size;
                                                fread(&nome_servidor, (nome_servidor_size - 1) , 1, arq);
                                                register_bytes_readed += nome_servidor_size;
                                            }
                                            else if(tag_campo == header.tag_campo5)
                                            {

                                                cargo_servidor_size = var_field_size;
                                                fread(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                                                register_bytes_readed += cargo_servidor_size;
                                            }
                                        }
                                    }
                                    if((nome_servidor_size == 0 && strlen(nome) == 0) || (nome_servidor_size > 0 && strcmp(nome_servidor, nome) == 0)) //  Se o valor do campo for igual ao valor a ser buscado, leia o resto do registro.
                                    {
                                        flag_found = 0x00;
                                        printf("%s: %d\n", header.desc_campo1, id_servidor);
                                        printf("%s: ", header.desc_campo2);
                                        if(salario_servidor <= 0.0)
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%.2lf\n", salario_servidor);
                                        }
                                        printf("%s: ", header.desc_campo3);
                                        if(telefone_servidor[0] == '\0')
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%s\n", telefone_servidor);
                                        }
                                        printf("%s: ", header.desc_campo4);
                                        if(nome_servidor_size != 0)
                                        {
                                            printf("%s\n", nome_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("%s: ", header.desc_campo5);
                                        if(cargo_servidor_size != 0)
                                        {
                                            printf("%s\n", cargo_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("\n");
                                    }
                                    else
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                if(flag_found != 0x00)
                {
                    printf("Registro inexistente.\n");
                }
                printf("Numero de paginas de disco acessadas: %d\n", disk_pages);
                fclose(arq);
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
    }
}

void search_for_cargo(const char *file_name, const char *cargo)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@', flag_found = 0x01;
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1;
    double salario_servidor = 0.0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) == 0)
        {
            arq = fopen(file_name, "r+b");
            if(arq != NULL)
            {
                fread(&header.status, sizeof(header.status), 1, arq);
                if(header.status == '1')
                {
                    fread(&header.topo_lista, sizeof(header.topo_lista), 1, arq);
                    fread(&header.tag_campo1, sizeof(header.tag_campo1), 1, arq);
                    fread(&header.desc_campo1, sizeof(header.desc_campo1), 1, arq);
                    fread(&header.tag_campo2, sizeof(header.tag_campo2),1, arq);
                    fread(&header.desc_campo2, sizeof(header.desc_campo2), 1, arq);
                    fread(&header.tag_campo3, sizeof(header.tag_campo3),1, arq);
                    fread(&header.desc_campo3, sizeof(header.desc_campo3), 1, arq);
                    fread(&header.tag_campo4, sizeof(header.tag_campo4),1, arq);
                    fread(&header.desc_campo4, sizeof(header.desc_campo4), 1, arq);
                    fread(&header.tag_campo5, sizeof(header.tag_campo5), 1, arq);
                    fread(&header.desc_campo5, sizeof(header.desc_campo5), 1, arq);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '0';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    while(1)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                fread(&removido_token, sizeof(char), 1, arq);
                                total_bytes_readed += sizeof(char);
                                if(feof(arq) != 0)
                                {
                                    break;
                                }
                                else if(removido_token == '-')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&salario_servidor, sizeof(double), 1, arq);
                                    register_bytes_readed += sizeof(double);
                                    fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                                    register_bytes_readed += sizeof(telefone_servidor) - 1;
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
                                            if(tag_campo == header.tag_campo4)
                                            {
                                                nome_servidor_size = var_field_size;
                                                fread(&nome_servidor, (nome_servidor_size - 1) , 1, arq);
                                                register_bytes_readed += nome_servidor_size;
                                            }
                                            else if(tag_campo == header.tag_campo5)
                                            {

                                                cargo_servidor_size = var_field_size;
                                                fread(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                                                register_bytes_readed += cargo_servidor_size;
                                            }
                                        }
                                    }
                                    if((cargo_servidor_size == 0 && strlen(cargo) == 0) || (cargo_servidor_size > 0 && strcmp(cargo_servidor, cargo) == 0)) //  Se o valor do campo for igual ao valor a ser buscado, leia o resto do registro.
                                    {
                                        flag_found = 0x00;
                                        printf("%s: %d\n", header.desc_campo1, id_servidor);
                                        printf("%s: ", header.desc_campo2);
                                        if(salario_servidor <= 0.0)
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%.2lf\n", salario_servidor);
                                        }
                                        printf("%s: ", header.desc_campo3);
                                        if(telefone_servidor[0] == '\0')
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        else
                                        {
                                            printf("%s\n", telefone_servidor);
                                        }
                                        printf("%s: ", header.desc_campo4);
                                        if(nome_servidor_size != 0)
                                        {
                                            printf("%s\n", nome_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("%s: ", header.desc_campo5);
                                        if(cargo_servidor_size != 0)
                                        {
                                            printf("%s\n", cargo_servidor);
                                        }
                                        else
                                        {
                                            printf("valor nao declarado\n");
                                        }
                                        printf("\n");
                                    }
                                    else
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                if(flag_found != 0x00)
                {
                    printf("Registro inexistente.\n");
                }
                printf("Numero de paginas de disco acessadas: %d\n", disk_pages);
                fclose(arq);
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
    }
}

void print_file_header(FILE_HEADER header)
{
    // if(header != NULL)
    // {
        printf("%c\n", header.status);
        printf("%ld\n", header.topo_lista);
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
