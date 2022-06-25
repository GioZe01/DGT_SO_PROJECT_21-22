@mainpage Information about the project

# GT_SO_PROJECT_21-22

For more detail on the project see the [docs](/docs/html/index.html)

# Description of the project

The project is meant to be an example of the use of IPC (Inter-Process Communication) between different processes;
simulating a ledger.
Utilizing the IPC, the project will be able to send and receive messages between different processes:

- A **master** for the handling of the simulation, the creation of the other processes, etc..
- SO_USER_NUM number of **users**, each one of them will be able to send money to other users via transactions.
- SO_NODE_NUM number of **nodes** that elaborate the transactions.

For the full text of the project, please visit the [Consegna Progetto](/docs/2021.12.19_consegna_progetto.pdf).\
The implementation follows the SYSTEM V IPC (Semaphores, Shared Memory, Message Queues) model and C89 standard.

# Requirements

* Doxygen: for rendering the documentation
* Unix Sys: Tested on Ubuntu 21.10
* GCC or GDB: make sure to set the ***correct compiler flags into the Makefile***
* Cmake

# Installing

Using .zip file:

```
unzip gt-so-project-21-22.zip
```

Using git :zipper_mouth_face: :

```
Not allowed by University of Turin 
```
# Running
```
cd gt-so-project-21-22
make all 
make run
```
**NOTE: make sure to have all the folders in the right place and checked the compiler fleg into the Makefile**
# Information about the chosen implementation:

For each IPC there is a specific implementation check the .h files to see which implementation is used and how it is implemented.
Specifically for the message queue.

## Message Queue
There are 3 message queues with the respected [values](src/local_lib/headers/glob.h) (click to see).
1. Master message queue whose messages are defined in the [master_msg_report](src/local_lib/headers/master_msg_report.h) header.
2. Users message queue in which each user has its own priority queue of messages. The value of each process position is incremental with a delta described in [user_msg_report.h](src/local_lib/headers/user_msg_report.h) header, so that when a process is created his value will be the one before + the delta.
3. Nodes message queue in which each node has its own priority queue of messages. The value of each process position is incremental with a delta described in [node_msg_report.h](src/local_lib/headers/node_msg_report.h) header, so that when a process is created his value will be the one before + the delta.
## Shared Memory
There are 2 shared memory with the respected [values](src/local_lib/headers/glob.h) (click to see).
1. Shared mamory for the configuration of the simulation.
2. Shared memory for the ledger.
Each of which has its own way of regulating the access to the shared memory via semaphores. In the respected header files you can find the details.

## Signals utilized by the project

* SIGINT: used to terminate the program
* SIGALRM: used to simulate the time of the program and make routine tasks by the processes
* SIGUSR2: used by the Master to get the status of the processes
* SIGUSR1: involved in the handling of a new node process (the node process is created by the Master)

## Master proc:
##### Activity:
1. The master process read the conf file, creates all the necessary IPC resources starting from the semaphores and the
   shared memory. It creates
   a shm for the configuration in order to share between the processes the information about the number of users and the
   number of nodes and their
   positioning into the message queue, and in case of the nodes the friends of each node.
2. Proceeds to create the processes of the users and the nodes adding them to the shm for the configuration.
3. Fill the friends positioning into the shm for the configuration.
4. Configure a signal handler
5. start's the simulation utilizing a semaphore to sinchronize the processes.
6. Lunch a SIGALRM to print information about the current status of the simulation.
7. Wait for the SIGINT signal to terminate the program.

##### Additional info:
While the node generate the processes they are added into a Linked List call ProcList which is and Abstract Data Type.
For his implementation check the [process_info_list.c](/src/local_lib/process_info_list.c) file.\
The implementation of the ADT is based on the information acquired during the reading of the book "C Programming A Modern Approach" by K.N. King second edition for C89 and C99.

The printing of the information of the state of the simulation utilizes a merge sort algorithm on a linked list of process_info_list.c.
Could have been done differently, but for the sake of simplicity I decided to use the merge sort algorithm which is a stable sort for linked lists.

The information regarding the node's friends is store into an integer value using the bitwise operators, for his implementation check the [int_condenser.c](/src/local_lib/int_condenser.c) file.
## Users:

1. The user after reading the conf file attaches to the IPC resources and waits on the starting semaphore.
2. Lunch a SIGALRM to generate a transaction. (is not related to point 3)
3. Start generating transactions. until the balance allows to do so.
4. Wants to get richer, and if it is possible, it does and get back to point 3.

##### Additional info:
As it has been said the user is kept in a loop until the reception of a SIGINT signal. otherwise it keeps hoping for more money to then spend.
I'm totally aware of the fact that the user is kept in a loop, and can be improved by waiting for a Specific signal to resume his activity instead of waiting for a message.
## Nodes:

1. The node after reading the conf file attaches to the IPC resources and waits on the starting semaphore.
2. Lunch a SIGALRM to generate send a transaction to a random node friend. (is not related to point 3)
3. Handle transactions. In case of tp_full and hops exceeded the node advice the master via a master msg queue.
4. Add the block into the ledger.
5. back to point 3.
##### Additional info:
In case of SIGUSR1 the node will wait for a message containing the new friend of the node and will add it to the "list" of friends.


## Discussion:
I'm aware of the fact that while running many processes at the same time the system is not able to print the information because some processes keep blocking the signals 
of request of information. I'm also aware of the fact that running with high block size cause a similar problem: no update on budget. Indeed the nodes are so fast to free 
their transaction pool that they never reach the min block size to load into the shm.

This is probably due to the fact that I was not able to discuss my choice of implementation of the IPC with other students.
Truly my fault, I'm open to a free discussion. Criticisms are welcome. 

Tnx for the reading and time spent. 

Giovanni Terzuolo. 

