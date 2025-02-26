#include "pch.hpp"
using namespace std;

// Get boolean from string
bool strToBool(const std::string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

    return (lowerStr == "true" || lowerStr == "1");
}

int main(int argc, char* argv[]) {
    // Setting woorking dir
    std::filesystem::path exePath = std::filesystem::canonical(argv[0]).parent_path();
    std::filesystem::current_path(exePath);
    std::cout << "Working directory set to: " << std::filesystem::current_path() << std::endl;

    // *Load config.txt* //
    // Config vars //
    // User
    string username;
    string password;
    // Ollama
    string ollamaServer;
    string model;
    string basePrompt;
    // Bot
    bool botDebugMsg;
    bool botService;

    // Find file
    ifstream file("config.txt");
    if (!file) {
        std::cerr << "ERR: config.txt no opening!" << std::endl;
        return 1;
    }

    // Read file
    string confLine;
    while (getline(file, confLine)) {
        // Ignore #comments
        if (confLine.empty() || confLine[0] == '#')
            continue;
        istringstream iss(confLine);
        string key, value;

        // Set values set by =
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            if (key == "username") { // USER VALUES
                username = value;
            } else if (key == "password") {
                password = value;
            } else if (key == "ollamaServer") { // Ollama values
                ollamaServer = value;
            } else if (key == "model") {
                model = value;
            } else if (key == "basePrompt") {
                basePrompt = value;
            } else if (key == "botService") { // Bot config
                botService = strToBool(value);
            } else if (key == "botDebugMsg") {
                botDebugMsg = strToBool(value);
            }
        }
    }

    // Confirm config load
    cout << "===== Configuration Loaded! =====" << endl;
    cout << "User:" << endl;
    cout << "  Username:     " << username << endl;
    cout << "  Password:     " << password << endl;
    cout << "Ollama:" << endl;
    cout << "  Server:       " << ollamaServer << endl;
    cout << "  Model:        " << model << endl;
    cout << "  Base Prompt:  " << basePrompt << endl;
    cout << "Bot Settings:" << endl;
    cout << "  Debug Messages: " << (botDebugMsg ? "Enabled" : "Disabled") << endl;
    cout << "  Service Mode:  " << (botService ? "Enabled" : "Disabled") << endl;
    cout << "========================" << endl;

    //* VAR DUMP *//
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
