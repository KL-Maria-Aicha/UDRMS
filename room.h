#pragma once
#include <QString>

class Room {
private:
    int     id;
    int     roomNumber;
    int     floor;
    int     capacity;
    int     currentOccupancy;
    QString roomType;
public:
    Room()
        : id(0), roomNumber(0), floor(1), capacity(1),
          currentOccupancy(0), roomType("Standard") {}

    Room(int id, int number, int floor, int cap, const QString& type = "Standard")
        : id(id), roomNumber(number), floor(floor), capacity(cap),
          currentOccupancy(0), roomType(type) {}

    bool isFull()      const { return currentOccupancy >= capacity; }
    bool isAvailable() const { return currentOccupancy < capacity; }

    bool addStudent() {
        if (isFull()) return false;
        ++currentOccupancy; return true;
    }
    bool removeStudent() {
        if (currentOccupancy <= 0) return false;
        --currentOccupancy; return true;
    }

    QString getStatus() const {
        if (currentOccupancy == 0)       return "Available";
        if (currentOccupancy < capacity) return "Partial";
        return "Full";
    }

    int     getId()               const { return id; }
    int     getRoomNumber()       const { return roomNumber; }
    int     getFloor()            const { return floor; }
    int     getCapacity()         const { return capacity; }
    int     getCurrentOccupancy() const { return currentOccupancy; }
    int     getAvailableBeds()    const { return capacity - currentOccupancy; }
    QString getRoomType()         const { return roomType; }

    void setCurrentOccupancy(int o) {
        if (o >= 0 && o <= capacity) currentOccupancy = o;
    }
    void setId(int i) { id = i; }
};