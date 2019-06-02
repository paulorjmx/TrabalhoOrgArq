/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/


#include "inc/handle_file.h"
#include "inc/func_aux.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

// Estrutura de dados utilizada para guardar o registros lidos de arquivos de dados
typedef struct data_register_t
{
    char nome[150], cargo[120], telefone[15];
    double salario;
    long int encadeamento_lista;
    int id, tamanho_registro, nome_size, cargo_size;
} DATA_REGISTER;

// Registro de cabecalho
typedef struct file_header_t
{
    long int topo_lista;
    char desc_campo1[40], desc_campo2[40], desc_campo3[40], desc_campo4[40], desc_campo5[40];
    char status, tag_campo1, tag_campo2, tag_campo3, tag_campo4, tag_campo5;
} FILE_HEADER;

typedef struct file_list_t
{
    long int byte_offset;
    int reg_size;
} FILE_LIST;


void write_file_header(const char *file_name, FILE_HEADER *header); // Funcao que escreve o arquivo de cabecalho no arquivo de dados binario
void read_file_header(FILE *fp, FILE_HEADER *header); // Funcao que le o registro de cabecalho do arquivo
void init_file_header(FILE_HEADER *header, char *desc); // Funcao que inicializa a estrutura de dados FILE_HEADER
void print_file_header(FILE_HEADER header); // Funcao utilidade que mostra na tela todos os campos do registro de cabecalho
void init_file_list(FILE_LIST *l, int list_size); // Funcao utilizada para inicializar a lista de registros removidos
int binary_search(FILE_LIST *l, int list_size); // Funcao utilizada para buscar na lista de registros removidos, seguindo a abordagem best fit
void insert_full_disk_page(FILE *file, int id, double salario, const char *telefone, char tag_campo4, const char *nome, char tag_campo5, const char *cargo); // Funcao utilizada para inserir o registro em uma pagina de disco
void edit_register(const char *file_name, const char *campo, void *valor_campo, long int comeco_registro, char tag_campo4, char tag_campo5); // Funcao utilizada para atualizar um campo no registro que tem comeco em comeco_registro
void *get_user_clean_input(const char *campo); // Funcao utilizada para entrada do usuario removendo aspas
int compare_data_register(const void *a, const void *b); // Funcao de comparacao utilizada pelo qsort
void write_sorted_file(const char *file_name, FILE_HEADER *header, DATA_REGISTER *data, unsigned int n_items); // Funcao utilizada para escrever registros em um arquivo de dados.
void read_register(FILE *fp, FILE_HEADER *header, DATA_REGISTER *data); // Funcao utilizada para ler um registro para onde 'fp' esta 'apontando'
void write_register(FILE *fp, FILE_HEADER *header, DATA_REGISTER *data);

#define SIZE_FILE_HEADER 214 // Tamanho do cabecalho
#define CLUSTER_SIZE 32000 // Tamanho do cluster em bytes
#define LIST_TOTAL_SIZE 1000 // Tamanho da lista de registros removidos


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
                    fgets(line_readed, sizeof(line_readed), arq_csv); // Recupera a linha do arquivo .csv
                    token = line_readed; // Faz token apontar para a primeira posicao de line_readed
                    if(feof(arq_csv) != 0) // Se a flag de EOF foi "setada", sai do loop
                    {
                        break;
                    }
                    else
                    {
                        reg_size = 34; // Independente do registro, o seu tamanho inicial sempre sera 34 bytes.
                        // A cada passagem no loop, eh necessario zerar essas variaveis temporarias.
                        nome_servidor_size = 0;
                        cargo_servidor_size = 0;
                        memset(&nome_servidor, 0, sizeof(nome_servidor));
                        memset(&cargo_servidor, 0, sizeof(cargo_servidor));

                        // Pega o id do Servidor que foi lido do arquivo .csv e armazena na variavel que sera escrita no arquivo binario.
                        sscanf(token, "%d", &id_servidor);
                        token = strchr(token, ','); // Avança ate a virgula
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
                        if(token[0] == ',') // Se forem duas virgulas seguidas, entao o telefone eh nulo
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
                        if((cluster_size_free - (reg_size + 5)) < 0) // Verifica se nao ha espaco na pagina de disco
                        {
                            for(int i = 0; i < cluster_size_free; i++) // Se nao ha espaco, preenche com lixo
                            {
                                fwrite(&bloat, sizeof(char), 1, arq);
                            }
                            fseek(arq, (last_registry_inserted + 1), SEEK_SET); // Volta o ponteiro do arquivo ate o ultimo registro inserido.
                            fread(&last_registry_size, sizeof(int), 1, arq); // Le o seu tamanho
                            last_registry_size += cluster_size_free; // Atualiza seu tamanho, para incluir na conta o lixo inserido
                            fseek(arq, -4, SEEK_CUR); // Volta 4 bytes para escrever o tamanho atualizado no arquivo
                            fwrite(&last_registry_size, sizeof(int), 1, arq);
                            fseek(arq, last_registry_size, SEEK_CUR); // Volta para a posicao no primeiro byte da nova pagina de disco
                            cluster_size_free = CLUSTER_SIZE;
                        }
                        last_registry_inserted = ftell(arq); // Armazena a posicao do ultimo registro inserido no arquivo binario

                        // Os procedimentos abaixo sao de escrita no arquivo binario
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
                fseek(arq, 0, SEEK_SET);
                header.status = '1';
                fwrite(&header.status, sizeof(char), 1, arq);
                fclose(arq);
            }
            else
            {
                printf("Falha no carregamento do arquivo.\n");
            }
            fclose(arq_csv);
            printf("%s", data_file_name);
        }
        else
        {
            printf("Falha no carregamento do arquivo.\n");
        }
    }
    else
    {
        printf("Falha no carregamento do arquivo.\n");
    }
}

void get_all_data_file(const char *file_name)
{
    /* As variaveis abaixo seguem o mesmo principio do procedimento de criar o arquivo binario, exceto que, neste caso,
      sao para recuperar os campos do registro do arquivo de dados binario */
    FILE_HEADER header;
    FILE *arq = NULL;
    // A variavel flag_r eh 0x00 se nao ha algum registro no arquivo de dados, ou 0x01 caso contrario.
    char telefone_servidor[15], nome_servidor[500], cargo_servidor[200], removido_token = '-', tag_campo = '0', bloat = '@', flag_r = 0x01;
    // disk_pages eh utilizada para contar paginas de disco acessadas.
    // var_field_size eh utilizada em um loop para terminar de ler todo o registro
    int id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1, file_size = 0;
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
                    fseek(arq, 0, SEEK_END);
                    file_size = ftell(arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    file_size = (ftell(arq) - file_size);

                    if(file_size < 0) { flag_r = 0x00; } // Se existerem registros, seta a flag_r como 0x00

                    while(flag_r == 0x00) // Checa se o arquivo tem
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

                                    while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
                                    {
                                        /*
                                            Se o registro eh o utilmo da pagina de disco, entao le-se byte a byte
                                            ate acabar a pagina de disco.
                                        */
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
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += reg_size + sizeof(int);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    if(flag_r >= 0x01)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        printf("Número de páginas de disco acessadas: %d\n", disk_pages);
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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

// As buscas seguem a mesma logica de recuperar todos os dados do arquivo binario.
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
                    if(flag_found != 0x00)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        printf("\nNúmero de páginas de disco acessadas: %d\n", disk_pages);
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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
                    if(flag_found != 0x00)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        printf("Número de páginas de disco acessadas: %d\n", disk_pages);
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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
                    if(flag_found != 0x00)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        printf("Número de páginas de disco acessadas: %d\n", disk_pages);
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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
                    if(flag_found != 0x00)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        printf("Número de páginas de disco acessadas: %d\n", disk_pages);
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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
                    if(flag_found != 0x00)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        printf("Número de páginas de disco acessadas: %d\n", disk_pages);
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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
}


int remove_by_id(const char *file_name, int id)
{
    FILE_HEADER header;
    FILE_LIST list[LIST_TOTAL_SIZE]; // List eh utilizada para guardar os byte offsets dos registros removidos logicamente.
    FILE *arq = NULL;
    // removido_mark serve para marcar o registro como removido.
    // removido_token eh utilizado para ler o primeiro byte do registro
    // flag_found e flag_removed sao utilizados para parar o loop
    char removido_token = '-', removido_mark = '*', bloat = '@', flag_found = 0x01, flag_removed = 0x01;
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, disk_pages = 0, ptr_list = -1;
    long int encadeamento_lista = -1, current_register = 0;
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
                    init_file_list(list, LIST_TOTAL_SIZE);
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
                    disk_pages++;
                    if(header.topo_lista != -1)
                    {
                        list[++ptr_list].byte_offset = header.topo_lista; // Recupera o primeiro elemento da lista de registros removidos.
                        fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Vai ate o primeiro elemento da lista de registros removidos.
                        fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq); // Recupera o tamanho do registro
                        fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq); // Recupera o endereco do segundo no da lista

                        while(list[ptr_list].byte_offset != -1) // Recupera todos os elementos da lista
                        {
                            fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Recupera o proximo endereco onde esta o proximo elemento.
                            fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq);
                            fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq);
                        }
                    }
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

                    while(flag_found != 0x00 && flag_removed != 0x00)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                current_register = ftell(arq);
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
                                    if(id_servidor == id) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        i = ptr_list;
                                        while(i > -1 && reg_size <= list[i].reg_size) // Insere ordenadamente na lista o novo registro removido
                                        {
                                            list[(i + 1)].reg_size = list[i].reg_size;
                                            list[(i + 1)].byte_offset = list[i].byte_offset;
                                            i--;
                                        }
                                        list[(i + 1)].byte_offset = current_register;
                                        list[(i + 1)].reg_size = reg_size;
                                        ptr_list++;
                                        fseek(arq, current_register, SEEK_SET);
                                        fwrite(&removido_mark, sizeof(char), 1, arq);
                                        fseek(arq, 12, SEEK_CUR);
                                        register_bytes_readed = reg_size - 8;
                                        while(register_bytes_readed > 0)
                                        {
                                            fwrite(&bloat, sizeof(char), 1, arq);
                                            register_bytes_readed--;
                                        }
                                        flag_found = 0x00;
                                        flag_removed = 0x00;
                                        break;
                                    }
                                    else
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    if(id_servidor == id)
                                    {
                                        flag_removed = 0x00;
                                        break;
                                    }
                                    else
                                    {
                                        fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro removido
                                    }
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    // Atualiza a lista
                    for(int j = 0; j < ptr_list; j++)
                    {
                        fseek(arq, list[j].byte_offset + 5, SEEK_SET);
                        fwrite(&(list[(j + 1)].byte_offset), sizeof(long int), 1, arq);
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fwrite(&(list[0].byte_offset), sizeof(long int), 1, arq);
                }
                else
                {
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
}

int remove_by_salario(const char *file_name, double salario)
{
    FILE_HEADER header;
    FILE_LIST list[LIST_TOTAL_SIZE]; // List eh utilizada para guardar os byte offsets dos registros removidos logicamente.
    FILE *arq = NULL;
    // removido_mark serve para marcar o registro como removido.
    char removido_token = '-', removido_mark = '*', bloat = '@';
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, disk_pages = 0, ptr_list = -1;
    // current_register e um ponteiro auxiliar para apontar para o comeco de cada registro do arquivo no decorrer do loop
    long int encadeamento_lista = -1, current_register = 0;
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
                    init_file_list(list, LIST_TOTAL_SIZE);
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
                    disk_pages++;
                    if(header.topo_lista != -1)
                    {
                        list[++ptr_list].byte_offset = header.topo_lista; // Recupera o primeiro elemento da lista de registros removidos.
                        fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Vai ate o primeiro elemento da lista de registros removidos.
                        fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq); // Recupera o tamanho do registro
                        fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq); // Recupera o endereco do segundo no da lista

                        while(list[ptr_list].byte_offset != -1) // Recupera todos os elementos da lista
                        {
                            fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Recupera o proximo endereco onde esta o proximo elemento.
                            fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq);
                            fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq);
                        }
                    }
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                current_register = ftell(arq);
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
                                    if(salario_servidor == salario) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        i = ptr_list;
                                        while(i > -1 && reg_size <= list[i].reg_size) // Insere ordenadamente na lista o novo registro removido
                                        {
                                            list[(i + 1)].reg_size = list[i].reg_size; // Desloca o no para a proxima posicao
                                            list[(i + 1)].byte_offset = list[i].byte_offset;
                                            i--;
                                        }
                                        list[(i + 1)].byte_offset = current_register;
                                        list[(i + 1)].reg_size = reg_size;
                                        ptr_list++; // Atualiza o ponteiro da lista.
                                        fseek(arq, current_register, SEEK_SET); // Volta para o comeco do registro
                                        fwrite(&removido_mark, sizeof(char), 1, arq); // Marca como removido.
                                        fseek(arq, 12, SEEK_CUR); // Pula o campo do tamanho do registro e do encadeamento da lista
                                        register_bytes_readed = reg_size - 8;
                                        while(register_bytes_readed > 0)
                                        {
                                            fwrite(&bloat, sizeof(char), 1, arq);
                                            register_bytes_readed--;
                                        }
                                        // register_bytes_readed -= 12; // Retira o tamanho do campo idServidor e do salarioServidor, para preencher diretamente com lixo
                                        // while(register_bytes_readed < reg_size) // Coloca lixo nos campos do registro.
                                        // {
                                        //     fwrite(&bloat, sizeof(char), 1, arq);
                                        //     register_bytes_readed++;
                                        // }
                                    }
                                    else // Se o campo nao for igual o buscado, pula o tamanho do registro.
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                }
                                else if(removido_token == '*') // Se este registro estiver removido, pula ele
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int) + reg_size;
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    // Atualiza a lista
                    for(int j = 0; j < ptr_list; j++)
                    {
                        // printf("BYTE_OFFSET: %ld\n", list[j].byte_offset);
                        // printf("REG_SIZE: %d\n", list[j].reg_size);
                        fseek(arq, list[j].byte_offset + 5, SEEK_SET);
                        fwrite(&list[(j + 1)].byte_offset, sizeof(long int), 1, arq);
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fwrite(&(list[0].byte_offset), sizeof(long int), 1, arq); // Escreve no cabecalho o primeiro no da lista
                    // printf("DISK PAGES: %d\n", disk_pages);
                }
                else
                {
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    return r;
}

int remove_by_telefone(const char *file_name, const char *telefone)
{
    FILE_HEADER header;
    FILE_LIST list[LIST_TOTAL_SIZE]; // List eh utilizada para guardar os byte offsets dos registros removidos logicamente.
    FILE *arq = NULL;
    // removido_mark serve para marcar o registro como removido.
    char telefone_servidor[15], removido_token = '-', removido_mark = '*', bloat = '@';
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, disk_pages = 0, ptr_list = -1;
    // current_register e um ponteiro auxiliar para apontar para o comeco de cada registro do arquivo no decorrer do loop
    long int encadeamento_lista = -1, current_register = 0;
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
                    init_file_list(list, LIST_TOTAL_SIZE);
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
                    disk_pages++;
                    if(header.topo_lista != -1)
                    {
                        list[++ptr_list].byte_offset = header.topo_lista; // Recupera o primeiro elemento da lista de registros removidos.
                        fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Vai ate o primeiro elemento da lista de registros removidos.
                        fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq); // Recupera o tamanho do registro
                        fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq); // Recupera o endereco do segundo no da lista

                        while(list[ptr_list].byte_offset != -1) // Recupera todos os elementos da lista
                        {
                            fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Recupera o proximo endereco onde esta o proximo elemento.
                            fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq);
                            fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq);
                        }
                    }
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                current_register = ftell(arq);
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
                                    register_bytes_readed += (sizeof(telefone_servidor) - 1);
                                    if(strcmp(telefone_servidor, telefone) == 0) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        i = ptr_list;
                                        while(i > -1 && reg_size <= list[i].reg_size) // Insere ordenadamente na lista o novo registro removido
                                        {
                                            list[(i + 1)].reg_size = list[i].reg_size; // Desloca o no para a proxima posicao
                                            list[(i + 1)].byte_offset = list[i].byte_offset;
                                            i--;
                                        }
                                        list[(i + 1)].byte_offset = current_register;
                                        list[(i + 1)].reg_size = reg_size;
                                        ptr_list++; // Atualiza o ponteiro da lista.
                                        fseek(arq, current_register, SEEK_SET); // Volta para o comeco do registro
                                        fwrite(&removido_mark, sizeof(char), 1, arq); // Marca como removido.
                                        fseek(arq, 12, SEEK_CUR); // Pula o campo do tamanho do registro e do encadeamento da lista
                                        register_bytes_readed = reg_size - 8; // Muda o valor para preencher os campos com lixo
                                        while(register_bytes_readed > 0) // Coloca lixo nos campos do registro.
                                        {
                                            fwrite(&bloat, sizeof(char), 1, arq);
                                            register_bytes_readed--;
                                        }
                                    }
                                    else // Se o campo nao for igual o buscado, pula o tamanho do registro.
                                    {
                                        fseek(arq, (reg_size - register_bytes_readed), SEEK_CUR);
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                }
                                else if(removido_token == '*') // Se este registro estiver removido, pula ele
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int) + reg_size;
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    // Atualiza a lista
                    for(int j = 0; j < ptr_list; j++)
                    {
                        // printf("BYTE_OFFSET: %ld\n", list[j].byte_offset);
                        // printf("REG_SIZE: %d\n", list[j].reg_size);
                        fseek(arq, list[j].byte_offset + 5, SEEK_SET);
                        fwrite(&list[(j + 1)].byte_offset, sizeof(long int), 1, arq);
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fwrite(&(list[0].byte_offset), sizeof(long int), 1, arq); // Escreve no cabecalho o primeiro no da lista
                    // printf("DISK PAGES: %d\n", disk_pages);
                }
                else
                {
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
}


int remove_by_nome(const char *file_name, const char *nome)
{
    FILE_HEADER header;
    FILE_LIST list[LIST_TOTAL_SIZE]; // List eh utilizada para guardar os byte offsets dos registros removidos logicamente.
    FILE *arq = NULL;
    // removido_mark serve para marcar o registro como removido.
    char nome_servidor[500], cargo_servidor[200], telefone_servidor[15], removido_token = '-', removido_mark = '*', bloat = '@', trash = '@', tag_campo = '#';
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, var_field_size = 0, disk_pages = 0, ptr_list = -1;
    // current_register e um ponteiro auxiliar para apontar para o comeco de cada registro do arquivo no decorrer do loop
    long int encadeamento_lista = -1, current_register = 0;
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
                    init_file_list(list, LIST_TOTAL_SIZE);
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
                    disk_pages++;
                    if(header.topo_lista != -1)
                    {
                        list[++ptr_list].byte_offset = header.topo_lista; // Recupera o primeiro elemento da lista de registros removidos.
                        fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Vai ate o primeiro elemento da lista de registros removidos.
                        fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq); // Recupera o tamanho do registro
                        fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq); // Recupera o endereco do segundo no da lista

                        while(list[ptr_list].byte_offset != -1) // Recupera todos os elementos da lista
                        {
                            fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Recupera o proximo endereco onde esta o proximo elemento.
                            fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq);
                            fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq);
                        }
                    }
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                memset(nome_servidor, 0x00, sizeof(nome_servidor));
                                memset(telefone_servidor, 0x00, sizeof(telefone_servidor));
                                memset(cargo_servidor, 0x00, sizeof(cargo_servidor));
                                current_register = ftell(arq);
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

                                    while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
                                    {
                                        fread(&trash, sizeof(char), 1, arq);
                                        if(trash == '@')
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
                                                fread(&nome_servidor, (var_field_size - 1) , 1, arq);
                                                register_bytes_readed += var_field_size;
                                            }
                                            else if(tag_campo == header.tag_campo5)
                                            {

                                                fread(&cargo_servidor, (var_field_size - 1), 1, arq);
                                                register_bytes_readed += var_field_size;
                                            }
                                        }
                                    }
                                    if(strcmp(nome_servidor, nome) == 0)
                                    {
                                        // printf("ACHOUT\n");
                                        i = ptr_list;
                                        while(i > -1 && reg_size <= list[i].reg_size) // Insere ordenadamente na lista o novo registro removido
                                        {
                                            list[(i + 1)].reg_size = list[i].reg_size; // Desloca o no para a proxima posicao
                                            list[(i + 1)].byte_offset = list[i].byte_offset;
                                            i--;
                                        }
                                        list[(i + 1)].byte_offset = current_register;
                                        list[(i + 1)].reg_size = reg_size;
                                        ptr_list++; // Atualiza o ponteiro da lista.
                                        fseek(arq, current_register, SEEK_SET);
                                        fwrite(&removido_mark, sizeof(char), 1, arq); // Marca como removido.
                                        fseek(arq, 12, SEEK_CUR); // Pula o campo do tamanho do registro e do encadeamento da lista
                                        register_bytes_readed = reg_size - 8;
                                        while(register_bytes_readed > 0)
                                        {
                                            fwrite(&bloat, sizeof(char), 1, arq);
                                            register_bytes_readed--;
                                        }
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += reg_size + sizeof(int);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    // Atualiza a lista
                    for(int j = 0; j < ptr_list; j++)
                    {
                        // printf("BYTE_OFFSET: %ld\n", list[j].byte_offset);
                        // printf("REG_SIZE: %d\n", list[j].reg_size);
                        fseek(arq, list[j].byte_offset + 5, SEEK_SET);
                        fwrite(&list[(j + 1)].byte_offset, sizeof(long int), 1, arq);
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fwrite(&(list[0].byte_offset), sizeof(long int), 1, arq); // Escreve no cabecalho o primeiro no da lista
                    // printf("DISK PAGES: %d\n", disk_pages);
                }
                else
                {
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
}

int remove_by_cargo(const char *file_name, const char *cargo)
{
    FILE_HEADER header;
    FILE_LIST list[LIST_TOTAL_SIZE]; // List eh utilizada para guardar os byte offsets dos registros removidos logicamente.
    FILE *arq = NULL;
    // removido_mark serve para marcar o registro como removido.
    char nome_servidor[500], cargo_servidor[200], telefone_servidor[15], removido_token = '-', removido_mark = '*', bloat = '@', trash = '@', tag_campo = '#';
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, var_field_size = 0, disk_pages = 0, ptr_list = -1;
    // current_register e um ponteiro auxiliar para apontar para o comeco de cada registro do arquivo no decorrer do loop
    long int encadeamento_lista = -1, current_register = 0;
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
                    init_file_list(list, LIST_TOTAL_SIZE);
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
                    disk_pages++;
                    if(header.topo_lista != -1)
                    {
                        list[++ptr_list].byte_offset = header.topo_lista; // Recupera o primeiro elemento da lista de registros removidos.
                        fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Vai ate o primeiro elemento da lista de registros removidos.
                        fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq); // Recupera o tamanho do registro
                        fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq); // Recupera o endereco do segundo no da lista

                        while(list[ptr_list].byte_offset != -1) // Recupera todos os elementos da lista
                        {
                            fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Recupera o proximo endereco onde esta o proximo elemento.
                            fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq);
                            fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq);
                        }
                    }
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                memset(nome_servidor, 0x00, sizeof(nome_servidor));
                                memset(cargo_servidor, 0x00, sizeof(cargo_servidor));
                                current_register = ftell(arq);
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

                                    while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
                                    {
                                        fread(&trash, sizeof(char), 1, arq);
                                        if(trash == '@')
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
                                                fread(&nome_servidor, (var_field_size - 1) , 1, arq);
                                                register_bytes_readed += var_field_size;
                                            }
                                            else if(tag_campo == header.tag_campo5)
                                            {

                                                fread(&cargo_servidor, (var_field_size - 1), 1, arq);
                                                register_bytes_readed += var_field_size;
                                            }
                                        }
                                    }
                                    if(strcmp(cargo_servidor, cargo) == 0)
                                    {
                                        i = ptr_list;
                                        while(i > -1 && reg_size <= list[i].reg_size) // Insere ordenadamente na lista o novo registro removido
                                        {
                                            list[(i + 1)].reg_size = list[i].reg_size; // Desloca o no para a proxima posicao
                                            list[(i + 1)].byte_offset = list[i].byte_offset;
                                            i--;
                                        }
                                        list[(i + 1)].byte_offset = current_register;
                                        list[(i + 1)].reg_size = reg_size;
                                        ptr_list++; // Atualiza o ponteiro da lista.
                                        fseek(arq, current_register, SEEK_SET);
                                        fwrite(&removido_mark, sizeof(char), 1, arq); // Marca como removido.
                                        fseek(arq, 12, SEEK_CUR); // Pula o campo do tamanho do registro e do encadeamento da lista
                                        register_bytes_readed = reg_size - 8;
                                        while(register_bytes_readed > 0)
                                        {
                                            fwrite(&bloat, sizeof(char), 1, arq);
                                            register_bytes_readed--;
                                        }
                                    }
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed = reg_size + sizeof(int);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    // Atualiza a lista
                    for(int j = 0; j < ptr_list; j++)
                    {
                        // printf("BYTE_OFFSET: %ld\n", list[j].byte_offset);
                        // printf("REG_SIZE: %d\n", list[j].reg_size);
                        fseek(arq, list[j].byte_offset + 5, SEEK_SET);
                        fwrite(&list[(j + 1)].byte_offset, sizeof(long int), 1, arq);
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    fwrite(&(list[0].byte_offset), sizeof(long int), 1, arq); // Escreve no cabecalho o primeiro no da lista
                    // printf("DISK PAGES: %d\n", disk_pages);
                }
                else
                {
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    return r;
}

int insert_bin(const char *file_name, int id, double salario, const char *telefone, const char *nome, const char *cargo)
{
    FILE_HEADER header;
    FILE_LIST list[LIST_TOTAL_SIZE];
    FILE *arq = NULL;
    char nome_servidor[200], cargo_servidor[500], telefone_servidor[15], removido_token = '-',  byte = '-', bloat = '@';
    int  r = 0, new_reg_size = 34, reg_size = 0, disk_pages = 0, ptr_list = -1, nome_servidor_size = 0, cargo_servidor_size = 0, min = -1, diff_result = 0, j = 0;
    long int encadeamento_lista = -1, file_ptr = -1, file_size = 0, last_disk_page_size = 0;
    double qt_disk_pages = 0.0, total_disk_pages = 0.0;
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
                    if(strlen(nome) > 0)
                    {
                        strcpy(nome_servidor, nome);
                        nome_servidor_size = strlen(nome) + 2;
                        new_reg_size += nome_servidor_size + 4;
                    }
                    if(strlen(cargo) > 0)
                    {
                        strcpy(cargo_servidor, cargo);
                        cargo_servidor_size = strlen(cargo) + 2;
                        new_reg_size += cargo_servidor_size + 4;
                    }
                    if(strlen(telefone) == 0)
                    {
                        telefone_servidor[0] = '\0';
                        for(int i = 1; i < sizeof(telefone_servidor); i++)
                        {
                            telefone_servidor[i] = '@';
                        }
                    }
                    else
                    {
                        strcpy(telefone_servidor, telefone);
                    }
                    init_file_list(list, LIST_TOTAL_SIZE);
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
                    disk_pages++;
                    if(header.topo_lista != -1)
                    {
                        list[++ptr_list].byte_offset = header.topo_lista; // Recupera o primeiro elemento da lista de registros removidos.
                        fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Vai ate o primeiro elemento da lista de registros removidos.
                        fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq); // Recupera o tamanho do registro
                        fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq); // Recupera o endereco do segundo no da lista

                        while(list[ptr_list].byte_offset != -1) // Recupera todos os elementos da lista
                        {
                            fseek(arq, list[ptr_list].byte_offset + 1, SEEK_SET); // Recupera o proximo endereco onde esta o proximo elemento.
                            fread(&(list[ptr_list].reg_size), sizeof(int), 1, arq);
                            fread(&(list[++ptr_list].byte_offset), sizeof(long int), 1, arq);
                        }
                        for(int i = 0; i < ptr_list; i++) // Busca na lista o registro seguindo a abordagem best-fit
                        {
                            diff_result = list[i].reg_size - new_reg_size;
                            if(diff_result >= 0) // Se o registro a ser inserido cabe no espaco do registro removido.
                            {
                                if(min != -1)
                                {
                                    if(diff_result < (list[min].reg_size - new_reg_size))
                                    {
                                        min = i;
                                    }
                                }
                                else
                                {
                                    min = i;
                                }
                            }
                        }
                        if(min != -1)
                        {
                            fseek(arq, list[min].byte_offset, SEEK_SET);
                            fwrite(&removido_token, sizeof(char), 1, arq);
                            fseek(arq, sizeof(int), SEEK_CUR); // Nao escreve o tamanho do novo registro
                            fwrite(&encadeamento_lista, sizeof(long int), 1, arq);
                            fwrite(&id, sizeof(int), 1, arq);
                            fwrite(&salario, sizeof(double), 1, arq);
                            fwrite(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, arq);
                            if(nome_servidor_size > 0)
                            {
                                fwrite(&nome_servidor_size, sizeof(int), 1, arq);
                                fwrite(&header.tag_campo4, sizeof(char), 1, arq);
                                fwrite(&nome_servidor, (nome_servidor_size - 1), 1, arq);
                            }
                            if(cargo_servidor_size > 0)
                            {
                                fwrite(&cargo_servidor_size, sizeof(int), 1, arq);
                                fwrite(&header.tag_campo5, sizeof(char), 1, arq);
                                fwrite(&cargo_servidor, (cargo_servidor_size - 1), 1, arq);
                            }
                            list[min].byte_offset = -1;
                        }
                        else // Se nao achou na lista um registro que caiba o registro a ser inserido
                        {
                            insert_full_disk_page(arq, id, salario, telefone, header.tag_campo4, nome, header.tag_campo5, cargo);
                        }
                    }
                    else
                    {
                        // Lista vazia
                        insert_full_disk_page(arq, id, salario, telefone, header.tag_campo4, nome, header.tag_campo5, cargo);
                    }
                    // Atualiza a lista
                    for(int k = 0; k < ptr_list; k++)
                    {
                        if(list[k].byte_offset != -1)
                        {
                            // printf("BYTE_OFFSET: %ld\n", list[k].byte_offset);
                            // printf("REG_SIZE: %d\n", list[k].reg_size);
                            fseek(arq, list[k].byte_offset + 5, SEEK_SET);
                            if(list[(k + 1)].byte_offset == -1)
                            {
                                fwrite(&list[(k + 2)].byte_offset, sizeof(long int), 1, arq);
                            }
                            else
                            {
                                fwrite(&list[(k + 1)].byte_offset, sizeof(long int), 1, arq);
                            }
                        }
                    }
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    for(j = 0; (j < ptr_list) && (list[j].byte_offset == -1); j++) {  }
                    fwrite(&(list[j].byte_offset), sizeof(long int), 1, arq); // Escreve no cabecalho o primeiro no da lista
                    // printf("DISK PAGES: %d\n", disk_pages);
                }
                else
                {
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
            fclose(arq);
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    return r;
}

void insert_full_disk_page(FILE *file, int id, double salario, const char *telefone, char tag_campo4, const char *nome, char tag_campo5, const char *cargo)
{
    char telefone_servidor[15], nome_servidor[200], cargo_servidor[500], removido_token = '-', byte = '-', bloat = '@';
    int new_reg_size = 34, reg_size = 0, nome_servidor_size = 0, cargo_servidor_size = 0;
    long int encadeamento_lista = -1, file_ptr = -1, file_size = 0, last_disk_page_size = 0;
    double qt_disk_pages = 0.0, total_disk_pages = 0.0;

    if(file != NULL)
    {
        if(strlen(nome) > 0)
        {
            strcpy(nome_servidor, nome);
            nome_servidor_size = strlen(nome) + 2;
            new_reg_size += nome_servidor_size + 4;
        }
        if(strlen(cargo) > 0)
        {
            strcpy(cargo_servidor, cargo);
            cargo_servidor_size = strlen(cargo) + 2;
            new_reg_size += cargo_servidor_size + 4;
        }
        if(strlen(telefone) == 0)
        {
            telefone_servidor[0] = '\0';
            for(int i = 1; i < sizeof(telefone_servidor); i++)
            {
                telefone_servidor[i] = '@';
            }
        }
        else
        {
            strcpy(telefone_servidor, telefone);
        }
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        qt_disk_pages = file_size / 32000.0;
        modf(qt_disk_pages, &total_disk_pages);
        fseek(file, (total_disk_pages * CLUSTER_SIZE), SEEK_SET); // Vai para o comeco do primeiro registro da ultima pagina de disco
        last_disk_page_size = file_size - ftell(file); // Calcula o tamanho da ultima pagina de disco
        // Eh necessario verificar se o registro cabe na ultima pagina de disco
        if((last_disk_page_size + new_reg_size) > CLUSTER_SIZE) // Se o registro nao cabe na ultima pagina de disco
        {
            while(1) // Loop para ir ate o ultimo registro
            {
                fread(&byte, sizeof(char), 1, file); // Le o primeiro byte do registro
                if(feof(file) != 0)
                {
                    break;
                }
                else
                {
                    file_ptr = ftell(file); // Armazena a posicao do ultimo registro
                    fread(&reg_size, sizeof(int), 1, file); // reg_size tem o tamanho do ultimo registro
                    fseek(file, reg_size, SEEK_CUR);
                }
            }
            while(last_disk_page_size < CLUSTER_SIZE)
            {
                fwrite(&bloat, sizeof(char), 1, file); // Escreve lixo no fim do registro
                last_disk_page_size++;
                reg_size++; // Atualiza o tamanho do ultimo registro da ultima pagina de disco.
            }
            fseek(file, file_ptr, SEEK_SET); // Vai ate o ultimo registro da ultima pagina de disco;
            fwrite(&reg_size, sizeof(int), 1, file); // Atualiza o tamanho do ultimo registro da ultima pagina de disco.
        }
        fseek(file, 0, SEEK_END); // Vai ate o fim do arquivo
        // Faz a insercao do registro novo.
        fwrite(&removido_token, sizeof(char), 1, file);
        fwrite(&new_reg_size, sizeof(int), 1, file);
        fwrite(&encadeamento_lista, sizeof(long int), 1, file);
        fwrite(&id, sizeof(int), 1, file);
        fwrite(&salario, sizeof(double), 1, file);
        fwrite(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, file);
        if(nome_servidor_size > 0)
        {
            fwrite(&nome_servidor_size, sizeof(int), 1, file);
            fwrite(&tag_campo4, sizeof(char), 1, file);
            fwrite(&nome_servidor, (nome_servidor_size - 1), 1, file);
        }
        if(cargo_servidor_size > 0)
        {
            fwrite(&cargo_servidor_size, sizeof(int), 1, file);
            fwrite(&tag_campo5, sizeof(char), 1, file);
            fwrite(&cargo_servidor, (cargo_servidor_size - 1), 1, file);
        }
    }
    else
    {
        printf("Falha no processamento do arquivo.\n");
    }
}

int edit_by_id(const char *file_name, int id, const char *campo)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    // removido_token eh utilizado para ler o primeiro byte do registro
    // O ponteiro para void eh necessario para passar parametros para a funcao edit register
    void *valor_campo = NULL;
    char trash = '-', tag_campo = '#', removido_token = '-', bloat = '@', flag_found = 0x01, flag_removed = 0x01;
    char telefone_servidor[15], nome_servidor[200], cargo_servidor[500];
    int disk_pages = 0, ptr_list = -1, var_field_size = 0;
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, cargo_servidor_size = 0, nome_servidor_size = 0;
    long int encadeamento_lista = -1, current_register = 0;
    double salario_servidor = 0.0;
    // Le o novo valor que o campo a ser editado tera
    valor_campo = get_user_clean_input(campo);
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
                    disk_pages++;
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

                    while(flag_found != 0x00 && flag_removed != 0x00)
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE)
                            {
                                current_register = ftell(arq);
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
                                    if(id_servidor == id) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        // Edita o registro.
                                        fseek(arq, 0, SEEK_SET);
                                        header.status = '1';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        fclose(arq);
                                        edit_register(file_name, campo, valor_campo, current_register, header.tag_campo4, header.tag_campo5);
                                        arq = fopen(file_name, "r+b");
                                        header.status = '0';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        flag_found = 0x00;
                                        break;
                                    }
                                    else
                                    {
                                        fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro removido
                                    }
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += sizeof(int);
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    fread(&id_servidor, sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    if(id_servidor == id)
                                    {
                                        flag_removed = 0x00;
                                        break;
                                    }
                                    else
                                    {
                                        fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro removido
                                    }
                                }
                                register_bytes_readed = 0;
                                cargo_servidor_size = 0;
                                nome_servidor_size = 0;
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
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    free(valor_campo);
    return r;
}

int edit_by_salario(const char *file_name, double salario, const char *campo)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    // removido_token eh utilizado para ler o primeiro byte do registro
    // O ponteiro para void eh necessario para passar parametros para a funcao edit register
    void *valor_campo = NULL;
    char trash = '-', tag_campo = '#', removido_token = '-', bloat = '@';
    char telefone_servidor[15], nome_servidor[200], cargo_servidor[500];
    int disk_pages = 0, ptr_list = -1, var_field_size = 0;
    int r = 0, i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, cargo_servidor_size = 0, nome_servidor_size = 0;
    long int encadeamento_lista = -1, current_register = 0;
    double salario_servidor = 0.0;
    // Le o novo valor que o campo a ser editado tera
    valor_campo = get_user_clean_input(campo);
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
                    disk_pages++;
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                current_register = ftell(arq);
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
                                    if(salario_servidor == salario)
                                    {
                                        // Edita o registro.
                                        fseek(arq, 0, SEEK_SET);
                                        header.status = '1';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        fclose(arq);
                                        // Eh necessario fechar o arquivo para que o edit_register possa alterar o registro.
                                        edit_register(file_name, campo, valor_campo, current_register, header.tag_campo4, header.tag_campo5);
                                        arq = fopen(file_name, "r+b");
                                        header.status = '0';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                    }
                                    fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                                register_bytes_readed = 0;
                                cargo_servidor_size = 0;
                                nome_servidor_size = 0;
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
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    free(valor_campo);
    return r;
}

int edit_by_telefone(const char *file_name, const char *telefone, const char *campo)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    // removido_token eh utilizado para ler o primeiro byte do registro
    // O ponteiro para void eh necessario para passar parametros para a funcao edit register
    void *valor_campo = NULL;
    char trash = '-', tag_campo = '#', removido_token = '-', bloat = '@';
    char telefone_servidor[15], nome_servidor[200], cargo_servidor[500];
    int r = 0, disk_pages = 0, ptr_list = -1, var_field_size = 0, new_id = 0;
    int i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, cargo_servidor_size = 0, nome_servidor_size = 0;
    long int encadeamento_lista = -1, current_register = 0;
    double salario_servidor = 0.0;
    // Le o novo valor que o campo a ser editado tera
    valor_campo = get_user_clean_input(campo);
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
                    disk_pages++;
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                memset(telefone_servidor, 0x00, sizeof(telefone_servidor));
                                current_register = ftell(arq);
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
                                    // printf("PHONE: %s\n", telefone_servidor);
                                    if(strcmp(telefone_servidor, telefone) == 0) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        // Edita o registro.
                                        fseek(arq, 0, SEEK_SET);
                                        header.status = '1';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        fclose(arq);
                                        // Eh necessario fechar o arquivo para que o edit_register possa alterar o registro.
                                        edit_register(file_name, campo, valor_campo, current_register, header.tag_campo4, header.tag_campo5);
                                        arq = fopen(file_name, "r+b");
                                        header.status = '0';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                    }
                                    fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                                register_bytes_readed = 0;
                                cargo_servidor_size = 0;
                                nome_servidor_size = 0;
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
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    free(valor_campo);
    return r;
}

int edit_by_nome(const char *file_name, const char *nome, const char *campo)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    // removido_token eh utilizado para ler o primeiro byte do registro
    // O ponteiro para void eh necessario para passar parametros para a funcao edit register
    void *valor_campo = NULL;
    char trash = '-', tag_campo = '#', removido_token = '-', bloat = '@';
    char telefone_servidor[15], nome_servidor[200], cargo_servidor[500];
    int r = 0, disk_pages = 0, ptr_list = -1, var_field_size = 0;
    int i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, cargo_servidor_size = 0, nome_servidor_size = 0;
    long int encadeamento_lista = -1, current_register = 0;
    double salario_servidor = 0.0;
    // Le o novo valor que o campo a ser editado tera
    valor_campo = get_user_clean_input(campo);
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
                    disk_pages++;
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                memset(nome_servidor, 0x00, sizeof(nome_servidor));
                                memset(cargo_servidor, 0x00, sizeof(cargo_servidor));
                                memset(telefone_servidor, 0x00, sizeof(telefone_servidor));
                                current_register = ftell(arq);
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
                                    while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
                                    {
                                        fread(&trash, sizeof(char), 1, arq);
                                        if(trash == '@')
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
                                    if(strcmp(nome_servidor, nome) == 0) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        // Edita o registro.
                                        fseek(arq, 0, SEEK_SET);
                                        header.status = '1';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        fclose(arq);
                                        // Eh necessario fechar o arquivo para que o edit_register possa alterar o registro.
                                        edit_register(file_name, campo, valor_campo, current_register, header.tag_campo4, header.tag_campo5);
                                        arq = fopen(file_name, "r+b");
                                        header.status = '0';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                    }
                                    fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                                register_bytes_readed = 0;
                                cargo_servidor_size = 0;
                                nome_servidor_size = 0;
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
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    free(valor_campo);
    return r;
}

int edit_by_cargo(const char *file_name, const char *cargo, const char *campo)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    // O ponteiro para void eh necessario para passar parametros para a funcao edit register
    void *valor_campo = NULL;
    char trash = '-', tag_campo = '#', removido_token = '-', bloat = '@';
    char telefone_servidor[15], nome_servidor[200], cargo_servidor[500];
    int r = 0, disk_pages = 0, ptr_list = -1, var_field_size = 0;
    int i = 0, id_servidor = 0, reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, cargo_servidor_size = 0, nome_servidor_size = 0;
    long int encadeamento_lista = -1, current_register = 0;
    double salario_servidor = 0.0;
    valor_campo = get_user_clean_input(campo);
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
                    disk_pages++;
                    fseek(arq, CLUSTER_SIZE, SEEK_SET); // Volta ao comeco do arquivo

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
                                memset(nome_servidor, 0x00, sizeof(nome_servidor));
                                memset(cargo_servidor, 0x00, sizeof(cargo_servidor));
                                memset(telefone_servidor, 0x00, sizeof(telefone_servidor));
                                current_register = ftell(arq);
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
                                    while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
                                    {
                                        fread(&trash, sizeof(char), 1, arq);
                                        if(trash == '@')
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
                                    if(strcmp(cargo_servidor, cargo) == 0) //  Se o valor do campo for igual ao valor a ser buscado e nao foi removido.
                                    {
                                        // Edita o registro.
                                        fseek(arq, 0, SEEK_SET);
                                        header.status = '1';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        fclose(arq);
                                        // Eh necessario fechar o arquivo para que o edit_register possa alterar o registro.
                                        edit_register(file_name, campo, valor_campo, current_register, header.tag_campo4, header.tag_campo5);
                                        arq = fopen(file_name, "r+b");
                                        header.status = '0';
                                        fwrite(&header.status, sizeof(header.status), 1, arq);
                                        fseek(arq, current_register + reg_size + sizeof(int) + sizeof(char), SEEK_SET); // Pula o registro
                                    }
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                                register_bytes_readed = 0;
                                cargo_servidor_size = 0;
                                nome_servidor_size = 0;
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
                    r = -1;
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(arq);
            }
            else
            {
                r = -1;
                printf("Falha no processamento do arquivo.\n");
            }
        }
        else
        {
            r = -1;
            printf("Falha no processamento do arquivo.\n");
        }
    }
    free(valor_campo);
    return r;
}

void edit_register(const char *file_name, const char *campo, void *valor_campo, long int comeco_registro, char tag_campo4, char tag_campo5)
{
    FILE *file = NULL;
    int r = 0, id = 0, reg_size = 0, nome_servidor_size = 0, cargo_servidor_size = 0, register_bytes_readed = 0, var_field_size = 0, reg_empty_space = 0;
    char telefone_servidor[15], cargo_servidor[500], nome_servidor[200], var_value[500], bloat = '@', removido_token = '-', byte = '@', tag_campo = '#';
    long int encadeamento_lista = -1;
    double salario = 0.0;
    memset(&telefone_servidor, 0x00, sizeof(telefone_servidor));
    memset(&cargo_servidor, 0x00, sizeof(cargo_servidor));
    memset(&nome_servidor, 0x00, sizeof(nome_servidor));
    file = fopen(file_name, "r+b");
    if(file != NULL)
    {
        fseek(file, comeco_registro, SEEK_SET);
        fread(&removido_token, sizeof(char), 1, file);
        fread(&reg_size, sizeof(int), 1, file);
        fread(&encadeamento_lista, sizeof(long int), 1, file);
        register_bytes_readed += sizeof(long int);
        if(strcmp(campo, "idServidor") == 0)
        {
            id = *((int *) valor_campo);
            fwrite(&id, sizeof(int), 1, file);
        }
        else if(strcmp(campo, "salarioServidor") == 0)
        {
            fseek(file, 4, SEEK_CUR);
            salario = *((double *) valor_campo);
            fwrite(&salario, sizeof(double), 1, file);
        }
        else if(strcmp(campo, "telefoneServidor") == 0)
        {
            fseek(file, 12, SEEK_CUR);
            if(strcmp(valor_campo, "NULO") == 0)
            {
                telefone_servidor[0] = '\0';
                for(int i = 1; i < sizeof(telefone_servidor); i++)
                {
                    telefone_servidor[i] = '@';
                }
            }
            else
            {
                strcpy(telefone_servidor, valor_campo);
            }
            fwrite(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, file);
        }
        else // Caso seja algum campo de tamanho variavel
        {
            fread(&id, sizeof(int), 1, file);
            register_bytes_readed += sizeof(int);
            fread(&salario, sizeof(double), 1, file);
            register_bytes_readed += sizeof(double);
            fread(&telefone_servidor, (sizeof(telefone_servidor) - 1), 1, file);
            register_bytes_readed += sizeof(telefone_servidor) - 1;
            while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
            {
                fread(&byte, sizeof(char), 1, file);
                if(byte == '@')
                {
                    register_bytes_readed++;
                    reg_empty_space++;
                }
                else
                {
                    fseek(file, -1, SEEK_CUR);
                    fread(&var_field_size, sizeof(int), 1, file);
                    register_bytes_readed += sizeof(int);
                    fread(&tag_campo, sizeof(char), 1, file);
                    register_bytes_readed += sizeof(char);
                    if(tag_campo == tag_campo4)
                    {
                        nome_servidor_size = var_field_size;
                        fread(&nome_servidor, (nome_servidor_size - 1) , 1, file);
                        register_bytes_readed += (nome_servidor_size - 1);
                    }
                    else if(tag_campo == tag_campo5)
                    {
                        cargo_servidor_size = var_field_size;
                        fread(&cargo_servidor, (cargo_servidor_size - 1), 1, file);
                        register_bytes_readed += (cargo_servidor_size - 1);
                    }
                }
            }
            if(strcmp(campo, "nomeServidor") == 0)
            {
                strcpy(var_value, valor_campo);
                fseek(file, comeco_registro + 39, SEEK_SET); // Pula para a posicao onde fica o comeco do campo nome
                if(nome_servidor_size > 0) // Se o registro contem um campo nomeServidor
                {
                    if(strcmp(var_value, "NULO") == 0)
                    {
                        if(cargo_servidor_size > 0) // Se o cargo servidor nao for nulo, desloca ele e escreve no arquivo
                        {
                            fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                            fwrite(&tag_campo5, sizeof(char), 1, file);
                            fwrite(&cargo_servidor, (cargo_servidor_size - 1), 1, file);
                        }
                        reg_empty_space = (nome_servidor_size + 4); // Atualiza o espaco vazio dentro do registro
                    }
                    else if(strlen(nome_servidor) < strlen(var_value))
                    {
                        reg_empty_space = reg_empty_space + strlen(nome_servidor) - strlen(var_value); // Calcula o espaco vazio restante, se ocorrer a edicao
                        if(reg_empty_space < 0) // Se nao ha espaco no registro para inserir
                        {
                            r = -1;
                        }
                        else // Se houver espaco no registro para inserir o novo nome
                        {
                            nome_servidor_size = strlen(var_value) + 2; // Atualiza o tamanho do nome
                            // Escreve o novo nome no registro
                            fwrite(&nome_servidor_size, sizeof(int), 1, file);
                            fwrite(&tag_campo4, sizeof(char), 1, file);
                            fwrite(&var_value, (nome_servidor_size - 1), 1, file);
                            if(cargo_servidor_size > 0) // Se o registro tem um campo cargoServidor
                            {
                                // Atualiza a posicao, no registro, onde o campo cargoServidor ficara, escrevendo-o novamente
                                fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                                fwrite(&tag_campo5, sizeof(char), 1, file);
                                fwrite(&cargo_servidor, (cargo_servidor_size - 1), 1, file);
                            }
                        }
                    }
                    else if(strlen(nome_servidor) > strlen(var_value)) // Se o nome a ser inserido eh menor do que o existente
                    {
                        nome_servidor_size = strlen(var_value) + 2; // Atualiza o tamanho do nome
                        fwrite(&nome_servidor_size, sizeof(int), 1, file);
                        fwrite(&tag_campo4, sizeof(char), 1, file);
                        fwrite(&var_value, (nome_servidor_size - 1), 1, file);
                        if(cargo_servidor_size > 0)
                        {
                            fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                            fwrite(&tag_campo5, sizeof(char), 1, file);
                            fwrite(&cargo_servidor, (cargo_servidor_size - 1), 1, file);
                        }
                        reg_empty_space = strlen(nome_servidor) - strlen(var_value); // Atualiza o espaco vazio dentro do registro
                    }
                    else // Se o tamanho do novo nomeServidor a ser inserido eh igual ao antigo
                    {
                        fseek(file, 5, SEEK_CUR);
                        fwrite(&var_value, (nome_servidor_size - 1), 1, file);
                    }
                    // Se sobrou algum espaco dentro do registro, preenche com lixo
                    while(reg_empty_space > 0)
                    {
                        fwrite(&bloat, sizeof(char), 1, file);
                        reg_empty_space--;
                    }
                }
                else // Caso o registro nao tenha um campo nomeServidor
                {
                    if(strcmp(var_value, "NULO") != 0)
                    {
                        reg_empty_space -= (strlen(var_value) + 6); // Calcula o espaco vazio restante do registro, se ocorrer a edicao
                        if(reg_empty_space >= 0) // Se o registro possui espaco para o nomeServidor
                        {
                            nome_servidor_size = strlen(var_value) + 2;
                            fwrite(&nome_servidor_size, sizeof(int), 1, file);
                            fwrite(&tag_campo4, sizeof(char), 1, file);
                            fwrite(&var_value, (nome_servidor_size - 1), 1, file);
                            if(cargo_servidor_size > 0) // Se o registro tem um campo cargoServidor
                            {
                                fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                                fwrite(&tag_campo5, sizeof(char), 1, file);
                                fwrite(&cargo_servidor, (cargo_servidor_size - 1), 1, file);
                            }
                        }
                        else
                        {
                            r = -1;
                        }
                    }
                }
            }
            else if(strcmp(campo, "cargoServidor") == 0)
            {
                strcpy(var_value, valor_campo);
                if(nome_servidor_size > 0) // Se o registro tem um campo nomeServidor
                {
                    // Pula para a posicao onde fica o comeco do campo cargoServidor contabilizando o tamanho do campo nomeServidor
                    fseek(file, comeco_registro + 43 + nome_servidor_size, SEEK_SET);
                }
                else
                {
                    // Pula para a posicao onde fica o comeco do campo cargoServidor nao contabilizando o tamanho do campo nomeServidor
                    fseek(file, comeco_registro + 39, SEEK_SET);
                }
                if(cargo_servidor_size > 0) // Se o registro contem um campo cargoServidor
                {
                    if(strcmp(var_value, "NULO") == 0)
                    {
                        reg_empty_space = (cargo_servidor_size + 4); // Atualiza o espaco vazio dentro do registro
                    }
                    else if(strlen(cargo_servidor) < strlen(var_value))
                    {
                        reg_empty_space = reg_empty_space + strlen(cargo_servidor) - strlen(var_value); // Calcula o espaco vazio restante, se ocorrer a edicao
                        if(reg_empty_space < 0) // Se nao ha espaco no registro para inserir
                        {
                            r = -2;
                        }
                        else // Se houver espaco no registro para inserir o novo cargoServidor
                        {
                            cargo_servidor_size = strlen(var_value) + 2; // Atualiza o tamanho do campo cargoServidor
                            // Escreve o novo valor que o campo cargoServidor tera
                            fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                            fwrite(&tag_campo5, sizeof(char), 1, file);
                            fwrite(&var_value, (cargo_servidor_size - 1), 1, file);
                        }
                    }
                    else if(strlen(cargo_servidor) > strlen(var_value)) // Se o cargo a ser inserido eh menor do que o existente
                    {
                        cargo_servidor_size = strlen(var_value) + 2; // Atualiza o tamanho do cargoServidor
                        fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                        fwrite(&tag_campo5, sizeof(char), 1, file);
                        fwrite(&var_value, (cargo_servidor_size - 1), 1, file);
                        reg_empty_space = strlen(cargo_servidor) - strlen(var_value); // Atualiza o espaco vazio dentro do registro
                    }
                    else // Se o tamanho do novo cargoServidor a ser inserido eh igual ao antigo
                    {
                        fseek(file, 5, SEEK_CUR);
                        fwrite(&var_value, (cargo_servidor_size - 1), 1, file);
                    }
                    // Se sobrou algum espaco dentro do registro, preenche com lixo
                    while(reg_empty_space > 0)
                    {
                        fwrite(&bloat, sizeof(char), 1, file);
                        reg_empty_space--;
                    }
                }
                else // Caso o registro nao tenha um campo cargoServidor
                {
                    if(strcmp(var_value, "NULO") != 0)
                    {
                        reg_empty_space -= (strlen(var_value) + 6); // Calcula o espaco vazio restante do registro, se ocorrer a edicao
                        if(reg_empty_space >= 0) // Se o registro possui espaco para o cargoServidor
                        {
                            cargo_servidor_size = strlen(var_value) + 2;
                            fwrite(&cargo_servidor_size, sizeof(int), 1, file);
                            fwrite(&tag_campo5, sizeof(char), 1, file);
                            fwrite(&var_value, (cargo_servidor_size - 1), 1, file);
                        }
                        else
                        {
                            r = -2;
                        }
                    }
                }
            }
        }
        fclose(file);
        if(r < 0)
        {
            remove_by_id(file_name, id);
            if(r == -1)
            {
                insert_bin(file_name, id, salario, telefone_servidor, var_value, cargo_servidor);
            }
            else
            {
                insert_bin(file_name, id, salario, telefone_servidor, nome_servidor, var_value);
            }
        }
    }
}

void init_file_list(FILE_LIST *l, int list_size)
{
    if(l != NULL)
    {
        for(int i = 0; i < list_size; i++)
        {
            l[i].byte_offset = -1;
            l[i].reg_size = INT_MAX;
        }
    }
}

int binary_search(FILE_LIST *l, int list_size)
{
    if(l != NULL)
    {

    }
}

void *get_user_clean_input(const char *campo)
{
    void *valor_campo = NULL;
    int new_id = 0;
    char new_salario_string[300], new_telefone_servidor[15], new_nome_servidor[200], new_cargo_servidor[500];
    double new_salario_servidor = 0.0;
    memset(new_salario_string, 0x00, sizeof(new_salario_string));
    memset(new_telefone_servidor, 0x00, sizeof(new_telefone_servidor));
    memset(new_nome_servidor, 0x00, sizeof(new_nome_servidor));
    memset(new_cargo_servidor, 0x00, sizeof(new_cargo_servidor));
    if(campo != NULL)
    {
        if(strcmp(campo, "idServidor") == 0)
        {
            scanf("%d", &new_id);
            valor_campo = malloc(sizeof(int));
            *((int *) valor_campo) = new_id;
        }
        else if(strcmp(campo, "salarioServidor") == 0)
        {
            scanf("%s", new_salario_string);
            if(strcmp(new_salario_string, "NULO") == 0)
            {
                new_salario_servidor = -1.0;
            }
            else
            {
                new_salario_servidor = strtod(new_salario_string, NULL);
            }
            valor_campo = malloc(sizeof(double));
            *((double *) valor_campo) = new_salario_servidor;
        }
        else if(strcmp(campo, "telefoneServidor") == 0)
        {
            scanf("%s", new_telefone_servidor);
            if(new_telefone_servidor[0] != 'N' && new_telefone_servidor[0] != 'n')
            {
                sscanf(new_telefone_servidor, " %c%14[^\"]", &new_telefone_servidor[0], new_telefone_servidor); // Retira as aspas
            }
            valor_campo = malloc(strlen(new_telefone_servidor) + 1);
            strcpy(valor_campo, new_telefone_servidor);
        }
        else if(strcmp(campo, "nomeServidor") == 0)
        {
            scanf(" %200[^\n\r]", new_nome_servidor);
            if(new_nome_servidor[0] != 'N' && new_nome_servidor[0] != 'n')
            {
                sscanf(new_nome_servidor, " %c%200[^\"]", &new_nome_servidor[0], new_nome_servidor); // Retira as aspas
            }
            if(strcmp(new_nome_servidor, "NULO") == 0)
            {
                memset(new_nome_servidor, 0x00, sizeof(new_nome_servidor));
            }
            valor_campo = malloc(strlen(new_nome_servidor) + 1);
            strcpy(valor_campo, new_nome_servidor);
        }
        else if(strcmp(campo, "cargoServidor") == 0)
        {
            scanf(" %500[^\n\r]", new_cargo_servidor);
            if(new_cargo_servidor[0] != 'N' && new_cargo_servidor[0] != 'n')
            {
                sscanf(new_cargo_servidor, " %c%500[^\"]", &new_cargo_servidor[0], new_cargo_servidor); // Retira as aspas
            }
            if(strcmp(new_cargo_servidor, "NULO") == 0)
            {
                memset(new_cargo_servidor, 0x00, sizeof(new_cargo_servidor));
            }
            valor_campo = malloc(strlen(new_cargo_servidor) + 1);
            strcpy(valor_campo, new_cargo_servidor);
        }
    }
    return valor_campo;
}

int sort_data_file(const char *file_name, const char *sorted_file_name)
{
    FILE_HEADER header;
    FILE *arq = NULL;
    // in_data eh onde estao os registros que serao gravados no arquivo ordenada
    DATA_REGISTER in_data[6000];
    // A variavel flag_r eh 0x00 se nao ha algum registro no arquivo de dados, ou 0x01 caso contrario.
    char removido_token = '-', tag_campo = '0', bloat = '@', flag_r = 0x01;
    // disk_pages eh utilizada para contar paginas de disco acessadas.
    // var_field_size eh utilizada em um loop para terminar de ler todo o registro
    // ptr eh utilizado como ponteiro para a estrutura in_data
    // file_size eh utilizada para verificar se ha algum registro no arquivo de dados nao ordenado
    // A variavel 'r' eh o retorno da funcao
    int ptr = 0, r = -1;
    int reg_size = 0, total_bytes_readed = 0, register_bytes_readed = 0, nome_servidor_size = 0, cargo_servidor_size = 0, var_field_size = 0, disk_pages = 0;
    long int encadeamento_lista = -1, file_size = 0;
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
                    // Le o cabecalho do arquivo de dados nao ordenado
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
                    fseek(arq, 0, SEEK_END);
                    file_size = ftell(arq);
                    fseek(arq, CLUSTER_SIZE, SEEK_SET);
                    disk_pages++;
                    file_size = (ftell(arq) - file_size);

                    if(file_size < 0) { flag_r = 0x00; } // Se existerem registros, seta a flag_r como 0x00

                    while(flag_r == 0x00) // Checa se o arquivo tem
                    {
                        if(feof(arq) != 0)
                        {
                            break;
                        }
                        else
                        {
                            while(total_bytes_readed < CLUSTER_SIZE) // Enquanto houver registros na pagina de disco
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
                                    in_data[ptr].tamanho_registro = reg_size;
                                    fread(&encadeamento_lista, sizeof(long int), 1, arq);
                                    register_bytes_readed += sizeof(long int);
                                    in_data[ptr].encadeamento_lista = -1;
                                    fread(&(in_data[ptr].id), sizeof(int), 1, arq);
                                    register_bytes_readed += sizeof(int);
                                    fread(&(in_data[ptr].salario), sizeof(double), 1, arq);
                                    register_bytes_readed += sizeof(double);
                                    fread(&(in_data[ptr].telefone), (sizeof(in_data[ptr].telefone) - 1), 1, arq);
                                    register_bytes_readed += sizeof(in_data[ptr].telefone) - 1;
                                    in_data[ptr].nome_size = 0;
                                    in_data[ptr].cargo_size = 0;
                                    while(register_bytes_readed < reg_size) // Loop utilizado para ler o resto do registro
                                    {
                                        /*
                                            Se o registro eh o utilmo da pagina de disco, entao le-se byte a byte
                                            ate acabar a pagina de disco.
                                        */
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
                                                fread(&(in_data[ptr].nome), (nome_servidor_size - 1) , 1, arq);
                                                register_bytes_readed += nome_servidor_size;
                                                in_data[ptr].nome_size = nome_servidor_size;
                                            }
                                            else if(tag_campo == header.tag_campo5)
                                            {
                                                cargo_servidor_size = var_field_size;
                                                fread(&(in_data[ptr].cargo), (cargo_servidor_size - 1), 1, arq);
                                                register_bytes_readed += cargo_servidor_size;
                                                in_data[ptr].cargo_size = cargo_servidor_size;
                                            }
                                        }
                                    }
                                    ptr++; // Avanca o ponteiro para apontar para a proxima posicao livre
                                    total_bytes_readed += reg_size;
                                    register_bytes_readed = 0;
                                    cargo_servidor_size = 0;
                                    nome_servidor_size = 0;
                                }
                                else if(removido_token == '*')
                                {
                                    fread(&reg_size, sizeof(int), 1, arq);
                                    total_bytes_readed += reg_size + sizeof(int);
                                    fseek(arq, reg_size, SEEK_CUR);
                                }
                            }
                            total_bytes_readed = 0;
                            disk_pages++;
                        }
                    }
                    // Ordena os registros
                    qsort(in_data, ptr, sizeof(DATA_REGISTER), compare_data_register);
                    fseek(arq, 0, SEEK_SET);
                    header.status = '1';
                    fwrite(&header.status, sizeof(header.status), 1, arq);
                    if(flag_r >= 0x01)
                    {
                        printf("Registro inexistente.\n");
                    }
                    else
                    {
                        // Grava os registros, ordenados, no arquivo 'sorted_file_name'
                        write_sorted_file(sorted_file_name, &header, in_data, ptr);
                        r = 0;
                    }
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
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
    return r;
}

int compare_data_register(const void *a, const void *b)
{
    return ((DATA_REGISTER *) a)->id - ((DATA_REGISTER *) b)->id;
}

void write_sorted_file(const char *file_name, FILE_HEADER *header, DATA_REGISTER *data, unsigned int n_items)
{
    FILE *arq = NULL;
    // 'removido_token' e 'bloat' sao constantes utilizadas para escrever no arquivo de dados ordenado
    const char removido_token = '-', bloat = '@';
    // 'update_reg_size' eh utilizado para armazenar o tamanho do ultimo registro da pagina de disco
    // 'cluster_size_free' eh utilizado para saber quanto espaco livre tem na pagina de disco.
    // 'reg_size' eh utilizado para calcular o tamanho do registro a ser armazenado
    int cluster_size_free = CLUSTER_SIZE, reg_size = 0, update_reg_size = 0;
    // 'last_registry_inserted' eh um 'ponteiro' para o ultimo registro armazenado no arquivo de dados ordenado
    long int last_registry_inserted = 0;
    if(file_name != NULL)
    {
        if(access(file_name, F_OK) != 0) // Checa se o arquivo nao existe
        {
            header->topo_lista = -1;
            write_file_header(file_name, header); // Escreve o cabecalho no arquivo de dados ordenado
            arq = fopen(file_name, "r+b");
            fseek(arq, CLUSTER_SIZE, SEEK_SET);
            if(arq != NULL)
            {
                for(int i = 0; i < n_items; i++) // Loop para escrever os registros no arquivo de dados ordenado
                {
                    reg_size = 34;
                    if(data[i].nome_size > 0)
                    {
                        reg_size += 4 + data[i].nome_size;
                    }
                    if(data[i].cargo_size > 0)
                    {
                        reg_size += 4 + data[i].cargo_size;
                    }
                    if((cluster_size_free - (reg_size + 5)) < 0) // Se o registro nao cabe na pagina de disco
                    {
                        for(int j = 0; j < cluster_size_free; j++) // Preenche com lixo o restante da pagina de disco
                        {
                            fwrite(&bloat, sizeof(char), 1, arq);
                        }
                        fseek(arq, (last_registry_inserted + 1), SEEK_SET); // Vai ate o ultimo registro inserido
                        fread(&update_reg_size, sizeof(int), 1, arq); // Le o tamanho deste registro
                        update_reg_size += cluster_size_free; // Atualiza o tamanho do registro
                        fseek(arq, (last_registry_inserted + 1), SEEK_SET);
                        fwrite(&update_reg_size, sizeof(int), 1, arq); // escreve o tamanho do registro atualizado
                        cluster_size_free = CLUSTER_SIZE; // Comeca uma nova pagina de disco
                        fseek(arq, 0, SEEK_END);
                    }
                    last_registry_inserted = ftell(arq); // Pega o byte offset do ultimo registro inserido
                    cluster_size_free -= (reg_size + 5); // Diminui o tamanho do espco livre que tem na pag. de disco
                    // Escreve os campos dos registros
                    fwrite(&removido_token, sizeof(char), 1, arq);
                    fwrite(&reg_size, sizeof(int), 1, arq);
                    fwrite(&(data[i].encadeamento_lista), sizeof(long int), 1, arq);
                    fwrite(&(data[i].id), sizeof(int), 1, arq);
                    fwrite(&(data[i].salario), sizeof(double), 1, arq);
                    fwrite(&(data[i].telefone), (sizeof(data[i].telefone) - 1), 1, arq);
                    if(data[i].nome_size > 0)
                    {
                        fwrite(&(data[i].nome_size), sizeof(int), 1, arq);
                        fwrite(&(header->tag_campo4), sizeof(char), 1, arq);
                        fwrite(&(data[i].nome), (data[i].nome_size - 1), 1, arq);
                    }
                    if(data[i].cargo_size > 0)
                    {
                        fwrite(&(data[i].cargo_size), sizeof(int), 1, arq);
                        fwrite(&(header->tag_campo5), sizeof(char), 1, arq);
                        fwrite(&(data[i].cargo), (data[i].cargo_size - 1), 1, arq);
                    }
                }
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

void read_register(FILE *fp, FILE_HEADER *header, DATA_REGISTER *data)
{
    char bloat = '@', tag_campo = '-', end_page = 0x00;
    int register_bytes_readed = 0, var_field_size = 0, reg_size = 0, true_reg_size = 0;
    if(fp != NULL)
    {
        fread(&reg_size, sizeof(int), 1, fp);
        data->tamanho_registro = reg_size;
        fread(&(data->encadeamento_lista), sizeof(long int), 1, fp);
        register_bytes_readed += sizeof(long int);
        fread(&(data->id), sizeof(int), 1, fp);
        register_bytes_readed += sizeof(int);
        fread(&(data->salario), sizeof(double), 1, fp);
        register_bytes_readed += sizeof(double);
        fread(&(data->telefone), (sizeof(data->telefone) - 1), 1, fp);
        register_bytes_readed += sizeof(data->telefone) - 1;
        data->nome_size = 0;
        data->cargo_size = 0;
        true_reg_size = register_bytes_readed;

        while(register_bytes_readed < data->tamanho_registro)
        {
            fread(&bloat, sizeof(char), 1, fp);
            if(feof(fp) != 0)
            {
                break;
            }
            else
            {
                if(bloat == '@')
                {
                    end_page = 0x01;
                    register_bytes_readed++;
                }
                else
                {
                    fseek(fp, -1, SEEK_CUR);
                    fread(&var_field_size, sizeof(int), 1, fp);
                    register_bytes_readed += sizeof(int);
                    fread(&tag_campo, sizeof(char), 1, fp);
                    register_bytes_readed += sizeof(char);
                    if(tag_campo == header->tag_campo4)
                    {
                        data->nome_size = var_field_size;
                        fread(&(data->nome), (data->nome_size - 1), 1, fp);
                        register_bytes_readed += var_field_size;
                        true_reg_size += var_field_size + 4;
                    }
                    else if(tag_campo == header->tag_campo5)
                    {
                        data->cargo_size = var_field_size;
                        fread(&(data->cargo), (data->cargo_size - 1), 1, fp);
                        register_bytes_readed += var_field_size;
                        true_reg_size += var_field_size + 4;
                    }
                }
            }
        }
    }
    if(end_page == 0x01)
    {
        data->tamanho_registro = true_reg_size;
    }
}

void write_register(FILE *fp, FILE_HEADER *header, DATA_REGISTER *data)
{
    char removido_token = '-';
    if(fp != NULL)
    {
        fwrite(&removido_token, sizeof(char), 1, fp);
        fwrite(&(data->tamanho_registro), sizeof(int), 1, fp);
        fwrite(&(data->encadeamento_lista), sizeof(long int), 1, fp);
        fwrite(&(data->id), sizeof(int), 1, fp);
        fwrite(&(data->salario), sizeof(double), 1, fp);
        fwrite(&(data->telefone), (sizeof(data->telefone) - 1), 1, fp);
        if(data->nome_size > 0)
        {
            fwrite((&data->nome_size), sizeof(int), 1, fp);
            fwrite(&(header->tag_campo4), sizeof(char), 1, fp);
            fwrite(&(data->nome), (data->nome_size - 1), 1, fp);
        }
        if(data->cargo_size > 0)
        {
            fwrite((&data->cargo_size), sizeof(int), 1, fp);
            fwrite(&(header->tag_campo5), sizeof(char), 1, fp);
            fwrite(&(data->cargo), (data->cargo_size - 1), 1, fp);
        }
    }
}

int merging_data_file(const char *file_name1, const char *file_name2, const char *merged_file_name)
{
    char removido_token1 = '-', removido_token2 = '-';
    const char bloat = '@';
    FILE *arq1 = NULL, *arq2 = NULL, *out_arq = NULL;
    int r = -1, cluster_size_free = CLUSTER_SIZE, reg_size = 0, i = 0, qt_bloat = 0;
    long int last_registry_inserted = 0, ptr_file1 = 0, ptr_file2 = 0;
    FILE_HEADER header1, header2;
    DATA_REGISTER in_data1, in_data2, *reg_to_write = NULL;
    arq1 = fopen(file_name1, "r+b");
    arq2 = fopen(file_name2, "r+b");
    if(arq1 != NULL && arq2 != NULL)
    {
        read_file_header(arq1, &header1);
        read_file_header(arq2, &header2);
        if(header1.status == '1' && header2.status == '1') // Se os dois arquivos estao consistentes
        {
            rewind(arq1);
            rewind(arq2);
            header1.status = '0';
            header2.status = '0';
            fwrite(&header1.status, sizeof(header1.status), 1, arq1);
            fwrite(&header2.status, sizeof(header2.status), 1, arq2);
            header1.topo_lista = -1;
            header1.status = '1';
            write_file_header(merged_file_name, &header1); // Escreve o cabecalho do arquivo 1 no novo arquivo
            fseek(arq1, CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que o registro de cabecalho estao (arquivo 1)
            fseek(arq2, CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que o registro de cabecalho estao (arquivo 2)
            out_arq = fopen(merged_file_name, "r+b"); // Cria o novo arquivo
            fseek(out_arq, CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que o registro de cabecalho estao (arquivo novo)
            // Insercao no arquivo novo realizando merging
            while(1) // Loop para insercao de registros no novo arquivo
            {
                reg_size = 0;
                ptr_file1 = ftell(arq1); // Guarda o byte offset do comeco do registro (arquivo 1)
                ptr_file2 = ftell(arq2); // Guarda o byte offset do comeco do registro (arquivo 2)
                fread(&removido_token1, sizeof(char), 1, arq1); // Le o primeiro byte do registro (arquivo 1)
                fread(&removido_token2, sizeof(char), 1, arq2); // Le o primeiro byte do registro (arquivo 2)

                if(feof(arq1) != 0 || feof(arq2) != 0) // Se um dos dois arquivos chegou ao fim, sai do loop
                {
                    break;
                }
                else
                {
                    read_register(arq1, &header1, &in_data1); // Le o registro do arquivo 1
                    read_register(arq2, &header2, &in_data2); // Le o registro do arquivo 2
                    in_data1.encadeamento_lista = -1;
                    in_data2.encadeamento_lista = -1;
                    fseek(arq1, (ptr_file1 + 1), SEEK_SET);
                    fseek(arq2, (ptr_file2 + 1), SEEK_SET);
                    fread(&reg_size, sizeof(int), 1, arq1);
                    fseek(arq1, reg_size, SEEK_CUR);
                    fread(&reg_size, sizeof(int), 1, arq2);
                    fseek(arq2, reg_size, SEEK_CUR);

                    qt_bloat = cluster_size_free;
                    if(removido_token1 == '-' && removido_token2 == '-')
                    {
                        if(in_data1.id < in_data2.id)
                        {
                            reg_to_write = &in_data1;
                            fseek(arq2, ptr_file2, SEEK_SET);
                        }
                        else if(in_data2.id < in_data1.id)
                        {
                            reg_to_write = &in_data2;
                            fseek(arq1, ptr_file1, SEEK_SET);
                        }
                        else
                        {
                            reg_to_write = &in_data1;
                        }
                    }
                    else if(removido_token1 == '-') // Se o registro do arquivo 2 esta removido
                    {
                        reg_to_write = &in_data1;
                    }
                    else if(removido_token2 == '-') // Se o registro do arquivo 1 esta removido
                    {
                        reg_to_write = &in_data2;
                    }
                    else
                    {
                        reg_to_write = NULL;
                    }

                    if(reg_to_write != NULL)
                    {
                        cluster_size_free -= (reg_to_write->tamanho_registro + 5);
                        if(cluster_size_free < 0)
                        {
                            for(int j = 0; j < qt_bloat; j++)
                            {
                                fwrite(&bloat, sizeof(char), 1, out_arq);
                            }
                            fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                            fread(&reg_size, sizeof(int), 1, out_arq);
                            reg_size += qt_bloat;
                            fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                            fwrite(&reg_size, sizeof(int), 1, out_arq);
                            fseek(out_arq, reg_size, SEEK_CUR);
                            cluster_size_free = CLUSTER_SIZE - (reg_to_write->tamanho_registro + 5);
                        }
                        last_registry_inserted = ftell(out_arq);
                        write_register(out_arq, &header1, reg_to_write);
                    }
                }
                i++;
            }

            if((reg_to_write->id == in_data1.id) && (feof(arq2) == 0)) // Se faltou escrever o registro do arquivo 2, significa que o tem mais registros para serem inseridos e todos sao do arquivo 2
            {
                qt_bloat = cluster_size_free;
                cluster_size_free -= (in_data2.tamanho_registro + 5);
                if(cluster_size_free < 0)
                {
                    for(int j = 0; j < qt_bloat; j++)
                    {
                        fwrite(&bloat, sizeof(char), 1, out_arq);
                    }
                    fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                    fread(&reg_size, sizeof(char), 1, out_arq);
                    reg_size += qt_bloat;
                    fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                    fwrite(&reg_size, sizeof(int), 1, out_arq);
                    fseek(out_arq, reg_size, SEEK_CUR);
                    cluster_size_free = CLUSTER_SIZE - (in_data2.tamanho_registro + 5);
                }
                last_registry_inserted = ftell(out_arq);
                write_register(out_arq, &header1, &in_data2);
                while(feof(arq2))
                {
                    qt_bloat = cluster_size_free;
                    ptr_file2 = ftell(arq2);
                    fread(&removido_token2, sizeof(char), 1, out_arq);
                    if(feof(arq2) != 0)
                    {
                        break;
                    }
                    else
                    {
                        read_register(arq2, &header2, &in_data2);
                        in_data2.encadeamento_lista = -1;
                        fseek(arq2, (ptr_file2 + 1), SEEK_SET);
                        fread(&reg_size, sizeof(int), 1, arq2);
                        fseek(arq2, reg_size, SEEK_CUR);
                        if(removido_token2 == '-')
                        {
                            cluster_size_free -= (in_data2.tamanho_registro + 5);
                            if(cluster_size_free < 0)
                            {
                                for(int j = 0; j < qt_bloat; j++)
                                {
                                    fwrite(&bloat, sizeof(char), 1, out_arq);
                                }
                                fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                                fread(&reg_size, sizeof(char), 1, out_arq);
                                reg_size += qt_bloat;
                                fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                                fwrite(&reg_size, sizeof(int), 1, out_arq);
                                fseek(out_arq, reg_size, SEEK_CUR);
                                cluster_size_free = CLUSTER_SIZE - (in_data2.tamanho_registro + 5);
                            }
                            last_registry_inserted = ftell(out_arq);
                            write_register(out_arq, &header1, &in_data2);
                        }
                    }
                }
            }
            else if((reg_to_write->id == in_data2.id) && (feof(arq1) == 0))// Se faltou escrever o registro do arquivo 1, significa que o tem mais registros para serem inseridos e todos sao do arquivo 1
            {
                qt_bloat = cluster_size_free;
                cluster_size_free -= (in_data1.tamanho_registro + 5);
                if(cluster_size_free < 0)
                {
                    for(int j = 0; j < qt_bloat; j++)
                    {
                        fwrite(&bloat, sizeof(char), 1, out_arq);
                    }
                    fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                    fread(&reg_size, sizeof(char), 1, out_arq);
                    reg_size += qt_bloat;
                    fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                    fwrite(&reg_size, sizeof(int), 1, out_arq);
                    fseek(out_arq, reg_size, SEEK_CUR);
                    cluster_size_free = CLUSTER_SIZE - (in_data1.tamanho_registro + 5);
                }
                last_registry_inserted = ftell(out_arq);
                write_register(out_arq, &header1, &in_data1);
                while(feof(arq1))
                {
                    qt_bloat = cluster_size_free;
                    ptr_file1 = ftell(arq1);
                    fread(&removido_token1, sizeof(char), 1, out_arq);
                    if(feof(arq1) != 0)
                    {
                        break;
                    }
                    else
                    {
                        read_register(arq1, &header1, &in_data1);
                        in_data1.encadeamento_lista = -1;
                        fseek(arq1, (ptr_file1 + 1), SEEK_SET);
                        fread(&reg_size, sizeof(int), 1, arq1);
                        fseek(arq1, reg_size, SEEK_CUR);
                        if(removido_token1 == '-')
                        {
                            cluster_size_free -= (in_data1.tamanho_registro + 5);
                            if(cluster_size_free < 0)
                            {
                                for(int j = 0; j < qt_bloat; j++)
                                {
                                    fwrite(&bloat, sizeof(char), 1, out_arq);
                                }
                                fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                                fread(&reg_size, sizeof(char), 1, out_arq);
                                reg_size += qt_bloat;
                                fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                                fwrite(&reg_size, sizeof(int), 1, out_arq);
                                fseek(out_arq, reg_size, SEEK_CUR);
                                cluster_size_free = CLUSTER_SIZE - (in_data1.tamanho_registro + 5);
                            }
                            last_registry_inserted = ftell(out_arq);
                            write_register(out_arq, &header1, &in_data1);
                        }
                    }
                }
            }
            rewind(arq1);
            rewind(arq2);
            rewind(out_arq);
            header1.status = '1';
            header2.status = '1';
            fwrite(&header1.status, sizeof(header1.status), 1, arq1);
            fwrite(&header2.status, sizeof(header2.status), 1, arq2);
            fwrite(&header1.status, sizeof(header1.status), 1, out_arq);
            r = 0;
            fclose(out_arq);
        }
        else
        {
            printf("Falha no processamento do arquivo.\n");
        }
        fclose(arq1);
        fclose(arq2);
    }
    else
    {
        printf("Falha no processamento do arquivo.\n");
    }
    return r;
}

int matching_data_file(const char *file_name1, const char *file_name2, const char *merged_file_name)
{
    char removido_token1 = '-', removido_token2 = '-';
    const char bloat = '@';
    FILE *arq1 = NULL, *arq2 = NULL, *out_arq = NULL;
    int r = -1, cluster_size_free = CLUSTER_SIZE, reg_size = 0, i = 0, qt_bloat = 0;
    long int last_registry_inserted = 0, ptr_file1 = 0, ptr_file2 = 0;
    FILE_HEADER header1, header2;
    DATA_REGISTER in_data1, in_data2, *reg_to_write = NULL;
    arq1 = fopen(file_name1, "r+b");
    arq2 = fopen(file_name2, "r+b");
    if(arq1 != NULL && arq2 != NULL)
    {
        read_file_header(arq1, &header1);
        read_file_header(arq2, &header2);
        if(header1.status == '1' && header2.status == '1') // Se os dois arquivos estao consistentes
        {
            rewind(arq1);
            rewind(arq2);
            header1.status = '0';
            header2.status = '0';
            fwrite(&header1.status, sizeof(header1.status), 1, arq1);
            fwrite(&header2.status, sizeof(header2.status), 1, arq2);
            header1.topo_lista = -1;
            header1.status = '1';
            write_file_header(merged_file_name, &header1); // Escreve o cabecalho do arquivo 1 no novo arquivo
            fseek(arq1, CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que o registro de cabecalho estao (arquivo 1)
            fseek(arq2, CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que o registro de cabecalho estao (arquivo 2)
            out_arq = fopen(merged_file_name, "r+b"); // Cria o novo arquivo
            fseek(out_arq, CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que o registro de cabecalho estao (arquivo novo)
            // Insercao no arquivo novo realizando merging
            while(1) // Loop para insercao de registros no novo arquivo
            {
                reg_size = 0;
                ptr_file1 = ftell(arq1); // Guarda o byte offset do comeco do registro (arquivo 1)
                ptr_file2 = ftell(arq2); // Guarda o byte offset do comeco do registro (arquivo 2)
                fread(&removido_token1, sizeof(char), 1, arq1); // Le o primeiro byte do registro (arquivo 1)
                fread(&removido_token2, sizeof(char), 1, arq2); // Le o primeiro byte do registro (arquivo 2)

                if(feof(arq1) != 0 || feof(arq2) != 0) // Se um dos dois arquivos chegou ao fim, sai do loop
                {
                    break;
                }
                else
                {
                    read_register(arq1, &header1, &in_data1); // Le o registro do arquivo 1
                    read_register(arq2, &header2, &in_data2); // Le o registro do arquivo 2
                    in_data1.encadeamento_lista = -1;
                    in_data2.encadeamento_lista = -1;
                    fseek(arq1, (ptr_file1 + 1), SEEK_SET);
                    fseek(arq2, (ptr_file2 + 1), SEEK_SET);
                    fread(&reg_size, sizeof(int), 1, arq1);
                    fseek(arq1, reg_size, SEEK_CUR);
                    fread(&reg_size, sizeof(int), 1, arq2);
                    fseek(arq2, reg_size, SEEK_CUR);

                    qt_bloat = cluster_size_free;
                    if(removido_token1 == '-' && removido_token2 == '-')
                    {
                        if(in_data1.id < in_data2.id)
                        {
                            reg_to_write = NULL;
                            fseek(arq2, ptr_file2, SEEK_SET);
                        }
                        else if(in_data2.id < in_data1.id)
                        {
                            reg_to_write = NULL;
                            fseek(arq1, ptr_file1, SEEK_SET);
                        }
                        else if(in_data1.id == in_data2.id)
                        {
                            reg_to_write = &in_data1;
                        }
                    }
                    else if(removido_token1 == '-') // Se o registro do arquivo 2 esta removido
                    {
                        reg_to_write = &in_data1;
                    }
                    else if(removido_token2 == '-') // Se o registro do arquivo 1 esta removido
                    {
                        reg_to_write = &in_data2;
                    }
                    else
                    {
                        reg_to_write = NULL;
                    }

                    if(reg_to_write != NULL)
                    {
                        cluster_size_free -= (reg_to_write->tamanho_registro + 5);
                        if(cluster_size_free < 0)
                        {
                            for(int j = 0; j < qt_bloat; j++)
                            {
                                fwrite(&bloat, sizeof(char), 1, out_arq);
                            }
                            fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                            fread(&reg_size, sizeof(int), 1, out_arq);
                            reg_size += qt_bloat;
                            fseek(out_arq, (last_registry_inserted + 1), SEEK_SET);
                            fwrite(&reg_size, sizeof(int), 1, out_arq);
                            fseek(out_arq, reg_size, SEEK_CUR);
                            cluster_size_free = CLUSTER_SIZE - (reg_to_write->tamanho_registro + 5);
                        }
                        last_registry_inserted = ftell(out_arq);
                        write_register(out_arq, &header1, reg_to_write);
                    }
                }
                i++;
            }
            rewind(arq1);
            rewind(arq2);
            rewind(out_arq);
            header1.status = '1';
            header2.status = '1';
            fwrite(&header1.status, sizeof(header1.status), 1, arq1);
            fwrite(&header2.status, sizeof(header2.status), 1, arq2);
            fwrite(&header1.status, sizeof(header1.status), 1, out_arq);
            r = 0;
            fclose(out_arq);
        }
        else
        {
            printf("Falha no processamento do arquivo.\n");
        }
        fclose(arq1);
        fclose(arq2);
    }
    else
    {
        printf("Falha no processamento do arquivo.\n");
    }
    return r;
}

void read_file_header(FILE *fp, FILE_HEADER *header)
{
    if(fp != NULL)
    {
        rewind(fp);
        fread(&header->status, sizeof(header->status), 1, fp);
        fread(&header->topo_lista, sizeof(header->topo_lista), 1, fp);
        fread(&header->tag_campo1, sizeof(header->tag_campo1), 1, fp);
        fread(&header->desc_campo1, sizeof(header->desc_campo1), 1, fp);
        fread(&header->tag_campo2, sizeof(header->tag_campo2), 1, fp);
        fread(&header->desc_campo2, sizeof(header->desc_campo2), 1, fp);
        fread(&header->tag_campo3, sizeof(header->tag_campo3),1, fp);
        fread(&header->desc_campo3, sizeof(header->desc_campo3), 1, fp);
        fread(&header->tag_campo4, sizeof(header->tag_campo4),1, fp);
        fread(&header->desc_campo4, sizeof(header->desc_campo4), 1, fp);
        fread(&header->tag_campo5, sizeof(header->tag_campo5), 1, fp);
        fread(&header->desc_campo5, sizeof(header->desc_campo5), 1, fp);
    }
}
