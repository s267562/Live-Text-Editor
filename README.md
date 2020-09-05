# PDS Project - Live Text Editor

## Server side

The main thread allows you to accept new connections and manage the user's authentication request. Once the client decides to create/request a file, the server hands the socket management to a secondary thread, managed by the Thread class. 

The sockets used are belong to the QTcpSocket class persisted in the QT library. They allow you to manage incoming messages thanks to the use of events.

### DB

We used a relational DB (SQLite) with this structure:
- users (User data)
  - username 
  - password 
  - salt
  - avatar (BLOB)
- files
  - fileID 
  - username (owner of the file)
- sharing
  - fileID 
  - owner 
  - sharedToUser
  
The DB object is managed via the QSqlDatabase class.
 

### File persistency and backup 

#### File persistency
File persistency is implemented using Qt Serialization inside the thread who's managing the user's file. We set a timer 
paired with a boolean who indicates if the CRDT has been modified since the last save. 
When the crdt is modified and the timer goes in timeout we save the CRDT structure to a JSON file in a recursive way.
We had to implement a "write" method for every custom object we need to save inside the JSON (that's why we said recursive way).

#### Backup
Alongside the serialization we implemented a mechanism of backup for preventing file corruption in case of server crash.
We went for a 3 level backup, using a save counter, every 2 and 10 times we save the file we do a backup.
The primary backup make a copy of the original JSON file (the one in use by the server), while the secondary backup make
a copy of the primary backup. 
We use backups file when trying to read a JSON this is corrupted: we experimented that a corrupted JSON result in a blank
file (0B dimension) while an empty file (from the application point of view) results in a JSON containing at least some 
basic structures. In this way we can check the JSON status by watching his size and try to recover one of the previous backups
in case of corruption.


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
