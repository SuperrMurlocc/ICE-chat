#ifndef __chatI_h__
#define __chatI_h__

#include <algorithm>
#include <unordered_map>

#include "chat.h"
#include "../../utils/utils.h"

namespace Chat {

    class LobbyI : public virtual Lobby {
    public:

        virtual void _cpp_register(::std::shared_ptr <UserPrx>, ::std::string, const Ice::Current &) override;

        virtual ::std::string login(::std::shared_ptr <UserPrx>, ::std::string, const Ice::Current &) override;

        virtual void logout(const Ice::Current &) override;

        virtual Rooms getRooms(const Ice::Current &) override;

        virtual Users getUsers(const Ice::Current &) override;

        virtual ::std::shared_ptr <RoomPrx> createRoom(::std::string, const Ice::Current &) override;

        virtual ::std::shared_ptr <RoomPrx> findRoom(::std::string, const Ice::Current &) override;

        virtual void registerRoomFactory(::std::shared_ptr <RoomFactoryPrx>, const Ice::Current &) override;

        virtual void unregisterRoomFactory(::std::shared_ptr <RoomFactoryPrx>, const Ice::Current &) override;

    private:
        std::pair <std::string, std::shared_ptr<Chat::UserPrx>> getUserPrxFromToken(const Ice::Current &current);

        std::vector <std::shared_ptr<RoomFactoryPrx>> roomFactories;
        std::unordered_map <std::string, std::pair<std::shared_ptr < RoomPrx>, std::shared_ptr<RoomFactoryPrx>>> rooms;
        std::unordered_map <std::string, std::shared_ptr<UserPrx>> userTokens;
        std::unordered_map <std::string, std::string> usersDatabase;
    };

    class UserI : public virtual User {
    public:
        std::string name;
        UserStatus userStatus;

        virtual ::std::string getName(const Ice::Current &) override;

        virtual UserStatus getStatus(const Ice::Current &) override;

        virtual void
        receivePrivateMessageAsync(::std::shared_ptr <UserPrx>, ::std::string, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current &) override;

        virtual void
        receiveMessageAsync(::std::shared_ptr <RoomPrx>, ::std::shared_ptr <UserPrx>, ::std::string, std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current &) override;
    };

    class RoomI : public virtual Room {
    public:
        std::string name;

        virtual ::std::string getName(const Ice::Current &) override;

        virtual Users getUsers(const Ice::Current &) override;

        virtual void join(::std::shared_ptr <UserPrx>, const Ice::Current &) override;

        virtual void leave(::std::shared_ptr <UserPrx>, const Ice::Current &) override;

        virtual void sendMessage(::std::shared_ptr <UserPrx>, ::std::string, const Ice::Current &) override;

    private:
        std::vector <std::shared_ptr<UserPrx>> users;
    };

    class RoomFactoryI : public virtual RoomFactory {
    public:
        ::std::shared_ptr <Ice::ObjectAdapter> objectAdapter;

        virtual double getServerLoad(const Ice::Current &) override;

        virtual ::std::shared_ptr <RoomPrx> createRoom(::std::string, const Ice::Current &) override;
    };

}

#endif
