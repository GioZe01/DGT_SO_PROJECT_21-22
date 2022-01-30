# GT_SO_PROJECT_21-22

----

# Requirements

* Unix Sys.
* GCC
* Cmake

# Installing

*

# Principal Process Running

# User Proc

Viene tenuto in vita fino a che la lista di transazioni non è vuota (if cashing is on). otherwise check for balance and
terminate if balance is > 2 o riceve il segnale di terminazione o il bilancio non lo permette.

ACTUALLY THERE ARE DIFFERENT CHOICES FOR ENDING USER PROC:
1) MAKE IT RUN FOREVER UNTIL ALARM SEND FROM MAIN -> TIMEOUT
2) MAKE IT PAUSE ON BALANCE <2 AND RESUME IT
3) MAKE IT END ON BALANCE <2 AND RUN A NEW PROC WITH SAME CHARACTERISTICS IF MONEY ARRIVES
4) MAKE IT END WITHOUT CONSIDERING INCOMING FLOW OF CASH

- further implementation would be to resume the user (because can listen to the same ref into the message queue)
