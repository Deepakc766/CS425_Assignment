
# Chat Server CS425_Assignment 1  - README file 

## Introduction or overview about implemented chat server 
This project is about to implements a multi-threaded chat server that supports private messages, group communica-
tion, and user authentication,in C++ using sockets Programming. The server allows clients to:
- Authenticate using a username and password 
- Send messages to all connected clients (using broadcast function in server code ).
- Send private messages to specific users.(using Private message function,from client socket to recepient )
- Create, join, and leave groups.
- Send messages to groups using group_msg.
- Handle multiple clients concurrently using multithreading.


## Features of implemented in chat server 
- User Authentication: Reads user credentials from `users.txt` and verifies login using Authentication function in servere code ,if authentical successful then it will return "welcone to chat server" else "Authentication failed" .
- Multithreading: Each client runs in a separate thread to handle concurrent connections.
- Broadcast Messaging: Allows a client to send messages to all connected users through server .
- Private Messaging: Enables one-to-one messaging between users.
- Group Management: Clients can create, join, leave, and message within groups using appropriate command.
- Thread Safety: Uses mutex locks to prevent race conditions(concrency) and ensure data integrity.



### High Level idea of server implementation and  flow chart of server code and important function 
A simple Flow chart is made that show how chat server is implemented 
This is a multi-threaded chat server written in C++ using sockets, supporting features like:
1.User authentication (from users.txt)
2.Broadcast messages
3.Private messages
4.Group creation, joining, leaving, and group messaging
5.Multi-threading for handling multiple clients 
###   Important Functions & Their Roles
1. load_users() from user.txt file 
Reads user credentials from users.txt into an unordered map (users).
Helps in user authentication.

2. authenticate_user(int client_socket)
Prompts the client for username & password.
Verifies credentials from users map.
If successful, adds the client to clients map.
If authentication fails, the client is disconnected.

3. broadcastmessage(const string &message, int sender_socket)
Sends the given message to all connected clients except the sender because sender should not send message to itself.

4. privatemessage(const string &recipient, const string &message, int sender_socket)
Sends a private message to a specific username if they are online.

5. createGroup(const string &group_name, int client_socket)
Creates a new group with the given name by groups map .
Adds the sender to the group.

6. joinGroup(const string &group_name, int client_socket)
Adds the sender to an existing group.

7. leaveGroup(const string &group_name, int client_socket)
Removes the sender from the specified group.

8. group_message(int sender_socket, const string &message, const string &group_name)
Sends a message to all members of a group except the sender.

9. handle_client(int client_socket)
Handles communication with a single client.
Calls authenticate_user(), then waits for messages.
Parses and executes commands like /broadcast, /msg, /create_group,/leave_group etc.
If a client disconnects, it removes them from the clients map.

Then finally Sets up the server socket and binds it to a port.
Listens for new connections  and starts a new thread (handle_client()) for each client.

###          Code Flow for server Code 
   +-------------------------------------+
   |          Start Server              |
   +-------------------------------------+
                    |
                    v
   +-------------------------------------+
   |   Load Users from users.txt         |
   +-------------------------------------+
                    |
                    v
   +-------------------------------------+
   |   listen for incoming connections   |
   +-------------------------------------+
                    |
                    v
   +-------------------------------------+
   |  Accept new client connection       |
   +-------------------------------------+
                    |
                    v
   +-------------------------------------+
   |  create new thread for client       |
   +-------------------------------------+
                    |
                    v
   +-------------------------------------+
   |  Authenticate client                |
   +-------------------------------------+
       /         |           \
      /          |            \
  Success    Failure      Connection closed
      |          |               |
      |          v               v
      |    Close socket      Remove client from list
      |
      v
+----------------------------------------------------------+
|   Wait for messages from the client                      |
+----------------------------------------------------------+
        |          |           |           |               |
        |          |           |           |               |
        |          |           |           |               |
  /broadcast   /msg user  /create_group  /group_msg      /group_leave 
    |          |            |             |                   |
    v          v            v             v                   v
Broadcast   Private    Create group    Send message        Leave the group if
 Message    Message       & join       to group members     you don't want to continue 
    |          |            |             |
    v          v            v             v
+-----------------------------------------------------------------------------------+
|                        Continue until client disconnects                          |
+-----------------------------------------------------------------------------------+
                                    |
                                    v
                +------------------------------------------+
                |   Close socket and remove client        |
                +------------------------------------------+





### C++ Libraries used  for chat  Implementation 
-iostream: For standard input/output operations.
-fstream: For file reading/writing for user.txt file.
-thread: To create and manage multiple threads.
-vector: A dynamic array that resizes automatically.
-unordered_map: A hash table implementation of key-value pairs.
-unordered_set: A hash table-based set for unique unordered elements.
-mutex: For thread synchronization to prevent race conditions and concurrency.
-cstring: For handling C-style strings and memory operations.
-netinet/in.h: Provides structures and constants for socket programming.
-unistd.h: Contains system calls like read(), write(), and close() to use this call.


## Files Included
- `server.cpp`: The main server implementation,which include all features like broadcast,create group ,join group,leave group ,private message etc 
- `client.cpp`: The client-side implementation ,will connect to server .
- `users.txt`: A file storing usernames and passwords in `username:password` format.

## Testing & Compilation
To compile the server and client, use the following commands:

```sh
# Compile the server using c++20 command to avoid error in (start_with) function 
g++ -std=c++20 server.cpp -o server (c++20 to avoid error with (start_with))

# Compile the client
g++ -std=c++20 client.cpp -o client
```

## Running the Server
To start the server, run:

```sh
./server
```

The server will listen on port `8080` for incoming client connections.

## Running the Client
To test the server, compile and run the client with:

```sh
./client 
```

## Server Code :

### 1. Socket Creation and Binding
The server starts by creating a socket using the `socket()` function:

```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
```

Then it binds the socket to an IP address and port and will be ready listen and connect from server side:

'''cpp
address.sin_family = AF_INET;//socket will use the IPv4 address family.
address.sin_addr.s_addr = INADDR_ANY;//allows the server to listen on all available network interfaces.
address.sin_port = htons(PORT);//convert PORT from host byte(little endian) to network byte (big edian) to work on network(because network work on big endian)
bind(server_fd, (struct sockaddr*)&address, sizeof(address));//This binds the socket to the IP address and port number specified in address, making it ready to listen for incoming client connections from clients side 
```

### 2. Listening for Clients
The server listens for incoming client connections:

```cpp
listen(server_fd, 3);
```

When a client connects, the server accepts the connection and spawns a new thread to handle it and further communication start:

```cpp
thread client_thread(handle_client, new_socket);
client_thread.detach();
```

### 3. User Authentication
The server prompts the client for a username and password. It verifies credentials from `users.txt`:

```cpp
ifstream file("users.txt");//open the file in read mode username:password 
```

If authentication fails, the connection is closed:

```cpp
send(client_socket, "Authentication failed.\n", 23, 0);
close(client_socket);
```

### 4. Handling Messages(like broadcast ,private message,create group ,join group ,leave group etc )
The server processes various commands that the client can send:

### Broadcast Message:
  - Clients can send messages to all users with the command: `/broadcast <message>`
  - This is implemented using:

### Private Message:
  - Users can send private messages with: `/msg <username> <message>`
  - Implemented using:
### Group Messaging:
  - Create a group: `/create_group <group_name>` will create of a group of <group_name> you will recieved a message after creating  group and implemented using map groups 
  first find count of group name ,if its zero then make a group using using groups[group_name]={client} 
  - Join a group: `/join_group <group_name>` will join  a group of <group_name> you will recieved a message after joining group and implemented suing map groups
  first we will check if that particular group exist using its count and then if exist then we will join group by inserting client socket in groups[group_name]; 
  - Leave a group: `/leave_group <group_name>`similarly erase will aslo work by erasing client socket from groups[group_name]
  - Send a message to a group: `/group_msg <group_name> <message>`checks if the group exists using groups.count(group_name), and sends the message to all group members except the sender. If the group doesn't exist, an error message is sent to the sender.

### 5. Handling Client Disconnection
When a client disconnects, it is removed from the active clients list:

```cpp
lock_guard<mutex> lock(client_mutex);
clients.erase(client_socket);
close(client_socket);
```

## Example `users.txt`
Ensure we have a `users.txt` file with the following format:

```
username1:password1
username2:password2
```

## Sample Expected Output
### Server Output
```
Starting server...
Server is listening on port 8080
Waiting for a new connection...
New connection accepted.
New client connected. Authenticating...
Users loaded from file:
username1 : password1
username2 : password2
```

### Client Output
```
Creating client socket...
Client socket created successfully.
Connected to server.
Enter username: username1
Enter password: password1
Welcome to the chat server!
Enter message: /broadcast Hello everyone!
broadcast Hello everyone! will print in connected clients
then further we can create group and chat or private chat and so on .
```

### Contributor
 Deepak chaurasia 220330(100%,code implemetation,readme)
 Amruth Raj 220642

### Source used 
https://youtu.be/okzEZmnVWnM?si=qQhWPlY-s_oaMoeD
https://beej.us/guide/bgnet/html/#connect
https://youtube.com/playlist?list=PLhnN2F9NiVmAMn9iGB_Rtjs3aGef3GpSm&si=cYjBEobhcd5UVtvr




