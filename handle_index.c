/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/

#include "inc/handle_index.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDEX_CLUSTER_SIZE 32000

typedef struct header_index_t
{
    int nroReg;
    char status;
} HEADER_INDEX;

struct index_data_t
{
    long int byteOffset;
    char chaveBusca[120];
};

void init_index_header(HEADER_INDEX *h); // Funcao para inicialiar o registro de cabecalho de um arquivo de indice
void write_index_header(const char *file_name, HEADER_INDEX *h); // Funcao utilizada para escrever o registro de cabecalho do arquivo de indice, criando-o tambem
void read_index_header(const char *file_name, HEADER_INDEX *h); // Funcao utilizada para ler o registro de cabecalho do arquivo de indice
int compare_index_function(const void *a, const void *b); // Funcao utilizada em 'qsort' para ordenar os indices
int *binary_seach_index(INDEX_DATA *data, unsigned int nitems, int *items_finded, const char *nome); // Funcao utilizada para buscar o nome no indice carregado na memoria primaria. Retorna um conjunto de indices


void init_index_header(HEADER_INDEX *h)
{
    if(h != NULL)
    {
        h->nroReg = 0;
        h->status = '1';
    }
}

void read_index_header(const char *file_name, HEADER_INDEX *h)
{
    FILE *index_arq = NULL;
    if(file_name != NULL && h != NULL)
    {
        index_arq = fopen(file_name, "rb");
        if(index_arq != NULL)
        {
            fread(&(h->status), sizeof(char), 1, index_arq);
            fread(&(h->nroReg), sizeof(int), 1, index_arq);
        }
        else
        {
            printf("Falha no processamento do arquivo.\n");
        }
        fclose(index_arq);
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
            for(int i = 5; i < INDEX_CLUSTER_SIZE; i++) // Loop para preencher o restante da pagina de disco com lixo
            {
                fwrite(&bloat, sizeof(char), 1, arq);
            }
        }
        fclose(arq);
    }
}

void create_index_file(const char *file_name, const char *data_file_name)
{
    // 'reg_size' armazena o tamanho do registro que sera lido do arquivo de dados
    // 'register_bytes_readed' armazena a quantidade de bytes lidos do registro do arquivo de dados
    // 'var_field_size' armazena a quantidade de bytes dos campos variaveis (nomeServidor e cargoServidor)
    // 'ptr' aponta para a primeira posicao livre de 'index_data'
    int cluster_size_free = INDEX_CLUSTER_SIZE, reg_size = 0, register_bytes_readed = 0, var_field_size = 0, ptr = 0, qt_r = 0;
    // 'nome' e 'cargo' servem para armazenar os valores do nomeServidor e cargoServidor lidos do arquivo de dados
    char nome[120], cargo[200];
    // 'removido_token' eh para ler o primeiro byte do registro
    // 'status_arq' eh o status do arquivo de dado
    // 'tag_campo' armazena a tag dos campos variaveis
    // 'tag_campo4' e 'tag_campo5' armazenam as tags lidas do registro de cabecalho do arquivo de dados
    char removido_token = '-', status_arq = '0', bloat = '@', tag_campo = '*', tag_campo4 = '#', tag_campo5 = '$';
    // 'index_arq' eh o arquivo de indice secundario fortemente ligado
    // 'arq' eh o arquivo de dados
    FILE *index_arq = NULL, *arq = NULL;
    // 'bo' guar o byte offset do comeco de cada registro lido do arquivo de dados
    long int bo = 0;
    // 'header' eh o header do arquivo de registro
    HEADER_INDEX header;
    // 'index_data' sao os registros que serao inseridos no arquivo de indice secundario fortemente ligado
    INDEX_DATA index_data[6000];
    if((file_name != NULL) && (data_file_name != NULL))
    {
        arq = fopen(data_file_name, "rb");
        if(arq != NULL)
        {
            fread(&status_arq, sizeof(char), 1, arq);
            if(status_arq == '1')
            {
                fseek(arq, 131, SEEK_CUR); // Pula para recuperar a tagCampo4 do arquivo de dados
                fread(&tag_campo4, sizeof(char), 1, arq);
                fseek(arq, 40, SEEK_CUR); // Pula para recuperar a tagCampo4 do arquivo de dados
                fread(&tag_campo5, sizeof(char), 1, arq);
                fseek(arq, INDEX_CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco em que se encontra o cabecalho
                while (1)
                {
                    memset(&index_data[ptr].chaveBusca, '@', sizeof(index_data[ptr].chaveBusca)); // Preenche chaveBusca com '@'
                    register_bytes_readed = 34; // Atribui 34 para pular os campos de tamanho fixo
                    var_field_size = 0;
                    bo = ftell(arq); // Armazena o endereco do comeco do registro
                    fread(&removido_token, sizeof(char), 1, arq); // Le o primeiro byte do registro
                    if(feof(arq) != 0)
                    {
                        break;
                    }
                    else
                    {
                        fread(&reg_size, sizeof(int), 1, arq); // Le o tamanho do registro
                        if(removido_token == '-') // Se o registro nao eh um registro removido
                        {
                            fseek(arq, 34, SEEK_CUR); // Pula para o comeco dos campos variaveis
                            while(register_bytes_readed < reg_size) // Loop para ler os campos de tamanho variavel
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
                                    if(tag_campo == tag_campo4) // Se o campo for do tipo nomeServidor
                                    {
                                        index_data[ptr].byteOffset = bo; // Salva o byte offset do comeco do arquivo na estrutura que sera armazenada no arquivo de indice
                                        fread(nome, (var_field_size - 1), 1, arq);
                                        strcpy(index_data[ptr].chaveBusca, nome); // Salva o nome tambem
                                        ptr++;
                                        qt_r++;
                                    }
                                    else if(tag_campo == tag_campo5) // Se o campo for do tipo cargoServidor
                                    {
                                        fread(cargo, (var_field_size - 1), 1, arq);
                                    }
                                    register_bytes_readed += (var_field_size - 1);
                                }
                            }
                        }
                        else if(removido_token == '*') // Se eh um registro removido
                        {
                            fseek(arq, reg_size, SEEK_CUR); // Pula o registro removido
                        }
                    }
                }
                FILE *t = fopen("binario-2-ge.index", "r+b");
                fread(&header.status, sizeof(char), 1, t);
                fread(&header.nroReg, sizeof(int), 1, t);
                fclose(t);
                printf("ORIGINAL: %d", header.nroReg);
                printf("%d\n", qt_r);
                printf("%d\n", ptr);
                header.status = '0';
                header.nroReg = ptr;
                write_index_header(file_name, &header); // Cria e escreve o arquivo de indice
                index_arq = fopen(file_name, "r+b");
                if(index_arq != NULL)
                {
                    fseek(index_arq, INDEX_CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco que contem o cabecalho
                    qsort(index_data, ptr, sizeof(INDEX_DATA), compare_index_function); // Ordena a estrutura que contem os indices que serao armazenados no arquivo
                    for(int i = 0; i < ptr; i++)
                    {
                        if((cluster_size_free - 128) < 0) // Se nao ha espaco na pagina de disco
                        {
                            for(int j = 0; j < cluster_size_free; j++) // Preenche o resto da pagina de disco com lixo
                            {
                                fwrite(&bloat, sizeof(char), 1, index_arq);
                            }
                            cluster_size_free = INDEX_CLUSTER_SIZE; // Cria uma nova pagina de disco
                        }
                        // Escreve o registro de indice no arquivo de indice
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
                printf("Falha no processamento do arquivo1.\n");
            }
        }
        else
        {
            printf("Falha no processamento do arquivo2.\n");
        }
        fclose(index_arq);
        fclose(arq);
    }
}

long int *search_name_index(const char *file_name, const char *nome, int *items_finded)
{
    // 'ptr_byte_offsets' armazena os bytes offset dos registros de indices encontrados que contem 'nome'
    long int *ptr_byte_offsets = NULL;
    // 'qt_reg' armazena a quantidade de registros carregados na memoria
    // 'qt_indexes' armazena a quantidade que 'ptr_byte_offsets' tem
    // 'ptr_indexes' contem as posicoes de 'index_data' que contem 'nome'
    int qt_reg = 0, *ptr_indexes = NULL, qt_indexes = 0;
    HEADER_INDEX header;
    // 'index_data' armazena o arquivo de indice em memoria primaria
    INDEX_DATA *index_data = NULL;
    *items_finded = 0;
    if(file_name != NULL)
    {
        qt_reg = load_index(file_name, &index_data); // Carrega o indice na memoria
        if(qt_reg != -1)
        {
            ptr_indexes = binary_seach_index(index_data, qt_reg, &qt_indexes, nome); // Busca nos registros de indice que contem 'nome' na memoria primaria
            if(ptr_indexes != NULL)
            {
                ptr_byte_offsets = (long int *) malloc(sizeof(long int) * qt_indexes); // Aloca memoria para os bytes offsets encontrados em 'index_data'
                if(ptr_byte_offsets != NULL)
                {
                    for(int i = 0; i < qt_indexes; i++) // Para cada nome encontrado no indice secundario, recupera o byte offset
                    {
                        ptr_byte_offsets[i] = index_data[ptr_indexes[i]].byteOffset; // Armazena-os em 'ptr_byte_offsets'
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
    // 'bloat' eh utilizado para ler lixo no fim de paginas de disco
    char bloat = '@';
    // 'qt_reg' armazena a quantidade de registros carregados na memoria
    // 'cluster_size_free' mantem a quantidade de bytes livres na pagina de disco
    // 'ptr' eh utilizado para percorrer 'data'
    int qt_reg = -1, cluster_size_free = INDEX_CLUSTER_SIZE, ptr = 0;
    HEADER_INDEX header;
    FILE *index_arq = NULL;
    if(file_name != NULL && data != NULL)
    {
        index_arq = fopen(file_name, "rb"); // Abre o arquivo de indices
        if(index_arq != NULL)
        {
            fread(&header.status, sizeof(char), 1, index_arq);
            if(header.status == '1')
            {
                fread(&header.nroReg, sizeof(int), 1, index_arq);
                fseek(index_arq, INDEX_CLUSTER_SIZE, SEEK_SET);
                qt_reg = header.nroReg; // Atualiza a quantidade de registros
                (*data) = (INDEX_DATA *) malloc(sizeof(INDEX_DATA) * qt_reg); // Aloca memoria para os registros de indices
                if((*data) != NULL)
                {
                    for(int i = 0; i < qt_reg; i++)
                    {
                        if((cluster_size_free - 128) < 0) // Se chegou no fim de uma pagina de disco
                        {
                            for(int k = 0; k < cluster_size_free; k++)
                            {
                                fread(&bloat, sizeof(char), 1, index_arq);
                            }
                            cluster_size_free = INDEX_CLUSTER_SIZE;
                        }
                        fread(&( (*data)[ptr].chaveBusca), 120, 1, index_arq); // Recupera o nome
                        fread(&( (*data)[ptr].byteOffset), sizeof(long int), 1, index_arq); // Recupera o byte offset
                        ptr++; // Avanca o ponteiro para a proxima posicao livre
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
     // 'sup' eh o limite superior de 'data'
     // 'inf' eh o limite inferior de 'data'
     // 'mid' eh o meio de 'data'
     // 'qt_reg' armazena a quantidade de registros de indice que tem 'nome'
    unsigned int sup = nitems, inf = 0, mid = -1, qt_reg = 0;
    // 'findit' eh uma flag para indicar que o registro de indice contem nome
    char findit = 0x00;
    // 'cmp' eh 0 quando o nome eh encontrado em alguma registro de indice
    // 'indexes' eh um array que armazena as posicoes do array do registro de indices ('data') que contem 'nome'
    // 'j' eh utilizado na insercao das posicoes em 'indexes'
    int cmp = 0, *indexes = NULL, j = 0;
    if(data != NULL)
    {
        mid = (inf + sup) / 2; // calcula o meio
        cmp = strcmp(nome, data[mid].chaveBusca);
        while(inf <= sup)
        {
            if(cmp == 0)
            {
                findit = 0x01;
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
            mid = (inf + sup) / 2; // calcula o novo meio
            cmp = strcmp(nome, data[mid].chaveBusca);
        }
        if(findit == 0x01) // Se encontrou um registro
        {
            qt_reg++;
            sup = (mid - 1); // Necessario para buscar nomes iguais no loop abaixo
            while(strcmp(nome, data[sup].chaveBusca) == 0 && sup < nitems) // Enquanto houver mais nomes iguais
            {
                sup++;
                qt_reg++;
            }
            *items_finded = (qt_reg - 1); // Atualiza a quantidade de registros que 'items_finded' aponta
            indexes = (int *) malloc(sizeof(int) * qt_reg);
            if(indexes != NULL)
            {
                if(sup == (mid - 1)) // Se encontrou apenas um nome
                {
                    indexes[0] = mid;
                    *items_finded = qt_reg; // Atualiza a quantidade de registros que 'items_finded' aponta
                }
                else // Se encontrou varios nomes
                {
                    for(int i = (mid - 1); i < sup; i++, j++)
                    {
                        indexes[j] = i;
                    }
                }
            }
        }
    }
    return indexes;
 }

int remove_index_file(const char *file_name, INDEX_DATA *data, unsigned int nitems, const char *nome)
{
    // 'r' eh o retorna da funcao
    // 'items_finded' armazena a quantidade de registros encontrados em 'data' que contem 'nome'
    // 'ptr_indexes' eh um array de inteiros que servem para indexar os registros encontrados em 'data' que contem 'nome'
    int r = -1, items_finded = 0, *ptr_indexes = NULL;
    // 'index_arq' eh utilizado para escrever o novo cabecalho no fim da execucao da funcao
    FILE *index_arq = NULL;
    // 'header' armazena informacoes do cabecalho do arquivo de indices
    HEADER_INDEX header;
    if(file_name != NULL && data != NULL)
    {
        read_index_header(file_name, &header);
        if(header.status == '1')
        {
            if(nitems > 0)
            {
                index_arq = fopen(file_name, "r+b");
                if(index_arq != NULL)
                {
                    header.status = '0';
                    fwrite(&header.status, sizeof(char), 1, index_arq);
                    ptr_indexes = binary_seach_index(data, nitems, &items_finded, nome); // Realiza a buca binaria para encontrar os registros de indices que contem nome
                    if(ptr_indexes != NULL)
                    {
                        for(int i = 0; i < items_finded; i++) // Loop para marcar os registros de indices como removidos
                        {
                            data[ptr_indexes[i]].byteOffset = -1; // Marca os registros de indices como removidos
                        }
                        header.nroReg -= items_finded; // Diminui a quantidade de registros que o arquivo de indice contem
                        fseek(index_arq, 1, SEEK_SET); // Volta para o comeco do arquivo de indice pulando o 'status' para escrever o novo 'nroReg'
                        fwrite(&header.nroReg, sizeof(int), 1, index_arq); // Escreve a nova quantidade de registros de indices o arquivo tem
                    }
                    free(ptr_indexes);
                    rewind(index_arq);
                    header.status = '1';
                    fwrite(&header.status, sizeof(char), 1, index_arq);
                    r = 0;
                }
                else
                {
                    printf("Falha no processamento do arquivo.\n");
                }
                fclose(index_arq);
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
    else
    {
        printf("Falha no processamento do arquivo.\n");
    }
    return r;
}


void write_index_data(const char *file_name, INDEX_DATA *data, unsigned int nitems)
{
    // 'bloat' eh utilizado para preencher a pagina de disco com lixo
    char bloat = '@';
    // 'cluster_size_free' serve para consultar o quanto a pagina de disco tem de espaco livre
    int cluster_size_free = INDEX_CLUSTER_SIZE;
    // 'header' armazena informacoes do cabecalho do arquivo de indices
    HEADER_INDEX header;
    // 'index_arq' eh o pointeiro para o arquivo de indices
    FILE *index_arq = NULL;
    if(file_name != NULL)
    {
        read_index_header(file_name, &header); // Le o registro de cabecalho do arquivo de indice
        write_index_header(file_name, &header); // Escreve o registro de cabecalho do arquivo de indice
        index_arq = fopen(file_name, "r+b");
        if(index_arq != NULL)
        {
            fseek(index_arq, INDEX_CLUSTER_SIZE, SEEK_SET); // Pula a pagina de disco em que o cabecalho esta
            for(int i = 0; i < nitems; i++) // Loop para escrever todos os registros de indices no arquivo de indices
            {
                if(data[i].byteOffset != -1) // Se o registro nao foi removido, o escreve no arquivo de indice
                {
                    if((cluster_size_free - 128) < 0) // Se a pagina de disco nao tem espaco livre para o registro de indice
                    {
                        for(int j = 0; j < cluster_size_free; j++) // Preenche com lixo o restante da pagina de disco
                        {
                            fwrite(&bloat, sizeof(char), 1, index_arq);
                        }
                        cluster_size_free = INDEX_CLUSTER_SIZE; // Cria uma nova pagina de disco
                    }

                    // Escreve o registro e atualiza o tamanho da nova pagina de disco
                    fwrite(&data[i].chaveBusca, sizeof(data[i].chaveBusca), 1, index_arq);
                    fwrite(&data[i].byteOffset, sizeof(long int), 1, index_arq);
                    cluster_size_free -= 128;
                }
            }
        }
        else
        {
            printf("Falha no processamento do arquivo.\n");
        }
        fclose(index_arq);
    }
}

int insert_index_file(const char *file_name, INDEX_DATA **data, unsigned int *nitems, const char *nome, long int byte_offset)
{
    // 'r' eh o retorno da funcao
    // 'i' eh utilizado no loop de insercao do novo registro
    int i = 0, r = -1;
    // 'index_arq' eh utilizado para escrever o novo cabecalho no fim da execucao da funcao
    FILE *index_arq = NULL;
    // 'header' armazena o cabecalho do arquivo
    HEADER_INDEX header;
    if(file_name != NULL)
    {
        read_index_header(file_name, &header); // le o arquivo de cabecalho
        if(header.status == '1')
        {
            (*data) = (INDEX_DATA *) realloc((*data), sizeof(INDEX_DATA) * ((*nitems) + 1)); // Altera o tamanho para conter o novo registro a ser inserido
            if((*data) != NULL)
            {
                i = ((*nitems) - 1); // 'i' Aponta para o ultimo registro
                while((strcmp(nome, (*data)[i].chaveBusca) < 0) && i >= 0) // Loop para encontrar a posicao para inserir o novo registro
                {
                    /*
                        A insercao ocorre deslocando os registros ate encontrar uma posicao
                        correta para inserir o registro
                    */
                    (*data)[i + 1] = (*data)[i]; // Desloca uma posicao a frente
                    i--;
                }
                if(strcmp(nome, (*data)[i].chaveBusca) == 0) // Se a posicao achada tem um registro de nome igual
                {
                    while(byte_offset < (*data)[i].byteOffset) // Ordena por byte offset
                    {
                        (*data)[i + 1] = (*data)[i]; // Desloca uma posicao a frente
                        i--;
                    }
                }
                memset((*data)[i + 1].chaveBusca, '@', sizeof((*data)[i + 1].chaveBusca));
                strncpy((*data)[i + 1].chaveBusca, nome, (strlen(nome) + 1)); // Insere o novo registro de indice
                (*data)[i + 1].byteOffset = byte_offset;
                (*nitems)++; // Atualiza o valor da variavel que guarda a quantidade de registros que 'data' tem
                index_arq = fopen(file_name, "r+b"); // Abre o arquivo de indices para atualizar o cabecalho
                header.nroReg++; // Atualiza a quantidade de registros que o arquivo de indices tem
                fwrite(&header.status, sizeof(char), 1, index_arq);
                fwrite(&header.nroReg, sizeof(int), 1, index_arq);
                fclose(index_arq);
                r = 0;
            }
            else
            {
                printf("Falha ao realocar memoria.\n");
            }
        }
        else
        {
            printf("Falha no processamento do arquivo.\n");
        }
    }
    return r;
}
