#ifndef ROOM_H
#define ROOM_H

#include <string>

class Room {
private:
    int id;
    std::string name;
    int capacity;

public:
    Room();
    Room(int id, const std::string& name, int capacity);

    int getId() const;
    std::string getName() const;
    int getCapacity() const;

    void setName(const std::string& newName);
    void setCapacity(int newCapacity);
};

#endif
