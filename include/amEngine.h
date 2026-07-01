/*Maman14
Created by:
Dvora Keleti
Avigail Dangor
*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


struct DF_STRUCT_AM_Code {
    const char* mv_struct_sName; 
    int mv_struct_nOPCode;
    int mv_struct_nOperand;
};

/* Struct of .am file*/
struct DF_STRUCT_AM_FileData {
	int mv_struct_nCurIndex;
	int mv_struct_nLineInfo;
	int mv_struct_szData;
	int mv_struct_szCode;
	int mv_struct_nErrCount;
};

struct DF_STRUCT_AM_Keys {
    struct DF_STRUCT_AM_Key* mv_struct_pLabelList;
    int mv_struct_nIndex;
    int mv_struct_nCount;
};

struct DF_STRUCT_AM_Key {
    char mv_struct_sName[256];
    int mv_struct_address;
};

/* Struct of Temp files */
struct DF_STRUCT_TEMP_FILES {
    FILE*   mv_struct_AM;
    FILE*   mv_struct_OBJ;
    FILE*   mv_struct_ENT;
    FILE*   mv_struct_EXT;
    FILE*   mv_struct_ERROR;
};


#define _G_DEF_AM_ABSOLUTE   0 
#define _G_DEF_AM_RELATIVE   2 
#define _G_DEF_AM_EXTERNAL   1 

#define _G_DEF_ADDR_IMMEDIATE    0 
#define _G_DEF_ADDR_DIRECT       1 
#define _G_DEF_ADDR_MATRIX       2
#define _G_DEF_ADDR_REGISTER     3 



/* regite token index*/
int subfunc_register_index(const char* p_token);

/* change to 4-digit*/
void subfunc_get_quat(int p_value, char*  p_buff, int p_nBuffLen);

/* count the number of token */
int subfunc_key_count(const char* p_line);

/* get token from line */
char* subfunc_key(const char* p_line, int index);

/* get address method  */
int subfunc_addressing(const char* p_token);


/* encode a line */
void subfunc_proc_am_line(const char* line, int pIndex, int count_tokens, struct DF_STRUCT_AM_FileData* status) ;

/* code line for step1 */
void subfunc_proc_line_1(const char* line, struct DF_STRUCT_AM_FileData* info);

/* main step2 process */
void subfunc_proc_line_2(const char* line, struct DF_STRUCT_AM_FileData* status);


/* encode opcode */
void subfunc_proc_op_0(const struct DF_STRUCT_AM_Code* asm_code, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_proc_op_1(const struct DF_STRUCT_AM_Code* asm_code, const char* operand, struct DF_STRUCT_AM_FileData* status);

void subfunc_proc_op_2(const struct DF_STRUCT_AM_Code* asm_code, const char* src_code_op, const char* tgt_code_op, struct DF_STRUCT_AM_FileData* status) ;

/* step1 pass */
void subfunc_proc_pass_1(struct DF_STRUCT_AM_FileData* p_stASMInfo, FILE*   p_File_AS);

/* step2 pass */
void subfunc_proc_pass_2(struct DF_STRUCT_AM_FileData* p_stALLInfo);

const struct DF_STRUCT_AM_Code* subfunc_code(const char* p_name);

/* add new label */
int subfunc_asm_addkey(const char* name, int address, struct DF_STRUCT_AM_Keys* table);

/* find token keyword*/
struct DF_STRUCT_AM_Key* subfunc_find_token(const char* name, struct DF_STRUCT_AM_Keys* table) ;


/* encode operand type*/
void subfunc_immediate_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_direct_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_matrix_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_register_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status, int p_nNum);

void subfunc_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status, int p_nNum);

/* label and category*/
int subfunc_labels(const char* line, struct DF_STRUCT_AM_FileData* info, int* _index);

int subfunc_detect_category(const char* line, int _index, int nToken); 

void subfunc_guideinfo(const char* line, int _index, int nToken, int category, struct DF_STRUCT_AM_FileData* info);

/* encode data, string, mat, guide*/
void subfunc_data_guide(const char* line, int pIndex, int count_tokens, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_string_guide(const char* line, int pIndex, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_mat_guide(const char* line, int pIndex, int count_tokens, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_guide(const char* line, int pIndex, int count_tokens, struct DF_STRUCT_AM_FileData* status) ;

void subfunc_entry_update();