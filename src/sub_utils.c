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

/* update entry */
void subfunc_entry_update() {
    int i, j, k, swapped;
    char w_addr[5] = {0};
    struct DF_STRUCT_AM_Key* w_entry;
    struct DF_STRUCT_AM_Key* w_label;

    /* sort */
    for (k = 0; k < GD_VAR_AM_EntryList.mv_struct_nIndex; k++) {
        w_entry = &GD_VAR_AM_EntryList.mv_struct_pLabelList[k];
        for (j = 0; j < GD_VAR_AM_LabelList.mv_struct_nIndex; j++) {
            w_label = &GD_VAR_AM_LabelList.mv_struct_pLabelList[j];
            if (strcmp(w_entry->mv_struct_sName, w_label->mv_struct_sName) == 0) {
                w_entry->mv_struct_address = w_label->mv_struct_address;
                break;
            }
        }
    }

    /* update */
    do {
        swapped = 0;
        for (i = 1; i < GD_VAR_AM_EntryList.mv_struct_nIndex; i++) {
            if (GD_VAR_AM_EntryList.mv_struct_pLabelList[i - 1].mv_struct_address >
                GD_VAR_AM_EntryList.mv_struct_pLabelList[i].mv_struct_address) {
                struct DF_STRUCT_AM_Key temp = GD_VAR_AM_EntryList.mv_struct_pLabelList[i - 1];
                GD_VAR_AM_EntryList.mv_struct_pLabelList[i - 1] = GD_VAR_AM_EntryList.mv_struct_pLabelList[i];
                GD_VAR_AM_EntryList.mv_struct_pLabelList[i] = temp;
                swapped++;
            }
        }
    } while (swapped > 0);

    for (i = 0; i < GD_VAR_AM_EntryList.mv_struct_nIndex; i++) {
        w_entry = &GD_VAR_AM_EntryList.mv_struct_pLabelList[i];
        subfunc_get_quat(w_entry->mv_struct_address, w_addr, 5);
        fprintf(GD_VAR_AM_TempFiles.mv_struct_ENT, "%s %s\n", w_entry->mv_struct_sName, w_addr);
    }
}

/* get address type */
int subfunc_addressing(const char* p_token) {

    int w_nMatrixStatus = 0;
    const char* _pattern = "[r";
    int _count = 0;
    const char* w_Token = p_token;

    while ((w_Token = strstr(w_Token, _pattern)) != NULL) {
        _count++;
        w_Token++;
    }

    w_nMatrixStatus = (_count == 2) ? 0 : 1;

    if (w_nMatrixStatus == 0) {
        return _G_DEF_ADDR_MATRIX;
    } else {
        if (p_token[0] == '#') return _G_DEF_ADDR_IMMEDIATE;
        else if (p_token[0] == 'r') return _G_DEF_ADDR_REGISTER;
        else return _G_DEF_ADDR_DIRECT;
    }
}
/* encodes matrix values as quaternary data */
void subfunc_guide(const char* line, int start_index, int nTokens, struct DF_STRUCT_AM_FileData* status) {
    char* _token;
    int w_type;

    _token = subfunc_key(line, start_index);
    w_type = subfunc_label_type(_token);

    if (w_type == _G_DEF_GUIDE_EDATA) {
        subfunc_data_guide(line, start_index, nTokens, status);
    }
    else if (w_type == _G_DEF_GUIDE_ESTRING) {
        subfunc_string_guide(line, start_index, status);
    }
    else if (w_type == _G_DEF_GUIDE_EMAT) {
        subfunc_mat_guide(line, start_index, nTokens, status);
    }
}
/* Converts an integer value to a 4-base character*/
void subfunc_get_quat(int value, char*  buff, int p_nBuffLen) {
    int i;
    memset(buff, 0x00, p_nBuffLen);

    for (i = p_nBuffLen - 2; i >= 0; i--) {
        buff[i] = 'a' + (value % 4);
        value /= 4;
    }
    buff[p_nBuffLen - 1] = '\0';

    return;

}

/* Converts integer to quaternary (base-4) representation */
void subfunc_change_quat(int value, char* buffer, int bufferSize) {
    int i = bufferSize - 2;  
    int j;
    int start;
    buffer[bufferSize - 1] = '\0';  

    if (value == 0) {
        buffer[i] = '0';
        i--;
    }
    else {
        while (value > 0 && i >= 0) {
            buffer[i] = 'a' + (value % 4);
            value /= 4;
            i--;
        }
    }
    
    start = i + 1;
    for (j = 0; j < bufferSize - start - 1; j++) {
        buffer[j] = buffer[start + j];
    }
    buffer[bufferSize - start - 1] = '\0';  
}

/*init for each line*/
void subfunc_init_macro_proc(struct DF_STRUCT_AM_FileData* p_stASMInfo,
                           struct DF_STRUCT_AM_Macros** macros,
                           struct DF_STRUCT_AM_Macro** active_macro) {
    *macros = subfunc_new_macros();
    *active_macro = NULL;

    p_stASMInfo->mv_struct_nLineInfo = 0;
    p_stASMInfo->mv_struct_szCode = 0;
    p_stASMInfo->mv_struct_szData = 0;
    p_stASMInfo->mv_struct_nErrCount = 0;
    p_stASMInfo->mv_struct_nCurIndex = 100;
}
/* Handle macro definitions and macro usage */
int subfunc_am_handle_macro(struct DF_STRUCT_AM_FileData* p_stASMInfo,
                            char* _line,
                            struct DF_STRUCT_AM_Macros* macros,
                            struct DF_STRUCT_AM_Macro** active_macro) {
    int _keys = subfunc_key_count(_line);
    char* w_token;
    struct DF_STRUCT_AM_Macro* _macro;
    int k;

    if (_keys == 1) {
        w_token = subfunc_key(_line, 0);
        if (strcmp(w_token, "mcroend") == 0) {
            if (*active_macro == NULL) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "macro error ... Unexpected 'mcroend' at line %d.\n", p_stASMInfo->mv_struct_nLineInfo);
                p_stASMInfo->mv_struct_nErrCount++;
            } else {
                *active_macro = NULL;
            }
            return 1;  
        }

        if (!subfunc_check_reserved(w_token)) {
            _macro = subfunc_find_macro(macros, w_token);
            if (_macro == NULL) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "macro error ... Undefined 'macro' at line %d.\n", p_stASMInfo->mv_struct_nLineInfo);
                p_stASMInfo->mv_struct_nErrCount++;
                return 1; 
            }

            for (k = 0; k < _macro->mv_struct_nLineIndex; k++) {
                subfunc_proc_line_1(_macro->mv_struct_pLineInfo[k], p_stASMInfo);
                fputs(_macro->mv_struct_pLineInfo[k], GD_VAR_AM_TempFiles.mv_struct_AM);
            }
            return 1;  
        }
    }

    if (_keys == 2) {
        w_token = subfunc_key(_line, 0);
        if (strcmp(w_token, "mcro") == 0) {
            w_token = subfunc_key(_line, 1);
            *active_macro = subfunc_reg_macro(macros, w_token);
            if (*active_macro == NULL) {
                fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR, "macro error ... Failed to define 'macro' at line %d.\n", p_stASMInfo->mv_struct_nLineInfo);
                p_stASMInfo->mv_struct_nErrCount++;
            }
            return 1;  
        }
    }

    return 0;  
}

/* Main function to process a line (macro-aware) */
void subfunc_am_line_proc(struct DF_STRUCT_AM_FileData* p_stASMInfo,
                          char* _line,
                          struct DF_STRUCT_AM_Macros* macros,
                          struct DF_STRUCT_AM_Macro** active_macro) {
    p_stASMInfo->mv_struct_nLineInfo++;

    if (subfunc_am_handle_macro(p_stASMInfo, _line, macros, active_macro)) {
        return;  
    }

    if (*active_macro == NULL) {
        subfunc_proc_line_1(_line, p_stASMInfo);
        fputs(_line, GD_VAR_AM_TempFiles.mv_struct_AM);
    } else {
        subfunc_add_macro_line(*active_macro, _line);
    }
}

/*finalize for each line*/
void finalize_macro_processing(struct DF_STRUCT_AM_Macros* macros) {
    subfunc_delete_macros(macros);
    free(macros);
}

/* Processes the assembly file */
void subfunc_proc_pass_1(struct DF_STRUCT_AM_FileData* p_stASMInfo, FILE* p_File_AS) {
    char _line[256];
    struct DF_STRUCT_AM_Macros* macros;
    struct DF_STRUCT_AM_Macro* active_macro;

    /* Init */
    subfunc_init_macro_proc(p_stASMInfo, &macros, &active_macro);

    /* each line */
    while (fgets(_line, sizeof(_line), p_File_AS) != NULL) {
        subfunc_am_line_proc(p_stASMInfo, _line, macros, &active_macro);
    }

    /* Free */
    finalize_macro_processing(macros);
}

/* Extracts the token at the specified index from a line */
char* subfunc_key(const char* p_line, int p_token_index) {
    static char GD_VAR_TokenInfo[256];  
    int current_pos = 0;
    int current_index = 0;
    int buf_index = 0;
    int i;
    int word_start = 0;

    GD_VAR_TokenInfo[0] = '\0';

    while (1) {
        /* skip space */
        while (p_line[current_pos] != '\0') {
            char c = (p_line[current_pos] == ';') ? '\0' : p_line[current_pos];
            if (!isspace(c)) break;
            current_pos++;
        }

        /* check line end */ 
        if (p_line[current_pos] == '\0' || p_line[current_pos] == ';') break;

        word_start = current_pos;

        /* find token */
        while (p_line[current_pos] != '\0') {
            char c = (p_line[current_pos] == ';') ? '\0' : p_line[current_pos];
            if (isspace(c) || c == ',' || c == ':' || c == '\0') {
                if (word_start == current_pos && (c == ',' || c == ':')) {
                    current_pos++;  
                }
                break;
            }
            current_pos++;
        }
         /* If this is the desired token, copy it to buffer */
        if (p_token_index == current_index) {
            buf_index = 0;
            for ( i = word_start; i < current_pos; i++) {
                char c = (p_line[i] == ';') ? '\0' : p_line[i];
                GD_VAR_TokenInfo[buf_index++] = c;
            }
            GD_VAR_TokenInfo[buf_index] = '\0';
            return GD_VAR_TokenInfo;
        }

        current_index++;
    }

    return GD_VAR_TokenInfo;
}
/* Determines the type of assembly guide directive based on its string identifier */
int subfunc_label_type(const char* p_data) {
    if (strcmp(p_data, ".data") == 0) 
        return _G_DEF_GUIDE_EDATA;
    else if (strcmp(p_data, ".string") == 0) 
        return _G_DEF_GUIDE_ESTRING;
    else if (strcmp(p_data, ".entry") == 0) 
        return _G_DEF_GUIDE_EENTRY;
    else if (strcmp(p_data, ".extern") == 0) 
        return _G_DEF_GUIDE_EEXTERN;
    else if (strcmp(p_data, ".mat") == 0) 
        return _G_DEF_GUIDE_EMAT;
    else 
        /* Unknown or unsupported guide */
        return _G_DEF_GUIDE_ENONE;
}

/* Handle a single line of assembly during encoding step 2 */
void subfunc_proc_line_2(const char* line, struct DF_STRUCT_AM_FileData* status) {
    char* _word;
    int _word_counts, _pos;
    _pos = 0;
    _word_counts = subfunc_key_count(line);

     /* Skip label definitions (e.g., LABEL:) */
    while (_word_counts >= 2) {
        _word = subfunc_key(line, _pos + 1);
        if (strcmp(_word, ":") != 0)
            break;
        _word_counts -= 2;
        _pos += 2;
    }

    if (_word_counts == 0) return;

    /* Dispatch to encoding logic for code or guide directives */
    if (subfunc_check_code(line, _pos, _word_counts))
        subfunc_proc_am_line(line, _pos, _word_counts, status);
    else if (subfunc_check_guide(line, _pos, _word_counts))
        subfunc_guide(line, _pos, _word_counts, status);
}

/* Processes the instruction based on its category (code or data) */
void subfunc_guideinfo(const char* line, int begin_index, int count_words, int category, struct DF_STRUCT_AM_FileData* info) {
    int calc_size = 0;

    switch (category) {
        case _G_DEF_TYPE_CODE:
            calc_size = subfunc_code_size(line, begin_index);
            info->mv_struct_szCode += calc_size;
            break;
        case _G_DEF_TYPE_DATA:
            calc_size = subfunc_label_size(line, begin_index, count_words);
            info->mv_struct_szData += calc_size;
            break;
        default:
            fprintf(GD_VAR_AM_TempFiles.mv_struct_ERROR,  "category error ... Invalid section type at line %d.\n", info->mv_struct_nLineInfo);
            info->mv_struct_nErrCount++;
            break;
    }

    info->mv_struct_nCurIndex += calc_size;
}

/* code line for step1 */
void subfunc_proc_line_1(const char* line, struct DF_STRUCT_AM_FileData* info) {
    int begin_index = 0;
    int category;
    int count_words = subfunc_labels(line, info, &begin_index);
    
    if (count_words == 0) 
        return;
    /*Detect instruction category*/
    category = subfunc_detect_category(line, begin_index, count_words);
    /*Process instruction based on its category*/
    subfunc_guideinfo(line, begin_index, count_words, category, info);
}

/* find macro */
struct DF_STRUCT_AM_Macro* subfunc_find_macro(struct DF_STRUCT_AM_Macros* p_pMacroList, const char* name) {
    int _i;
    struct DF_STRUCT_AM_Macro* _macro = NULL;

    for (_i = 0; _i < p_pMacroList->mv_struct_nIndex; _i++) {
        if (strcmp(p_pMacroList->mv_struct_pMacroList[_i]->mv_struct_sName, name) == 0) {
            _macro = p_pMacroList->mv_struct_pMacroList[_i];
            break;
        }
    }

    return _macro;
}
