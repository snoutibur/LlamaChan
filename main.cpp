#include "ixwebsocket/IXWebSocket.h"
#include "ollama.hpp"

#include <iostream>
using namespace std;

int main() {
    cout << "hi mom" << endl;

    // ! SETTINGS ! //
    // User
    const string username = "Llamachan";
    const string password = "LlamasAreSoCuteUWU";
    // Ollama
    const string ollamaServer = "173.190.231.120:11434";
    const string model = "llama3.2:1b";


    // Var dump
    string prompt;

    // Connects to websocket server
    ix::WebSocket socket;
    socket.setUrl("wss://chat.stormyyy.dev");

    // Login as user
    socket.setOnMessageCallback([&socket, username, password](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            std::cout << "Connection with server established!" << std::endl;

            // Attempting login!!
            string loginData = "LOGIN," + username + "," + password + ",NULL";
            socket.send(loginData);
            std::cout << "LOGIN AS:" << loginData << std::endl;
        }
        else if (msg->type == ix::WebSocketMessageType::Error) {
            std::cerr << "ERR: " << msg->errorInfo.reason << std::endl;
        }
    });

    socket.start();


    //* OLLAMA *//
    // Init connection
    ollama::setServerURL(ollamaServer);
    // Test prompt
    cout << ollama::generate(model, "Hi!") << endl;

    // Command system
    string userInput;

    // Commands
    while (true) {
        cout << ">";
        getline(cin, userInput);

        // q! to exit program
        if (userInput == "q!") {
            break;
        } else if (userInput == "olb") {
            cout << "Serving Ollama model as bot";
            // TODO: Implement bot
        } else if (userInput == "olc") {

        }
    }


    // string output;
    // string ollamaOut;
    // while (true) {
    //
    //
    //     ollamaOut = ollama::generate(model, prompt);
    //     output = "USER_MESSAGE," + username + "," + password + "," + ollamaOut;
    //     cout << output << " >>" << endl << endl;
    //     socket.send(output);
    //
    //
    // }

    cout << "Quitting process..." << endl;
    socket.stop();
    return 0;
}