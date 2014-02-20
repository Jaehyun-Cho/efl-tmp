#include <getopt.h>

#include <Eina.h>
#include <Ecore_File.h>

#include "Eolian.h"
#include "legacy_generator.h"
#include "eo1_generator.h"
#include "common_funcs.h"

#define EO_SUFFIX ".eo"

static int eo_version = 0;
static Eina_Bool legacy_support = EINA_FALSE;

static Eina_Bool
_generate_h_file(char *filename, const char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;
   Eina_Strbuf *hfile = eina_strbuf_new();
   if (append)
     {
        Eina_File *fn = eina_file_open(filename, EINA_FALSE);
        if (!fn)
          {
            ERR ("Cant open file \"%s\" for updating.", filename);
            goto end;
          }

        eina_strbuf_append(hfile, (char*)eina_file_map_all(fn, EINA_FILE_SEQUENTIAL));
        eina_file_close(fn);

        if (!legacy_header_append(classname, eo_version, hfile))
          {
             ERR("Failed to generate header for %s", classname);
             goto end;
          }
     }
   else
     {
        if (!eo1_header_generate(classname, hfile))
          {
             ERR("Failed to generate header for %s", classname);
             goto end;
          }
     }
   const char *htext = eina_strbuf_string_get(hfile);

   FILE* fd = fopen(filename, "w");
   if (!fd)
     {
        ERR ("Couldn't open file %s for writing", filename);
        goto end;
     }
   if (htext) fputs(htext, fd);
   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(hfile);

   return ret;
}

static Eina_Bool
_generate_c_file(char *filename, const char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *cfile = eina_strbuf_new();
   if (!legacy_source_generate(classname, legacy_support, eo_version, cfile))
     {
        ERR("Failed to generate source for %s", classname);
        goto end;
     }

   FILE* fd = fopen(filename, (append) ? "a" : "w");
   if (!fd)
     {
        ERR("Couldnt open file %s for writing", filename);
        goto end;
     }
   const char *ctext = eina_strbuf_string_get(cfile);
   if (ctext) fputs(ctext, fd);
   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(cfile);
   return ret;
}

// TODO join with header gen.
static Eina_Bool
_generate_legacy_header_file(char *filename, const char *classname, Eina_Bool append)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *lfile = eina_strbuf_new();

   if (append)
     {
        Eina_File *fn = eina_file_open(filename, EINA_FALSE);
        if (!fn)
          {
            ERR ("Cant open file \"%s\" for updating.", filename);
            goto end;
          }
        eina_strbuf_append(lfile, (char*)eina_file_map_all(fn, EINA_FILE_SEQUENTIAL));
        eina_file_close(fn);

        if (!legacy_header_append(classname, eo_version, lfile))
          {
             ERR("Failed to generate header for %s", classname);
             goto end;
          }
     }
   else
     {
        if (!eo1_header_generate(classname, lfile))
          {
             ERR("Failed to generate header for %s", classname);
             goto end;
          }
     }

   FILE* fd = fopen(filename, "w");
   if (!fd)
     {
        ERR ("Couldnt open file %s for writing", filename);
        goto end;
     }
   const char *ltext = eina_strbuf_string_get(lfile);
   if (ltext) fputs(ltext, fd);
   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(lfile);
   return ret;
}

static Eina_Bool
_generate_eo_and_legacy_h_file(char *filename, const char *classname)
{
   Eina_Bool ret = EINA_FALSE;

   Eina_Strbuf *hfile = eina_strbuf_new();

   if (!eo1_header_generate(classname, hfile))
     {
        ERR("Failed to generate header for %s", classname);
        goto end;
     }
   if (!legacy_header_generate(classname, eo_version, hfile))
     {
        ERR("Failed to generate header for %s", classname);
        goto end;
     }

   const char *htext = eina_strbuf_string_get(hfile);
   FILE* fd = fopen(filename, "w");
   if (!fd)
     {
        ERR ("Couldnt open file %s for writing", filename);
        goto end;
     }

   if (htext) fputs(htext, fd);

   fclose(fd);

   ret = EINA_TRUE;
end:
   eina_strbuf_free(hfile);
   return ret;
}

enum
{
   NO_WAY_GEN,
   H_GEN,
   C_GEN,
   H_EO_APP,
   H_LEG_APP,
   H_LEG_EO_GEN
};
int gen_opt = NO_WAY_GEN;

int main(int argc, char **argv)
{
   int ret = 1;
   Eina_Bool help = EINA_FALSE, show = EINA_FALSE;
   Eina_List *included_files = NULL, *itr;
   Eina_List *files4gen = NULL;
   const char *classname;
   char *output_filename = NULL; /* if NULL, have to generate, otherwise use the name stored there */

   eina_init();
   eolian_init();

   const char *log_dom = "eolian_gen";
   _eolian_gen_log_dom = eina_log_domain_register(log_dom, EINA_COLOR_GREEN);
   if (_eolian_gen_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", log_dom);
        goto end;
     }

   eina_log_timing(_eolian_gen_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   static struct option long_options[] =
     {
        /* These options set a flag. */
          {"eo1",        no_argument,         &eo_version, 1},
          {"eo2",        no_argument,         &eo_version, 2},
          {"verbose",    no_argument,         0, 'v'},
          {"help",       no_argument,         0, 'h'},
          {"gh",         no_argument,         &gen_opt, H_GEN},
          {"gc",         no_argument,         &gen_opt, C_GEN},
          {"ah",         no_argument,         &gen_opt, H_EO_APP},
          {"al",         no_argument,         &gen_opt, H_LEG_APP},
          {"gle",        no_argument,         &gen_opt, H_LEG_EO_GEN},
          {"output",     required_argument,   0, 'o'},
          {"legacy",     no_argument,         (int *)&legacy_support, EINA_TRUE},
          {"include",    required_argument,   0, 'I'},
          {0, 0, 0, 0}
     };
   int long_index =0, opt;
   while ((opt = getopt_long(argc, argv,"vho:I:", long_options, &long_index )) != -1)
     {
        switch (opt) {
           case 0: break;
           case 'o':
                   {
                      output_filename = strdup(optarg);
                      break;
                   }
           case 'v': show = EINA_TRUE; break;
           case 'h': help = EINA_TRUE; break;
           case 'I':
                     {
                        const char *dir = optarg;
                        if (ecore_file_is_dir(dir))
                          {
                             Eina_List *dir_files;
                             char *file;
                             /* Get all files from directory. Not recursively!!!*/
                             dir_files = ecore_file_ls(dir);
                             EINA_LIST_FREE(dir_files, file)
                               {
                                  char *filepath = malloc(strlen(dir) + 1 + strlen(file) + 1);
                                  sprintf(filepath, "%s/%s", dir, file);
                                  if ((!ecore_file_is_dir(filepath)) && eina_str_has_suffix(filepath, EO_SUFFIX))
                                    {
                                       /* Allocated string will be freed during deletion of "included_files" list. */
                                       included_files = eina_list_append(included_files, strdup(filepath));
                                    }
                                  free(filepath);
                                  free(file);
                               }
                          }
                        break;
                     }
           default: help = EINA_TRUE;
        }
     }
   while (optind < argc)
      files4gen = eina_list_append(files4gen, argv[optind++]);

   if (help)
     {
        printf("Usage: %s [-h/--help] [-v/--verbose] [-I/--include input_dir] [--legacy] [--gh|--gc|--ah] [--output/-o outfile] file.eo ... \n", argv[0]);
        printf("       --help/-h Print that help\n");
        printf("       --include/-I Include 'input_dir' as directory to search .eo files into\n");
        printf("       --output/-o Force output filename to 'outfile'\n");
        printf("       --eo1/--eo2 Set generator to eo1/eo2 mode. Must be specified\n");
        printf("       --gh Generate c header file [.h]\n");
        printf("       --gc Generate c source file [.c]\n");
        printf("       --ah Append eo class definitions to an existing c header file [.h]\n");
        printf("       --al Append legacy function definitions to an existing c header file [.h]\n");
        printf("       --gle Generate eo and legacy file [.h]\n");
        printf("       --legacy Generate legacy\n");
        ret = 0;
        goto end;
     }

   if (!files4gen)
     {
        ERR("No input files specified.\nTerminating.\n");
        goto end;
     }

   const char *filename;
   EINA_LIST_FOREACH(included_files, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             ERR("Error during parsing file %s\n", filename);
             goto end;
          }
     }

   EINA_LIST_FOREACH(files4gen, itr, filename)
     {
        if (!eolian_eo_file_parse(filename))
          {
             ERR("Error during parsing file %s\n", filename);
             goto end;
          }
     }

   if (show)
     {
        EINA_LIST_FOREACH(files4gen, itr, filename)
          {
             const char *cname = eolian_class_find_by_file(filename);
             if (cname) eolian_show(cname);
          }
     }

   if (!eo_version)
     {
        ERR("No eo version specified (use --eo1 or --eo2). Aborting eo generation.\n");
        goto end;
     }

   classname = eolian_class_find_by_file(eina_list_data_get(files4gen));

   if (gen_opt)
     {
        if (!output_filename)
          {
             output_filename = malloc(strlen(eina_list_data_get(files4gen)) + 5);
             strcpy(output_filename, eina_list_data_get(files4gen));
             if (C_GEN == gen_opt) strcat(output_filename, ".c");
             else strcat(output_filename, ".h");
          }
        switch (gen_opt)
          {
           case H_GEN: case H_EO_APP:
                {
                   INF("%s header file %s\n", (gen_opt == H_EO_APP) ? "Appending" : "Generating", output_filename);
                   ret = (_generate_h_file(output_filename, classname, gen_opt == H_EO_APP)?0:1);
                   break;
                }
           case H_LEG_APP:
                {
                   INF("Appending legacy file %s\n", output_filename);
                   ret = _generate_legacy_header_file(output_filename, classname, EINA_TRUE)?0:1;
                   break;
                }
           case H_LEG_EO_GEN:
                {
                   INF("Generating eo and legacy header file %s\n", output_filename);
                   ret = _generate_eo_and_legacy_h_file(output_filename, classname)?0:1;
                   break;
                }
           case C_GEN:
                {
                   INF("Generating source file %s\n", output_filename);
                   ret = _generate_c_file(output_filename, classname, EINA_FALSE)?0:1;
                   break;
                }
           default:
              ERR("Bad generation option\n");
              break;
          }
        free(output_filename);
     }
   else ret = 0;

end:
   EINA_LIST_FREE(included_files, filename)
      free((char *)filename);
   eina_list_free(files4gen);

   eina_log_timing(_eolian_gen_log_dom,
         EINA_LOG_STATE_START,
         EINA_LOG_STATE_SHUTDOWN);
   eina_log_domain_unregister(_eolian_gen_log_dom);
   _eolian_gen_log_dom = -1;

   eolian_shutdown();
   eina_shutdown();
   return ret;
}
