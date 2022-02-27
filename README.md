# GT_SO_PROJECT_21-22

----

Note for the programmer:
in any case if termination of the node proc a message would be pushed into master messagge queue
if available before termination occure
# Requirements

* Unix Sys.
* GCC
* Cmake

# Installing

*

# Principal Process Running
* Printing:
    if we want to save current budget into bookmaster shm the problem is that i'm saving to much info
        into the shm consuming bytes. so it'would be great to get info via message report queue
        by sending snapshot of users

# Node Proc
Node_tp_shm_key => current_node.pid
node_tp_semaphore => current_node.id

L'apt della reward per singola transazione viene fatta solo quando block size e caricato -> parte più critica
    *SHM: part of the responsability of a node proc is to create the tp_shm that will be used by the
         node for loading the transaction_block to be processed
    *Node_tp_proc: it just start one tp_proc and kill it when node_proc terminate. (Maybe can be
     implemented a waiting time out of wich node_proc terminate in case of no new block loaded into
     tp shm)
Note for the programmer:
in any case if termination of the node proc a message would be pushed into master messagge queue
if available before termination occure
# Node Tp Proc
*  Scopes:
    1)  Keep the queue empty from all the messages related to is node parent. Serve and mantain updated
        the tp_shm memory in order to allow node_proc to process all the blocks in order
    2)  Make sure that the tp_size is respected and advice user in case of tp_size full
    3)  In case of executing the 30 point version it is able to handle the transaction incoming when
        the tp size is full. If full it will send the transaction to another node
# User Proc

Viene tenuto in vita fino a che la lista di transazioni non è vuota (if cashing is on). otherwise check for balance and
terminate if balance is > 2 o riceve il segnale di terminazione o il bilancio non lo permette.

ACTUALLY THERE ARE DIFFERENT CHOICES FOR ENDING USER PROC:

1) MAKE IT RUN FOREVER UNTIL ALARM SEND FROM MAIN -> TIMEOUT
2) MAKE IT PAUSE ON BALANCE <2 AND RESUME IT
3) MAKE IT END ON BALANCE <2 AND RUN A NEW PROC WITH SAME CHARACTERISTICS IF MONEY ARRIVES
4) MAKE IT END WITHOUT CONSIDERING INCOMING FLOW OF CASH

- further implementation would be to resume the user (because can listen to the same ref into the message queue)
# Transazioni
    -> timestamp della transazione è caricato una prima volta dallo user per poi essere aggiornato dal
    nodo
    -> quando lo user la crea il reward non è ancora impostato sulla transazione

