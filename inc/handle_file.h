#ifndef HANDLE_FILE_H
    #define HANDLE_FILE_H

        typedef struct t_file_header
        {
            int topo_lista;
            char desc_campo1[40], desc_campo2[40], desc_campo3[40], desc_campo4[40], desc_campo5[40];
            char status, tag_campo1, tag_campo2, tag_campo3, tag_campo4, tag_campo5;
        } FILE_HEADER;

        void init_file_header(FILE_HEADER *header);
        void read_file(const char *file_name);
        void print_file_header(FILE_HEADER header);
#endif
