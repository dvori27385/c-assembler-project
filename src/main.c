/*Maman14
Created by:
Dvora Keleti
Avigail Dangor
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "amEngine.h"
#include "amUtils.h"


struct DF_STRUCT_AM_Keys GD_VAR_AM_LabelList;
struct DF_STRUCT_AM_Keys GD_VAR_AM_EntryList;
struct DF_STRUCT_TEMP_FILES  GD_VAR_AM_TempFiles;



/* run processing */
int run_assembler(const char* file_prefix) {
    FILE* input_file = NULL;
    FILE* output_files[4];
    struct DF_STRUCT_AM_FileData* asm_info = NULL;

    if (subfunc_init_outfiles(file_prefix, &input_file, output_files, &asm_info) != 0)
        return -1;

    subfunc_am_process(asm_info, input_file);

    if (asm_info->mv_struct_nErrCount == 0)
        subfunc_entry_update();

    subfunc_am_cleanup(file_prefix, output_files, asm_info,
                      asm_info->mv_struct_nErrCount, input_file);

    return 0;
}

/* main function */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Program usage: %s [file prefix]\n", argv[0]);
        return -1;
    }
    return run_assembler(argv[1]);
}