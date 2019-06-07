/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/

#ifndef HANDLE_INDEX_H
    #define HANDLE_INDEX_H

        void create_index_file(const char *file_name, const char *data_file_name); // Funcao utilizada para criar o arquivo de indice referente ao arquivo 'data_file_name'
        long int *search_name_index(const char *file_name, const char *nome, int *items_finded); // Funcao utilizada para buscar um nome e retornar um conjunto de bytes offset dos registros encontrados
#endif
