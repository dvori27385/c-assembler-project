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

/* immediate operand into object code */
void subfunc_immediate_code_op(const char* operand, struct DF_STRUCT_AM_FileData* status) {
    int w_temp, w_value;
    char s_code[6] = { 0 };
    char s_addr[5] = { 0 };

    sscanf(&operand[1], "%d", &w_temp);
    w_temp &= 0xFF;
    w_value = (w_temp << 2) | _G_DEF_AM_ABSOLUTE;
    
    subfunc_get_quat(w_value, s_code, 6);
    subfunc_get_quat(status->mv_struct_nCurIndex, s_addr, 5);

    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", s_addr, s_code);
    status->mv_struct_nCurIndex++;
}

/* Handles macro directives (mcro and mcroend) */
void subfunc_macro_directives(char* line, struct DF_STRUCT_AM_FileData* info, struct DF_STRUCT_AM_Macros* macros, struct DF_STRUCT_AM_Macro** active_macro) {
    int word_count = subfunc_key_count(line);
    char* word;

    /*If there's only one word in the line */
    if (word_count == 1) {
        word = subfunc_key(line, 0);
        if (strcmp(word, "mcroend") == 0) {
            if (*active_macro == NULL) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "macro error ... Unexpected 'mcroend' at line %d.\n", info->mv_struct_nLineInfo);
                info->mv_struct_nErrCount++;
            } else {
                *active_macro = NULL;
            }
        }
    }
    /*If there are two words */ 
    else if (word_count == 2) {
        word = subfunc_key(line, 0);
        if (strcmp(word, "mcro") == 0) {
            word = subfunc_key(line, 1);
            *active_macro = subfunc_reg_macro(macros, word);
            if (*active_macro == NULL) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "macro error ... Failed to define 'macro' at line %d.\n", info->mv_struct_nLineInfo);
                info->mv_struct_nErrCount++;
            }
        }
    }
}


/* register new macro */
struct DF_STRUCT_AM_Macro* subfunc_reg_macro(struct DF_STRUCT_AM_Macros* p_pMacroList, const char* name) {
    struct DF_STRUCT_AM_Macro* _macro;

    _macro = subfunc_find_macro(p_pMacroList, name);
    if (_macro != NULL) return NULL;
    
    _macro = subfunc_new_macro(name);
    
    if (p_pMacroList->mv_struct_nIndex == p_pMacroList->mv_struct_nCount) {
        p_pMacroList->mv_struct_nCount += 16;
        p_pMacroList->mv_struct_pMacroList = realloc(p_pMacroList->mv_struct_pMacroList, sizeof(*p_pMacroList->mv_struct_pMacroList) * p_pMacroList->mv_struct_nCount);
    }

    p_pMacroList->mv_struct_pMacroList[p_pMacroList->mv_struct_nIndex] = _macro;
    p_pMacroList->mv_struct_nIndex++;

    return _macro;
}

/* Free macro */
void subfunc_delete_macros(struct DF_STRUCT_AM_Macros* p_pMacroList) {
    int _i;
    for (_i = 0; _i < p_pMacroList->mv_struct_nIndex; _i++) {
        subfunc_delete_macro(p_pMacroList->mv_struct_pMacroList[_i]);
        free(p_pMacroList->mv_struct_pMacroList[_i]);
    }

    free(p_pMacroList->mv_struct_pMacroList);
}

/* Add line of instruction to macro structure */
void subfunc_add_macro_line(struct DF_STRUCT_AM_Macro* p_pstMacro, const char* p_pline) {
    if (p_pstMacro->mv_struct_nLineIndex == p_pstMacro->mv_struct_nLineCnt) {
        p_pstMacro->mv_struct_nLineCnt += 16;
        p_pstMacro->mv_struct_pLineInfo = realloc(p_pstMacro->mv_struct_pLineInfo, sizeof(*p_pstMacro->mv_struct_pLineInfo) * p_pstMacro->mv_struct_nLineCnt);
    }

    p_pstMacro->mv_struct_pLineInfo[p_pstMacro->mv_struct_nLineIndex] = malloc(strlen(p_pline) + 1);
    strcpy(p_pstMacro->mv_struct_pLineInfo[p_pstMacro->mv_struct_nLineIndex], p_pline);

    p_pstMacro->mv_struct_nLineIndex++;
}

/* Free macro structure */
void subfunc_delete_macro(struct DF_STRUCT_AM_Macro* macro) {
    int k;
    for (k = 0; k < macro->mv_struct_nLineIndex; k++) 
        free(macro->mv_struct_pLineInfo[k]);
    free(macro->mv_struct_pLineInfo);
}

/* Create new macro array */
struct DF_STRUCT_AM_Macros* subfunc_new_macros() {
    struct DF_STRUCT_AM_Macros* st_mac;
    st_mac = malloc(sizeof(*st_mac));

    st_mac->mv_struct_nCount = 0;
    st_mac->mv_struct_nIndex = 0;
    st_mac->mv_struct_pMacroList = NULL;

    return st_mac;
}


/* Computes the total size of an instruction in assembly based on its operands */
int subfunc_code_size(const char* p_line, int p_begin_index) {
    char* _word;
    char* _token1;
    char* _token2;

    int _bRegAddr1 = 0;
    int _bRegAddr2 = 0;

    int _result = 0;
    const struct DF_STRUCT_AM_Code* _guideinfo = NULL;

    _word = subfunc_key(p_line, p_begin_index);
    _guideinfo = subfunc_code(_word);
    if (_guideinfo == NULL) return _result;

    _result = 1;
    if (_guideinfo->mv_struct_nOperand == 1) {
        _token1 = subfunc_key(p_line, p_begin_index + 1);
        if (subfunc_addressing(_token1) == _G_DEF_ADDR_MATRIX)
            _result += 2;
        else
            _result += 1;
    } else if (_guideinfo->mv_struct_nOperand == 2) {
        /* First operand */
         _token1 = subfunc_key(p_line, p_begin_index + 1);
        if (subfunc_addressing(_token1) == _G_DEF_ADDR_MATRIX)
            _result += 2;
        else
            _result += 1;

        if (subfunc_addressing(_token1) == _G_DEF_ADDR_REGISTER) {
            _bRegAddr1 = 1;
        }

        /* Second operand */
        _token2 = subfunc_key(p_line, p_begin_index + 3);
        if (subfunc_addressing(_token2) == _G_DEF_ADDR_MATRIX)
            _result += 2;
        else
            _result += 1;

        if (subfunc_addressing(_token2) == _G_DEF_ADDR_REGISTER) {
            _bRegAddr2 = 1;
        }

        if (_bRegAddr1 == 1 && _bRegAddr2 == 1) {
            _result -= 1;
        }
    }

    return _result;
}

/* Validates a guide directive in a line starting from a given index. */
int subfunc_check_guide(const char* _line, int _pos, int _keys) {
    char* w_token;
    int v_gType, _result = 1;

    w_token = subfunc_key(_line, _pos);
    v_gType = subfunc_label_type(w_token);

    switch (v_gType) {
    case _G_DEF_GUIDE_EDATA:
        /* Check that data directives have pairs of operands */
        if (_keys % 2 != 0) {
            _result = 0;
            break;
        }
        while (1) {
            _keys -= 2;
            if (_keys <= 0) 
                break;
            w_token = subfunc_key(_line, _pos + 2);
            /* Ensure operands are separated by commas */
            if (strcmp(w_token, ",") != 0) {
                _result = 0;
                break;
            }
            _pos += 2;
        }
        break;
    case _G_DEF_GUIDE_ESTRING:
        /* Must contain exactly one string argument */
        if (_keys != 2) {
            _result = 0;
            break;
        }
        w_token = subfunc_key(_line, _pos + 1);
         /* Ensure it starts and ends with double quotes */
        if (w_token[0] != '"' || w_token[strlen(w_token) - 1] != '"') 
            _result = 0;
        break;
    case _G_DEF_GUIDE_EENTRY:
        if (_keys != 2) {
            _result = 0;
            break;
        }
        w_token = subfunc_key(_line, _pos + 1);
        /* Add label to entry info list */
        subfunc_asm_addkey(w_token, 0, &GD_VAR_AM_EntryList);
        break;
    case _G_DEF_GUIDE_EEXTERN:
        if (_keys != 2) {
            _result = 0;
            break;
        }
        break;
    case _G_DEF_GUIDE_EMAT:
        if ((_keys - 1) % 2 != 0) {
            _result = 0;
            break;
        }
        break;
    default:
        _result = 0;
        break;
    }
    return _result;
}

/* Processes labels in the given line */
int subfunc_labels(const char* line, struct DF_STRUCT_AM_FileData* info, int* p_index) {
    int count_words = subfunc_key_count(line);
    int label_valid, label_unique;
    char* word;
    int index = 0;

    /*Loop through tokens to identify and process labels*/
    while (count_words - index >= 2) {
        if (strcmp(subfunc_key(line, index + 1), ":") == 0) {
            word = subfunc_key(line, index);
            label_valid = subfunc_check_label(word);

            /* If label is invalid, report error*/
            if (!label_valid) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "label error ... Invalid label name '%s' at line %d.\n", word, info->mv_struct_nLineInfo);
                info->mv_struct_nErrCount++;
                index += 2;
                continue;
            }
            /* add the label information*/
            label_unique = subfunc_asm_addkey(word, info->mv_struct_nCurIndex, &GD_VAR_AM_LabelList);
            if (!label_unique) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "label error ... Duplicate label '%s' at line %d.\n", word, info->mv_struct_nLineInfo);
                info->mv_struct_nErrCount++;
                index += 2;
                continue;
            }
            index += 2;
        } else {
            break;
        }
    }

    *p_index = index;
    return subfunc_key_count(line) - index; 
}

/* Encode guide information */
void subfunc_data_guide(const char* line, int start_index, int count_tokens, struct DF_STRUCT_AM_FileData* status) {
    char* _token;
    int w_value;
    char _quatCode[6] = { 0 };
    char _quatAddr[5] = { 0 };

    while (count_tokens > 0) {
        _token = subfunc_key(line, start_index + 1);
        w_value = 0;
        sscanf(_token, "%d", &w_value);
        w_value &= 0x3FF;
        /* convert 4-digit*/
        subfunc_get_quat(w_value, _quatCode, 6);
        subfunc_get_quat(status->mv_struct_nCurIndex, _quatAddr, 5);

        /* output the information*/
        fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", _quatAddr, _quatCode);
        status->mv_struct_nCurIndex++;

        start_index += 2;
        count_tokens -= 2;
    }
}
/* encodes characters as quaternary code*/
void subfunc_string_guide(const char* line, int start_index, struct DF_STRUCT_AM_FileData* status) {
    char* _token;
    int k, _str_len, w_value;
    char _quatCode[6] = { 0 };
    char _quatAddr[5] = { 0 };

    _token = subfunc_key(line, start_index + 1);
    _str_len = (int)(strlen(_token) - 1);
    for (k = 1; k < _str_len; k++) {
        char _character = _token[k];
        w_value = _character & 0xFF;
        w_value &= 0x3FF;
        subfunc_get_quat(w_value, _quatCode, 6);
        subfunc_get_quat(status->mv_struct_nCurIndex, _quatAddr, 5);

        fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", _quatAddr, _quatCode);
        status->mv_struct_nCurIndex++;
    }
    /* null-terminate string*/
    subfunc_get_quat(0, _quatCode, 6);
    subfunc_get_quat(status->mv_struct_nCurIndex, _quatAddr, 5);

    fprintf(GD_VAR_AM_TempFiles.mv_struct_OBJ, "%s %s\n", _quatAddr, _quatCode);
    status->mv_struct_nCurIndex++;
}