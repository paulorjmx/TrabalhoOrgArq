/* ##########################################
#                                           #
#   Autor: Paulo Ricardo Jordao Miranda     #
#   N USP: 10133456                         #
#                                           #
###########################################*/

#ifndef HANDLE_FILE_H
    #define HANDLE_FILE_H

        void create_bin_file(const char *data_file_name, const char *csv_file_name); //  Funcao que criar o arquivo de dados binario
        void get_all_data_file(const char *file_name); // Funcao que recupera todos os registros do arquivo de dados binario e mostra na tela
        void search_for_id(const char *file_name, int id); //  Funcao que recupera o registro que tem como campo incomum o id procurado
        void search_for_salario(const char *file_name, double salario); //  Funcao que recupera o registro que tem como campo incomum o salario procurado
        void search_for_telefone(const char *file_name, const char *telefone); //  Funcao que recupera o registro que tem como campo incomum o telefone procurado
        void search_for_nome(const char *file_name, const char *nome); //  Funcao que recupera o registro que tem como campo incomum o nome procurado
        void search_for_nome_index(const char *file_name, const char *index_file_name, const char *nome); //  Funcao que recupera o registro que tem como campo incomum o nome procurado utilizando o arquivo de indice fortemente ligado
        void search_for_cargo(const char *file_name, const char *cargo); //  Funcao que recupera o registro que tem como campo incomum o cargo procurado
        int remove_by_id(const char *file_name, int id); // Funcao que remove um registro baseado em seu idServidor
        int remove_by_salario(const char *file_name, double salario); // Funcao que remove um registro baseado no campo salario
        int remove_by_cargo(const char *file_name, const char *cargo); // Funcao que remove um registro baseado no campo cargo
        int remove_by_telefone(const char *file_name, const char *telefone); // Funcao que remove um registro baseado no campo telefone
        int remove_by_nome(const char *file_name, const char *nome); // Funcao que remove um registro baseado no campo nome
        int remove_by_cargo(const char *file_name, const char *cargo); // Funcao que remove um registro baseado no campo cargo
        int insert_bin(const char *file_name, int id, double salario, const char *telefone, const char *nome, const char *cargo); // Funcao que insere um registro novo
        int edit_by_id(const char *file_name, int id, const char *campo); // Funcao utilizada para editar registros baseado em um idServidor e um campo que deseja editar
        int edit_by_salario(const char *file_name, double salario, const char *campo); // Funcao utilizada para editar registros baseado em um salarioServidor e um campo que deseja editar
        int edit_by_telefone(const char *file_name, const char *telefone, const char *campo); // Funcao utilizada para editar registros baseado em um telefoneServidor e um campo que deseja editar
        int edit_by_nome(const char *file_name, const char *nome, const char *campo); // Funcao utilizada para editar registros baseado em um nomeServidor e um campo que deseja editar
        int edit_by_cargo(const char *file_name, const char *cargo, const char *campo); // Funcao utilizada para editar registros baseado em um cargoServidor e um campo que deseja editar
        int sort_data_file(const char *file_name, const char *sorted_file_name); // Funcao utilizada para ordenar um arquivo 'file_name' salvando-o como 'sorted_file_name'
        int merging_data_file(const char *file_name1, const char *file_name2, const char *merged_file_name); // Funcao utilizada para realizar a uniao de dois arquivos de dados
        int matching_data_file(const char *file_name1, const char *file_name2, const char *merged_file_name); // Funcao utilizada para realizar a intersecao de dois arquivos de dados
#endif
