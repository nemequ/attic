/* Meant to check the enmem.h implementations. You shouldn't need this
   file. Some caveats for the tests:

   * Compiler optimizations will probably optimize out a lot of this,
     you probably don't want to enable them when testing.
   * For more thorough testing you may want to set MALLOC_PERTURB_
     (<https://debarshiray.wordpress.com/2016/04/09/malloc_perturb_/>),
     but I don't think it works with ASan, so you may want 2 runs.
   * If you're using ASan, you'll probably want to set
     ASAN_OPTIONS="allocator_may_return_null=1"
*/

#include "enmem.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
  int *x, *y;

  x = ennew(int);
  assert(x != NULL);
  memset(x, 0, sizeof(*x));
  assert(*x == 0);
  x = enfree(x);
  assert(x == NULL);



  x = ennew0(int);
  assert(x != NULL);
  assert(*x == 0);
  x = enfree(x);
  assert(x == NULL);



  x = ennewa(int, 0);
  assert(x == NULL);


  x = ennewa(int, 42);
  assert(x != NULL);
  memset(x, 0, sizeof(*x) * 42);
  for (int i = 0 ; i < 42 ; i++)
    assert(x[i] == 0);
  x = enfree(x);
  assert(x == NULL);

  x = ennewa(int, SIZE_MAX / sizeof(*x));
  /* if (x == NULL) */
  /*   assert(errno == ENOMEM); */
  x = enfree(x);

  errno = 0;
  x = ennewa(int, (SIZE_MAX / sizeof(*x)) + 1);
  assert(x == NULL);
  assert(errno == ENOMEM);
  x = enfree(x);



  x = ennewa0(int, 42);
  assert(x != NULL);
  for (int i = 0 ; i < 42 ; i++)
    assert(x[i] == 0);
  x = enfree(x);
  assert(x == NULL);



  /* double* z = NULL; */
  /* z = enrealloc(z, int, 3); */



  x = enrealloc(x, int, 1);
  assert(x != NULL);
  x[0] = 1729;
  y = enrealloc(x, int, 2);
  if (y != NULL) {
    x = y;
    assert(x[0] == 1729);
    x[1] = 1701;
  }
  x = enfree(x);
  assert(x == NULL);



  x = enrealloc(x, int, (SIZE_MAX / sizeof(*x)) + 1);
  assert(x == NULL);



  x = enresize(x, int, 1);
  assert(x != NULL);
  x[0] = 1729;
  x = enresize(x, int, (SIZE_MAX / sizeof(*x)) + 1);
  assert(x == NULL);



  x = enresize(x, int, 1);
  x[0] = 1729;
  x = enresize(x, int, 0);
  assert(x == NULL);


  return 0;
}
