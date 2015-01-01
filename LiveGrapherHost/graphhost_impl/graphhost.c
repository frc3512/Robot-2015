#include "graphhost.h"
#include <stdlib.h>
#include "list.h"
#include "queue.h"
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#ifdef __VXWORKS__

#include <ioLib.h>
#include <pipeDrv.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <sockLib.h>
#include <hostLib.h>
#include <selectLib.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/select.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#endif

/* Listens on a specified port (listenport), and returns the file
 * descriptor to the listening socket.
 */
int
sockets_listen_int(int port, sa_family_t sin_family, uint32_t s_addr)
{
  struct sockaddr_in serv_addr;
  int error;
  int sd;

  /* Create a TCP socket */
  sd = socket(sin_family, SOCK_STREAM, 0);
  if(sd == -1){
    perror("");
    return -1;
  }

  /* Zero out the serv_addr struct */
  bzero((char *) &serv_addr, sizeof(struct sockaddr_in));

  /* Set up the listener sockaddr_in struct */
  serv_addr.sin_family = sin_family;
  serv_addr.sin_addr.s_addr = s_addr;
  serv_addr.sin_port = htons(port);

  /* Bind the socket to the listener sockaddr_in */
  error = bind(sd, (struct sockaddr *) &serv_addr,
    sizeof(struct sockaddr_in));
  if(error != 0){
    perror("");
    close(sd);
    return -1;
  }

  /* Listen on the socket for incoming conncetions */
  error = listen(sd, 5);
  if(error != 0){
    perror("");
    close(sd);
    return -1;
  }

  /* Make sure we aren't killed by SIGPIPE */
  signal(SIGPIPE, SIG_IGN);

  return sd;
}

struct graphhost_t *
GraphHost_create(int port)
{
  /* int i; */
  struct graphhost_t *inst;
  int pipefd[2];
#ifndef __VXWORKS__
  int error;
#endif

  /* Allocate memory for the graphhost_t structure */
  inst = malloc(sizeof(struct graphhost_t));

  /* Mark the thread as not running, this will be set to 1 by the thread */
  inst->running = 0;

  /* Store the port to listen on */
  inst->port = port;

  /* Create a pipe for IPC with the thread */
#ifdef __VXWORKS__
  pipeDevCreate("/pipe/graphhost", 10, 100);
  pipefd[0] = open("/pipe/graphhost", O_RDONLY, 0644);
  pipefd[1] = open("/pipe/graphhost", O_WRONLY, 0644);

  if(pipefd[0] == -1 || pipefd[1] == -1) {
    perror("");
    free(inst);
    return NULL;
  }
#else
  error = pipe(pipefd);
  if(error == -1) {
    perror("");
    free(inst);
    return NULL;
  }
#endif

  inst->ipcfd_r = pipefd[0];
  inst->ipcfd_w = pipefd[1];

  /* Launch the thread */
  if(pthread_create(&inst->thread, NULL, sockets_threadmain, (void *)inst) != 0) {
    fprintf(stderr, "pthread_create(3) failed\n");
    free(inst);
    return NULL;
  }

  return inst;
}

void
GraphHost_destroy(struct graphhost_t *inst)
{
  /* Tell the other thread to stop */
  write(inst->ipcfd_w, "x", 1);

  /* Join to the other thread */
  pthread_join(inst->thread, NULL);

  /* Close file descriptors and clean up */
  close(inst->ipcfd_r);
  close(inst->ipcfd_w);
  free(inst);

  return;
}

void
sockets_listremovesocket(struct list_t *list, struct socketconn_t *conn)
{
  list_delete(list, conn->elem);

  return;
}

void
sockets_accept(struct list_t *connlist, int listenfd)
{
  int new_fd;
#ifdef __VXWORKS__
  int clilen;
#else
  unsigned int clilen;
#endif
  struct socketconn_t *conn;
  struct sockaddr_in cli_addr;
  int error;
#ifdef __VXWORKS__
  int on;
#endif
#ifndef __VXWORKS__
  int flags;
#endif

  clilen = sizeof(struct sockaddr_in);

  /* Accept a new connection */
  new_fd = accept(listenfd, (struct sockaddr *) &cli_addr,
    &clilen);

  /* Make sure that the file descriptor is valid */
  if(new_fd < 1) {
    perror("");
    return;
  }

#ifdef __VXWORKS__
  /* Set the socket non-blocking. */
  on = 1;
  error = ioctl(new_fd, (int)FIONBIO, on);
  if(error == -1){
    perror("");
    close(new_fd);
    return;
  }

#else

  /* Set the socket non-blocking. */
  flags = fcntl(new_fd, F_GETFL, 0);
  if(flags == -1) {
    perror("");
    close(new_fd);
    return;
  }

  error = fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);
  if(error == -1) {
    perror("");
    close(new_fd);
    return;
  }

#endif

  conn = malloc(sizeof(struct socketconn_t));
  conn->fd = new_fd;
  conn->selectflags = SOCKET_READ | SOCKET_ERROR;

  conn->datasets = list_create();

  conn->writequeue = queue_init(20);
  conn->writebuf = NULL;
  conn->writebuflength = 0;
  conn->writebufoffset = 0;

  conn->readbuf = NULL;
  conn->readbuflength = 0;
  conn->readbufoffset = 0;

  conn->orphan = 0;

  /* Add it to the list, this makes it a bit non-thread-safe */
  conn->elem = list_add_after(connlist, NULL, conn);

  return;
}

/* Mark the socket for closing and deleting */
void
sockets_close(struct list_t *list, struct list_elem_t *elem)
{
  struct socketconn_t *conn = elem->data;

  conn->orphan = 1;

  return;
}

/* NOTE: Does not remove the element from the list */
void
sockets_remove_orphan(struct socketconn_t *conn)
{
  struct writebuf_t *writebuf = NULL;
  struct list_elem_t *dataset;

  /* Give up on the current write buffer */
  if(conn->writebuf != NULL) {
    free(conn->writebuf);
  }

  /* Give up on the current read buffer */
  if(conn->readbuf != NULL) {
    free(conn->readbuf);
  }

  /* Give up on all other queued buffers too */
  while(queue_dequeue(conn->writequeue, (void *)&writebuf) == 0) {
    free(writebuf->buf);
    free(writebuf);
  }

  queue_free(conn->writequeue);

  for(dataset = conn->datasets->start; dataset != NULL; dataset = dataset->next) {
    free(dataset->data);
  }

  list_destroy(conn->datasets);

  /* Free it when we get back to it, this is a hack */
  conn->orphan = 1;

  /* free(conn->dataset); */
  close(conn->fd);
  free(conn);

  return;
}

/* Closes and clears orpahans from the list */
void
sockets_clear_orphans(struct list_t *list)
{
  struct list_elem_t *elem;
  struct list_elem_t *last = NULL;
  struct socketconn_t *conn;

  for(elem = list->start; elem != NULL; ) {
    last = elem;
    elem = elem->next;

    if(last != NULL) {
      conn = last->data;
      if(conn->orphan == 1) {
        conn = last->data;
        sockets_remove_orphan(conn);
        list_delete(list, last);
      }
    }
  }

  return;
}

int
sockets_readh(struct graphhost_t *inst, struct list_t *list, struct list_elem_t *elem)
{
  struct socketconn_t *conn = elem->data;
  int error;

  if(conn->readbuflength == 0) {
    conn->readbufoffset = 0;
    conn->readbuflength = 16; /* This should be configurable somewhere */
    conn->readbuf = malloc(conn->readbuflength);
  }

  error = recv(conn->fd, (char*)conn->readbuf, conn->readbuflength - conn->readbufoffset, 0);
  if(error < 1) {
    /* Clean up the socket here */
    sockets_close(list, elem);
    return 0;
  }
  conn->readbufoffset += error;

  if(conn->readbufoffset == conn->readbuflength) {
    sockets_readdoneh(inst, conn->readbuf, conn->readbuflength, list, elem);
    conn->readbufoffset = 0;
    conn->readbuflength = 0;
    free(conn->readbuf);
    conn->readbuf = NULL;
  }


  return 0;
}

/* Recieves 16 byte buffers which will be freed upon return */
int
sockets_readdoneh(struct graphhost_t *inst, uint8_t *inbuf, size_t bufsize, struct list_t *list, struct list_elem_t *elem)
{
  struct socketconn_t *conn = elem->data;
  char *buf;
  char *graphstr;
  char *tmpstr;
  struct list_elem_t *clelem;

  inbuf[15] = 0;
  graphstr = ((char *)inbuf)+1;

  switch(inbuf[0]) {
  case 'c':
    /* Start sending data for the graph specified by graphstr. */
    buf = malloc(strlen(graphstr));
    strcpy(buf, graphstr);
    list_add_after(conn->datasets, NULL, buf);
    break;
  case 'd':
    /* Stop sending data for the graph specified by graphstr. */
    for(clelem = inst->connlist->start; clelem != NULL;
      clelem = clelem->next) {

      tmpstr = clelem->data;
      if(strcmp(tmpstr, graphstr) == 0) {
        list_delete(conn->datasets, clelem);
        free(tmpstr);
        break;
      }
    }
    break;
  case 'l':
    /* If this fails, we just ignore it. There's really nothing we can
       do about it right now. */
    sockets_sendlist(inst, list, elem);
  }

  return 0;
}

/* Send to the client a list of available graphs */
int
sockets_sendlist(struct graphhost_t *inst, struct list_t *list, struct list_elem_t *elem) {
  struct list_t *graphlist = inst->graphlist;
  struct socketconn_t *conn = elem->data;
        struct graph_list_t replydg;
  struct list_elem_t *clelem;
  char *tmpstr;

  for(clelem = graphlist->start; clelem != NULL;
    clelem = clelem->next) {

    tmpstr = clelem->data;

    /* Set up the response body, and queue it for sending. */
    memset((void *)&replydg, 0x00, sizeof(struct graph_list_t));

    /* Set the type of the datagram. */
    replydg.type = 'l';

    /* Is this the last element in the list? */
    if(clelem->next == NULL) {
      replydg.end = 1;
    }else{
                  replydg.end = 0;
    }

    /* Copy in the string */
    strcpy(replydg.dataset, tmpstr);

    /* Queue the datagram for writing */
    if(sockets_queuewrite(inst, conn, (void *)&replydg, sizeof(struct graph_list_t)) == -1) {
      return -1;
    }
  }

  return 0;
}


/* Write queued data to a socket when the socket becomes ready */
int
sockets_writeh(struct graphhost_t *inst, struct list_t *list, struct list_elem_t *elem)
{
  int error;
  struct socketconn_t *conn = elem->data;
  struct writebuf_t *writebuf;
  struct writebuf_t **writebufPtr = &writebuf;

  while(1) {

    /* Get another buffer to send */
    if(conn->writebuflength == 0) {
      error = queue_dequeue(conn->writequeue, (void **)writebufPtr);
      /* There are no more buffers in the queue */
      if(error != 0) {
        /* Call the write finished callback in the upper layer */
        /*if(conninfo->listener->wdoneh != NULL)
          conninfo->listener->wdoneh(conninfo); */

        /* Stop selecting on write */
        conn->selectflags &= ~(SOCKET_WRITE);

        return 0;
      }
      conn->writebuf = writebuf->buf;
      conn->writebuflength = writebuf->buflength;
      conn->writebufoffset = 0;
      free(writebuf);
    }

    /* These descriptors are ready for writing */
    conn->writebufoffset += send(conn->fd, (char*)conn->writebuf, conn->writebuflength - conn->writebufoffset, 0);

    /* Have we finished writing the buffer? */
    if(conn->writebufoffset == conn->writebuflength) {

      /* Reset the write buffer */
      conn->writebuflength = 0;
      conn->writebufoffset = 0;
      free(conn->writebuf);
      conn->writebuf = NULL;
    }else{
      /* We haven't finished writing, keep selecting. */
      return 0;
    }

  }

  /* We always return from within the loop, this is unreachable */
  return -1;
}

/* Queue a buffer for writing. Returns 0 on success, returns -1 if buffer
 * wasn't queued. Only one buffer can be queued for writing at a time.
 */
int
sockets_queuewrite(struct graphhost_t *inst, struct socketconn_t *conn, uint8_t *buf, size_t buflength)
{
  int error;
  struct writebuf_t *writebuf;

  writebuf = malloc(sizeof(struct writebuf_t));
  writebuf->buf = malloc(buflength);
  writebuf->buflength = buflength;
  memcpy(writebuf->buf, buf, buflength);
  error = queue_queue(conn->writequeue, writebuf);
  if(error != 0) {
    free(writebuf->buf);
    free(writebuf);
    return 0;
  }

  /* Select on write */
  conn->selectflags |= SOCKET_WRITE;
  write(inst->ipcfd_w, "r", 1);

  return 0;
}

void *
sockets_threadmain(void *arg)
{
  int listenfd;
  struct graphhost_t *inst = arg;
  struct socketconn_t *conn;
  struct list_elem_t *elem;
  int maxfd;
  int fd;
  uint8_t ipccmd;

  fd_set readfds;
  fd_set writefds;
  fd_set errorfds;

  pthread_mutex_init(&inst->mutex, NULL);

  /* Create a list to store all the open connections */
  inst->connlist = list_create();
  inst->graphlist = list_create();

  /* Listen on a socket */
  listenfd = sockets_listen_int(inst->port, AF_INET, 0x00000000);
  if(listenfd == -1) {
    pthread_mutex_destroy(&inst->mutex);
    list_destroy(inst->graphlist);
    list_destroy(inst->connlist);
    pthread_exit(NULL);
    return NULL;
  }

  /* Set the running flag after we've finished initializing everything */
  inst->running = 1;

  while(1) {

    /* Clear the fdsets */
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errorfds);

    /* Reset the maxfd */
    maxfd = listenfd;

    /* Add the file descriptors to the list */
    pthread_mutex_lock(&inst->mutex);
    for(elem = inst->connlist->start; elem != NULL;
      elem = elem->next) {
      conn = elem->data;
      fd = conn->fd;

      if(conn->orphan == 1) continue;

      if(maxfd < fd)
        maxfd = fd;
      if(conn->selectflags & SOCKET_READ)
        FD_SET(fd, &readfds);
      if(conn->selectflags & SOCKET_WRITE)
        FD_SET(fd, &writefds);
      if(conn->selectflags & SOCKET_ERROR)
        FD_SET(fd, &errorfds);
    }
    pthread_mutex_unlock(&inst->mutex);

    /* Select on the listener fd */
    FD_SET(listenfd, &readfds);

    /* ipcfd will recieve data when the thread needs to exit */
    FD_SET(inst->ipcfd_r, &readfds);

    /* Select on the file descrpitors */
    select(maxfd+1, &readfds, &writefds, &errorfds, NULL);

    pthread_mutex_lock(&inst->mutex);
    for(elem = inst->connlist->start; elem != NULL;
      elem = elem->next) {
      conn = elem->data;
      fd = conn->fd;

      if(conn->orphan == 1) continue;

      if(FD_ISSET(fd, &readfds)) {
        /* Handle reading */
        sockets_readh(inst, inst->connlist, elem);
      }
      if(FD_ISSET(fd, &writefds)) {
        /* Handle writing */
        sockets_writeh(inst, inst->connlist, elem);
      }
      if(FD_ISSET(fd, &errorfds)) {
        /* Handle errors */
        sockets_close(inst->connlist, elem);
      }
    }

    /* Close all the file descriptors marked for closing */
    sockets_clear_orphans(inst->connlist);
    pthread_mutex_unlock(&inst->mutex);

    /* Check for listener condition */
    if(FD_ISSET(listenfd, &readfds)) {
      /* Accept connections */
      sockets_accept(inst->connlist, listenfd);
    }

    /* Handle IPC commands */
    if(FD_ISSET(inst->ipcfd_r, &readfds)) {
      read(inst->ipcfd_r,(char*)&ipccmd, 1);
      if(ipccmd == 'x') {
        break;
      }
    }
  }

  /* We're done, clear the running flag and clean up */
  inst->running = 0;
  pthread_mutex_lock(&inst->mutex);

  /* Mark all the open file descriptors for closing */
  for(elem = inst->connlist->start; elem != NULL;
    elem = elem->next) {
    sockets_close(inst->connlist, elem);
    /* We don't need to delete the element form the
       because we just delete all of them below. */
  }

  /* Actually close all the open file descriptors */
  sockets_clear_orphans(inst->connlist);

  /* Free the list of connections */
  list_destroy(inst->connlist);

  /* Close the listener file descriptor */
  close(listenfd);

  /* Clean up & free the global dataset list */
  for(elem = inst->graphlist->start; elem != NULL; elem = elem->next) {
    free(elem->data);
  }
  list_destroy(inst->graphlist);

  /* Destroy the mutex */
  pthread_mutex_unlock(&inst->mutex);
  pthread_mutex_destroy(&inst->mutex);

  pthread_exit(NULL);
  return NULL;
}

/* If the dataset name isn't in the list already, add it. */
int
socket_recordgraph(struct list_t *graphlist, const char *dataset)
{
  char *graphstr;
  int graphinlist;
  struct list_elem_t *graphelem;
  char *tmpstr;

  graphinlist = 0;

  /* Add the graph name to the list of available graphs */
  for(graphelem = graphlist->start; graphelem != NULL;
    graphelem = graphelem->next)
  {
    graphstr = graphelem->data;

    /* Graph is already in list */
    if(strcmp(graphstr, dataset) == 0) {
      graphinlist = 1;
      return 1;
    }
  }

  /* If the graph wasn't in the list, add it. */
  if(!graphinlist) {
    tmpstr = malloc(strlen(dataset));
    strcpy(tmpstr, dataset);
    list_add_after(graphlist, NULL, tmpstr);
  }

  return 0;
}

/* We assume that a float is 32 bits long */
int
GraphHost_graphData(float x, float y, const char *dataset, struct graphhost_t *graphhost)
{
  struct list_elem_t *elem;
  struct list_elem_t *datasetp;
  struct socketconn_t *conn;
  struct graph_payload_t payload;
  uint32_t tmp;

  char *dataset_str;

  if(!graphhost->running) return -1;

  /* Zero the payload structure */
  memset((void *)&payload, 0x00, sizeof(struct graph_payload_t));

  /* Change to network byte order */
  payload.type = 'd';

  /* Swap bytes in x, and copy into the payload struct */
  memcpy(&tmp, &x, sizeof(uint32_t));
  tmp = htonl(tmp);
  memcpy(&payload.x, &tmp, sizeof(uint32_t));

  /* Swap bytes in y, and copy into the payload struct */
  memcpy(&tmp, &y, sizeof(uint32_t));
  tmp = htonl(tmp);
  memcpy(&payload.y, &tmp, sizeof(uint32_t));

  strncpy(payload.dataset, dataset, 15);

  /* Giant lock approach */
  pthread_mutex_lock(&graphhost->mutex);

  /* If the dataset name isn't in the list already, add it. */
  socket_recordgraph(graphhost->graphlist, dataset);

  /* Send the point to connected clients */
  for(elem = graphhost->connlist->start; elem != NULL; elem = elem->next) {
    conn = elem->data;
    for(datasetp = conn->datasets->start; datasetp != NULL; datasetp = datasetp->next) {
      dataset_str = datasetp->data;
      if(dataset_str != NULL && strcmp(dataset_str, dataset) == 0) {
        /* Send the value off */
        /* qpayload = malloc(sizeof(struct graph_payload_t));
        memcpy(qpayload, &payload, sizeof(struct graph_payload_t)); */
        sockets_queuewrite(graphhost, conn, (void *)&payload, sizeof(struct graph_payload_t));
      }
    }
  }

  pthread_mutex_unlock(&graphhost->mutex);

  return 0;
}
