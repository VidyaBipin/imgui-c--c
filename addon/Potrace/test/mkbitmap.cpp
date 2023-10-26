#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "greymap.h"
#include "backend.h"
#include "bitmap_io.h"

#define MKBITMAP "mkbitmap"

/* ---------------------------------------------------------------------- */

/* process a single file, containing one or more images. On error,
   print error message to stderr and exit with code 2. On warning,
   print warning message to stderr. */

static void process_file(FILE *fin, FILE *fout, const char *infile, const char *outfile)
{
  int r;
  greymap_t *gm;
  potrace_bitmap_t *bm;
  void *sm;
  int x, y;
  int count;

  for (count = 0;; count++)
  {
    r = gray_read(fin, &gm);
    switch (r)
    {
    case -1: /* system error */
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", infile, strerror(errno));
      exit(2);
    case -2: /* corrupt file format */
      fprintf(stderr, "" MKBITMAP ": %s: file format error: %s\n", infile, gm_read_error);
      exit(2);
    case -3: /* empty file */
      if (count > 0)
      { /* end of file */
        return;
      }
      fprintf(stderr, "" MKBITMAP ": %s: empty file\n", infile);
      exit(2);
    case -4: /* wrong magic */
      if (count > 0)
      {
        fprintf(stderr, "" MKBITMAP ": %s: warning: junk at end of file\n", infile);
        return;
      }
      fprintf(stderr, "" MKBITMAP ": %s: file format not recognized\n", infile);
      fprintf(stderr, "Possible input file formats are: pnm (pbm, pgm, ppm), bmp.\n");
      exit(2);
    case 1: /* unexpected end of file */
      fprintf(stderr, "" MKBITMAP ": %s: warning: premature end of file\n", infile);
      break;
    }

    if (info.invert)
    {
      for (y = 0; y < gm->h; y++)
      {
        for (x = 0; x < gm->w; x++)
        {
          GM_UPUT(gm, x, y, 255 - GM_UGET(gm, x, y));
        }
      }
    }

    if (info.highpass)
    {
      r = highpass(gm, info.lambda);
      if (r)
      {
        fprintf(stderr, "" MKBITMAP ": %s: %s\n", infile, strerror(errno));
        exit(2);
      }
    }

    if (info.lowpass)
    {
      lowpass(gm, info.lambda1);
    }

    if (info.scale == 1 && info.bilevel)
    { /* no interpolation necessary */
      sm = threshold(gm, info.level);
      gm_free(gm);
    }
    else if (info.scale == 1)
    {
      sm = gm;
    }
    else if (info.linear)
    { /* linear interpolation */
      sm = interpolate_linear(gm, info.scale, info.bilevel, info.level);
      gm_free(gm);
    }
    else
    { /* cubic interpolation */
      sm = interpolate_cubic(gm, info.scale, info.bilevel, info.level);
      gm_free(gm);
    }
    if (!sm)
    {
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", infile, strerror(errno));
      exit(2);
    }

    if (info.bilevel)
    {
      bm = (potrace_bitmap_t *)sm;
      bm_writepbm(fout, bm);
      bm_free(bm);
    }
    else
    {
      gm = (greymap_t *)sm;
      gm_writepgm(fout, gm, NULL, 1, GM_MODE_POSITIVE, 1.0);
      gm_free(gm);
    }
  }
}

/* ---------------------------------------------------------------------- */
/* some info functions and option processing */

static int license(FILE *f)
{
  fprintf(f,
          "This program is free software; you can redistribute it and/or modify\n"
          "it under the terms of the GNU General Public License as published by\n"
          "the Free Software Foundation; either version 2 of the License, or\n"
          "(at your option) any later version.\n"
          "\n"
          "This program is distributed in the hope that it will be useful,\n"
          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
          "GNU General Public License for more details.\n"
          "\n"
          "You should have received a copy of the GNU General Public License\n"
          "along with this program; if not, write to the Free Software Foundation\n"
          "Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n");
  return 0;
}

static int usage(FILE *f)
{
  fprintf(f, "Usage: " MKBITMAP " [options] [file...]\n");
  fprintf(f, "Options:\n");
  fprintf(f, " -h, --help           - print this help message and exit\n");
  fprintf(f, " -v, --version        - print version info and exit\n");
  fprintf(f, " -l, --license        - print license info and exit\n");
  fprintf(f, " -o, --output <file>  - output to file\n");
  fprintf(f, " -x, --nodefaults     - turn off default options\n");
  fprintf(f, "Inversion:\n");
  fprintf(f, " -i, --invert         - invert the input (undo 'blackboard' effect)\n");
  fprintf(f, "Highpass filtering:\n");
  fprintf(f, " -f, --filter <n>     - apply highpass filter with radius n (default 4)\n");
  fprintf(f, " -n, --nofilter       - no highpass filtering\n");
  fprintf(f, " -b, --blur <n>       - apply lowpass filter with radius n (default: none)\n");
  fprintf(f, "Scaling:\n");
  fprintf(f, " -s, --scale <n>      - scale by integer factor n (default 2)\n");
  fprintf(f, " -1, --linear         - use linear interpolation\n");
  fprintf(f, " -3, --cubic          - use cubic interpolation (default)\n");
  fprintf(f, "Thresholding:\n");
  fprintf(f, " -t, --threshold <n>  - set threshold for bilevel conversion (default 0.45)\n");
  fprintf(f, " -g, --grey           - no bilevel conversion, output a greymap\n");

  fprintf(f, "\n");
  fprintf(f, "Possible input file formats are: pnm (pbm, pgm, ppm), bmp.\n");
  fprintf(f, "The default options are: -f 4 -s 2 -3 -t 0.45\n");

  return 0;
}

static struct option longopts[] = {
    {"help", 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {"license", 0, 0, 'l'},
    {"output", 1, 0, 'o'},
    {"reset", 0, 0, 'x'},
    {"invert", 0, 0, 'i'},
    {"filter", 1, 0, 'f'},
    {"nofilter", 0, 0, 'n'},
    {"blur", 1, 0, 'b'},
    {"scale", 1, 0, 's'},
    {"linear", 0, 0, '1'},
    {"cubic", 0, 0, '3'},
    {"grey", 0, 0, 'g'},
    {"threshold", 1, 0, 't'},
    {0, 0, 0, 0}};

static const char *shortopts = "hvlo:xif:nb:s:13gt:";

/* process options. On error, print error message to stderr and exit
   with code 1 */
static void dopts(int ac, char *av[])
{
  int c;
  char *p;

  /* set defaults for command line parameters */
  info.outfile = NULL;  /* output file */
  info.infiles = NULL;  /* input files */
  info.infilecount = 0; /* how many input files? */
  info.invert = 0;      /* invert input? */
  info.highpass = 1;    /* use highpass filter? */
  info.lambda = 4;      /* highpass filter radius */
  info.lowpass = 0;     /* use lowpass filter? */
  info.lambda1 = 0;     /* lowpass filter radius */
  info.scale = 2;       /* scaling factor */
  info.linear = 0;      /* linear scaling? */
  info.bilevel = 1;     /* convert to bilevel? */
  info.level = 0.45;    /* cutoff grey level */

  while ((c = getopt_long(ac, av, shortopts, longopts, NULL)) != -1)
  {
    switch (c)
    {
    case 'h':
      fprintf(stdout, "" MKBITMAP " " VERSION ". Transforms images into bitmaps with scaling and filtering.\n\n");
      usage(stdout);
      exit(0);
      break;
    case 'v':
      fprintf(stdout, "" MKBITMAP " " VERSION ". Copyright (C) 2001-2019 Peter Selinger.\n");
      exit(0);
      break;
    case 'l':
      fprintf(stdout, "" MKBITMAP " " VERSION ". Copyright (C) 2001-2019 Peter Selinger.\n\n");
      license(stdout);
      exit(0);
      break;
    case 'o':
      free(info.outfile);
      info.outfile = strdup(optarg);
      if (!info.outfile)
      {
        fprintf(stderr, "" MKBITMAP ": %s\n", strerror(errno));
        exit(2);
      }
      break;
    case 'x':
      info.invert = 0;
      info.highpass = 0;
      info.scale = 1;
      info.bilevel = 0;
      // info.outext = ".pgm";
      break;
    case 'i':
      info.invert = 1;
      break;
    case 'f':
      info.highpass = 1;
      info.lambda = strtod(optarg, &p);
      if (*p || info.lambda < 0)
      {
        fprintf(stderr, "" MKBITMAP ": invalid filter radius -- %s\n", optarg);
        exit(1);
      }
      break;
    case 'n':
      info.highpass = 0;
      break;
    case 'b':
      info.lowpass = 1;
      info.lambda1 = strtod(optarg, &p);
      if (*p || info.lambda1 < 0)
      {
        fprintf(stderr, "" MKBITMAP ": invalid filter radius -- %s\n", optarg);
        exit(1);
      }
      break;
    case 's':
      info.scale = strtol(optarg, &p, 0);
      if (*p || info.scale <= 0)
      {
        fprintf(stderr, "" MKBITMAP ": invalid scaling factor -- %s\n", optarg);
        exit(1);
      }
      break;
    case '1':
      info.linear = 1;
      break;
    case '3':
      info.linear = 0;
      break;
    case 'g':
      info.bilevel = 0;
      // info.outext = ".pgm";
      break;
    case 't':
      info.bilevel = 1;
      // info.outext = ".pbm";
      info.level = strtod(optarg, &p);
      if (*p || info.level < 0)
      {
        fprintf(stderr, "" MKBITMAP ": invalid threshold -- %s\n", optarg);
        exit(1);
      }
      break;
    case '?':
      fprintf(stderr, "Try --help for more info\n");
      exit(1);
      break;
    default:
      fprintf(stderr, "" MKBITMAP ": Unimplemented option -- %c\n", c);
      exit(1);
    }
  }
  info.infiles = &av[optind];
  info.infilecount = ac - optind;
  return;
}

/* ---------------------------------------------------------------------- */
/* auxiliary functions for file handling */

/* open a file for reading. Return stdin if filename is NULL or "-" */
static FILE *my_fopen_read(const char *filename)
{
  if (filename == NULL || strcmp(filename, "-") == 0)
  {
    return stdin;
  }
  return fopen(filename, "rb");
}

/* open a file for writing. Return stdout if filename is NULL or "-" */
static FILE *my_fopen_write(const char *filename)
{
  if (filename == NULL || strcmp(filename, "-") == 0)
  {
    return stdout;
  }
  return fopen(filename, "wb");
}

/* close a file, but do nothing is filename is NULL or "-" */
static void my_fclose(FILE *f, const char *filename)
{
  if (filename == NULL || strcmp(filename, "-") == 0)
  {
    return;
  }
  fclose(f);
}

/* make output filename from input filename. Return an allocated value. */
static char *make_outfilename(const char *infile, const char *ext)
{
  char *outfile;
  char *p;

  if (strcmp(infile, "-") == 0)
  {
    return strdup("-");
  }

  outfile = (char *)malloc(strlen(infile) + strlen(ext) + 5);
  if (!outfile)
  {
    return NULL;
  }
  strcpy(outfile, infile);
  p = strrchr(outfile, '.');
  if (p)
  {
    *p = 0;
  }
  strcat(outfile, ext);

  /* check that input and output filenames are different */
  if (strcmp(infile, outfile) == 0)
  {
    strcpy(outfile, infile);
    strcat(outfile, "-out");
  }

  return outfile;
}

/* ---------------------------------------------------------------------- */
/* Main function */

int main(int ac, char *av[])
{
  FILE *fin, *fout;
  int i;
  char *outfile;

  /* process options */
  dopts(ac, av);

  /* there are several ways to call us:
     mkbitmap                    -- stdin to stdout
     mkbitmap -o outfile         -- stdin to outfile
     mkbitmap file...            -- encode each file and generate outfile names
     mkbitmap file... -o outfile -- concatenate files and write to outfile
  */

  if (info.infilecount == 0 && info.outfile == NULL)
  { /* stdin to stdout */

    process_file(stdin, stdout, "stdin", "stdout");
    return 0;
  }
  else if (info.infilecount == 0)
  { /* stdin to outfile */

    fout = my_fopen_write(info.outfile);
    if (!fout)
    {
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.outfile, strerror(errno));
      exit(2);
    }
    process_file(stdin, fout, "stdin", info.outfile);
    my_fclose(fout, info.outfile);
    free(info.outfile);
    return 0;
  }
  else if (info.outfile == NULL)
  { /* infiles -> multiple outfiles */

    for (i = 0; i < info.infilecount; i++)
    {
      outfile = make_outfilename(info.infiles[i], ".pbm");
      if (!outfile)
      {
        fprintf(stderr, "" MKBITMAP ": %s\n", strerror(errno));
        exit(2);
      }
      fin = my_fopen_read(info.infiles[i]);
      if (!fin)
      {
        fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.infiles[i], strerror(errno));
        exit(2);
      }
      fout = my_fopen_write(outfile);
      if (!fout)
      {
        fprintf(stderr, "" MKBITMAP ": %s: %s\n", outfile, strerror(errno));
        exit(2);
      }
      process_file(fin, fout, info.infiles[i], outfile);
      my_fclose(fin, info.infiles[i]);
      my_fclose(fout, outfile);
      free(outfile);
    }
    return 0;
  }
  else
  { /* infiles to single outfile */

    fout = my_fopen_write(info.outfile);
    if (!fout)
    {
      fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.outfile, strerror(errno));
      exit(2);
    }
    for (i = 0; i < info.infilecount; i++)
    {
      fin = my_fopen_read(info.infiles[i]);
      if (!fin)
      {
        fprintf(stderr, "" MKBITMAP ": %s: %s\n", info.infiles[i], strerror(errno));
        exit(2);
      }
      process_file(fin, fout, info.infiles[i], info.outfile);
      my_fclose(fin, info.infiles[i]);
    }
    my_fclose(fout, info.outfile);
    free(info.outfile);
    return 0;
  }

  /* not reached */
}
