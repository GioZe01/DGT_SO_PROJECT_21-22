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

Using git:

```
Not allowed by University of Turin :zipper_mouth_face:
```

# Information about the chosen implementation

## Signals utilized by the project

* SIGINT: used to terminate the program
* SIGALRM: used to simulate the time of the program and make routine tasks by the processes
* SIGUSR2: used by the Master to get the status of the processes
* SIGUSR1: involved in the handling of a new node process (the node process is created by the Master)

## Master proc:

1. The master process read the conf file, creates all the necessary IPC resources starting from the semaphores and the
   shared memory. It creates
   a shm for the configuration in order to share between the processes the information about the number of users and the
   number of nodes and their
   positioning into the message queue, and in case of the nodes the friends of each node.\
2. Proceeds to create the processes of the users and the nodes adding them to the shm for the configuration.\
3. Fill the friends positioning into the shm for the configuration.\
4. Configure a signal handler
5. start's the simulation utilizing a semaphore to sinchronize the processes.\
6. Lunch a SIGALRM to print information about the current status of the simulation.
7. Wait for the SIGINT signal to terminate the program.

## Users:

1. The user after reading the conf file attaches to the IPC resources and waits on the starting semaphore.\
2. Lunch a SIGALRM to generate a transaction. (is not related to point 3)\
3. Start generating transactions. until the balance allows to do so.\
4. Wants to get richer, and if it is possible, it does and get back to point 3.\

## Nodes:

1. The node after reading the conf file attaches to the IPC resources and waits on the starting semaphore.\
2. Lunch a SIGALRM to generate send a transaction to a random node friend. (is not related to point 3)\
3. Handle transactions. In case of tp_full and hops exceeded the node advice the master via a master msg queue.\
4. Add the block into the ledger.\
5. back to point 3.\
