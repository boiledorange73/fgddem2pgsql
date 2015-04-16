#include <stdio.h>
#include <string.h>
#include <ctype.h> /* isspace */

#include "printerror.h"
#include "fgddemreader.h"
#include "rasterbuffer.h"
#include "writer.h"

/*
  cc -g -I/usr/local/include -L/usr/local/lib \
    fgddem2pgsql.c \
    fgddemreader.c \
    writer.c rasterbuffer.c bandbuffer.c band.c \
    datalist.c datalistcell.c printerror.c \
    -lm -lexpat
./a.out -a -sd 9000000 -o dem5.sql dem5 work/xml-5m/FG-GML-5133-63-33-DEM5A-20130702.xml
./a.out -a -sd 9000000 -o dem10.sql dem10 work/xml-10m/FG-GML-5133-63-dem10b-20090201.xml
*/

enum Mode {
  mode_create = 0,
  mode_append = 1,
  mode_prepare = 2
};

/* 1MiB */
#define MAX_INFILE_PATH_LEN (1048576)

#if defined(_WIN32) || defined(_WIN64)
  #define FILESEPARATOR "\\"
  #define FILESEPARATORLEN (1)
#else
  #define FILESEPARATOR "/"
  #define FILESEPARATORLEN (1)
#endif


static inline char *make_infilename(char *infile) {
  size_t infilelen;
  char *r, *ret;

  if( infile == NULL ) {
    return NULL;
  }
  infilelen = strnlen(infile, MAX_INFILE_PATH_LEN);
  if( infilelen >= MAX_INFILE_PATH_LEN ) {
    /* too large infile */
    return NULL;
  }
  ret =infile;
  r = infile;
  while( (r = strnstr(r, FILESEPARATOR, infilelen + r - infile)) != NULL ) {
    r = r + FILESEPARATORLEN;
    ret = r;
  }
  return ret;
}

static int doit_one(FILE *fp, char *infile, FILE *out, uint32_t srid, int snapdenom, double expansion_rate, int quiet) {
  int ret;
  char *infilename;
  RasterBuffer *rasterbuffer;

  /* infile -> infilename */
  rasterbuffer = readFGDDEMXML(fp);
  if (rasterbuffer == NULL) {
	  /* DOES NOTHING */
  }
  else {
    if( !quiet ) {
      RasterOutputHeader outputheader;
      int bandnum;
      fprintf(stderr,
        "Raw Data:\n"
        "  pixels : %d %d\n"
        "  extent: %.14e %.14e\n"
        "        - %.14e %.14e\n"
        "  bands: %d\n",
        rasterbuffer->header.pmaxx - rasterbuffer->header.pminx + 1,
        rasterbuffer->header.pmaxy - rasterbuffer->header.pminy + 1,
        rasterbuffer->header.uminx,
        rasterbuffer->header.uminy,
        rasterbuffer->header.umaxx,
        rasterbuffer->header.umaxy,
        rasterbuffer->bandlist->length
        );
      assignRasterOutputHeader(&(rasterbuffer->header), &outputheader, 0, 0);
      fprintf(stderr,
        "  cellsize: %.14e %.14e\n",
        outputheader.cellsize_x,
        outputheader.cellsize_y
      );
      if( snapdenom != 0 || expansion_rate != 0 ) {
        assignRasterOutputHeader(&(rasterbuffer->header), &outputheader, snapdenom, expansion_rate);
        if( snapdenom != 0 ) {
          fprintf(stderr, "Snaped ");
        }
        if( expansion_rate != 0 ) {
          fprintf(stderr, "Expanded ");
        }
        fprintf(stderr, ":\n");
        fprintf(stderr,
          "  extent: %.14e %.14e\n"
          "        - %.14e %.14e\n"
          "  cellsize: %.14e %.14e\n",
          outputheader.uminx,
          outputheader.uminy,
          outputheader.umaxx,
          outputheader.umaxy,
          outputheader.cellsize_x,
          outputheader.cellsize_y
        );
      }
      for( bandnum = 0; bandnum < rasterbuffer->bandlist->length; bandnum++ ) {
        BandBuffer *bandbuffer;
        bandbuffer = RasterBuffer_GetBand(rasterbuffer, bandnum);
        fprintf(stderr,
          "band %d:\n"
          "  startx,y: %d, %d\n"
          "  tupples: %d\n",
          bandnum,
          bandbuffer->pstartx, bandbuffer->pstarty,
          bandbuffer->datalist->length
        );
      }
    }
  }
  if( rasterbuffer == NULL ) {
    return 1;
  }
  ret = writeRasterBuffer_HEXWKB(rasterbuffer, out, srid, snapdenom, expansion_rate);
  RasterBuffer_Free(rasterbuffer);

  if( ret == 0 ) {
    fprintf(out, "\\N");
  }
  /* infile -> infilename */
  infilename = make_infilename(infile);
  if( infilename == NULL ) {
    fprintf(out, "\t\\N");
  }
  else {
    fprintf(out, "\t%s\n", infilename);
  }
  return 0;
}

static int doit(char **infiles, int count, FILE *out, char *table, uint32_t srid, int snapdenom, double expansion_rate, enum Mode mode, int createindex, int quiet) {
  if( !quiet ) {
    char *modetext = "UNKNOWN";
	char *createindextext = "UNKNOWN";
    switch( mode ) {
    case mode_create:
      modetext = "CREATE";
      createindextext = createindex ? "YES" : "NO";
      break;
    case mode_append:
      modetext = "APPEND";
      createindextext = createindex ? "YES (But Ignored)" : "NO";
      break;
    case mode_prepare:
      modetext = "PREPARE";
      createindextext = createindex ? "YES" : "NO";
      break;
    default:
      /* DOES NOTHING */
      break;
    }
    fprintf(stderr, "srid: %u\nmode: %s\ncreateinedx: %s\nsnapdenom: %d\nexpansion rate: %e\n",
      srid, modetext, createindextext, snapdenom, expansion_rate);
  }

  fprintf(out, "BEGIN;\n");
  if( mode == mode_create || mode == mode_prepare ) {
    fprintf(out,
     "CREATE TABLE %s (\n"
     "  rid SERIAL PRIMARY KEY,\n"
     "  rast RASTER,\n"
     "  filename TEXT\n"
     ");\n", table);
   if( createindex ) {
      fprintf(out, "CREATE INDEX ix_%s_rast ON %s USING GiST(ST_ConvexHull(rast));\n",
        table, table);
   }
  }
  if( mode == mode_create || mode == mode_append ) {
    int n;
    fprintf(out, "COPY %s (rast, filename) FROM stdin;\n", table);
    for( n = 0; n < count; n++ ) {
      FILE *fp;
      if( quiet == 0 ) {
        fprintf(stderr, "----------------\nfile: %s\n", infiles[n]);
      }
      if( (fp = fopen(infiles[n], "r")) == NULL ) {
        print_perror(infiles[n]);
        return 1;
      }
      else {
        int ret;
        ret = doit_one(fp, infiles[n], out, srid, snapdenom, expansion_rate, quiet);
        fclose(fp);
        if( ret > 0 ) {
          return ret;
        }
      }
    }
    fprintf(out, "\\.\n");
  }
  fprintf(out, "END;\n");
  return 0;
}

static void usage(char *program) {
  fprintf(stderr,
    "Usage: %s [-q] [-c|-a|-p] [-sd <snapdenom>] [-er <expansion rate>] [-o <outfile>] <table> <infile> ...\n"
    "       %s -h\n", program, program);
}

static void help(char *program) {
  usage(program);
  fprintf(stderr,
      "\n"
      "OPTIONS:\n"
      "  -c: Create mode. Writes CREATE TABLE statement and COPY statement.\n"
      "  -a: Append mode. Writes COPY statement.\n"
      "  -p: Prepare mode. Writes CREATE TABLE statement.\n"
      "  -sd <snapdenom>: Denomination of snap grid size.\n"
      "                   If not spcified, does not snap.\n"
      "  -er <expansion rate>: Expansion rate of cellsize.\n"
      "  -o <outfile>: Outputfile. If not specified, writes statements to standard out.\n"
      "  -q: Quiet mode.\n"
      "\n"
      "  (corner position) = round((corner position)/<sd>)*<sd>\n"
      "  (cellsize) = round((original cellsize)/<sd>)*<sd> * (1.0+<er>)\n"
      "\n"
  );
}


int main(int ac, char *av[]) {
  char **infiles = NULL;
  FILE *fpout;
  char *outfile;
  int infiles_count;
  char *table;
  RasterBuffer *rasterbuffer;
  int snapdenom = 0;
  double expansion_rate = 0;
  int quiet = 0;
  enum Mode mode = mode_create;
  int createindex = 0;
  int aix;
  int ret;
  uint32_t srid = 0;

  infiles = malloc(sizeof(char *) * (ac - 1));
  if( infiles == NULL ) {
    print_perror("malloc");
    return 1;
  }

  table = NULL;
  outfile = NULL;
  infiles_count = 0;
  aix = 1;
  while( av[aix] != NULL ) {
    if( strcmp("-h", av[aix]) == 0 ) {
      help(av[0]);
      free(infiles);
      return 0;
    }
    else if( strcmp("-sd", av[aix]) == 0 && aix + 1 < ac ) {
      if( sscanf(av[aix+1], "%d", &snapdenom) != 1 ) {
        fprintf(stderr, "Invalid -sd value (%s)\n", av[aix+1]);
        free(infiles);
        return 1;
      }
      aix++;
    }
    else if( strcmp("-er", av[aix]) == 0 && aix + 1 < ac ) {
      if( sscanf(av[aix+1], "%lf", &expansion_rate) != 1 ) {
        fprintf(stderr, "Invalid -er value (%s)\n", av[aix+1]);
        free(infiles);
        return 1;
      }
      aix++;
    }
    else if( strcmp("-o", av[aix]) == 0 && aix + 1 < ac ) {
      outfile = av[aix+1];
      aix++;
    }
    else if( strcmp("-s", av[aix]) == 0 && aix + 1 < ac ) {
      if( sscanf(av[aix+1], "%u", &srid) != 1 ) {
        fprintf(stderr, "Invalid SRID (%s)\n", av[aix+1]);
        free(infiles);
        return 1;
      }
      outfile = av[aix+1];
      aix++;
    }
    else if( strcmp("-c", av[aix]) == 0 ) {
      mode = mode_create;
    }
    else if( strcmp("-a", av[aix]) == 0 ) {
      mode = mode_append;
    }
    else if( strcmp("-p", av[aix]) == 0 ) {
      mode = mode_prepare;
    }
    else if( strcmp("-I", av[aix]) == 0 ) {
      createindex = 1;
    }
    else if( strcmp("-q", av[aix]) == 0 ) {
      quiet = 1;
    }
    else {
      if( table == NULL ) {
        table = av[aix];
      }
      else {
        infiles[infiles_count++] = av[aix];
      }
    }
    aix++;
  }

  if( infiles_count == 0 ) {
    fprintf(stderr, "No input file.\n");
    free(infiles);
    return 1;
  }

  if( outfile != NULL ) {
    if( (fpout = fopen(outfile, "w")) == NULL ) {
      print_perror(outfile);
      return 1;
    }
  }
  else {
    fpout = stdout;
  }

  ret = doit(infiles, infiles_count, fpout, table, srid, snapdenom, expansion_rate, mode, createindex, quiet);

  if( outfile != NULL ) {
    fclose(fpout);
  }
  return ret;
}
