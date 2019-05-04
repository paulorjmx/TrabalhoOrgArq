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
        void search_for_cargo(const char *file_name, const char *cargo); //  Funcao que recupera o registro que tem como campo incomum o cargo procurado
        void remove_by_id(const char *file_name, int id); // Funcao que remove um registro baseado em seu idServidor
        void remove_by_salario(const char *file_name, double salario); // Funcao que remove um registro baseado no campo salario
        void remove_by_cargo(const char *file_name, const char *cargo); // Funcao que remove um registro baseado no campo cargo
        void remove_by_telefone(const char *file_name, const char *telefone); // Funcao que remove um registro baseado no campo telefone
        void remove_by_nome(const char *file_name, const char *nome); // Funcao que remove um registro baseado no campo nome
        void remove_by_cargo(const char *file_name, const char *cargo); // Funcao que remove um registro baseado no campo cargo
#endif
