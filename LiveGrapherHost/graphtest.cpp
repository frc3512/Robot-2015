/* A very simple test harness for GraphHost */

#if 0
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "graphhost.h"

/* main function */
int
main()
{
  struct graphhost_t *gh;

  /* Ignore SIGPIPE */
  signal(SIGPIPE, SIG_IGN);

  /* Create a GraphHost */
  gh = GraphHost_create(4098);

  /* Send some bogus data */
  while(1) {
    GraphHost_graphData(0, 0, "PID0", gh);
    sleep(1);
  }

  /* NOTREACHED */
  GraphHost_destroy(gh);

  return 0;
}
#endif
