#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "macro.h"
#include "utils.h"   /* trim_string, split_string */
#include "error.h"   /* print_error */

/* Initialize macro table */
void init_macro_table(MacroTable *mt) {
    mt->count = 0;
}

/* Helper: find macro definition by name */
static MacroDef *find_macro(MacroTable *mt, const char *name) {
    for (int i = 0; i < mt->count; i++)
        if (strcmp(mt->macros[i].name, name) == 0)
            return &mt->macros[i];
    return NULL;
}

/* Scan once for “MACRO name p1,p2… “ until “ENDM” */
bool scan_macros(const char *lines[], int line_count, MacroTable *mt) {
    for (int i = 0; i < line_count; i++) {
        char buf[MAX_LINE_LEN];
        strncpy(buf, lines[i], MAX_LINE_LEN-1);
        buf[MAX_LINE_LEN-1] = '\0';
        trim_string(buf);
        if (strncasecmp(buf, "MACRO", 5)==0 && isspace((unsigned char)buf[5])) {
            if (mt->count >= MAX_MACROS) {
                print_error("Too many macros");
                return false;
            }
            /* parse header: MACRO name param,param… */
            char *p = buf + 5;
            trim_string(p);
            char *tok = strtok(p, " \t");
            if (!tok) { print_error("Invalid MACRO header"); return false; }
            MacroDef *md = &mt->macros[mt->count++];
            strncpy(md->name, tok, MAX_MACRO_NAME-1);
            md->body_len = 0;

            /* parse params if any */
            char *plist = strtok(NULL, "");
            md->param_count = 0;
            if (plist) {
                char *p2 = strtok(plist, ",");
                while (p2 && md->param_count<MAX_MACRO_PARAMS) {
                    trim_string(p2);
                    strncpy(md->params[md->param_count++], p2, MAX_MACRO_NAME-1);
                    p2 = strtok(NULL, ",");
                }
            }
            /* collect body until ENDM */
            int j = i+1;
            for (; j < line_count; j++) {
                char tmp[MAX_LINE_LEN];
                strncpy(tmp, lines[j], MAX_LINE_LEN-1);
                tmp[MAX_LINE_LEN-1] = '\0';
                trim_string(tmp);
                if (strcasecmp(tmp, "ENDM")==0) {
                    break;
                }
                md->body[md->body_len++] = strdup(tmp);
            }
            if (j>=line_count) {
                print_error("Missing ENDM for MACRO");
                return false;
            }
            i = j;  /* continue after ENDM */
        }
    }
    return true;
}

/* Replace each macro invocation with its body, substituting params */
char **expand_macros(const char *lines[], int in_count, int *out_count, MacroTable *mt) {
    char **out = malloc(sizeof(char*) * (in_count * 2));
    int oc = 0;

    for (int i = 0; i < in_count; i++) {
        char buf[MAX_LINE_LEN];
        strncpy(buf, lines[i], MAX_LINE_LEN-1);
        buf[MAX_LINE_LEN-1] = '\0';
        trim_string(buf);
        if (buf[0]=='\0') {
            out[oc++] = strdup(buf);
            continue;
        }
        /* check first token = macro name? */
        char *tok = strtok(buf, " \t");
        MacroDef *md = find_macro(mt, tok);
        if (!md) {
            out[oc++] = strdup(lines[i]);
        } else {
            /* parse arguments on invocation */
            char *aplist = strtok(NULL, "");
            char *args[MAX_MACRO_PARAMS];
            int ac=0;
            if (aplist) {
                char *p = strtok(aplist, ",");
                while (p && ac<md->param_count) {
                    trim_string(p);
                    args[ac++] = p;
                    p = strtok(NULL, ",");
                }
            }
            /* for each body line, substitute %param% */
            for (int b=0; b<md->body_len; b++) {
                char *tmp = strdup(md->body[b]);
                if (!tmp) error_exit("Memory allocation failed");
                for (int pi=0; pi<md->param_count; pi++) {
                    char pattern[64], repl[64];
                    snprintf(pattern, sizeof(pattern), "%%%s%%", md->params[pi]);
                    snprintf(repl, sizeof(repl), "%s", (pi<ac?args[pi]:""));
                    char *repl_tmp = replace_substring(tmp, pattern, repl);
                    free(tmp);
                    if (!repl_tmp) error_exit("Memory allocation failed");
                    tmp = repl_tmp;
                }
                out[oc++] = tmp;
            }
        }
    }

    *out_count = oc;
    return out;
}

