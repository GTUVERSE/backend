#include "../include/room.h"

Room::Room() : id(0), name(""), size(0) {}

Room::Room(int id, const std::string& name, int size)
    : id(id), name(name), size(size) {}

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

void Room::setSize(int newSize) {
    size = newSize;
}




int Room::getSize() const {
    return size;
}

bool Room::increaseSize() {
    // Kapasite kontrolü
    if (size < capacity) {
        size++;
        return true; // Size başarıyla arttı
    }
    return false; // Oda dolu, kullanıcı eklenemedi
}

bool Room::decreaseSize() {
    // Oda boş değilse çıkış işlemi yap
    if (size > 0) {
        size--;
        return true; // Size başarıyla azaltıldı
    }
    return false; // Odada zaten kimse yok
}