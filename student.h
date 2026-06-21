#pragma once
#include "person.h"

class Student : public Person {
private:
    int     academicYear;
    QString gender;
public:
    Student() : Person(0,"",18), academicYear(1), gender("M") {}
    Student(int id, const QString& name, int age,
            int year, const QString& g)
        : Person(id, name, age), academicYear(year), gender(g) {}

    QString display() const override {
        return QString("Student[%1] %2 | Year %3 | %4")
            .arg(id).arg(fullName).arg(academicYear).arg(gender);
    }
    int     getAcademicYear() const { return academicYear; }
    QString getGender()       const { return gender; }
    void setAcademicYear(int y) { if(y>=1&&y<=5) academicYear=y; }
    void setGender(const QString& g) { if(g=="M"||g=="F") gender=g; }
};