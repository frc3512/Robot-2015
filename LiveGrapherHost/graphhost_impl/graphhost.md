GraphHost
=========

GraphHost provides a method for sending data samples to a graphing tool on a network-connected workstation for real-time display. This can be used to perform on-line PID loop tuning of motors.

Application Programming Interface (API)
---------------------------------------

### GraphHost\_create ###

```C
struct graphhost_t *
GraphHost_create(int port);
```
The *GraphHost\_create()* function sets up a new GraphHost instance, and starts a thread listening for connections on the specified port. A GraphHost instance pointer to be used in subsequent calls to GraphHost functions is returned.

### GraphHost\_destroy ###

```C
void
GraphHost_destroy(struct graphhost_t *inst);
```
The *GraphHost\_destroy()* function notifies the thread associated with the specified GraphHost instance to exit, and frees all related data structures. The result of calling any GraphHost function with *inst* following this call is undefined.

### GraphHost\_graphData ###

```C
int
GraphHost_graphData(float x, float y, const char *dataset,
  struct graphhost_t *graphhost);
```
The *GraphHost\_graphData()* function sends the data point specified by the *x* and *y* arguments in the dataset specified by the *dataset* argument to all hosts connected to the specified GraphHost instance. This data point is not retained in memory, and will not be sent to clients requesting data from *dataset* after the function call.

Communication Protocol
----------------------

Clients should connect to the TCP port specified in the call to *graphHost\_create()* . Various requests can then be sent to the server. These requests may trigger the server to respond with zero or more responses. All communication with the server is asynchronous. A request may be sent at any time, even before a response has been received regarding a previous request. Therefore, the order in which responses are sent is unspecified.

All requests consist of sixteen byte ASCII strings. The first byte describes the type of request. The remaining fifteen bytes describe the dataset name associated with the request (if appropriate).

All responses consist of 24 bytes arranged in various configurations. The first byte describes the type of response.

### List Available Datasets ('l') ###

This request triggers the server to respond with a list of datasets from which data can be requested.

The 'l' character is sent to the server, followed by fifteen bytes of padding. One response is sent for each available dataset.

The body of each response contains the following fields:
* The ASCII character 'l', the type of response.
* A fifteen byte string whose contents represent the name of a dataset in the list.
* A nonzero value of this one byte field indicates the datagram is the last in the sequence.
* Seven bytes of padding.

### Begin Sending Data ('c') ###

This request notifies the server that it may begin sending data points associated with the specified dataset.

The 'c' character is sent to the server, followed by a fifteen byte string describing the dataset from which data is being requested. The server will then respond with a data point each time the *GraphHost\_graphData()* function is called with the specified dataset.

The body of each response contains the following fields:
* The ASCII character 'd', the type of response.
* The fifteen byte ASCII string describing the dataset to which the data point belongs.
* A 32-bit, IEEE floating point number representing the X component of the data points position as specified in the call to GraphHost_graphData.
* A 32-bit, IEEE floating point number representing the Y component of the data points position as specified in the call to GraphHost_graphData.

### Stop Sending Data ('d') ###

This request notifies the server to stop sending data from the specified dataset.

The 'd' character is sent to the server, followed by a fifteen byte string describing the dataset from which to stop sending data.

