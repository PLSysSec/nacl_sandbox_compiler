#include <stdio.h>

/* NOTE: this is a volatile global to keep a smart compiler (llvm)
 *       from treating this program as having undefined behavior.
 *       Try online: http://llvm.org/demo/index.cgi
 *       Description of behavior: http://llvm.org/bugs/show_bug.cgi?id=3828#c6
 */
volatile unsigned char *byteptr = 0;

int main(void) {
  unsigned char byte = *byteptr;
  printf("FAIL: address zero readable, contains %02x\n", byte);
  return 1;
}
