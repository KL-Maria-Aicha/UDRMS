#pragma once
#include <QString>
#include <QCryptographicHash>

// Roles: "admin", "manager", "staff", "student"
class User {
private:
    int     id;
    QString username;
    QString passwordHash;
    QString role;
    bool    isActiveFlag;
    int     studentId;   // 0 if not a student account

public:
    User() : id(0), isActiveFlag(true), studentId(0) {}

    User(int id, const QString& username, const QString& passwordHash,
         const QString& role, bool isActive, int studentId = 0)
        : id(id), username(username), passwordHash(passwordHash),
          role(role), isActiveFlag(isActive), studentId(studentId) {}

    static QString hashPassword(const QString& plain) {
        return QString(QCryptographicHash::hash(
            plain.toUtf8(), QCryptographicHash::Sha256).toHex());
    }

    // Password policy: minimum 8 characters
    static bool isPasswordValid(const QString& plain) {
        return plain.length() >= 8;
    }

    int     getId()           const { return id; }
    QString getUsername()     const { return username; }
    QString getPasswordHash() const { return passwordHash; }
    QString getRole()         const { return role; }
    bool    getIsActive()     const { return isActiveFlag; }
    int     getStudentId()    const { return studentId; }

    bool isAdmin()   const { return role == "admin"; }
    bool isManager() const { return role == "manager"; }
    bool isStaff()   const { return role == "staff" || role == "manager" || role == "admin"; }
    bool isStudent() const { return role == "student"; }

    void setStudentId(int sid) { studentId = sid; }
};