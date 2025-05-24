#include "../include/room_service.h"

  RoomService::RoomService()
  : dbSession(
      // URI formatında: kullanıcı:şifre@host:port ve SSL kapalı
      "mysqlx://remote:funda123@173.212.195.170:33060"
      "?ssl-mode=DISABLED"
    ),
    dbSchema(dbSession.getSchema("gtuverse_db")),
    roomsTable(dbSchema.getTable("rooms"))
{}


      
      void RoomService::createRoom(const Room& room) {
        if (roomExistsByName(room.getName())) {
            throw std::runtime_error("Room with the same name already exists");
        }
    
        roomsTable.insert("name", "size")
                  .values(room.getName(), room.getSize())
                  .execute();
    }
    
std::optional<Room> RoomService::getRoomById(int id) const {
    auto result = roomsTable
                    .select("id", "name", "size")
                    .where("id = :id").bind("id", id)
                    .execute();

    for (auto row : result) {
        return Room(
            row[0].get<int>(),
            row[1].get<std::string>(),
            row[2].get<int>()
        );
    }
    return std::nullopt;
}

std::vector<Room> RoomService::getAllRooms() const {
    std::vector<Room> resultList;
    auto result = roomsTable.select("id", "name", "size").execute();
    for (auto row : result) {
        resultList.push_back(Room(
            row[0].get<int>(),
            row[1].get<std::string>(),
            row[2].get<int>()
        ));
    }
    return resultList;
}

bool RoomService::deleteRoom(int id) {
    auto result = roomsTable
                    .remove()
                    .where("id = :id").bind("id", id)
                    .execute();
    return result.getAffectedItemsCount() > 0;
}


bool RoomService::roomExistsByName(const std::string& name) const {
    auto result = roomsTable
                    .select("id")
                    .where("name = :name")
                    .bind("name", name)
                    .execute();
    return result.count() > 0;
}

void RoomService::updateRoomSize(int roomId, int newSize) {
    roomsTable.update()
        .set("size", newSize)
        .where("id = :id")
        .bind("id", roomId)
        .execute();
}