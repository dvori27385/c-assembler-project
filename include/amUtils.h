/*Maman14
Created by:
Dvora Keleti
Avigail Dangor
*/
#pragma once

#include <stdio.h>
#include "amEngine.h"

extern struct DF_STRUCT_AM_Keys     GD_VAR_AM_LabelList;
extern struct DF_STRUCT_AM_Keys     GD_VAR_AM_EntryList;
extern struct DF_STRUCT_TEMP_FILES  GD_VAR_AM_TempFiles;

#define _G_DEF_GUIDE_ENONE		-1
#define _G_DEF_GUIDE_EDATA		0
#define _G_DEF_GUIDE_ESTRING	1
#define _G_DEF_GUIDE_EENTRY	    2
#define _G_DEF_GUIDE_EEXTERN	3
#define _G_DEF_GUIDE_EMAT	    4

#define _G_DEF_TYPE_NONE           0 
#define _G_DEF_TYPE_CODE           1 
#define _G_DEF_TYPE_DATA           2 


/* Struct of macro */
struct DF_STRUCT_AM_Macro {
    char mv_struct_sName[256];
    char** mv_struct_pLineInfo;
    int mv_struct_nLineIndex;
    int mv_struct_nLineCnt;
};

/* Struct of macros */
struct DF_STRUCT_AM_Macros {
    struct DF_STRUCT_AM_Macro** mv_struct_pMacroList;
    int mv_struct_nIndex; 
    int mv_struct_nCount; 
};

int subfunc_init_outfiles(const char* prefix,  FILE** input_file,  FILE* output_files[4],  struct DF_STRUCT_AM_FileData** asm_info);

void subfunc_am_cleanup(const char* prefix, FILE* output_files[4],  struct DF_STRUCT_AM_FileData* asm_info, int error_count,  FILE* input_file);

void subfunc_am_process(struct DF_STRUCT_AM_FileData* asm_info, FILE* input_file);

/*  new macro  */
struct DF_STRUCT_AM_Macro* subfunc_new_macro(const char* name);

/* init macro structure */
void subfunc_init_macro(struct DF_STRUCT_AM_Macro* macro, const char* name);

/* Add line information in macro structure */
void subfunc_add_macro_line(struct DF_STRUCT_AM_Macro* macro, const char* line); 

/* Free macro structure */
void subfunc_delete_macro(struct DF_STRUCT_AM_Macro* macro);

/* new macro array */
struct DF_STRUCT_AM_Macros* subfunc_new_macros();

/* find macro */
struct DF_STRUCT_AM_Macro* subfunc_find_macro(struct DF_STRUCT_AM_Macros* macros, const char* name);

/* Register new macro into macro array */
struct DF_STRUCT_AM_Macro* subfunc_reg_macro(struct DF_STRUCT_AM_Macros* macros, const char* name);

/* Free macro */
void subfunc_delete_macros(struct DF_STRUCT_AM_Macros* macros);

/* other macro function*/
void subfunc_macro_directives(char* line, struct DF_STRUCT_AM_FileData* stInfoData, struct DF_STRUCT_AM_Macros* macros, struct DF_STRUCT_AM_Macro** macro) ;

void subfunc_expand_macro_write(char* line, struct DF_STRUCT_AM_FileData* stInfoData, struct DF_STRUCT_AM_Macros* macros, FILE* file_am);


int subfunc_code_size(const char* p_line, int index);

/* get guide type, size */
int subfunc_label_type(const char* p_name);

int subfunc_label_size(const char* p_line, int index, int counts);

int subfunc_check_reserved(const char* p_token);

int subfunc_check_label(const char* p_token);

int subfunc_check_guide(const char* data_line, int _pos, int keys);

int subfunc_check_code(const char* data_line, int _pos, int keys) ;

void subfunc_change_quat(int value, char* buffer, int bufferSize);

/* init */
void subfunc_asm_initkey();

/* free */
void subfunc_asm_freekey();

