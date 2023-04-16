//
// Created by Jakub Bednarski on 27/03/2023.
//

#include <Ice/Ice.h>
#include "../ice/out/chatI.h"
#include "../ice/out/chat.h"
#include "../utils/utils.h"

int main(int argc, char *argv[]) {
    try {
        Ice::CtrlCHandler ctrlCHandler;

        Ice::CommunicatorHolder communicatorHolder(argc, argv);
        auto communicator = communicatorHolder.communicator();

        std::cout << "Creating Room Factory... ";
        auto roomFactoryAdapter = communicatorHolder->createObjectAdapterWithEndpoints("Chat/Lobby/RoomFactory", "default -p " + std::to_string(utils::getRandomUnusedPort()));
        auto [roomFactoryPrx, roomFactory] = utils::addServantToAdapter<Chat::RoomFactoryPrx, Chat::RoomFactoryI>(roomFactoryAdapter);
        roomFactoryAdapter->activate();

        roomFactory->objectAdapter = roomFactoryAdapter;
        std::cout << "created!" << std::endl;

        std::cout << "Registering Room Factory... ";
        auto lobbyPrx = utils::getLobbyPrx(communicatorHolder);
        try {
            lobbyPrx->registerRoomFactory(roomFactoryPrx);
            std::cout << "registered!" << std::endl;
        } catch (const Chat::RoomFactoryExists& e) {
            std::cout << "was already registered! Closing..." << std::endl;
            return 2;
        }

        ctrlCHandler.setCallback([communicator, lobbyPrx, roomFactoryPrx = roomFactoryPrx](int) {
            std::cout << "Unregistering Room Factory... ";
            try {
                lobbyPrx->unregisterRoomFactory(roomFactoryPrx);
                std::cout << "unregistered!" << std::endl;
            } catch (const Chat::NoSuchRoomFactory& e) {
                std::cout << "was not registered!" << std::endl;
            }
            communicator->shutdown();
        });
        
        communicatorHolder->waitForShutdown();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}