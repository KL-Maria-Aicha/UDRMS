#pragma once
#include <QString>

// Algerian university model: meals are subsidised, no individual pricing.
// Students reserve a slot (breakfast/lunch/dinner) per day.
class MenuItem {
private:
    int     id;
    QString name;
    QString mealType;   // "breakfast" | "lunch" | "dinner"
    QString photoPath;  // path inside AppDataLocation/photos/ ; empty = no photo
    bool    available;

public:
    MenuItem()
        : id(0), mealType("lunch"), available(true) {}

    MenuItem(int id, const QString& name,
             const QString& mealType,
             const QString& photoPath = "")
        : id(id), name(name), mealType(mealType),
          photoPath(photoPath), available(true) {}

    int     getId()        const { return id; }
    QString getName()      const { return name; }
    QString getMealType()  const { return mealType; }
    QString getPhotoPath() const { return photoPath; }
    bool    isAvailable()  const { return available; }

    void setName(const QString& n)      { if (!n.isEmpty()) name = n; }
    void setPhotoPath(const QString& p) { photoPath = p; }
    void setAvailable(bool a)           { available = a; }
    void setMealType(const QString& t)  { mealType = t; }
};