/*Maman14
Created by:
Dvora Keleti
Avigail Dangor
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "amEngine.h"
#include "amUtils.h"


char GD_VAR_TokenInfo[1024];



struct DF_STRUCT_AM_Code GD_VAR_AM_TokenList[] = {
    { "mov",  0, 2 },
    { "cmp",  1, 2 }, 
    { "add",  2, 2 }, 
    { "sub",  3, 2 },
    { "not",  4, 1 }, 
    { "clr",  5, 1 }, 
    { "lea",  6, 2 }, 
    { "inc",  7, 1 },
    { "dec",  8, 1 }, 
    { "jmp",  9, 1 }, 
    { "bne", 10, 1 }, 
    { "red", 11, 1 },
    { "prn", 12, 1 }, 
    { "jsr", 13, 1 }, 
    { "rts", 14, 0 }, 
    { "stop", 15, 0 }
};

char* GD_VAR_AM_GuideList[] = {
    ".data", 
    ".entry", 
    ".extern", 
    ".string", 
    ".mat",
    "mov",
    "cmp", 
    "add", 
    "sub", 
    "lea", 
    "clr", 
    "not", 
    "inc", 
    "dec",
    "jmp", 
    "bne", 
    "red", 
    "prn", 
    "jsr", 
    "rts", 
    "stop"
};


/* init processing */
int subfunc_init_outfiles(const char* prefix,
               FILE** input_file,
               FILE* output_files[4],
               struct DF_STRUCT_AM_FileData** asm_info) {

    char file_path[512];
    const char* extensions[] = {".ob", ".ent", ".ext", ".am"};
    int i;

    *asm_info = malloc(sizeof(struct DF_STRUCT_AM_FileData));
    if (!(*asm_info)) {
        fprintf(stderr, "system error ... memory malloc.\n");
        return -1;
    }
    memset(*asm_info, 0, sizeof(struct DF_STRUCT_AM_FileData));

    sprintf(file_path, "%s.as", prefix);
    *input_file = fopen(file_path, "r");
    if (!(*input_file)) {
        fprintf(stderr, "file error ... read input file %s.as.\n", prefix);
        free(*asm_info);
        return -1;
    }

    for (i = 0; i < 4; i++) {
        sprintf(file_path, "%s%s", prefix, extensions[i]);
        output_files[i] = fopen(file_path, "w+");
    }

    GD_VAR_AM_TempFiles.mv_struct_OBJ = output_files[0];
    GD_VAR_AM_TempFiles.mv_struct_ENT = output_files[1];
    GD_VAR_AM_TempFiles.mv_struct_EXT = output_files[2];
    GD_VAR_AM_TempFiles.mv_struct_AM  = output_files[3];
    GD_VAR_AM_TempFiles.mv_struct_ERROR = stderr;

    subfunc_asm_initkey();
    return 0;
}

/* Looks up an instruction */
const struct DF_STRUCT_AM_Code* subfunc_code(const char* p_code) 
{
    const struct DF_STRUCT_AM_Code* res = NULL;
    int i, w_nCount = sizeof(GD_VAR_AM_TokenList) / sizeof(struct DF_STRUCT_AM_Code);

    for (i = 0; i < w_nCount; i++) {
        if (strcmp(GD_VAR_AM_TokenList[i].mv_struct_sName, p_code) == 0) {
            res = &GD_VAR_AM_TokenList[i];
            break;
        }
    }
    return res;
}
/* Encode a direct operand */
void subfunc_direct_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status) {
    struct DF_STRUCT_AM_Key* w_key;
    int w_value;
    char temp_code[6] = { 0 };
    char temp_addr[5] = { 0 };

    w_key = subfunc_find_token(operand, &GD_VAR_AM_LabelList);
    if (w_key == NULL) {
        /* External label*/
        w_value = _G_DEF_AM_EXTERNAL;
        subfunc_get_quat(w_value, temp_code, 6);
        subfunc_get_quat(status->mv_struct_nCurIndex, temp_addr, 5);

        fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", temp_addr, temp_code);
        fprintf(GD_VAR_AM_TempFiles.mv_struct_EXT, "%s %s\n", operand, temp_addr);
    } else {
        /* Internal label: encode as relative address */
        w_value = (w_key->mv_struct_address << 2) | _G_DEF_AM_RELATIVE;
        subfunc_get_quat(w_value, temp_code, 6);
        subfunc_get_quat(status->mv_struct_nCurIndex, temp_addr, 5);

        fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", temp_addr, temp_code);
    }
    status->mv_struct_nCurIndex++;
}

/* Encode a matrix operand  */
void subfunc_matrix_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status) {
    struct DF_STRUCT_AM_Key* _label;
    int _r1, _r2, w_value;
    char w_matrix[32];
    char _quatCode[6] = { 0 };
    char _quatAddr[5] = { 0 };

    /* Validate matrix format and register indexes */
    if (sscanf(operand, "%[^[] [r%d] [r%d]", w_matrix, &_r1, &_r2) != 3 || 
        _r1 > 7 || _r1 < 0 || _r2 > 7 || _r2 < 0) {
        fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, 
                "error ... Matrix parsing at line %d.\n", status->mv_struct_nLineInfo);
        status->mv_struct_nErrCount++;
        return;
    }
    /* find matrix label*/
    _label = subfunc_find_token(w_matrix, &GD_VAR_AM_LabelList);
    if (_label == NULL) {
        fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "error ... Matrix Address at line %d.\n", status->mv_struct_nLineInfo);
        status->mv_struct_nErrCount++;
        return;
    }

    /* convert address and code */
    w_value = (_label->mv_struct_address << 2) | _G_DEF_AM_RELATIVE;
    subfunc_get_quat(w_value, _quatCode, 6);
    subfunc_get_quat(status->mv_struct_nCurIndex, _quatAddr, 5);

    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", _quatAddr, _quatCode);
    status->mv_struct_nCurIndex++;

    /* register address */
    w_value = (_r1 << 6) | (_r2 << 2) | _G_DEF_AM_ABSOLUTE;
    subfunc_get_quat(w_value, _quatCode, 6);
    subfunc_get_quat(status->mv_struct_nCurIndex, _quatAddr, 5);

    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", _quatAddr, _quatCode);
    status->mv_struct_nCurIndex++;
}

/* Encode a register operand (e.g., r3 or r5) */
void subfunc_register_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status, int p_nNum) {
    int _reg_index, w_value;
    char _quatCode[6] = { 0 };
    char _quatAddr[5] = { 0 };

    _reg_index = atoi(&(operand[1]));
    if (p_nNum == 1) {
        w_value = (_reg_index << 6) | _G_DEF_AM_ABSOLUTE;
    } else {
        w_value = (_reg_index << 2) | _G_DEF_AM_ABSOLUTE;
    }

    subfunc_get_quat(w_value, _quatCode, 6);
    subfunc_get_quat(status->mv_struct_nCurIndex, _quatAddr, 5);

    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", _quatAddr, _quatCode);
    status->mv_struct_nCurIndex++;
}

/* Counts the number of tokens in a given line */
int subfunc_key_count(const char* p_line) {
    int _result = 0;
    while (*subfunc_key(p_line, _result) != '\0')
        _result++;
    return _result;
}

/* Initialize entry and label structures */
void subfunc_asm_initkey() {
    int i;
    struct DF_STRUCT_AM_Keys* tables[] = {
        &GD_VAR_AM_EntryList,
        &GD_VAR_AM_LabelList
    };

    for (i = 0; i < 2; i++) {
        tables[i]->mv_struct_pLabelList = NULL;
        tables[i]->mv_struct_nIndex = 0;
        tables[i]->mv_struct_nCount = 0;
    }
}

/* free label structure  */
void subfunc_asm_freekey() {
    int i;
    struct DF_STRUCT_AM_Keys* tables[] = {
        &GD_VAR_AM_EntryList,
        &GD_VAR_AM_LabelList
    };

    for (i = 0; i < 2; i++) {
        if (tables[i] && tables[i]->mv_struct_pLabelList) {
            free(tables[i]->mv_struct_pLabelList);
            tables[i]->mv_struct_pLabelList = NULL;
        }
    }
}

/* Add new symbol  */
int subfunc_asm_addkey(const char* name, int address, struct DF_STRUCT_AM_Keys* table) {
    int _i;
    struct DF_STRUCT_AM_Key* w_key;

    for (_i = 0; _i < table->mv_struct_nIndex; _i++) {
        if (strcmp(table->mv_struct_pLabelList[_i].mv_struct_sName, name) == 0) {
            return 0;
        }
    }

    if (table->mv_struct_nIndex == table->mv_struct_nCount) {
        table->mv_struct_nCount += 16;
        table->mv_struct_pLabelList = realloc(table->mv_struct_pLabelList, sizeof(*table->mv_struct_pLabelList) * table->mv_struct_nCount);
    }

    w_key = &(table->mv_struct_pLabelList[table->mv_struct_nIndex]);
    strcpy(w_key->mv_struct_sName, name);
    w_key->mv_struct_address = address;

    table->mv_struct_nIndex++;
    return 1;
}

/* Expands a macro or writes the line directly to the AM file */
void subfunc_expand_macro_write(char* line, struct DF_STRUCT_AM_FileData* info, struct DF_STRUCT_AM_Macros* macros, FILE* file_am) {
    char* word = subfunc_key(line, 0);
    struct DF_STRUCT_AM_Macro* macro;
    int i;

    if (!subfunc_check_reserved(word)) {
        macro = subfunc_find_macro(macros, word);
        if (macro == NULL) {
            fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "error ... find macro at line %d.\n", info->mv_struct_nLineInfo);
            info->mv_struct_nErrCount++;
        } else {
            for (i = 0; i < macro->mv_struct_nLineIndex; i++) {
                subfunc_proc_line_1(macro->mv_struct_pLineInfo[i], info);
                fputs(macro->mv_struct_pLineInfo[i], file_am);
            }
        }
    }
}

/* function to process a line of code and encode its opcodes */
void subfunc_proc_am_line(const char* line, int start_index, int count_tokens, struct DF_STRUCT_AM_FileData* status) {
    char* token;
    const struct DF_STRUCT_AM_Code* _CodeInfo;
    char s_code_op[256];
    char t_code_op[256];

    /*  Get the first token (opcode)*/
    token = subfunc_key(line, start_index);
    if ((_CodeInfo = subfunc_code(token)) == NULL)
        return;

    /* check operand type*/    
    if (_CodeInfo->mv_struct_nOperand == 0) {
        subfunc_proc_op_0(_CodeInfo, status);
    } else if (_CodeInfo->mv_struct_nOperand == 1) {
        token = subfunc_key(line, start_index + 1);
        subfunc_proc_op_1(_CodeInfo, token, status);
    } else if (_CodeInfo->mv_struct_nOperand == 2) {
        token = subfunc_key(line, start_index + 1);
        strcpy(s_code_op, token);

        token = subfunc_key(line, start_index + 3);
        strcpy(t_code_op, token);
        subfunc_proc_op_2(_CodeInfo, s_code_op, t_code_op, status);
    }
}
/* Checks the given word. */
int subfunc_check_reserved(const char* w_token) {
    int rsv = 0, i;
    int count = sizeof(GD_VAR_AM_GuideList) / sizeof(char*);

    for (i = 0; i < count; i++) {
        if (strcmp(GD_VAR_AM_GuideList[i], w_token) == 0) {
            rsv = 1;
            break;
        }
    }
    return rsv;
}