# PDS Project - Live Text Editor

## Server side

The main thread allows you to accept new connections and manage the user's authentication request. Once the client decides to create/request a file, the server hands the socket management to a secondary thread, managed by the Thread class. 

The sockets used are belong to the QTcpSocket class persisted in the QT library. They allow you to manage incoming messages thanks to the use of events.

### DB

### Backup

### CRDT

## Client side

The client consists of only one thread, that is the main one, in order to manage local and remote operations in an orderly and atomic way, so as not to cause errors when managing the CRDT structure.

The UI is managed thanks to the use of the QT framework, where the classes within the UI folder allow the display of views.
The views of interest allow the management of the following operations:

- Login;
- Registration;
- Account modification;
- Viewing / management of user files;
- Editor;
- Creating a file;
- Sharing a file with other users;
- Changing the properties of a file (e.g. file name and privileges).

They are managed thanks to the use of the Controller class.
The Editor class allows the management of the content inserted within the document, by entering the characters within the CRDT structure. Once the characters are processed by the CRDT class, they are sent to the server which will send the inserted characters to the other active users.

The networking part is handled by the Messanger class, which uses a socket of the QTcpSocket class.
