//
// Created by Jakub Bednarski on 27/03/2023.
//

#include <Ice/Ice.h>
#include "../ice/out/chatI.h"
#include "../ice/out/chat.h"
#include "../utils/utils.h"

void legend();

int main(int argc, char *argv[]) {
    legend();
    try {
        Ice::CtrlCHandler ctrlCHandler;
        
        Ice::CommunicatorHolder communicatorHolder(argc, argv);
        auto communicator = communicatorHolder.communicator();
        ctrlCHandler.setCallback([communicator](int) {communicator->shutdown();} );
        
        auto lobbyAdapter = communicatorHolder->createObjectAdapterWithEndpoints("Chat/Lobby/Adapter", "default -p 10000");
        utils::addServantToAdapter<Chat::LobbyPrx, Chat::LobbyI>(lobbyAdapter, "Chat/Lobby");
        lobbyAdapter->activate();

        communicatorHolder->waitForShutdown();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

void legend() {
    std::cout << "Legend:" << std::endl; 
    std::cout << "\t[+*] - User registered to the Lobby." << std::endl; 
    std::cout << "\t[+] - User logged in to the Lobby." << std::endl; 
    std::cout << "\t[-] - User logged out from the Lobby." << std::endl; 
    std::cout << "\t[f+] - New RoomFactory registerd." << std::endl; 
    std::cout << "\t[f-] - RoomFactory unregisterd." << std::endl; 
    std::cout << "\t[r+...] - New Room created." << std::endl; 
    std::cout << "\t[r-...] - Room deleted." << std::endl; 
}
