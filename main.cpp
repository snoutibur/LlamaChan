#include "ixwebsocket/IXWebSocket.h"
#include "ollama.hpp"

#include <iostream>
using namespace std;



int main() {
    // ! SETTINGS ! //
    // User //
    const string username = "Llamachan";
    const string password = "LlamasAreSoCuteUWU";
    // Ollama //
    const string ollamaServer = "localhost:11434";
    const string model = "llama3.2:1b";
    ollama::setReadTimeout(120);
    ollama::setWriteTimeout(120);

    //* Vars: NO TOUCHY! *//
    string userInput;
    string prompt;
    string output;
    string packet;

    cout << "hi mom" << endl;

    //* WEBSOCKET INIT *//
    ix::WebSocket socket;
    socket.setUrl("wss://chat.stormyyy.dev");

    // Login as user
    socket.setOnMessageCallback([&socket, username, password](const ix::WebSocketMessagePtr &msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            cout << "Connection with server established!" << endl;

            // Attempting login!!
            string loginData = "LOGIN," + username + "," + password + ",NULL";
            socket.send(loginData);
            cout << "LOGIN AS:" << loginData << endl;
            cout << endl << endl;
        } else if (msg->type == ix::WebSocketMessageType::Error) {
            cerr << "ERR: " << msg->errorInfo.reason << endl;
        }
    });

    socket.start();

    //* OLLAMA *//
    // Init connection
    ollama::setServerURL(ollamaServer);
    // Test prompt

    //* Commands *//
    while (true) {
        cout << "command >";
        getline(cin, userInput);

        // q! to exit program
        if (userInput == "quit") {
            break;
        } else if (userInput == "bot") {
            // Enables the command for ollama chat
            cout << "Serving Ollama model as bot";
            // TODO: Implement bot

        } else if (userInput == "console") {
            // Ollama query w/ output sent in the chat
            while (true) {
                cout << "Ollama prompt:" << endl;
                getline(cin, prompt);

                if (prompt==":q!") {
                    break;
                }

                output = ollama::generate(model, prompt);
                packet = "USER_MESSAGE," + username + "," + password + "," + output;
                socket.send(packet);
            }

        } else if (userInput == "chat") {
            // Manually send messages into the chat //
            cout << "Sending messages through console" << endl << endl;
            while (true) {
                cout << ">";
                getline(cin, userInput);
                if (userInput == ":q!") {
                    break;
                }

                packet = "USER_MESSAGE," + username + "," + password + "," + userInput;
                cout << packet << "->" << endl;
                socket.send(packet);
            }
        }
    }

    cout << "Quitting process..." << endl;
    socket.stop();
    return 0;
}
