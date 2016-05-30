/** @mainpage ubiimgrdr - Elnico
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
 * Main class of project ubiimgrdr
 *
 * @param argc the number of arguments
 * @param argv the arguments from the commandline
 * @returns exit code of the application
 */
int main(int argc, char **argv) {
  int         fdMem;
  uint32_t    u32AddrStart = 0x85000000;
  uint32_t    u32AddrSize  = 0xA00000;   // 10 MB
  uint32_t    u32AllocMemSize;
  uint32_t    u32PageMask;
  uint32_t    u32PageSize;
  void      * pvMemPointer;
  void      * pvVirtAddr;
  char      * pc;
  char      * pz;

  //----------------------------------- Initialization: Memory Mapping -------//

  // open the /dev/mem device
  fdMem = open("/dev/mem", O_RDONLY | O_SYNC);
  if (-1 == fdMem) {
    perror("open() failed");
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
    perror("mmap() failed");
    close(fdMem);
    return 2;
  }
  // here's the virtual address pointer
  pvVirtAddr = (pvMemPointer + (u32AddrStart & u32PageMask));

  //----------------------------------- Program Body -------------------------//

  pc = (char*)pvVirtAddr;
  pz = pc + u32AddrSize;

  while (pc < pz) {
    putchar(*(pc++));
  };

  //----------------------------------- Cleanup ------------------------------//

  // unmap the pointer
  munmap(pvMemPointer, u32AllocMemSize);
  // close the /dev/mem device
  close(fdMem);
  return 0;
}
