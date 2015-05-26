/**
   Different stuff used in initialization
   spc_sanitize_files and open_devnull is taken from
   "Secure Programming CookbookTM for C and C++"
   by John Viega and Matt Messier
 **/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <paths.h>

#ifndef OPEN_MAX
#define OPEN_MAX 256
#endif

extern int fileno(FILE*);
extern int getdtablesize(void);

static int open_devnull(int fd) {
  FILE *f = 0;
  if (!fd) f = freopen(_PATH_DEVNULL, "rb", stdin);
  else if (fd == 1) f = freopen(_PATH_DEVNULL, "wb", stdout);
  else if (fd == 2) f = freopen(_PATH_DEVNULL, "wb", stderr);
  return (f && fileno(f) == fd);
}
void spc_sanitize_files(void) {
  int          fd, fds;
  struct stat st;
  /* Make sure all open descriptors other than the standard ones are closed */
  if ((fds = getdtablesize( )) == -1) fds = OPEN_MAX;
  for (fd = 3; fd < fds; fd++) close(fd);
  /* Verify that the standard descriptors are open. If they're not, attempt to
   * open them using /dev/null. If any are unsuccessful, abort.
   */
  for (fd = 0; fd < 3; fd++)
    if (fstat(fd, &st) == -1 && (errno != EBADF || !open_devnull(fd))) abort( );
}
