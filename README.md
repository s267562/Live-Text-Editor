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

---

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

---

## CRDT - Conflict-Free Replicated Data Type
CRDT is a strategy for achieving consistent data between replicas of data without any kind of coordination between the replicas. For the Live Text Editor there are two critical requirements:
* **Globally Unique Characters**  
With globally unique characters, the delete opeation, received from another user, looks for a globally unique character to delete. The idempotency of delete operations is reached. It is achieved by using the Site ID.
* **Globally Ordered Characters**  
Every character inserted in the text editor will be placed in the same position on every user’s copy of the shared document. We have used fractional indices to represent the position of a character in the CRDT structure. With globally ordered characters insertion and deletion operations commute: deleting a particular character has no effect on the insertion of a new character.

### CRDT structure
We have designed our CRDT strucure as a two-dimensional array of characters. With respect to a linear array structure, our solution is  more efficient for bigger documents. This is primarily due to shifting of the array when splicing characters into and out of a linear array. Moreover, whenever a character is inserted or deleted in the editor, QTextDocument returns a position object that indicates the row and the column on which that change was made.  
With the two-dimensional array the search operation is O(log L + log C) — L being the number of lines and C being the number of characters in that line. For the insertion and deletion the complexity is O(C), while the worst-case time complexity for the linear arrays is O(N) due to shifting.

### CRDT implementation
- **Local Insert:** when inserting a character locally, the only information needed is the character value and the editor index at which it is inserted. In the *HandleLocalInsert* function a new character object will be created and spliced into the CRDT array. Finally, the new character is returned so it can be sent to the other users. The *generateChar* function determines the globally unique fractional index position of the new character. 
- **Local Delete:** deleting a character from the CRDT is a simple operation because all that is needed is to find the index of the character in the structure.That index is used to splice out the character object from the strcture. Using a two-dimensional structure, in the *handleLocalDelete* function we need to differentiate the single-line deletes to the multiple-line deletes.
- **Remote Insert:** when a user receives an insert operation from another user, a binary search algorithm, in the *findInsertPosition* function, is used to find where it should be inserted in the structure and in the editor (the *handleRemoteInsert* function return the computed position). 
- **Remote Delete:** the remote delete messages are managed as the remote insert messages.


### Style
Differents styles can be applied to the text document. These operations can be divide into two categories: 
* style operations applied to characters: these are handled as attributes of Character class;
* style operations applied to lines: these are handled with an additional data structure, called *Style*.

The *style* data structures is an array whose elements take trace of style appied to a certain line. Each element of these vector is made by an unique identifier and the corresponding line style: right alignment, left alignement, etc.
These data structure is necessary to manage the style consistency (for the lines) among different clients (as for insert and delete case).

As for insert and delete case, these operations are managed locally at two different levels:
* QTextDocument level: the view of the application (showing the document) is modified immediately by means of UI tools (Bold,Italic,etc);
* Structure level: the style is saved into the character structure or line structure.

In the end the style change is sent to server.


## Cursors
During the document editing, a client can see other clients operations. Each remote client has an associated **cursor** inside a local client. 
Whenever a remote operations is applied to the document, these cursors positions are udated.
Cursors are mangaged by means of the **OtherCursor** class. The latter is made by:
* a QTextCursor object, that allow the editing operations;
* a QColor object, that is the color associated (only locally) to remote user
* two QLabel:
  * a narrow *rectangle* that emulate a common text editor cursor
  * a label showing the name of user
