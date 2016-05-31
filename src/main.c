/** @mainpage xubirdr - Elnico
 *
 * @author Petr Kubiznak <kubiznak.petr@elnico.cz>
 * @version 1.0.0
**/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

//******************************************************************************

enum {
  RET_SUCCESS = 0,
  RET_HELP,
  RET_OPEN_FAILURE,
  RET_MMAP_FAILURE,
  RET_INVALID_HEADER,
  RET_INVALID_ARGUMENT,
  RET_OUT_OF_MEMORY,
};

//******************************************************************************

typedef struct t_config_struct {
  uint8_t               bDryRun;                                                // do not output the file
} TConfig;

//******************************************************************************

/** Parses program arguments. */
int parseOptions (TConfig * poConfig, int argc, char * argv[]);
/** Prints the program usage.
 * @param[in]   progName    Program name, e.g. "xubirdr". First program argument
 *                          (argv[0]) can typically be used for this purpose. */
void printUsage (const char * sProgName);

//******************************************************************************
//******************************************************************************
//******************************************************************************

/**
 * Main class of project xubirdr
 *
 * @param argc the number of arguments
 * @param argv the arguments from the commandline
 * @returns exit code of the application
 */
int main(int argc, char **argv) {
  TConfig     oConfig = {
    .bDryRun    = 0,
  };
  int         fdMem;
  uint32_t    u32AddrStart = 0x85000000;
  uint32_t    u32AddrSize  = 0x05000000;   // 80 MB
  uint32_t    u32AllocMemSize;
  uint32_t    u32PageMask;
  uint32_t    u32PageSize;
  uint32_t    u32FileSize;
  void      * pvMemPointer;
  void      * pvVirtAddr;
  char      * pc;
  char      * pz;
  uint8_t     bValid;
  int         i;
  int         ret = RET_SUCCESS;

  // Initialization: Application Arguments -------------------------------------

  ret = parseOptions(&oConfig, argc, argv);
  if (RET_HELP == ret) printUsage(argv[0]);
  if (RET_SUCCESS != ret) {
    return ((RET_HELP == ret) ? RET_SUCCESS : ret);
  }

  // Initialization: Memory Mapping --------------------------------------------

  // open the /dev/mem device
  fdMem = open("/dev/mem", O_RDONLY | O_SYNC);
  if (-1 == fdMem) {
    perror("Error: open() failed");
    return RET_OPEN_FAILURE;
  }

  // calculate aligned address
  u32PageSize = sysconf(_SC_PAGESIZE);
  u32AllocMemSize = (((u32AddrSize / u32PageSize) + 1) * u32PageSize);
  u32PageMask = (u32PageSize - 1);

  // mmap the address
  pvMemPointer = mmap(NULL,
                      u32AllocMemSize,
                      PROT_READ,
                      MAP_SHARED,
                      fdMem,
                      (u32AddrStart & ~u32PageMask)
                     );
  if (MAP_FAILED == pvMemPointer) {
    perror("Error: mmap() failed");
    close(fdMem);
    return RET_MMAP_FAILURE;
  }
  // here's the virtual address pointer
  pvVirtAddr = (pvMemPointer + (u32AddrStart & u32PageMask));

  // Program Body --------------------------------------------------------------

  // data pointers
  pc = (char*)pvVirtAddr;
  pz = pc + u32AddrSize;      // maximum size trigger

  // check the file header
  bValid  = (0 == strncmp(pc  , "XUBI", 4));
  bValid &= (0 == strncmp(pc+8, "\x03\x02\x01\x00", 4));
  if (!bValid) {
    fprintf(stderr, "Error: Invalid XUBI header\n");
    ret = RET_INVALID_HEADER;
  }

  // read the file size
  if (RET_SUCCESS == ret) {
    u32FileSize = 0;
    for (i=7; i>=4; --i) {
      u32FileSize = (u32FileSize << 8) | *(pc+i);
    }
    fprintf(stderr, "UBI image file size: %lu\n", u32FileSize);
    if (oConfig.bDryRun) fprintf(stdout, "%lu", u32FileSize);
  }

  // update the data pointers
  if (RET_SUCCESS == ret) {
    pc += 12;   // skip the header
    if (pc+u32FileSize > pz) {
      fprintf(stderr, "Error: Declared image size is out of allowed range.\n");
      ret = RET_OUT_OF_MEMORY;
    } else {
      pz = pc+u32FileSize;
    }
  }

  // print the UBI file
  if (RET_SUCCESS == ret) {
    if (!oConfig.bDryRun) {
      while (pc < pz) {
        putchar(*(pc++));
      };
    }
  }

  // Cleanup -------------------------------------------------------------------

  // unmap the pointer
  munmap(pvMemPointer, u32AllocMemSize);
  // close the /dev/mem device
  close(fdMem);
  return ret;
}

//******************************************************************************

int parseOptions (TConfig * poConfig, int argc, char * argv[])
{
  struct option   aoArgOptions [] = {
      { "dry-run"       , no_argument       , 0,  'n' },
      { "help"          , no_argument       , 0,  'h' },
      { 0               , 0                 , 0,  0   },
  };
  int             iOptionIdx  = 0;
  int             iOptionChar = 0;
  int             ret;
  int             res = RET_SUCCESS;

  if (!poConfig) return RET_INVALID_ARGUMENT;

  while (iOptionChar >= 0) {
    iOptionChar = getopt_long(argc, argv, "nh", aoArgOptions, &iOptionIdx);

    switch (iOptionChar) {
    case -1:
      // no more options
      break;

    case 0:
      printf("iOptionChar=%d=%c, iOptionIdx=%d\n",
          (int)iOptionChar, (char)iOptionChar, iOptionIdx);
      break;

    case 'n':
      poConfig->bDryRun = 1;
      break;

    case 'h':
      if (RET_SUCCESS == res) res = RET_HELP;
      break;

    default:
      res = RET_INVALID_ARGUMENT;
      break;
    }
  }

  if ((res != RET_SUCCESS) && (res != RET_HELP)) {
    printf("Invalid argument. Use --help for usage.\n");
  }

  return res;
}

//******************************************************************************

void printUsage (const char * sProgName)
{
  fprintf(stdout,
          "Usage: %s [ -n ] [ -h ]\n"
          "Read UBI image from a memory at given physical address.\n\n",
          sProgName);

  fprintf(stdout,
          "  -n, --dry-run    Do not output the UBI image, just print its size\n"
          "                   in bytes to stdout.\n\n");
  fprintf(stdout,
          "  -h, --help       Invoke this help.\n\n");
}

//******************************************************************************
