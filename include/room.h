#ifndef ROOM_H
#define ROOM_H

#include <string>

class Room {
private:
    int id;
    std::string name;
    int size;
    const int capacity = 8;

public:
    Room();
    Room(int id, const std::string& name, int size);

    int getId() const;
    std::string getName() const;
    int getCapacity() const;

    void setName(const std::string& newName);
    void setSize(int newSize);
    int getSize() const;
     // Yeni metotlar
     bool increaseSize(); // Odaya kullanıcı eklendiğinde size arttır
     bool decreaseSize(); // Odadan kullanıcı çıktığında size azalt
};

#endif
