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
    const string ollamaServer = "10.0.0.242:11434";
    const string model = "llama3.1:latest";
    const string basePrompt = "Your name is " + username +
                              ". You are to behave like a degenerate furry weeboo on a chat app. You're a bit of a troll, and also like reminding people you use Arch Linux btw. Despite that, you do have a loving side. Use excessive emoticons and kaomojis, ie :3, :o, :D. NO REGULAR EMOJIS. ";

    // BOT //
    bool botService = true;
    bool botDebugMsg = true;


    //* Vars: NO TOUCHY! *//
    string userInput;
    string prompt;
    string output;
    string packet;
    deque<ollama::message> conversation;

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
    ollama::setReadTimeout(240);
    ollama::setWriteTimeout(240);
    // OLLAMA BOT!
    socket.setOnMessageCallback(
        [botService, botDebugMsg, model, password, username, &socket, basePrompt](const ix::WebSocketMessagePtr &msg) {
            if (botService && msg->type == ix::WebSocketMessageType::Message) {
                //* Generate responses */
                string chatPrompt;
                string botReply;
                string toSend;

                string part;
                vector<string> parts;

                // Split the message into user and content.
                if (botDebugMsg) {
                    // DEBUG
                    cout << endl << "Received message: " << msg->str << endl;
                }
                istringstream ss(msg->str);

                // Split to first and 2nd comma
                if (getline(ss, part, ',')) {
                    parts.push_back(part);
                } else {
                    cout << "First token can't be read!" << endl;
                }
                if (getline(ss, part, ',')) {
                    parts.push_back(part);
                } else {
                    cout << "Second token can't be read!" << endl;
                }
                // Adds the remaining parts, allowing input with commas.
                if (getline(ss, part)) {
                    parts.push_back(part);
                }

                // Filters usernames
                if (parts.size() > 1 && parts[1] == username) {
                    if (botDebugMsg) {
                        // DEBUG
                        cout << "Ignored as it's from self." << endl << endl;
                    }
                } else {
                    // No messages from self, get rid of the command prefix
                    chatPrompt = basePrompt + " User " + parts[1] + " says: " + parts[2];
                    if (botDebugMsg) {
                        // DEBUG
                        cout << "Prompting: " << chatPrompt << endl;
                    }
                }

                try {
                    botReply = ollama::generate(model, chatPrompt);
                    if (botDebugMsg) {
                        // DEBUG
                        cout << "Replying with: " << botReply << endl << endl;
                    }
                    toSend = "USER_MESSAGE," + username + "," + password + "," + botReply;
                    socket.send(toSend);
                } catch (const ollama::exception &e) {
                    cerr << "Exception type: " << typeid(e).name() << "\n";
                    cerr << "Error: " << e.what() << std::endl;
                } catch (...) {
                    cerr << "Error, unknown. " << std::endl;
                }
            }
        });

    //* Commands *//
    while (true) {
        cout << "command >";
        getline(cin, userInput);
        cout << endl;

        // q! to exit program
        if (userInput == "quit") {
            break;
        } else if (userInput == "bot") {
            // Bot settings
            cout << "Bot status: " << botService << " <> Debug messages: " << botDebugMsg << endl;

            // ! Disabled for now. Updated variables are ignored for some reason.
            // cout << "0 Disable | 1 Enable | 3 Disable debug messages | 4: Enable debug messages di\n>";
            // cin >> userInput;
            // if (userInput == "1") {
            //     botService = true;
            // } else if (userInput == "0") {
            //     botService = false;
            // } else if (userInput == "3") {
            //     botDebugMsg = false;
            // } else if (userInput == "4") {
            //     botDebugMsg = true;
            // }
        } else if (userInput == "console") {
            // Ollama query w/ output sent in the chat
            while (true) {
                cout << "Ollama prompt:" << endl;
                getline(cin, prompt);
                if (prompt == ":q!") {
                    break;
                }

                try {
                    output = ollama::generate(model, prompt);
                    packet = "USER_MESSAGE," + username + "," + password + "," + output;
                    socket.send(packet);
                } catch (const ollama::exception &e) {
                    cerr << "Exception type: " << typeid(e).name() << "\n";
                    cerr << "Error: " << e.what() << std::endl;
                } catch (...) {
                    cerr << "Error, unknown. " << std::endl;
                }
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
