/*Maman14
Created by:
Dvora Keleti
Avigail Dangor
*/
#include <stdlib.h>
#include <string.h>
#include "amEngine.h"
#include "amUtils.h"


extern struct DF_STRUCT_AM_Code GD_VAR_AM_TokenList[];
extern char* GD_VAR_AM_GuideList[];


/* kernel processing*/
void subfunc_am_process(struct DF_STRUCT_AM_FileData* asm_info, FILE* input_file) {
    /* asm step 1*/
    subfunc_proc_pass_1(asm_info, input_file);
    /* asm step 2*/
    if (asm_info->mv_struct_nErrCount == 0) {
        fseek(GD_VAR_AM_TempFiles.mv_struct_AM, 0, SEEK_SET);
        subfunc_proc_pass_2(asm_info);
    }
}


/* end processing */
void subfunc_am_cleanup(const char* prefix, FILE* output_files[4],
                        struct DF_STRUCT_AM_FileData* asm_info, int error_count,
                        FILE* input_file) {

    int i; 
    char file_path[512];
    const char* extensions[] = {".ob", ".ent", ".ext"};
    
    for (i = 0; i < 3; i++) {
        if (output_files[i]) {
            if (ftell(output_files[i]) == 0 || error_count) {
                fclose(output_files[i]);
                sprintf(file_path, "%s%s", prefix, extensions[i]);
                remove(file_path);
            } else {
                fclose(output_files[i]);
            }
        }
    }

    /* Close remaining resources*/
    if (GD_VAR_AM_TempFiles.mv_struct_AM)
        fclose(GD_VAR_AM_TempFiles.mv_struct_AM);

    if (input_file)
        fclose(input_file);

    subfunc_asm_freekey();

    if (asm_info)
        free(asm_info);
}

/* Converts a register name like "r3" into its index (0–7), or -1 if invalid */
int subfunc_register_index(const char* p_code) {
    int _number_register = -1;
    if (p_code[0] != 'r') return _number_register;

    sscanf(&p_code[1], "%d", &_number_register);
    if (_number_register > 7) _number_register = -1;
    return _number_register;
}

/* Validates whether a line of code contains a valid instruction format. */
int subfunc_check_code(const char* _line, int _pos, int _keys) {
    char* w_token;
    const struct DF_STRUCT_AM_Code* _guideinfo = NULL;

    w_token = subfunc_key(_line, _pos);
    _guideinfo = subfunc_code(w_token);

    if (_guideinfo == NULL) return 0;

    if (_guideinfo->mv_struct_nOperand == 1) {
        if (_keys != 2) return 0;
    } else if (_guideinfo->mv_struct_nOperand == 2) {
        if (_keys != 4) return 0;
        w_token = subfunc_key(_line, _pos + 2);
        if (strcmp(w_token, ",") != 0) return 0;
    } else {
        if (_keys != 1) return 0;
    }
    return 1;
}

/* Determines the size (number of words) required by a guide directive in assembly */
int subfunc_label_size(const char* _line, int _pos, int _keys) {
    char* _word;
    int sz_guide = 0;
    int v_gType;

    _word = subfunc_key(_line, _pos);
    
    /* Identify the guide directive type */
    v_gType = subfunc_label_type(_word);

    switch (v_gType) {
    case _G_DEF_GUIDE_EDATA:
        sz_guide = _keys / 2;
        break;
    case _G_DEF_GUIDE_ESTRING:
        _word = subfunc_key(_line, _pos + 1);
        sz_guide = (int)(strlen(_word) - 1);
        break;
    case _G_DEF_GUIDE_EMAT:
        /* Matrix elements, half words minus size indicator */
        sz_guide = (int)((_keys - 1) / 2);
        break;
    default:
        break;
    }
    return sz_guide;
}

/* Encode an operand based on addressing mode */
void subfunc_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status, int p_nNum) {
    int _addressing = subfunc_addressing(operand);

    switch (_addressing) {
        case _G_DEF_ADDR_IMMEDIATE:
            subfunc_immediate_code_op(operand, status);
            break;
        case _G_DEF_ADDR_DIRECT:
            subfunc_direct_code_op(operand, status);
            break;
        case _G_DEF_ADDR_MATRIX:
            subfunc_matrix_code_op(operand, status);
            break;
        case _G_DEF_ADDR_REGISTER:
            subfunc_register_code_op(operand, status, p_nNum);
            break;
    }
}

/* Function to encode opcode with two operands (opcode 2)*/
void subfunc_proc_op_2(const struct DF_STRUCT_AM_Code* _CodeInfo, const char* op_s, const char* op_t, struct DF_STRUCT_AM_FileData* p_FileCodeInfo) {
    int w_value;
    int s_addr, t_addr;
    int reg_pos;
    char quatCode[6] = {0};
    char quatAddr[5] = {0};

    /* Get addressing modes for source and target operands */
    s_addr = subfunc_addressing(op_s);
    t_addr = subfunc_addressing(op_t);

    w_value = (_CodeInfo->mv_struct_nOPCode << 6) | _G_DEF_AM_ABSOLUTE;
    w_value |= (s_addr << 4);
    w_value |= (t_addr << 2);

    /* Convert to quaternary code and output */
    subfunc_get_quat(w_value, quatCode, 6);
    subfunc_get_quat(p_FileCodeInfo->mv_struct_nCurIndex, quatAddr, 5);

    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", quatAddr, quatCode);
    p_FileCodeInfo->mv_struct_nCurIndex++;

    /* If both operands are registers */
    if (s_addr == _G_DEF_ADDR_REGISTER && t_addr == _G_DEF_ADDR_REGISTER) {
        reg_pos = subfunc_register_index(op_s);
        w_value = (reg_pos << 6) | _G_DEF_AM_ABSOLUTE;
        reg_pos = subfunc_register_index(op_t);
        w_value |= (reg_pos << 2);

        subfunc_get_quat(w_value, quatCode, 6);
        subfunc_get_quat(p_FileCodeInfo->mv_struct_nCurIndex, quatAddr, 5);

        fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", quatAddr, quatCode);
        p_FileCodeInfo->mv_struct_nCurIndex++;
    } else {
        /* If operands are not both registers, process operands separately*/
        subfunc_code_op(op_s, p_FileCodeInfo, 1);
        subfunc_code_op(op_t, p_FileCodeInfo, 2);
    }
}

/* new macro structure */
struct DF_STRUCT_AM_Macro* subfunc_new_macro(const char* p_pName) {
    struct DF_STRUCT_AM_Macro* st_macro;
    st_macro = malloc(sizeof(*st_macro));

    st_macro->mv_struct_nLineCnt = 0;
    st_macro->mv_struct_nLineIndex = 0;
    st_macro->mv_struct_pLineInfo = NULL;
    strcpy(st_macro->mv_struct_sName, p_pName);

    return st_macro;
}

/* init macro structure */
void subfunc_init_macro(struct DF_STRUCT_AM_Macro* p_pstMacro, const char* p_pName) {
    strcpy(p_pstMacro->mv_struct_sName, p_pName);
    p_pstMacro->mv_struct_pLineInfo = NULL;
    p_pstMacro->mv_struct_nLineIndex = 0;
    p_pstMacro->mv_struct_nLineCnt = 0;
}

/* Validates whether the given word can be used as a label. */
int subfunc_check_label(const char* w_token) {
    if (w_token == NULL) 
        return 0;
    if (subfunc_label_type(w_token) != _G_DEF_GUIDE_ENONE) 
        return 0;
    if (subfunc_code(w_token) != NULL) 
        return 0;
    /* Label should not start with a digit */
    if (isdigit(w_token[0])) 
        return 0;
    return 1;
}

/* Function to encode opcode with no operand (opcode 0)*/
void subfunc_proc_op_0(const struct DF_STRUCT_AM_Code* p_DF_STRUCT_AM_Code, struct DF_STRUCT_AM_FileData* p_FileCodeInfo) {
    int w_value;
    char quatCode[6] = {0};
    char quatAddr[5] = {0};

    /* Encode opcode with the addressing, code  */
    w_value = (p_DF_STRUCT_AM_Code->mv_struct_nOPCode << 6) | _G_DEF_AM_ABSOLUTE;
    subfunc_get_quat(w_value, quatCode, 6);
    subfunc_get_quat(p_FileCodeInfo->mv_struct_nCurIndex, quatAddr, 5);
    /* output */
    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", quatAddr, quatCode);
    p_FileCodeInfo->mv_struct_nCurIndex++;
}
/* Function to encode opcode with one operand (opcode 1) */
void subfunc_proc_op_1(const struct DF_STRUCT_AM_Code* _CodeInfo, const char* operand, struct DF_STRUCT_AM_FileData* p_FileCodeInfo) {
    int w_value, w_addr;
    char quatCode[6] = {0};
    char quatAddr[5] = {0};

    /* Encode opcode with the addressing, code  */
    w_value = (_CodeInfo->mv_struct_nOPCode << 6) | _G_DEF_AM_ABSOLUTE;
    w_addr = subfunc_addressing(operand);
    w_value |= (w_addr << 2);

    subfunc_get_quat(w_value, quatCode, 6);
    subfunc_get_quat(p_FileCodeInfo->mv_struct_nCurIndex, quatAddr, 5);

    /* output */
    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", quatAddr, quatCode);
    p_FileCodeInfo->mv_struct_nCurIndex++;
    /* Process operand */
    subfunc_code_op(operand, p_FileCodeInfo, 1);
}


/* Detects the category of instruction in the given line */
int subfunc_detect_category(const char* line, int begin_index, int count_words) {
    if (subfunc_check_code(line, begin_index, count_words))
        return _G_DEF_TYPE_CODE;
    if (subfunc_check_guide(line, begin_index, count_words))
        return _G_DEF_TYPE_DATA;
    return _G_DEF_TYPE_NONE;
}
/* step2 pass */
void subfunc_proc_pass_2(struct DF_STRUCT_AM_FileData* status) {
    char _line[256];
    char sz_code[20];
    char sz_data[20];

    /* init */
    status->mv_struct_nCurIndex = 100;
    status->mv_struct_nLineInfo = 0;

    /* output code and data information*/
    subfunc_change_quat(status->mv_struct_szCode, sz_code, sizeof(sz_code));
    subfunc_change_quat(status->mv_struct_szData, sz_data, sizeof(sz_data));
    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", sz_code, sz_data);

    /*read one line*/
    while (fgets(_line, sizeof(_line), GD_VAR_AM_TempFiles.mv_struct_AM) != NULL) {
        status->mv_struct_nLineInfo++;
        subfunc_proc_line_2(_line, status);
    }
}
/* Find a label */
struct DF_STRUCT_AM_Key* subfunc_find_token(const char* name, struct DF_STRUCT_AM_Keys* table) {
    int i;
    struct DF_STRUCT_AM_Key* w_Key = NULL;

    for (i = 0; i < table->mv_struct_nIndex; i++) {
        if (strcmp(table->mv_struct_pLabelList[i].mv_struct_sName, name) == 0) {
            w_Key = &table->mv_struct_pLabelList[i];
            break;
        }
    }
    return w_Key;
}

/* encodes matrix values as quaternary data */
void subfunc_mat_guide(const char* line, int pos, int nTokens, struct DF_STRUCT_AM_FileData* status) {
    char* _token;
    int w_value;
    char temp_code[6] = { 0 };
    char temp_addr[5] = { 0 };

    nTokens -= 1;
    while (nTokens > 0) {
        _token = subfunc_key(line, pos + 2);
        w_value = 0;
        sscanf(_token, "%d", &w_value);
        w_value &= 0x3FF;
        subfunc_get_quat(w_value, temp_code, 6);
        subfunc_get_quat(status->mv_struct_nCurIndex, temp_addr, 5);

        fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", temp_addr, temp_code);
        status->mv_struct_nCurIndex++;

        pos += 2;
        nTokens -= 2;
    }
}