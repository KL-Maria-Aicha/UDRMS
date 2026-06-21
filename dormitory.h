#pragma once
#include <QString>
#include <QVector>
#include "room.h"
#include "restaurant.h"

class Dormitory {
private:
    int     id;
    QString code;         // single-letter/short code used in room IDs, e.g. "A"
    QString displayName;  // friendly name, e.g. "Résidence Universitaire A"
    QString address;
    QVector<Room> rooms;
    Restaurant    restaurant;
public:
    Dormitory() : id(0), restaurant("Cafeteria") {}

    Dormitory(int id, const QString& code, const QString& displayName,
              const QString& addr, const QString& restName)
        : id(id), code(code), displayName(displayName),
          address(addr), restaurant(restName) {}

    void addRoom(const Room& r) { rooms.append(r); }
    bool removeRoom(int number) {
        for (int i = 0; i < rooms.size(); ++i)
            if (rooms[i].getRoomNumber() == number) {
                if (rooms[i].getCurrentOccupancy() > 0) return false;
                rooms.remove(i); return true;
            }
        return false;
    }
    QVector<Room> getAvailableRooms() const {
        QVector<Room> a;
        for (const auto& r : rooms) if (r.isAvailable()) a.append(r);
        return a;
    }
    int getTotalCapacity() const {
        int t = 0; for (const auto& r : rooms) t += r.getCapacity(); return t;
    }
    int getCurrentOccupancy() const {
        int o = 0; for (const auto& r : rooms) o += r.getCurrentOccupancy();
        return o;
    }

    int               getId()          const { return id; }
    QString           getCode()        const { return code; }
    QString           getDisplayName() const { return displayName; }
    QString           getAddress()     const { return address; }
    QVector<Room>&    getRooms()             { return rooms; }
    Restaurant&       getRestaurant()        { return restaurant; }
    const Restaurant& getRestaurant()  const { return restaurant; }

    void setCode(const QString& c)        { code = c; }
    void setDisplayName(const QString& n) { displayName = n; }
};