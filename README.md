# tictactoe

## Description

A tictactoe project which implements a server and a client so that two clients can play tictactoe on a given network using the UDP protocol.

## How to run

First launch the server with the following command
> ./server \<address\> \<port\>
 
specifying the address and port number. Then launch the two clients with the following command
> ./client \<address\> \<port\>
 
 specifying the same address and port number. Runnning a third client will result in a connection error. When the grid appears, 
the player will be first prompted for the row value (starting from 0). After entering an integer, the player will be prompted
for the column value at which point if the move is valid, the grid will be printed for the second player and the game will continue
until termination. Upon termination, both the server and client programs will exit.
