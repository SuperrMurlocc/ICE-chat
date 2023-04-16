//
// Created by Jakub Bednarski on 26/03/2023.
//

#include <Ice/Ice.h>
#include <sstream>
#include "../ice/out/chatI.h"
#include "../ice/out/chat.h"
#include "../utils/utils.h"

#define fgBlack "\x1b[30m"
#define fgGray "\x1b[30;1m"
#define fgRed "\x1b[31m"
#define fgGreen "\x1b[32m"
#define fgYellow "\x1b[33m"
#define fgBlue "\x1b[34m"
#define fgMagenta "\x1b[35m"
#define fgCyan "\x1b[36m"
#define fgWhite "\x1b[37m"

#define colorClear "\x1b[0m"

void appLoop(const Ice::CommunicatorHolder&, std::shared_ptr<Chat::LobbyPrx>, std::shared_ptr<Chat::UserPrx>, std::shared_ptr<Chat::UserI>);

void legend();

void listUsers(std::shared_ptr<Chat::LobbyPrx>);

void listRooms(std::shared_ptr<Chat::LobbyPrx>);

int main(int argc, char *argv[]) {
    try {
        Ice::CtrlCHandler ctrlCHandler;

        Ice::CommunicatorHolder communicatorHolder(argc, argv);
        auto communicator = communicatorHolder.communicator();
        
        auto userAdapter = communicatorHolder->createObjectAdapterWithEndpoints("Chat/User/Adapter", "default -p " + std::to_string(utils::getRandomUnusedPort()));
        auto [userPrx, user] = utils::addServantToAdapter<Chat::UserPrx, Chat::UserI>(userAdapter);
        userAdapter->activate();
        
        auto lobbyPrx = utils::getLobbyPrx(communicatorHolder);

        std::string userName;
        std::cout << "login: ";
        std::getline(std::cin, userName);
        user->name = userName;
        
        std::string password;
        std::cout << "password: ";
        std::getline(std::cin, password);
        
        std::cout << "Checking if user exists... ";
        try {
            lobbyPrx->_cpp_register(userPrx, password);
            std::cout << "created new user!"<< std::endl;
        } catch (const Chat::UserExists& e) {
            std::cout << "user exists!" << std::endl;
        }
        
        std::string token;
        std::cout << "Logging in... ";
        try {
            token = lobbyPrx->login(userPrx, password);
            std::cout << fgGreen << "logged in correctly!" << colorClear << std::endl;
        } catch (const Chat::AccessDenied& e) {
            std::cout << fgRed << "wrong password!" << colorClear << std::endl;
            communicatorHolder->shutdown();
            return 1;
        }
        
        Ice::Context ctx;
        ctx["Token"] = token;
        lobbyPrx = lobbyPrx->ice_context(ctx);
        
        ctrlCHandler.setCallback([communicator, lobbyPrx](int) {
            std::cout << "Logging out... ";
            try {
                lobbyPrx->logout();
                std::cout << fgGreen << "logged out!" << colorClear << std::endl;
            } catch (const Chat::AccessDenied& e) { 
                std::cout << fgRed << "was not logged in!" << colorClear << std::endl;
            }
            communicator->destroy();
            exit(0);
        });

        appLoop(communicatorHolder, lobbyPrx, userPrx, user);

        communicatorHolder->waitForShutdown();
    } catch (const Chat::AccessDenied& e) {
        std::cout << fgRed << "access denied! Log in again." << colorClear << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0; 
}

std::string userStatusToString(Chat::UserStatus status) {
    switch(status) {
        case Chat::UserStatus::Online:
            return "online";
        case Chat::UserStatus::Offline:
            return "offline";
        case Chat::UserStatus::BeBack:
            return "be-right-back";
        default:
            return "Unknown";
    }
}

void presentRooms(std::shared_ptr<Chat::LobbyPrx> lobbyPrx, std::shared_ptr<Chat::UserI> user) {
    const auto roomPrxs = lobbyPrx->getRooms();

    if (!roomPrxs.size()) {
        std::cout << "There are no active rooms!" << std::endl;
        return;
    }

    auto myName = user->name;
    
    for (const auto roomPrx : roomPrxs) {
        std::cout << roomPrx->getName() << std::endl;
        const auto userPrxs = roomPrx->getUsers();
        
        if (!userPrxs.size()) {
            std::cout << fgGray << "\tempty" << colorClear << std::endl;
            continue;
        }

        std::cout << fgGray;
        for (const auto userPrx: userPrxs) {
            auto userName = userPrx->getName();
            if (userName == myName) {
                std::cout << "\t" << userName << fgBlue << "(you)" << fgGray <<  " : " << userStatusToString(userPrx->getStatus()) << std::endl;
            } else {
                std::cout << "\t" << userName << " : " << userStatusToString(userPrx->getStatus()) << std::endl;
            }
        }
        std::cout << colorClear;
    }
}

void presentUsers(std::shared_ptr<Chat::LobbyPrx> lobbyPrx, std::shared_ptr<Chat::UserI> user) {
    const auto userPrxs = lobbyPrx->getUsers();

    if (!userPrxs.size()) {
        std::cout << "There are no active users!" << std::endl;
        return;
    }
    
    auto myName = user->name;

    for (const auto userPrx: userPrxs) {
        auto userName = userPrx->getName();
        if (userName == myName) {
            std::cout << userName << fgBlue << "(you)" << colorClear <<  " : " << userStatusToString(userPrx->getStatus()) << std::endl;
        } else {
            std::cout << userName << " : " << userStatusToString(userPrx->getStatus()) << std::endl;
        }
    }
}

void createRoom(std::shared_ptr<Chat::LobbyPrx> lobbyPrx, const std::string& roomName) {
    std::cout << "Creating new room... ";
    try {
        lobbyPrx->createRoom(roomName);
        std::cout << fgGreen << "room created!" << colorClear << std::endl;
    } catch (const Chat::RoomExists& e) {
        std::cout << fgRed << "such room already exists!" << colorClear << std::endl;
    }
}

std::shared_ptr<Chat::RoomPrx> joinRoom(std::shared_ptr<Chat::LobbyPrx> lobbyPrx, const std::string& roomName, std::shared_ptr<Chat::UserPrx> userPrx) {
    std::cout << "Joining room... ";
    try {
        auto roomPrx = lobbyPrx->findRoom(roomName);
        roomPrx->join(userPrx);
        std::cout << fgGreen << "joined!" << colorClear << std::endl;
        return roomPrx;
    } catch (const Chat::NoSuchRoom& e) {
        std::cout << fgRed << "no such room!" << colorClear << std::endl;
    }
    return nullptr;
}

void changeStatus(std::shared_ptr<Chat::UserI> user, std::string newStatus) {
    std::cout << "Changing status... ";
    if (newStatus == "online") {
        user->userStatus = Chat::UserStatus::Online;
    } else if (newStatus == "offline") {
        user->userStatus = Chat::UserStatus::Offline;
    } else if (newStatus == "be-right-back") {
        user->userStatus = Chat::UserStatus::BeBack;
    } else {
        std::cout << fgRed << "no such status!" << colorClear << std::endl;
        return;
    }

    std::cout << fgGreen << "changed!" << colorClear << std::endl;
}

void sendPrivateMessage(std::shared_ptr<Chat::LobbyPrx> lobbyPrx, std::shared_ptr<Chat::UserPrx> fromUserPrx, const std::string& target, const std::string& body) {
    const auto userPrxs = lobbyPrx->getUsers();
    
    for (const auto userPrx : userPrxs) {
        if (userPrx->getName() == target) {
            userPrx->receivePrivateMessageAsync(fromUserPrx, body);
        }
    }
}

void sendMessage(std::shared_ptr<Chat::RoomPrx> roomPrx, std::shared_ptr<Chat::UserPrx> userPrx, const std::string& message) {
    std::cout << fgGreen << "sent!" << colorClear << std::endl;
    roomPrx->sendMessage(userPrx, message);
}

void leaveRoom(std::shared_ptr<Chat::UserPrx> userPrx, std::shared_ptr<Chat::RoomPrx> roomPrx) {
    roomPrx->leave(userPrx);
    std::cout << "Leaving room... " << fgGreen << "left!" << colorClear << std::endl;
}

void appLoop(const Ice::CommunicatorHolder& communicatorHolder, std::shared_ptr<Chat::LobbyPrx> lobbyPrx, std::shared_ptr<Chat::UserPrx> userPrx, std::shared_ptr<Chat::UserI> user) {
    legend();
    std::shared_ptr<Chat::RoomPrx> roomPrx;
    do {
        std::string prompt;
        std::cout << std::endl;
        std::getline(std::cin, prompt);

        if (prompt.rfind("/", 0) == 0) {
            auto [command, target, body] = utils::tokenizePrompt(prompt);

            if (command == "/rooms") {
                presentRooms(lobbyPrx, user);
            } else if (command == "/users") {
                presentUsers(lobbyPrx, user);
            } else if (command == "/room") {
                createRoom(lobbyPrx, target);
                if (roomPrx != nullptr) {
                    leaveRoom(userPrx, roomPrx);
                }
                roomPrx = joinRoom(lobbyPrx, target, userPrx);
            } else if (command == "/join") {
                if (roomPrx != nullptr) {
                    leaveRoom(userPrx, roomPrx);
                }
                roomPrx = joinRoom(lobbyPrx, target, userPrx);
            } else if (command == "/status") {
                changeStatus(user, target);
            } else if (command == "/msg") {
                sendPrivateMessage(lobbyPrx, userPrx, target, body);
            } else if (command == "/leave") {
                leaveRoom(userPrx, roomPrx);
                roomPrx = nullptr;
            } else {
                std::cout << fgRed << "No such command!" << colorClear << std::endl;
            }
        } else {
            if (roomPrx) {
                sendMessage(roomPrx, userPrx, prompt);
            } else {
                std::cout << fgRed << "Join a room first!" << colorClear << std::endl;
            }
        }
        
    } while (!communicatorHolder->isShutdown());
}

void legend() {
    std::cout << fgGray << "Commands:" << std::endl;
    std::cout << "    /rooms - List active rooms." << std::endl;
    std::cout << "    /users - List users." << std::endl;
    std::cout << "    /status [online|offline|be-right-back] - Change status." << std::endl;
    std::cout << "    /join <room> - Join room with given name." << std::endl;
    std::cout << "    /room <room> - Create new room with given name and join it." << std::endl;
    std::cout << "    /msg <user> <message> - Send private <message> to <user>." << std::endl;
    std::cout << "    CTRL + C - Logout and exit." << std::endl;
    std::cout << "  In-room commands:" << std::endl;
    std::cout << "        <message> - Send a <message> to the current room." << std::endl;
    std::cout << "        /leave - Leave the current room." << colorClear << std::endl;
}
