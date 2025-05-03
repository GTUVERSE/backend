#ifndef ROOM_SERVICE_H
#define ROOM_SERVICE_H

#include "room.h"
#include <vector>
#include <optional>

class RoomService {
public:
    RoomService();

    void createRoom(const Room& room);
    std::optional<Room> getRoomById(int id) const;
    std::vector<Room> getAllRooms() const;
    bool deleteRoom(int id);

private:
    std::vector<Room> rooms;
};

#endif // ROOM_SERVICE_H
