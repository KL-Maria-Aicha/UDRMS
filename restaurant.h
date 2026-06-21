#pragma once
#include <QString>
#include <QVector>
#include "menuitem.h"

class Restaurant {
private:
    QString           name;
    QVector<MenuItem> items;
public:
    explicit Restaurant(const QString& n = "Cafeteria") : name(n) {}
    void addItem(const MenuItem& item) { items.append(item); }
    bool removeItem(int id) {
        for (int i = 0; i < items.size(); ++i)
            if (items[i].getId() == id) { items.remove(i); return true; }
        return false;
    }
    QVector<MenuItem> getByType(const QString& type) const {
        QVector<MenuItem> r;
        for (const auto& m : items)
            if (m.getMealType() == type && m.isAvailable()) r.append(m);
        return r;
    }
    QVector<MenuItem> getMenu()  const { return items; }
    QString           getName()  const { return name; }
    void setName(const QString& n) { name = n; }
};