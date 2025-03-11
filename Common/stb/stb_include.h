// stb_include.h - v0.02 - parse and process #include directives - public domain
//
// To build this, in one source file that includes this file do
//      #define STB_INCLUDE_IMPLEMENTATION
//
// This program parses a string and replaces lines of the form
//         #include "foo"
// with the contents of a file named "foo". It also embeds the
// appropriate #line directives. Note that all include files must
// reside in the location specified in the path passed to the API;
// it does not check multiple directories.
//
// If the string contains a line of the form
//         #inject
// then it will be replaced with the contents of the string 'inject' passed to the API.
//
// Options:
//
//      Define STB_INCLUDE_LINE_GLSL to get GLSL-style #line directives
//      which use numbers instead of filenames.
//
//      Define STB_INCLUDE_LINE_NONE to disable output of #line directives.
//
// Standard libraries:
//
//      stdio.h     FILE, fopen, fclose, fseek, ftell
//      stdlib.h    malloc, realloc, free
//      string.h    strcpy, strncmp, memcpy
//
// Credits:
//
// Written by Sean Barrett.
//
// Fixes:
//  Michal Klos

#ifndef STB_INCLUDE_STB_INCLUDE_H
#define STB_INCLUDE_STB_INCLUDE_H

// Do include-processing on the string 'str'. To free the return value, pass it to free()
char *stb_include_string(const char *str, const char *inject, const char *path_to_includes, const char *filename_for_line_directive, char error[256]);

// Concatenate the strings 'strs' and do include-processing on the result. To free the return value, pass it to free()
char *stb_include_strings(const char **strs, int count, const char *inject, const char *path_to_includes, const char *filename_for_line_directive, char error[256]);

// Load the file 'filename' and do include-processing on the string therein. note that
// 'filename' is opened directly; 'path_to_includes' is not used. To free the return value, pass it to free()
char *stb_include_file(const char *filename, const char *inject, const char *path_to_includes, char error[256]);

#endif


#ifdef STB_INCLUDE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include <stdbool.h>

// glibc users like to live dangerously
#ifndef _WIN32
#include <stdarg.h>

static int validate_string_params(char* destination, size_t size, const char* source)
{
    int result = 0;

    if (destination == nullptr)
    {
        result = EINVAL;
    }
    else if (source == nullptr)
    {
        destination[0] = '\0';
        result = EINVAL;
    }
    else if (size == 0)
    {
        result = ERANGE;

    }
    else if (strlen(source) > size)
    {
        destination[0] = '\0';
        result = ERANGE;
    }

    return result;
}

int fopen_s(FILE** file, const char* filename, const char* mode)
{
    if (file == 0 || filename == 0 || mode == 0)
    {
        return EINVAL;
    }

    *file = fopen(filename, mode);

    if (*file)
    {
        return errno;
    }

    return 0;
}

int sprintf_s(char* buffer, size_t size_of_buffer, const char* format, ...)
{
    int     ret_val;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    ret_val = vsnprintf(buffer, size_of_buffer, format, arg_ptr);
    va_end(arg_ptr);
    return ret_val;
}

int fprintf_s(FILE* stream, const char* format, ...)
{
    int     ret_val;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    ret_val = vfprintf(stream, format, arg_ptr);
    va_end(arg_ptr);
    return ret_val;
}

size_t fread_s(void* buffer, size_t buffer_size, size_t element_size, size_t count, FILE* stream)
{
    if ((element_size * count) > buffer_size)
    {
        return 0;
    }

    return fread(buffer, element_size, buffer_size, stream);
}

int strcpy_s(char* destination, size_t size, const char* source)
{
    int result = validate_string_params(destination, size, source);

    if (result == 0)
    {
        if (strncpy(destination, source, size) == nullptr)
        {
            result = ERANGE;
        }
    }

    assert(result == 0);

    return result;
}

int strcat_s(char* destination, size_t size, const char* source)
{
    int result = validate_string_params(destination, size, source);

    if (result == 0)
    {
        if (strncat(destination, source, size) == nullptr)
        {
            result = ERANGE;
        }
    }

    assert(result == 0);

    return result;
}
#endif

static char *stb_include_load_file(const char *filename, size_t *plen)
{
   char *text;
   size_t len;
   FILE *f = NULL;
   fopen_s(&f, filename, "rb");
   if (f == 0) return 0;
   fseek(f, 0, SEEK_END);
   len = (size_t) ftell(f);
   if (plen) *plen = len;
   text = (char *) malloc(len+1);
   if (text == 0) return 0;
   fseek(f, 0, SEEK_SET);
   fread(text, 1, len, f);
   fclose(f);
   text[len] = 0;
   return text;
}

#define MAX_MACROS 100

typedef struct {
    char* name;
    bool defined;
} Macro;

// Store macro definitions
static Macro macros[MAX_MACROS];
static int macro_count = 0;

// Check if a macro is defined
static bool is_macro_defined(const char* macro) {
    for (int i = 0; i < macro_count; i++) {
        if (strcmp(macros[i].name, macro) == 0) {
            return macros[i].defined;
        }
    }
    return false;  // If not found, assume undefined
}

// Add or update a macro definition
static void add_macro(const char* macro, bool defined) {
    for (int i = 0; i < macro_count; i++) {
        if (strcmp(macros[i].name, macro) == 0) {
            macros[i].defined = defined;
            return;
        }
    }
    if (macro_count < MAX_MACROS) {
        macros[macro_count].name = _strdup(macro);
        macros[macro_count].defined = defined;
        macro_count++;
    }
}

static void stb_include_free_macros() {
    int i;
    for (i = 0; i < macro_count; ++i)
        free(macros[i].name);
    macro_count = 0;
}

typedef struct
{
   int offset;
   int end;
   char *filename;
   int next_line_after;
} include_info;

static include_info *stb_include_append_include(include_info *array, int len, int offset, int end, char *filename, int next_line)
{
   include_info *z = (include_info *) realloc(array, sizeof(*z) * (len+1));
   z[len].offset   = offset;
   z[len].end      = end;
   z[len].filename = filename;
   z[len].next_line_after = next_line;
   return z;
}

static void stb_include_free_includes(include_info *array, int len)
{
   int i;
   for (i=0; i < len; ++i)
      free(array[i].filename);
   free(array);
}

static int stb_include_isspace(int ch)
{
   return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
}

// find location of all #include and #inject
static int stb_include_find_includes(const char *text, include_info **plist)
{
   int line_count = 1;
   int inc_count = 0;
   const char *s = text, *start;
   include_info *list = NULL;
   // TK Change Begin
   int ifdef_nesting = 0; // Track how many levels of ifdef statements we're in
   bool only_skip_first_ifdef = true; // If true, we only skip past the first ifdef block in the file
   bool skipping = false;
   //char* macro_name = (char*)malloc(sizeof(char) * 256);
   // TK Change End
   while (*s) {
      // parse is always at start of line when we reach here
      start = s;
      while (*s == ' ' || *s == '\t')
         ++s;
      if (*s == '#') {
         ++s;
         while (*s == ' ' || *s == '\t')
            ++s;
         /* TK Change Begin : Small modification to avoid processing includes intended for cpp files e.g.
         #ifdef __cplusplus
         #include "GLSLTypeAliases.h"
         #endif
         Note: This change does mean that includes shouldn't be defined within ifdef blocks*/
         //if (0 == strncmp(s, "ifdef", 5) && stb_include_isspace(s[5])) {
         //    ifdef_nesting++;
         //    s += 5;
         //    while (*s == ' ' || *s == '\t')
         //        ++s;
         //}
         //else if (ifdef_nesting > 0) {
         //    if (0 == strncmp(s, "endif", 5) && stb_include_isspace(s[5])) {
         //        ifdef_nesting--; // Exit one level of #ifdef
         //    }
         //}
        // Handle #define
        if (0 == strncmp(s, "define", 6) && stb_include_isspace(s[6])) {
            s += 6;
            while (*s == ' ' || *s == '\t')
                ++s;
            char* macro_name = (char*)malloc(sizeof(char) * 256);
            sscanf_s(s, "%255s", macro_name);
            add_macro(macro_name, true);
        }
        // Handle #undef
        else if (0 == strncmp(s, "undef", 5) && stb_include_isspace(s[5])) {
            s += 5;
            while (*s == ' ' || *s == '\t')
                ++s;
            char* macro_name = (char*)malloc(sizeof(char) * 256);
            sscanf_s(s, "%255s", macro_name);
            add_macro(macro_name, false);
        }
        // Handle #ifdef
        else if (0 == strncmp(s, "ifdef", 5) && stb_include_isspace(s[5])) {
            s += 5;
            while (*s == ' ' || *s == '\t')
                ++s;
            char* macro_name = (char*)malloc(sizeof(char) * 256);
            sscanf_s(s, "%255s", macro_name);

            if (!is_macro_defined(macro_name)) {
                skipping = true;  // Start skipping if the macro is undefined
            }
            ifdef_nesting++;
        }
        // Handle #ifndef
        else if (0 == strncmp(s, "ifndef", 6) && stb_include_isspace(s[6])) {
            s += 6;
            while (*s == ' ' || *s == '\t')
                ++s;
            char* macro_name = (char*)malloc(sizeof(char) * 256);
            sscanf_s(s, "%255s", macro_name);

            if (is_macro_defined(macro_name)) {
                skipping = true;  // Start skipping if the macro is defined
            }
            ifdef_nesting++;
        }
        // Handle #endif
        else if (0 == strncmp(s, "endif", 5) && stb_include_isspace(s[5])) {
            if (ifdef_nesting > 0) {
                ifdef_nesting--;
                if (ifdef_nesting == 0) {
                    skipping = false;  // Stop skipping after the first matching #endif
                }
            }
        }
        if (skipping) {
            // Skip this block entirely
            while (*s != '\r' && *s != '\n' && *s != 0)
                ++s;
            continue;
        }
         else if (0==strncmp(s, "include", 7) && stb_include_isspace(s[7])) {
            s += 7;
            while (*s == ' ' || *s == '\t')
               ++s;
            if (*s == '"') {
              const char *t = ++s;
               while (*t != '"' && *t != '\n' && *t != '\r' && *t != 0)
                  ++t;
               if (*t == '"') {
                  char *filename = (char *) malloc(t-s+1);
                  memcpy(filename, s, t-s);
                  filename[t-s] = 0;
                  s=t;
                  while (*s != '\r' && *s != '\n' && *s != 0)
                     ++s;
                  // s points to the newline, so s-start is everything except the newline
                  list = stb_include_append_include(list, inc_count++, (int)(start - text), (int)(s - text), filename, line_count+1);
               }
            }
         } else if (0==strncmp(s, "inject", 6) && (stb_include_isspace(s[6]) || s[6]==0)) {
            while (*s != '\r' && *s != '\n' && *s != 0)
               ++s;
            list = stb_include_append_include(list, inc_count++, (int)(start-text), (int)(s-text), NULL, line_count+1);
         }
      }
      while (*s != '\r' && *s != '\n' && *s != 0)
         ++s;
      if (*s == '\r' || *s == '\n') {
         s = s + (s[0] + s[1] == '\r' + '\n' ? 2 : 1);
      }
      ++line_count;
   }
   *plist = list;
   return inc_count;
}

// avoid dependency on sprintf()
static void stb_include_itoa(char str[9], int n)
{
   int i;
   for (i=0; i < 8; ++i)
      str[i] = ' ';
   str[i] = 0;

   for (i=1; i < 8; ++i) {
      str[7-i] = '0' + (n % 10);
      n /= 10;
      if (n == 0)
         break;
   }
}

static char *stb_include_append(char *str, size_t *curlen, const char *addstr, size_t addlen)
{
   str = (char *) realloc(str, *curlen + addlen);
   memcpy(str + *curlen, addstr, addlen);
   *curlen += addlen;
   return str;
}

char *stb_include_string(const char *str, const char *inject, const char *path_to_includes, const char *filename, char error[256])
{
   char temp[4096];
   include_info *inc_list;
   int i, num = stb_include_find_includes(str, &inc_list);
   size_t source_len = strlen(str);
   char *text=0;
   size_t textlen=0, last=0;
   for (i=0; i < num; ++i) {
      text = stb_include_append(text, &textlen, str+last, inc_list[i].offset - last);
      // write out line directive for the include
      #ifndef STB_INCLUDE_LINE_NONE
      #ifdef STB_INCLUDE_LINE_GLSL
      if (textlen != 0)  // GLSL #version must appear first, so don't put a #line at the top
      #endif
      {
         strcpy_s(temp, sizeof(temp), "#line ");
         stb_include_itoa(temp+6, 1);
         strcat_s(temp, sizeof(temp), " ");
         #ifdef STB_INCLUDE_LINE_GLSL
         stb_include_itoa(temp+15, i+1);
         #else
         strcat(temp, "\"");
         if (inc_list[i].filename == 0)
            strcmp(temp, "INJECT");
         else
            strcat(temp, inc_list[i].filename);
         strcat(temp, "\"");
         #endif
         strcat_s(temp, sizeof(temp), "\n");
         text = stb_include_append(text, &textlen, temp, strlen(temp));
      }
      #endif
      if (inc_list[i].filename == 0) {
         if (inject != 0)
            text = stb_include_append(text, &textlen, inject, strlen(inject));
      } else {
         char *inc;
         strcpy_s(temp, sizeof(temp), path_to_includes);
         strcat_s(temp, sizeof(temp), "/");
         strcat_s(temp, sizeof(temp), inc_list[i].filename);
         inc = stb_include_file(temp, inject, path_to_includes, error);
         if (inc == NULL) {
            stb_include_free_includes(inc_list, num);
            return NULL;
         }
         text = stb_include_append(text, &textlen, inc, strlen(inc));
         free(inc);
      }
      // write out line directive
      #ifndef STB_INCLUDE_LINE_NONE
      strcpy_s(temp, sizeof(temp), "\n#line ");
      stb_include_itoa(temp+6, inc_list[i].next_line_after);
      strcat_s(temp, sizeof(temp), " ");
      #ifdef STB_INCLUDE_LINE_GLSL
      (void)filename; // suppress unused variable warning
      stb_include_itoa(temp+15, 0);
      #else
      strcat(temp, filename != 0 ? filename : "source-file");
      #endif
      text = stb_include_append(text, &textlen, temp, strlen(temp));
      // no newlines, because we kept the #include newlines, which will get appended next
      #endif
      last = inc_list[i].end;
   }
   text = stb_include_append(text, &textlen, str+last, source_len - last + 1); // append '\0'
   stb_include_free_includes(inc_list, num);
   stb_include_free_macros();
   return text;
}

char *stb_include_strings(char **strs, int count, char *inject, char *path_to_includes, char *filename, char error[256])
{
   char *text;
   char *result;
   int i;
   size_t length=0;
   for (i=0; i < count; ++i)
      length += strlen(strs[i]);
   text = (char *) malloc(length+1);
   length = 0;
   for (i=0; i < count; ++i) {
      strcpy_s(text + length, sizeof(text) - length, strs[i]);
      length += strlen(strs[i]);
   }
   result = stb_include_string(text, inject, path_to_includes, filename, error);
   free(text);
   return result;
}

char *stb_include_file(const char *filename, const char *inject, [[maybe_unused]] const char *path_to_includes, char error[256])
{
   size_t len;
   char *result;
   char *text = stb_include_load_file(filename, &len);
   if (text == NULL) {
      strcpy_s(error, 256, "Error: couldn't load '");
      strcat_s(error, 256, filename);
      strcat_s(error, 256, "'");
      return 0;
   }
   // Use the directory of the file being included when searching for further includes
   const auto path_dir = std::filesystem::path(filename).parent_path();
   result = stb_include_string(text, inject, path_dir.string().c_str(), filename, error);
   //result = stb_include_string(text, inject, path_to_includes, filename, error);
   free(text);
   return result;
}

#if 0 // @TODO, GL_ARB_shader_language_include-style system that doesn't touch filesystem
char *stb_include_preloaded(char *str, char *inject, char *includes[][2], char error[256])
{

}
#endif

#endif // STB_INCLUDE_IMPLEMENTATION