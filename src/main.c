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

/**
 * Main class of project xubirdr
 *
 * @param argc the number of arguments
 * @param argv the arguments from the commandline
 * @returns exit code of the application
 */
int main(int argc, char **argv) {
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
  int         ret = 0;

  //----------------------------------- Initialization: Memory Mapping -------//

  // open the /dev/mem device
  fdMem = open("/dev/mem", O_RDONLY | O_SYNC);
  if (-1 == fdMem) {
    perror("Error: open() failed");
    return 1;
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
    return 2;
  }
  // here's the virtual address pointer
  pvVirtAddr = (pvMemPointer + (u32AddrStart & u32PageMask));

  //----------------------------------- Program Body -------------------------//

  // data pointers
  pc = (char*)pvVirtAddr;
  pz = pc + u32AddrSize;      // maximum size trigger

  // check the file header
  bValid  = (0 == strncmp(pc  , "XUBI", 4));
  bValid &= (0 == strncmp(pc+8, "\x03\x02\x01\x00", 4));
  if (!bValid) {
    fprintf(stderr, "Error: Invalid XUBI header\n");
    ret = 3;
  }

  // read the file size
  if (0 == ret) {
    u32FileSize = 0;
    for (i=7; i>=4; --i) {
      u32FileSize = (u32FileSize << 8) | *(pc+i);
    }
    fprintf(stderr, "UBI image file size: %lu\n", u32FileSize);
  }

  // update the data pointers
  if (0 == ret) {
    pc += 12;   // skip the header
    if (pc+u32FileSize > pz) {
      fprintf(stderr, "Error: Declared image size is out of allowed range.\n");
      ret = 4;
    } else {
      pz = pc+u32FileSize;
    }
  }

  // print the UBI file
  if (0 == ret) {
    while (pc < pz) {
      putchar(*(pc++));
    };
  }

  //----------------------------------- Cleanup ------------------------------//

  // unmap the pointer
  munmap(pvMemPointer, u32AllocMemSize);
  // close the /dev/mem device
  close(fdMem);
  return ret;
}
