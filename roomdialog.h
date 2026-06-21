#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <tuple>

class RoomDialog : public QDialog {
    Q_OBJECT
    QSpinBox*  spinNumber;
    QSpinBox*  spinFloor;
    QSpinBox*  spinCapacity;
    QComboBox* comboType;
public:
    explicit RoomDialog(QWidget* parent = nullptr);
    // returns {roomNumber, floor, capacity, type}
    std::tuple<int,int,int,QString> getRoomData() const;
};