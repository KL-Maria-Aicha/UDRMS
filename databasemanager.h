#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVector>
#include "student.h"
#include "dormitory.h"
#include "user.h"
#include "menuitem.h"

class DatabaseManager {
private:
    static DatabaseManager* instance;
    QSqlDatabase db;
    QString      photoDir;  // writable path for food photos
    DatabaseManager();
public:
    static DatabaseManager* getInstance();
    bool   initDatabase();
    QString getPhotoDir() const { return photoDir; }

    // Auth
    User*                authenticateUser(const QString& u, const QString& p);
    bool                 addUser(const QString& u, const QString& p,
                                 const QString& r, int studentId = 0);
    QVector<QStringList> getUsers();
    bool                 setUserActive(int id, bool active);
    bool                 changePassword(int userId, const QString& newPlain);

    // Students
    bool             addStudent(const Student& s,
                                const QString& username, const QString& plainPwd);
    bool             updateStudent(const Student& s);
    bool             deleteStudent(int id);
    QVector<Student> getStudents(const QString& filter = "");
    bool             studentHasRoom(int studentId);

    // Dormitories
    bool               addDormitory(const QString& code, const QString& displayName,
                                    const QString& addr, int cap,
                                    const QString& restName);
    bool               updateDormitory(int id, const QString& code,
                                       const QString& displayName,
                                       const QString& addr, const QString& restName);
    QVector<Dormitory> getDormitories();

    // Rooms  — roomId format displayed as {dormCode}{floor}-{roomNumber}
    bool          addRoom(int dormId, int number, int floor,
                          int cap, const QString& type);
    bool          deleteRoom(int roomId);
    QVector<Room> getRooms(int dormId);
    QString       getRoomDisplayId(int roomId);  // returns e.g. "B3-2"

    // Assignments
    bool assignStudent(int studentId, int roomId);
    bool vacateStudent(int studentId);
    int  getStudentRoomId(int studentId);
    // Returns display info for a student's current assignment: {dormName, roomDisplayId}
    QStringList getStudentRoomInfo(int studentId);

    // Menu items (no price)
    bool              addMenuItem(int dormId, const QString& name,
                                  const QString& type, const QString& photoPath);
    bool              updateMenuItemPhoto(int itemId, const QString& photoPath);
    bool              deleteMenuItem(int itemId);
    QVector<MenuItem> getMenuItems(int dormId);

    // Meal reservations (one per student per meal-type per date)
    bool                 reserveMeal(int studentId, int menuItemId,
                                     const QString& date);   // date = "YYYY-MM-DD"
    bool                 cancelReservation(int studentId, int menuItemId,
                                           const QString& date);
    bool                 hasReservation(int studentId,
                                        const QString& mealType,
                                        const QString& date);
    QVector<QStringList> getReservationsForDate(int dormId, const QString& date);
    QVector<QStringList> getStudentReservations(int studentId);

    // Reports
    QVector<QStringList> getOccupancyReport();
    QVector<QStringList> getReservationReport(const QString& from, const QString& to);
};