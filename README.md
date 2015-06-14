unix-chat
========

The goal of this project is to implement a chat client and server which operates over a UNIX domain socket.
Once the server is started, clients can connect and send messages to it. Messages recieved by the server are then sent
to all clients. When users connect, their name in chat will be their login name.

### ssh-chat
In addition to allowing local users with shells (i.e. bash, zsh) to chat, we can also specify the client binary as the shell for a user,
and they will be forced to chat when they login. Users could either have dedicated accounts for chat, or users could sign into a shared
account and be prompted for a username upon logon.

THIS PROJECT IS CURRENTLY UNDER DEVELOPMENT.
CODE MAY NOT BE COMPLETE OR FUNCTIONAL.
USE AT YOUR OWN RISK.
