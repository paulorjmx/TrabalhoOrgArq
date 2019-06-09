/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/

#ifndef HANDLE_INDEX_H
    #define HANDLE_INDEX_H
        typedef struct index_data_t INDEX_DATA;

        int load_index(const char *file_name, INDEX_DATA **data); // Funcao utilizada para carregar o arquivo de indice para 'data'. Retorna a quantidade de registros lidos
        int create_index_file(const char *file_name, const char *data_file_name); // Funcao utilizada para criar o arquivo de indice referente ao arquivo 'data_file_name'
        long int *search_name_index(const char *file_name, const char *nome, int *items_finded); // Funcao utilizada para buscar um nome e retornar um conjunto de bytes offset dos registros encontrados
        int remove_index_file(const char *file_name, INDEX_DATA *data, unsigned int nitems, const char *nome); // Funcao utilizada para remover um registro do arquivo de indice
        int insert_index_file(const char *file_name, INDEX_DATA **data, unsigned int *nitems, const char *nome, long int byte_offset); // Funcao utilizada para inserir um novo registro de indice
        void write_index_data(const char *file_name, INDEX_DATA *data, unsigned int nitems); // Funcao utilizada para escrever os registros no arquivo de indice
#endif
