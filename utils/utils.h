//
// Created by Jakub Bednarski on 27/03/2023.
//

#ifndef ICE_UTILS_H
#define ICE_UTILS_H

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <mach/mach_host.h>

#include "../ice/out/chatI.h"

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>

namespace utils {
    int getRandomUnusedPort();

    template<typename prxType, typename servantType>
    std::pair <std::shared_ptr<prxType>, std::shared_ptr<servantType>> addServantToAdapter(std::shared_ptr<Ice::ObjectAdapter> objectAdapter, std::string stringIdentity = "") {
        auto servant = std::make_shared<servantType>();
        
        std::shared_ptr<Ice::ObjectPrx> base;
        if (stringIdentity == "") {
            base = objectAdapter->addWithUUID(servant);
        } else {
            base = objectAdapter->add(servant, Ice::stringToIdentity(stringIdentity));
        }
        
        auto proxy = Ice::uncheckedCast<prxType>(base);

        return std::make_pair(proxy, servant);
    }

    std::shared_ptr<Chat::LobbyPrx> getLobbyPrx(const Ice::CommunicatorHolder& communicatorHolder);
    
    double getProcessorUsage();

    std::string getToken();

    std::tuple<std::string, std::string, std::string> tokenizePrompt(const std::string& input);
}
#endif //ICE_UTILS_H
