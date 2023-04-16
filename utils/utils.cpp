//
// Created by Jakub Bednarski on 27/03/2023.
//

#include "utils.h"

int utils::getRandomUnusedPort() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket\n" << std::endl;
        return -1;
    }

    struct sockaddr_in actual_addr;
    socklen_t len = sizeof(actual_addr);
    getsockname(sockfd, (struct sockaddr*)&actual_addr, &len);
    int port = ntohs(actual_addr.sin_port);

    close(sockfd);

    return port;
}

std::shared_ptr<Chat::LobbyPrx> utils::getLobbyPrx(const Ice::CommunicatorHolder& communicatorHolder) {
    auto basePrx = communicatorHolder->stringToProxy("Chat/Lobby:default -p 10000");
    auto lobbyPrx = Ice::checkedCast<Chat::LobbyPrx>(basePrx);

    if (!lobbyPrx) {
        throw std::runtime_error("Invalid proxy!");
    }
    
    return lobbyPrx;
}

double utils::getProcessorUsage() {
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count) != KERN_SUCCESS) {
        std::cerr << "Failed to get host statistics" << std::endl;
        return 1;
    }

    uint64_t total = cpuinfo.cpu_ticks[CPU_STATE_USER] + cpuinfo.cpu_ticks[CPU_STATE_SYSTEM] + cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    return 100.0 * (cpuinfo.cpu_ticks[CPU_STATE_USER] + cpuinfo.cpu_ticks[CPU_STATE_SYSTEM]) / total;
}

std::string utils::getToken() {
    const std::string charSet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()";

    std::srand(std::time(nullptr));

    std::string token;
    for (int i = 0; i < 64; ++i) {
        token += charSet[std::rand() % charSet.size()];
    }

    return token;
}

std::tuple<std::string, std::string, std::string> utils::tokenizePrompt(const std::string& input) {
    std::tuple<std::string, std::string, std::string> result;
    std::string::size_type first_space_pos = input.find(' ');
    std::string::size_type second_space_pos = input.find(' ', first_space_pos+1);

    std::get<0>(result) = input.substr(0, first_space_pos);
    std::get<1>(result) = first_space_pos != std::string::npos ?
                          input.substr(first_space_pos+1, second_space_pos - first_space_pos - 1) : "";
    std::get<2>(result) = second_space_pos != std::string::npos ?
                          input.substr(second_space_pos+1) : "";

    return result;
}