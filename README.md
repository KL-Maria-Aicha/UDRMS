# UDRMS — University Dormitory & Restaurant Management System

A desktop application built with Qt (C++) and SQLite for managing university dormitory rooms, student records, and a subsidised restaurant/meal-reservation system. Built as an OOP school project (ENSIA).

## Features

- Authentication: role-based login (admin, manager, staff, student)
- Student management: register students, assign/vacate dormitory rooms
- Room management: dormitories, floors, rooms, occupancy tracking
- Restaurant management: cafeteria menus (breakfast/lunch/dinner), no pricing — meals are subsidised
- Meal reservations: students reserve meals by date; staff can view daily reservations
- Reports: occupancy statistics, reservation summaries, charts
- Student portal: students log in to view their room, the menu, and manage their own meal reservations

## Tech Stack

- C++17
- Qt 5/6 (Widgets, SQL, SVG modules)
- SQLite (via Qt SQL module) for persistence

## OOP Concepts Demonstrated

- Inheritance and polymorphism: Person (abstract base) -> Student
- Encapsulation: private fields with public getters/setters across all model classes
- Composition: Dormitory owns Rooms and a Restaurant; Restaurant owns MenuItems
- Singleton pattern: DatabaseManager is the single point of access to the SQLite database
- Separation of concerns: model classes vs. data access (DatabaseManager) vs. UI (MainWindow, dialogs)

## How to Build

1. Open UDRMS.pro in Qt Creator
2. Select a kit (Desktop Qt with MinGW or MSVC)
3. Build in Release mode
4. Run

Default admin login on first launch:
- Username: admin
- Password: Admin@2025

(Student accounts are created from the Students page by the admin/staff, who set each student's username and password at registration time.)

## UML Class Diagram

See the UML diagram file included in this repository for the full class diagram, showing relationships between Person, Student, Room, Dormitory, Restaurant, MenuItem, User, and DatabaseManager.

## Demo Video

Link: PASTE_YOUR_YOUTUBE_LINK_HERE

## Author

- Name: YOUR NAME HERE
- Course: ENSIA — OOP / GUI project
