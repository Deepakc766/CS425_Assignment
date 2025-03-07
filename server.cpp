// g++ -std=c++20  server.cpp -o server -lpthread
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;
#define PORT 8080

unordered_map<int, string> clients;  // Client socket -> username
unordered_map<string, string> users; // Username -> password
unordered_map<string, unordered_set<int>> groups; // Group -> sockets
mutex client_mutex, group_mutex;
//Load users from file
void load_users() {
    ifstream file("users.txt");//file containing username and password
    string line, username, password;//line to read from file, username and password

    while (getline(file, line)) {
        size_t delim = line.find(':');//find the delimiter
        if (delim != string::npos) {
            username = line.substr(0, delim);//get the username
            password = line.substr(delim + 1);//get the password
            users[username] = password;//store the username and password in the map users
        }
    }
//print the users loaded from file
    ::cout << "Users loaded from file:\n";
    for (const auto &user : users) {
        ::cout << user.first << " : " << user.second << endl;
    }
}
//Broadcast message to all clients except the sender
void broadcastmessage(const string &message, int sender_socket) {
    lock_guard<mutex> lock(client_mutex);//lock the client mutex
    for (const auto &client : clients) {
        if (client.first != sender_socket) {
            string msg = "["+ clients[sender_socket] + "] : " + message;//message to be sent
            send(client.first, msg.c_str(), msg.size(), 0);//send the message to the client
        }
    }
}

bool authenticate_user(int client_socket) {
    char buffer[1024] = {0};//buffer to store the message

    send(client_socket, "Enter username: ", 16, 0);//  send the message to the client
    recv(client_socket, buffer, 1024, 0);//receive the message from the client
    string username(buffer);//store the message in the string
    username.erase(username.find_last_not_of("\r\n") + 1);//erase the last character

    memset(buffer, 0, sizeof(buffer));//clear the buffer
    send(client_socket, "Enter password: ", 16, 0);//send the message to the client
    recv(client_socket, buffer, 1024, 0);//receive the message from the client
    string password(buffer);//  store the message in the string
    password.erase(password.find_last_not_of("\r\n") + 1);//erase the last character

    if (users.count(username) && users[username] == password) {
        lock_guard<mutex> lock(client_mutex);
        clients[client_socket] = username;//store the client socket and username in the map clients
        string message = "Welcome to the chat server!\n";//message to be sent
        send(client_socket, message.c_str(), message.size(), 0);//send the message to the client
        return true;
    }

    send(client_socket, "Authentication failed.\n", 23, 0);
    close(client_socket);
    return false;//return false if authentication fails
}



void privatemessage(const string &recipient, const string &message, int sender_socket) {
    lock_guard<mutex> lock(client_mutex);
    for (const auto &client : clients) {
        if (client.second == recipient) {//if the client.second is the recipient then sent the message
            string msg = "[" + clients[sender_socket] + "] : " + message;
            send(client.first, msg.c_str(), msg.size(), 0);
            return;
        }
    }
}

void createGroup(const string &group_name, int client_socket) {

    lock_guard<mutex> lock(group_mutex);
    if (groups.count(group_name) == 0) {
        groups[group_name] = {client_socket};
        std:: string message = "Group " + group_name + " created.\n";
        send(client_socket,message.c_str(),sizeof(message), 0);
    } else {
        send(client_socket, "Group already exists.\n", 22, 0);
    }

    ::cout<<"Group created with name: "<<group_name<<endl;
}


void joinGroup(const string &group_name, int client_socket) {
    lock_guard<mutex> lock(group_mutex);
    ::cout << "join group with name: " << group_name << endl;
    if (groups.count(group_name) != 0) {//if the group exists
        ::cout << "Group exists" << endl;
        if (groups[group_name].count(client_socket)) {//if the client is already a member of the group
            send(client_socket, "You are already a member of this group.\n", 40, 0);
            return;
        } else {
            groups[group_name].insert(client_socket);//add the client to the group
            std::string message = "You joined the group " + group_name + ".\n";//message to be sent
            send(client_socket, message.c_str(), message.length(), 0);
        }
    } else {
        send(client_socket, "Group does not exist.\n", 22, 0);
    }
}


void leaveGroup(const string &group_name, int client_socket) {
    lock_guard<mutex> lock(group_mutex);
    if (groups.count(group_name) && groups[group_name].count(client_socket)) {//if the group exists and the client is a member of the group
        groups[group_name].erase(client_socket);//remove the client from the group
        std:: string message = "You have left the group " + group_name + ".\n";
        send(client_socket,message.c_str(),message.length(), 0);
    } else {
        send(client_socket, "Group does not exist or you are not a member.\n", 45, 0);//
    }
}

void group_message(int sender_socket, const string &message, const string group_name) {
    ::cout<<"Group message received: "<<message<<endl;
    std::cout<<"Group name: "<<group_name<<std::endl;
    ::cout << "Group name (trimmed): " << group_name << endl;
    lock_guard<mutex> lock(group_mutex);
    if (groups.count(group_name)) {//if the group exists
        for (auto client : groups[group_name]) {//send the message to all clients in the group
            if (client != sender_socket) {//except the sender
                string msg = "[" + group_name + "] : " + message;//message to be sent
                send(client, msg.c_str(), msg.length(), 0);
            }
        }
    } else {
        send(sender_socket, "Group does not exist.\n", 22, 0);
    }
}

void broadcastjoinmessage(const string &message, int sender_socket) {
    lock_guard<mutex> lock(client_mutex);
    for (const auto &client : clients) {
        if (client.first != sender_socket) {
            send(client.first, message.c_str(), message.size(), 0);
        }
    }
}

void handle_client(int client_socket) {
    ::cout << "New client connected. Authenticating..." << endl;
    if (!authenticate_user(client_socket)) {
        return;
    }

    // messaging to all clients that: #username has joined the chat when a new client joins
     broadcastjoinmessage(clients[client_socket] + " has joined the chat.\n", client_socket);

    char buffer[1024] = {0};

    while (true) {
        memset(buffer, 0, sizeof(buffer));//clear the buffer
        int byte_recv = recv(client_socket, buffer, sizeof(buffer), 0);
        if (byte_recv <= 0) {
            ::cout << "Client Disconnected: " << client_socket << endl;
            break;
        }

        string message(buffer);
        message.erase(message.find_last_not_of("\r\n") + 1);

        if (message == "quit") {
            break;
        }

        if (message.starts_with("/broadcast")) {//if the message starts with /broadcast
            broadcastmessage(message.substr(11), client_socket);// broadcast the message to all clients
        } else if (message.starts_with("/msg")) {
            size_t pos = message.find(' ',5);// find the position of the space
            if (pos != string::npos) {
                string recipient = message.substr(5, pos - 5);//get the recipient
                string msg = message.substr(pos + 1);//get the message
                privatemessage(recipient, msg, client_socket);//send the private message
            }
        } else if (message.starts_with("/create_group")) {//if the message starts with /create_group
            createGroup(message.substr(14), client_socket);//  create a group
        } else if (message.starts_with("/join_group")) {//if the message starts with /join_group
            joinGroup(message.substr(12), client_socket);// join a group
        } else if (message.starts_with("/leave_group")) {//if the message starts with /leave_group
            leaveGroup(message.substr(13), client_socket);//leave a group
        } else if (message.starts_with("/group_msg")) {//if the message starts with /group_msg
            size_t pos = message.find(' ', 11);//find the position of the space
            if (pos != string::npos) {//if the position is not npos
                string group_name = message.substr(11, pos - 11);//get the group name
                ::cout<<"Group name: "<<group_name<<endl;//print the group name
                string msg = message.substr(pos + 1);//get the message
                ::cout<<"Message: "<<msg<<endl;//   print the message
                group_message(client_socket, msg, group_name);
            }
        }
    }

    lock_guard<mutex> lock(client_mutex);
    clients.erase(client_socket);
    close(client_socket);
}

int main() {
    ::cout << "Starting server..." << endl;

    int server_fd, new_socket;  // server_fd is the file descriptor for the socket that the server will use to listen for incoming connections
    // new_socket is the file descriptor for the socket that the server will use to communicate with the client
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    const char* hello = "Hello from server";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }




    // Set socket options (macOS compatible)
    // This allows the server to reuse the address and port helping to avoid the "Address already in use" error
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;//IPv4
    address.sin_addr.s_addr = INADDR_ANY;//IP address of the host
    address.sin_port = htons(PORT);//convert the little-endian(host byte order) to big-endian(network byte order)

    // Bind the socket to the address
    if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;
    load_users();

    while (true) {

        ::cout<<"Waiting for a new connection..."<<endl;
        
        // Accept an incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }

        ::cout<<"New connection accepted."<<endl;
        // Create a new thread to handle the client
        thread client_thread(handle_client, new_socket);
        client_thread.detach();//detach the thread

    }

    close(server_fd);
    return 0;
}