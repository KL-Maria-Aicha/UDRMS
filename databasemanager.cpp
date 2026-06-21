#include "databasemanager.h"
#include <QStandardPaths>
#include <QDir>

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appData);
    photoDir = appData + "/photos";
    QDir().mkpath(photoDir);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(appData + "/udrms.db");
}

DatabaseManager* DatabaseManager::getInstance() {
    if (!instance) instance = new DatabaseManager();
    return instance;
}

bool DatabaseManager::initDatabase() {
    if (!db.open()) {
        qCritical() << "DB open failed:" << db.lastError().text();
        return false;
    }
    QSqlQuery q;
    q.exec("PRAGMA foreign_keys = ON");
    q.exec("PRAGMA journal_mode = WAL");

    QStringList ddl = {
        // Users — single auth table for ALL roles including students
        "CREATE TABLE IF NOT EXISTS users ("
        " id              INTEGER PRIMARY KEY AUTOINCREMENT,"
        " username        TEXT    NOT NULL UNIQUE,"
        " password_hash   TEXT    NOT NULL,"
        " role            TEXT    NOT NULL DEFAULT 'staff',"
        " is_active       INTEGER NOT NULL DEFAULT 1,"
        " student_id      INTEGER REFERENCES students(id) ON DELETE CASCADE)",

        // Students
        "CREATE TABLE IF NOT EXISTS students ("
        " id            INTEGER PRIMARY KEY,"
        " full_name     TEXT    NOT NULL,"
        " academic_year INTEGER NOT NULL,"
        " gender        TEXT    NOT NULL,"
        " created_at    TEXT    DEFAULT (datetime('now')))",

        // Dormitories — split code (A,B…) from display_name
        "CREATE TABLE IF NOT EXISTS dormitories ("
        " id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        " code          TEXT    NOT NULL UNIQUE,"
        " display_name  TEXT    NOT NULL,"
        " address       TEXT,"
        " total_capacity INTEGER NOT NULL DEFAULT 0,"
        " restaurant_name TEXT  NOT NULL DEFAULT 'Cafeteria')",

        // Rooms — include floor for display ID
        "CREATE TABLE IF NOT EXISTS rooms ("
        " id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        " dormitory_id  INTEGER NOT NULL REFERENCES dormitories(id) ON DELETE CASCADE,"
        " room_number   INTEGER NOT NULL,"
        " floor         INTEGER NOT NULL DEFAULT 1,"
        " capacity      INTEGER NOT NULL,"
        " room_type     TEXT    DEFAULT 'Standard',"
        " UNIQUE(dormitory_id, floor, room_number))",

        // Assignments
        "CREATE TABLE IF NOT EXISTS dorm_assignments ("
        " id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        " student_id  INTEGER NOT NULL REFERENCES students(id) ON DELETE CASCADE,"
        " room_id     INTEGER NOT NULL REFERENCES rooms(id) ON DELETE CASCADE,"
        " assigned_at TEXT    DEFAULT (datetime('now')),"
        " vacated_at  TEXT)",

        // Menu items (no price — meals are subsidised)
        "CREATE TABLE IF NOT EXISTS menu_items ("
        " id           INTEGER PRIMARY KEY AUTOINCREMENT,"
        " dormitory_id INTEGER NOT NULL REFERENCES dormitories(id) ON DELETE CASCADE,"
        " name         TEXT    NOT NULL,"
        " meal_type    TEXT    NOT NULL,"
        " photo_path   TEXT    DEFAULT '',"
        " is_available INTEGER NOT NULL DEFAULT 1)",

        // Meal reservations — one per student per meal-type per date
        "CREATE TABLE IF NOT EXISTS meal_reservations ("
        " id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        " student_id    INTEGER NOT NULL REFERENCES students(id) ON DELETE CASCADE,"
        " menu_item_id  INTEGER NOT NULL REFERENCES menu_items(id) ON DELETE CASCADE,"
        " reserved_date TEXT    NOT NULL,"
        " created_at    TEXT    DEFAULT (datetime('now')),"
        " UNIQUE(student_id, menu_item_id, reserved_date))"
    };

    for (const QString& sql : ddl) {
        if (!q.exec(sql)) {
            qCritical() << "DDL error:" << q.lastError().text() << "\n" << sql;
            return false;
        }
    }

    // Seed default admin (only if no users exist at all)
    q.exec("SELECT COUNT(*) FROM users");
    if (q.next() && q.value(0).toInt() == 0) {
        QString hash = User::hashPassword("Admin@2025");
        q.prepare("INSERT INTO users(username,password_hash,role) VALUES(:u,:h,:r)");
        q.bindValue(":u", "admin");
        q.bindValue(":h", hash);
        q.bindValue(":r", "admin");
        q.exec();
    }

    // Seed demo dormitory if none
    q.exec("SELECT COUNT(*) FROM dormitories");
    if (q.next() && q.value(0).toInt() == 0) {
        q.exec("INSERT INTO dormitories(code,display_name,address,total_capacity,restaurant_name)"
               " VALUES('A','Résidence Universitaire A','ENSIA Campus, Sidi Abdellah',120,'Cafétéria A')");
    }
    return true;
}

// ─── Auth ──────────────────────────────────────────────────────────────────

User* DatabaseManager::authenticateUser(const QString& u, const QString& p) {
    QString hash = User::hashPassword(p);
    QSqlQuery q;
    q.prepare("SELECT id,username,password_hash,role,is_active,COALESCE(student_id,0) "
              "FROM users WHERE username=:u AND password_hash=:h AND is_active=1");
    q.bindValue(":u", u);
    q.bindValue(":h", hash);
    if (!q.exec() || !q.next()) return nullptr;
    return new User(q.value(0).toInt(), q.value(1).toString(),
                    q.value(2).toString(), q.value(3).toString(),
                    q.value(4).toBool(),  q.value(5).toInt());
}

bool DatabaseManager::addUser(const QString& u, const QString& p,
                              const QString& r, int studentId) {
    if (!User::isPasswordValid(p)) return false;
    QSqlQuery q;
    q.prepare("INSERT INTO users(username,password_hash,role,student_id)"
              " VALUES(:u,:h,:r,:sid)");
    q.bindValue(":u",   u);
    q.bindValue(":h",   User::hashPassword(p));
    q.bindValue(":r",   r);
    q.bindValue(":sid", studentId > 0 ? QVariant(studentId) : QVariant(QVariant::Int));
    return q.exec();
}

QVector<QStringList> DatabaseManager::getUsers() {
    QVector<QStringList> rows;
    QSqlQuery q("SELECT id,username,role,is_active,COALESCE(student_id,0)"
                " FROM users ORDER BY role,username");
    while (q.next())
        rows.append({q.value(0).toString(), q.value(1).toString(),
                     q.value(2).toString(), q.value(3).toString(),
                     q.value(4).toString()});
    return rows;
}

bool DatabaseManager::setUserActive(int id, bool active) {
    QSqlQuery q;
    q.prepare("UPDATE users SET is_active=:a WHERE id=:id");
    q.bindValue(":a",  active ? 1 : 0);
    q.bindValue(":id", id);
    return q.exec();
}

bool DatabaseManager::changePassword(int userId, const QString& newPlain) {
    if (!User::isPasswordValid(newPlain)) return false;
    QSqlQuery q;
    q.prepare("UPDATE users SET password_hash=:h WHERE id=:id");
    q.bindValue(":h",  User::hashPassword(newPlain));
    q.bindValue(":id", userId);
    return q.exec();
}

// ─── Students ──────────────────────────────────────────────────────────────

bool DatabaseManager::addStudent(const Student& s,
                                 const QString& username,
                                 const QString& plainPwd) {
    if (!User::isPasswordValid(plainPwd)) return false;
    db.transaction();
    QSqlQuery q;

    // Insert student record
    q.prepare("INSERT INTO students(id,full_name,academic_year,gender)"
              " VALUES(:id,:name,:year,:gender)");
    q.bindValue(":id",     s.getId());
    q.bindValue(":name",   s.getName());
    q.bindValue(":year",   s.getAcademicYear());
    q.bindValue(":gender", s.getGender());
    if (!q.exec()) { db.rollback(); return false; }

    // Create student login account
    q.prepare("INSERT INTO users(username,password_hash,role,student_id)"
              " VALUES(:u,:h,'student',:sid)");
    q.bindValue(":u",   username);
    q.bindValue(":h",   User::hashPassword(plainPwd));
    q.bindValue(":sid", s.getId());
    if (!q.exec()) { db.rollback(); return false; }

    db.commit();
    return true;
}

bool DatabaseManager::updateStudent(const Student& s) {
    QSqlQuery q;
    q.prepare("UPDATE students SET full_name=:name,academic_year=:year,"
              "gender=:gender WHERE id=:id");
    q.bindValue(":name",   s.getName());
    q.bindValue(":year",   s.getAcademicYear());
    q.bindValue(":gender", s.getGender());
    q.bindValue(":id",     s.getId());
    return q.exec();
}

bool DatabaseManager::deleteStudent(int id) {
    QSqlQuery q;
    q.prepare("DELETE FROM students WHERE id=:id");
    q.bindValue(":id", id);
    return q.exec();
}

QVector<Student> DatabaseManager::getStudents(const QString& filter) {
    QVector<Student> result;
    QSqlQuery q;
    QString sql = "SELECT id,full_name,academic_year,gender FROM students";
    if (!filter.isEmpty())
        sql += " WHERE full_name LIKE :f OR CAST(id AS TEXT) LIKE :f";
    sql += " ORDER BY full_name";
    q.prepare(sql);
    if (!filter.isEmpty()) q.bindValue(":f", "%" + filter + "%");
    if (!q.exec()) return result;
    while (q.next())
        result.append(Student(q.value(0).toInt(), q.value(1).toString(),
                              18, q.value(2).toInt(), q.value(3).toString()));
    return result;
}

bool DatabaseManager::studentHasRoom(int studentId) {
    QSqlQuery q;
    q.prepare("SELECT id FROM dorm_assignments"
              " WHERE student_id=:sid AND vacated_at IS NULL");
    q.bindValue(":sid", studentId);
    return q.exec() && q.next();
}

// ─── Dormitories ───────────────────────────────────────────────────────────

bool DatabaseManager::addDormitory(const QString& code,
                                   const QString& displayName,
                                   const QString& addr, int cap,
                                   const QString& restName) {
    QSqlQuery q;
    q.prepare("INSERT INTO dormitories(code,display_name,address,total_capacity,restaurant_name)"
              " VALUES(:c,:d,:a,:cap,:r)");
    q.bindValue(":c",   code.toUpper());
    q.bindValue(":d",   displayName);
    q.bindValue(":a",   addr);
    q.bindValue(":cap", cap);
    q.bindValue(":r",   restName);
    return q.exec();
}

bool DatabaseManager::updateDormitory(int id, const QString& code,
                                      const QString& displayName,
                                      const QString& addr,
                                      const QString& restName) {
    QSqlQuery q;
    q.prepare("UPDATE dormitories SET code=:c,display_name=:d,"
              "address=:a,restaurant_name=:r WHERE id=:id");
    q.bindValue(":c",   code.toUpper());
    q.bindValue(":d",   displayName);
    q.bindValue(":a",   addr);
    q.bindValue(":r",   restName);
    q.bindValue(":id",  id);
    return q.exec();
}

QVector<Dormitory> DatabaseManager::getDormitories() {
    QVector<Dormitory> result;
    QSqlQuery q("SELECT id,code,display_name,address,restaurant_name"
                " FROM dormitories ORDER BY code");
    while (q.next())
        result.append(Dormitory(q.value(0).toInt(), q.value(1).toString(),
                                q.value(2).toString(), q.value(3).toString(),
                                q.value(4).toString()));
    return result;
}

// ─── Rooms ─────────────────────────────────────────────────────────────────

bool DatabaseManager::addRoom(int dormId, int number, int floor,
                              int cap, const QString& type) {
    QSqlQuery q;
    q.prepare("INSERT INTO rooms(dormitory_id,room_number,floor,capacity,room_type)"
              " VALUES(:did,:num,:floor,:cap,:type)");
    q.bindValue(":did",   dormId);
    q.bindValue(":num",   number);
    q.bindValue(":floor", floor);
    q.bindValue(":cap",   cap);
    q.bindValue(":type",  type);
    return q.exec();
}

QVector<Room> DatabaseManager::getRooms(int dormId) {
    QVector<Room> result;
    QSqlQuery q;
    q.prepare("SELECT r.id,r.room_number,r.floor,r.capacity,r.room_type,"
              " COUNT(da.id) AS occ"
              " FROM rooms r"
              " LEFT JOIN dorm_assignments da"
              "   ON da.room_id=r.id AND da.vacated_at IS NULL"
              " WHERE r.dormitory_id=:did"
              " GROUP BY r.id ORDER BY r.floor,r.room_number");
    q.bindValue(":did", dormId);
    if (!q.exec()) return result;
    while (q.next()) {
        Room r(q.value(0).toInt(), q.value(1).toInt(),
               q.value(2).toInt(), q.value(3).toInt(),
               q.value(4).toString());
        r.setCurrentOccupancy(q.value(5).toInt());
        result.append(r);
    }
    return result;
}

bool DatabaseManager::deleteRoom(int roomId) {
    QSqlQuery q;
    q.prepare("DELETE FROM rooms WHERE id=:id");
    q.bindValue(":id", roomId);
    return q.exec();
}

QString DatabaseManager::getRoomDisplayId(int roomId) {
    QSqlQuery q;
    q.prepare("SELECT d.code, r.floor, r.room_number"
              " FROM rooms r JOIN dormitories d ON d.id=r.dormitory_id"
              " WHERE r.id=:rid");
    q.bindValue(":rid", roomId);
    if (!q.exec() || !q.next()) return QString::number(roomId);
    return QString("%1%2-%3")
        .arg(q.value(0).toString())
        .arg(q.value(1).toInt())
        .arg(q.value(2).toInt());
}

// ─── Assignments ───────────────────────────────────────────────────────────

bool DatabaseManager::assignStudent(int studentId, int roomId) {
    db.transaction();
    QSqlQuery q;

    // Check capacity
    q.prepare("SELECT r.capacity, COUNT(da.id) FROM rooms r"
              " LEFT JOIN dorm_assignments da"
              "   ON da.room_id=r.id AND da.vacated_at IS NULL"
              " WHERE r.id=:rid GROUP BY r.id");
    q.bindValue(":rid", roomId);
    if (!q.exec() || !q.next()) { db.rollback(); return false; }
    if (q.value(1).toInt() >= q.value(0).toInt()) { db.rollback(); return false; }

    // Check student not already assigned
    q.prepare("SELECT id FROM dorm_assignments"
              " WHERE student_id=:sid AND vacated_at IS NULL");
    q.bindValue(":sid", studentId);
    if (!q.exec()) { db.rollback(); return false; }
    if (q.next()) { db.rollback(); return false; }

    q.prepare("INSERT INTO dorm_assignments(student_id,room_id) VALUES(:sid,:rid)");
    q.bindValue(":sid", studentId);
    q.bindValue(":rid", roomId);
    bool ok = q.exec();
    if (ok) db.commit(); else db.rollback();
    return ok;
}

bool DatabaseManager::vacateStudent(int studentId) {
    QSqlQuery q;
    q.prepare("UPDATE dorm_assignments SET vacated_at=datetime('now')"
              " WHERE student_id=:sid AND vacated_at IS NULL");
    q.bindValue(":sid", studentId);
    return q.exec() && q.numRowsAffected() > 0;
}

int DatabaseManager::getStudentRoomId(int studentId) {
    QSqlQuery q;
    q.prepare("SELECT room_id FROM dorm_assignments"
              " WHERE student_id=:sid AND vacated_at IS NULL");
    q.bindValue(":sid", studentId);
    if (!q.exec() || !q.next()) return -1;
    return q.value(0).toInt();
}

QStringList DatabaseManager::getStudentRoomInfo(int studentId) {
    QSqlQuery q;
    q.prepare("SELECT d.display_name, d.code, r.floor, r.room_number,"
              " r.capacity, r.room_type, COUNT(da2.id) AS occ"
              " FROM dorm_assignments da"
              " JOIN rooms r ON r.id=da.room_id"
              " JOIN dormitories d ON d.id=r.dormitory_id"
              " LEFT JOIN dorm_assignments da2"
              "   ON da2.room_id=r.id AND da2.vacated_at IS NULL"
              " WHERE da.student_id=:sid AND da.vacated_at IS NULL"
              " GROUP BY da.id");
    q.bindValue(":sid", studentId);
    if (!q.exec() || !q.next()) return {};
    QString displayId = QString("%1%2-%3")
                            .arg(q.value(1).toString())
                            .arg(q.value(2).toInt())
                            .arg(q.value(3).toInt());
    return { q.value(0).toString(),   // dorm display name
            displayId,               // e.g. A2-3
            q.value(4).toString(),   // capacity
            q.value(5).toString(),   // type
            q.value(6).toString() }; // current occupancy
}

// ─── Menu items ────────────────────────────────────────────────────────────

bool DatabaseManager::addMenuItem(int dormId, const QString& name,
                                  const QString& type,
                                  const QString& photoPath) {
    QSqlQuery q;
    q.prepare("INSERT INTO menu_items(dormitory_id,name,meal_type,photo_path)"
              " VALUES(:did,:name,:type,:photo)");
    q.bindValue(":did",   dormId);
    q.bindValue(":name",  name);
    q.bindValue(":type",  type);
    q.bindValue(":photo", photoPath);
    return q.exec();
}

bool DatabaseManager::updateMenuItemPhoto(int itemId, const QString& photoPath) {
    QSqlQuery q;
    q.prepare("UPDATE menu_items SET photo_path=:p WHERE id=:id");
    q.bindValue(":p",  photoPath);
    q.bindValue(":id", itemId);
    return q.exec();
}

bool DatabaseManager::deleteMenuItem(int itemId) {
    QSqlQuery q;
    q.prepare("DELETE FROM menu_items WHERE id=:id");
    q.bindValue(":id", itemId);
    return q.exec();
}

QVector<MenuItem> DatabaseManager::getMenuItems(int dormId) {
    QVector<MenuItem> result;
    QSqlQuery q;
    q.prepare("SELECT id,name,meal_type,photo_path,is_available"
              " FROM menu_items WHERE dormitory_id=:did ORDER BY meal_type,name");
    q.bindValue(":did", dormId);
    if (!q.exec()) return result;
    while (q.next()) {
        MenuItem m(q.value(0).toInt(), q.value(1).toString(),
                   q.value(2).toString(), q.value(3).toString());
        m.setAvailable(q.value(4).toBool());
        result.append(m);
    }
    return result;
}

// ─── Meal reservations ─────────────────────────────────────────────────────

bool DatabaseManager::reserveMeal(int studentId, int menuItemId,
                                  const QString& date) {
    QSqlQuery q;
    // Check the student doesn't already have this meal type on this date
    q.prepare("SELECT COUNT(*) FROM meal_reservations mr"
              " JOIN menu_items mi ON mi.id=mr.menu_item_id"
              " WHERE mr.student_id=:sid AND mi.meal_type="
              "  (SELECT meal_type FROM menu_items WHERE id=:iid)"
              " AND mr.reserved_date=:date");
    q.bindValue(":sid",  studentId);
    q.bindValue(":iid",  menuItemId);
    q.bindValue(":date", date);
    if (q.exec() && q.next() && q.value(0).toInt() > 0) return false; // already reserved

    q.prepare("INSERT OR IGNORE INTO meal_reservations(student_id,menu_item_id,reserved_date)"
              " VALUES(:sid,:iid,:date)");
    q.bindValue(":sid",  studentId);
    q.bindValue(":iid",  menuItemId);
    q.bindValue(":date", date);
    return q.exec() && q.numRowsAffected() > 0;
}

bool DatabaseManager::cancelReservation(int studentId, int menuItemId,
                                        const QString& date) {
    QSqlQuery q;
    q.prepare("DELETE FROM meal_reservations"
              " WHERE student_id=:sid AND menu_item_id=:iid AND reserved_date=:date");
    q.bindValue(":sid",  studentId);
    q.bindValue(":iid",  menuItemId);
    q.bindValue(":date", date);
    return q.exec() && q.numRowsAffected() > 0;
}

bool DatabaseManager::hasReservation(int studentId,
                                     const QString& mealType,
                                     const QString& date) {
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM meal_reservations mr"
              " JOIN menu_items mi ON mi.id=mr.menu_item_id"
              " WHERE mr.student_id=:sid AND mi.meal_type=:type"
              " AND mr.reserved_date=:date");
    q.bindValue(":sid",  studentId);
    q.bindValue(":type", mealType);
    q.bindValue(":date", date);
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

QVector<QStringList> DatabaseManager::getReservationsForDate(int dormId,
                                                             const QString& date) {
    QVector<QStringList> rows;
    QSqlQuery q;
    q.prepare("SELECT s.full_name, s.id, mi.name, mi.meal_type, mr.created_at"
              " FROM meal_reservations mr"
              " JOIN students s ON s.id=mr.student_id"
              " JOIN menu_items mi ON mi.id=mr.menu_item_id"
              " WHERE mr.reserved_date=:date AND mi.dormitory_id=:did"
              " ORDER BY mi.meal_type, s.full_name");
    q.bindValue(":date", date);
    q.bindValue(":did",  dormId);
    if (!q.exec()) return rows;
    while (q.next())
        rows.append({q.value(0).toString(), q.value(1).toString(),
                     q.value(2).toString(), q.value(3).toString(),
                     q.value(4).toString()});
    return rows;
}

QVector<QStringList> DatabaseManager::getStudentReservations(int studentId) {
    QVector<QStringList> rows;
    QSqlQuery q;
    q.prepare("SELECT mr.reserved_date, mi.meal_type, mi.name"
              " FROM meal_reservations mr"
              " JOIN menu_items mi ON mi.id=mr.menu_item_id"
              " WHERE mr.student_id=:sid"
              " ORDER BY mr.reserved_date DESC, mi.meal_type");
    q.bindValue(":sid", studentId);
    if (!q.exec()) return rows;
    while (q.next())
        rows.append({q.value(0).toString(), q.value(1).toString(),
                     q.value(2).toString()});
    return rows;
}

// ─── Reports ───────────────────────────────────────────────────────────────

QVector<QStringList> DatabaseManager::getOccupancyReport() {
    QVector<QStringList> rows;
    QSqlQuery q("SELECT d.display_name, d.total_capacity,"
                " COUNT(DISTINCT da.student_id) AS occ"
                " FROM dormitories d"
                " LEFT JOIN rooms r ON r.dormitory_id=d.id"
                " LEFT JOIN dorm_assignments da"
                "   ON da.room_id=r.id AND da.vacated_at IS NULL"
                " GROUP BY d.id ORDER BY d.code");
    while (q.next()) {
        int cap = q.value(1).toInt(), occ = q.value(2).toInt();
        double pct = cap > 0 ? 100.0 * occ / cap : 0.0;
        rows.append({q.value(0).toString(),
                     QString::number(cap),
                     QString::number(occ),
                     QString::number(cap - occ),
                     QString::number(pct, 'f', 1) + "%"});
    }
    return rows;
}

QVector<QStringList> DatabaseManager::getReservationReport(const QString& from,
                                                           const QString& to) {
    QVector<QStringList> rows;
    QSqlQuery q;
    q.prepare("SELECT reserved_date,"
              " SUM(CASE WHEN mi.meal_type='breakfast' THEN 1 ELSE 0 END),"
              " SUM(CASE WHEN mi.meal_type='lunch'     THEN 1 ELSE 0 END),"
              " SUM(CASE WHEN mi.meal_type='dinner'    THEN 1 ELSE 0 END),"
              " COUNT(*)"
              " FROM meal_reservations mr"
              " JOIN menu_items mi ON mi.id=mr.menu_item_id"
              " WHERE reserved_date BETWEEN :from AND :to"
              " GROUP BY reserved_date ORDER BY reserved_date");
    q.bindValue(":from", from);
    q.bindValue(":to",   to);
    if (!q.exec()) return rows;
    while (q.next())
        rows.append({q.value(0).toString(), q.value(1).toString(),
                     q.value(2).toString(), q.value(3).toString(),
                     q.value(4).toString()});
    return rows;
}