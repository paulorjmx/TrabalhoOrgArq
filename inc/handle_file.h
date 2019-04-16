#ifndef HANDLE_FILE_H
    #define HANDLE_FILE_H

        void create_bin_file(const char *data_file_name, const char *csv_file_name);
        void get_all_data_file(const char *file_name);
        void search_for_id(const char *file_name, int id);
        void search_for_salario(const char *file_name, double salario);
        void search_for_telefone(const char *file_name, const char *telefone);
        void search_for_nome(const char *file_name, const char *nome);
        void search_for_cargo(const char *file_name, const char *cargo);
#endif
