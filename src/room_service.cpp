#include "room_service.h"

RoomService::RoomService() {}

void RoomService::createRoom(const Room& room) {
    rooms.push_back(room);
}

std::optional<Room> RoomService::getRoomById(int id) const {
    for (const auto& room : rooms) {
        if (room.getId() == id) {
            return room;
        }
    }
    return std::nullopt;
}

std::vector<Room> RoomService::getAllRooms() const {
    return rooms;
}

bool RoomService::deleteRoom(int id) {
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        if (it->getId() == id) {
            rooms.erase(it);
            return true;
        }
    }
    return false;
}
