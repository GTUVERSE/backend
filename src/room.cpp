#include "../include/room.h"

Room::Room() : id(0), name(""), capacity(0) {}

Room::Room(int id, const std::string& name, int capacity)
    : id(id), name(name), capacity(capacity) {}

int Room::getId() const {
    return id;
}

std::string Room::getName() const {
    return name;
}

int Room::getCapacity() const {
    return capacity;
}

void Room::setName(const std::string& newName) {
    name = newName;
}

void Room::setCapacity(int newCapacity) {
    capacity = newCapacity;
}
