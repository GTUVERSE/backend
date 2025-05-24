#include "../include/room_user_service.h"
#include "../include/room_service.h"
#include <iostream>
#include <mysqlx/xdevapi.h>
using namespace mysqlx;

RoomUserService::RoomUserService()
  : dbSession(
      // URI formatında: kullanıcı:şifre@host:port ve SSL kapalı
      "mysqlx://remote:funda123@173.212.195.170:33060"
      "?ssl-mode=DISABLED"
    ),
    dbSchema(dbSession.getSchema("gtuverse_db")),
    roomUsersTable(dbSchema.getTable("room_users"))
{}



      void RoomUserService::addUserToRoom(int userId, int roomId) {
        // Kullanıcı zaten odada mı kontrol et
        if (isUserInRoom(roomId, userId)) {
            return; // Kullanıcı zaten odada, işlem yapma
        }
        
        // Odayı getir ve kapasite kontrolü yap
        auto roomService = RoomService();
        auto roomOpt = roomService.getRoomById(roomId);
        
        if (!roomOpt.has_value()) {
            throw std::runtime_error("Room does not exist");
        }
        
        Room room = roomOpt.value();
        
        // Kapasite kontrolü ve oda güncelleme
        if (room.increaseSize()) {
            roomUsersTable.insert("room_id", "user_id")
                .values(roomId, userId)
                .execute();
                
            // Oda boyutunu güncelle
            roomService.updateRoomSize(roomId, room.getSize());
        } else {
            throw std::runtime_error("Room is full");
        }
    }




std::vector<int> RoomUserService::getUsersInRoom(int roomId) {
    std::vector<int> userIds;
    auto result = roomUsersTable.select("user_id")
                                .where("room_id = :rid")
                                .bind("rid", roomId)
                                .execute();
    for (auto row : result) {
        userIds.push_back(row[0].get<int>());
    }
    return userIds;
}

std::vector<std::pair<int, std::string>> RoomUserService::getUsersWithNamesInRoom(int roomId) {
    std::vector<std::pair<int, std::string>> users;

    std::string query =
    "SELECT users.id, users.username "
    "FROM gtuverse_db.room_users "
    "JOIN gtuverse_db.users ON room_users.user_id = users.id "
    "WHERE room_users.room_id = ?";

    auto stmt = dbSession.sql(query);
    stmt.bind(roomId);
    auto result = stmt.execute();

    for (auto row : result) {
        int id = row[0].get<int>();
        std::string username = row[1].get<std::string>();
        users.emplace_back(id, username);
    }

    return users;
}


bool RoomUserService::removeUserFromRoom(int roomId, int userId) {
    try {
        // Kullanıcı odada mı kontrol et
        if (!isUserInRoom(roomId, userId)) {
            return false; // Kullanıcı odada değilse işlem yapma
        }
        
        // Odayı getir
        auto roomService = RoomService();
        auto roomOpt = roomService.getRoomById(roomId);
        
        if (!roomOpt.has_value()) {
            return false;
        }
        
        Room room = roomOpt.value();
        room.decreaseSize();
       // Kullanıcıyı odadan çıkar
       auto result = roomUsersTable.remove()
       .where("room_id = :roomId AND user_id = :userId")
       .bind("roomId", roomId)
       .bind("userId", userId)
       .execute();
       
   // Oda boyutunu güncelle
   roomService.updateRoomSize(roomId, room.getSize());
   
   return result.getAffectedItemsCount() > 0;
} catch (const std::exception& e) {
   std::cerr << "Error removing user from room: " << e.what() << std::endl;
   return false;
}
} 


std::vector<Room> RoomUserService::getRoomsForUser(int userId) {
    std::vector<Room> rooms;

    std::string query =
        "SELECT rooms.id, rooms.name, rooms.size "
        "FROM gtuverse_db.room_users "
        "JOIN gtuverse_db.rooms ON room_users.room_id = rooms.id "
        "WHERE room_users.user_id = ?";

    auto stmt = dbSession.sql(query);
    stmt.bind(userId);
    auto result = stmt.execute();

    for (auto row : result) {
        int id = row[0].get<int>();
        std::string name = row[1].get<std::string>();
        int size = row[2].get<int>();
        rooms.emplace_back(id, name, size);
    }

    return rooms;
}


bool RoomUserService::isUserInRoom(int roomId, int userId) {
    try {
        auto result = roomUsersTable.select("room_id")
            .where("room_id = :rid AND user_id = :uid")
            .bind("rid", roomId)
            .bind("uid", userId)
            .execute();

        std::cerr << "isUserInRoom(" << roomId << ", " << userId << ") → count = " << result.count() << std::endl;

        return result.count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error in isUserInRoom: " << e.what() << std::endl;
        return false;
    }
}


