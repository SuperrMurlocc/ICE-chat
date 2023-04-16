#include "chatI.h"

void
Chat::LobbyI::_cpp_register(::std::shared_ptr <UserPrx> user, ::std::string password, const Ice::Current &current) {
    std::string userName = user->getName();
    if (usersDatabase.count(userName)) {
        throw Chat::UserExists();
    }

    std::cout << "[+*]" << std::endl;

    usersDatabase[userName] = password;
}

::std::string
Chat::LobbyI::login(::std::shared_ptr <UserPrx> user, ::std::string password, const Ice::Current &current) {
    std::string userName = user->getName();
    if (!usersDatabase.count(userName) || usersDatabase[userName] != password) {
        throw Chat::AccessDenied();
    }

    const auto token = utils::getToken();
    userTokens[token] = user;

    std::cout << "[+]" << std::endl;

    return token;
}

std::pair <std::string, std::shared_ptr<Chat::UserPrx>> Chat::LobbyI::getUserPrxFromToken(const Ice::Current &current) {
    const auto tokenPair = current.ctx.find("Token");
    if (tokenPair == current.ctx.end()) {
        throw Chat::AccessDenied();
    }
    const auto token = tokenPair->second;

    if (userTokens.find(token) == userTokens.end()) {
        throw Chat::AccessDenied();
    }
    const auto userPrx = userTokens[token];

    return std::make_pair(token, userPrx);
}


void Chat::LobbyI::logout(const Ice::Current &current) {
    const auto [token, userPrx] = getUserPrxFromToken(current);
    
    for (auto [roomName, roomData]: rooms) {
        auto [roomPrx, roomFactoryPrx] = roomData;
        roomPrx->leave(userPrx);
    }
    
    std::cout << "[-]" << std::endl;
}

::Chat::Rooms Chat::LobbyI::getRooms(const Ice::Current &current) {
    const auto [token, userPrx] = getUserPrxFromToken(current);

    std::vector <std::shared_ptr<Chat::RoomPrx>> roomPrxs;

    for (const auto [roomName, roomData]: rooms) {
        const auto [roomPrx, roomFactoryPrx] = roomData;

        roomPrxs.push_back(roomPrx);
    }

    return roomPrxs;
}

::Chat::Users Chat::LobbyI::getUsers(const Ice::Current &current) {
    std::vector <std::shared_ptr<Chat::UserPrx>> userPrxs;

    for (const auto [userToken, userPrx]: userTokens) {
        userPrxs.push_back(userPrx);
    }

    return userPrxs;
}

::std::shared_ptr<::Chat::RoomPrx> Chat::LobbyI::createRoom(::std::string name, const Ice::Current &current) {
    const auto [token, userPrx] = getUserPrxFromToken(current);

    if (rooms.count(name)) {
        throw Chat::RoomExists();
    }

    if (!roomFactories.size()) {
        throw std::runtime_error("No factories to create a room!");
    }

    auto lowestCapacityRoomFactory = *std::min_element(roomFactories.begin(), roomFactories.end(), [](std::shared_ptr <RoomFactoryPrx> a, std::shared_ptr <RoomFactoryPrx> b) {
        return a->getServerLoad() < b->getServerLoad();
    });
    auto roomPrx = lowestCapacityRoomFactory->createRoom(name);

    rooms[name] = std::make_pair(roomPrx, lowestCapacityRoomFactory);
    std::cout << "[r+" << name << "]" << std::endl;

    return roomPrx;
}

::std::shared_ptr<::Chat::RoomPrx> Chat::LobbyI::findRoom(::std::string name, const Ice::Current &current) {
    const auto [token, userPrx] = getUserPrxFromToken(current);

    if (!rooms.count(name)) {
        throw Chat::NoSuchRoom();
    }

    auto [roomPrx, roomFactoryPrx] = rooms[name];

    return roomPrx;
}

void Chat::LobbyI::registerRoomFactory(::std::shared_ptr <RoomFactoryPrx> roomFactory, const Ice::Current &current) {
    for (auto registeredRoomFactory: roomFactories) {
        if (Ice::targetEqualTo(registeredRoomFactory, roomFactory)) {
            throw Chat::RoomFactoryExists();
        }
    }

    std::cout << "[f+]" << std::endl;
    roomFactories.push_back(roomFactory);
}

void Chat::LobbyI::unregisterRoomFactory(::std::shared_ptr <RoomFactoryPrx> roomFactory, const Ice::Current &current) {
    for (auto registeredRoomFactory: roomFactories) {
        if (Ice::targetEqualTo(registeredRoomFactory, roomFactory)) {
            std::cout << "[f-]" << std::endl;
            roomFactories.erase(std::remove(roomFactories.begin(), roomFactories.end(), roomFactory), roomFactories.end());

            for (const auto [roomName, roomData]: rooms) {
                const auto [roomPrx, roomFactoryPrx] = roomData;

                if (Ice::targetEqualTo(roomFactoryPrx, roomFactory)) {
                    rooms.erase(roomName);
                    std::cout << "[r-" << roomName << "]" << std::endl;
                }
            }

            return;
        }
    }

    throw Chat::NoSuchRoomFactory();
}

::std::string Chat::UserI::getName(const Ice::Current &current) {
    return name;
}

::Chat::UserStatus Chat::UserI::getStatus(const Ice::Current &current) {
    return userStatus;
}

#define fgMagenta "\x1b[35m"
#define fgYellow "\x1b[33m"
#define colorClear "\x1b[0m"

void
::Chat::UserI::receivePrivateMessageAsync(::std::shared_ptr <UserPrx> fromUser, ::std::string message, std::function<void()> receivePrivateMessage_response, std::function<void(std::exception_ptr)>, const Ice::Current &current) {
    std::cout << fgMagenta << "from:" << fromUser->getName() << " " << message << colorClear << std::endl;
    receivePrivateMessage_response();
}

void
::Chat::UserI::receiveMessageAsync(::std::shared_ptr <RoomPrx> fromRoom, ::std::shared_ptr <UserPrx> fromUser, ::std::string message, std::function<void()> receiveMessage_response, std::function<void(std::exception_ptr)>, const Ice::Current &current) {
    std::cout << fgYellow << "from:" << fromRoom->getName() << "," << fromUser->getName() << " " << message << colorClear << std::endl;
    receiveMessage_response();
}

::std::string Chat::RoomI::getName(const Ice::Current &current) {
    return name;
}

::Chat::Users Chat::RoomI::getUsers(const Ice::Current &current) {
    return users;
}

void Chat::RoomI::join(::std::shared_ptr <UserPrx> user, const Ice::Current &current) {
    for (auto joinedUser: users) {
        if (Ice::targetEqualTo(joinedUser, user)) {
            return;
        }
    }

    users.push_back(user);
    std::cout << "[+"  << name << "]" << std::endl;
}

void Chat::RoomI::leave(::std::shared_ptr <UserPrx> user, const Ice::Current &current) {
    for (auto joinedUser: users) {
        if (Ice::targetEqualTo(joinedUser, user)) {
            users.erase(std::remove(users.begin(), users.end(), joinedUser), users.end());
            std::cout << "[-"  << name << "]" << std::endl;
        }
    }
}

void
Chat::RoomI::sendMessage(::std::shared_ptr <UserPrx> fromUser, ::std::string message, const Ice::Current &current) {
    for (auto joinedUser: users) {
        if (!Ice::targetEqualTo(joinedUser, fromUser)) {
            joinedUser->receiveMessageAsync(Ice::uncheckedCast<Chat::RoomPrx>(current.adapter->createProxy(current.id)), fromUser, message);
        }
    }
}

double Chat::RoomFactoryI::getServerLoad(const Ice::Current &current) {
    return utils::getProcessorUsage();
}

::std::shared_ptr<::Chat::RoomPrx> Chat::RoomFactoryI::createRoom(::std::string name, const Ice::Current &current) {
    auto [roomPrx, room] = utils::addServantToAdapter<RoomPrx, RoomI>(objectAdapter);
    room->name = name;
    return roomPrx;
}
