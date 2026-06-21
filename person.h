#pragma once
#include <QString>

class Person {
protected:
    int     id;
    QString fullName;
    int     age;
public:
    Person(int id, const QString& name, int age)
        : id(id), fullName(name), age(age) {}
    virtual ~Person() = default;
    virtual QString display() const = 0;
    int     getId()   const { return id; }
    QString getName() const { return fullName; }
    int     getAge()  const { return age; }
    void setName(const QString& n) { if (!n.isEmpty()) fullName = n; }
};