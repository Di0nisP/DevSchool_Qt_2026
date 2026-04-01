#pragma once

#include <QObject>

// Использовать имена из C++ (.h) в QML можно двумя способами:
// 1. C Qt5 использовать qmlRegister*
// 2. С Qt6 использовать QML_NAMED_ELEMENT + в CMake добавить этот .h в qt_add_qml_module()

#include <QtQml/qqmlregistration.h> // QML_NAMED_ELEMENT

// Note: The names of enum values must begin with a capital letter in order to be accessible from QML.
// (https://doc.qt.io/qt-6/qtqml-cppintegration-data.html)

namespace game::spawn_settings {
Q_NAMESPACE
QML_NAMED_ELEMENT(SpawnSettings)

enum class SpawnSettings : int {
    // Координаты поля спавна
    SpawnY0 = 0,    SpawnY1 = 100,
    SpawnX0 = 0, // SpawnX1 задаётся runtime
    // Разброс времени спавна
    MinTms = 100,   MaxTms = 1000,
    // Ограничение количества спавна
    SpawnLimit = 0
};
Q_ENUM_NS(SpawnSettings)

} // namespace game::spawn_settings

namespace game::block_settings {
Q_NAMESPACE
QML_NAMED_ELEMENT(BlockSettings)

enum BlockSettings : int {
    // Размер блока
    Size = 40,
    // Скорость смещения блока
    VelocityMin_x10 = 4, VelocityMax_x10 = 8,
    Boost_x10 = 20
};
Q_ENUM_NS(BlockSettings)

} // namespace game::block_settings

namespace game::other_settings {
Q_NAMESPACE
QML_NAMED_ELEMENT(UpdateSettings)

enum class UpdateSettings : int {
    UpdateFHz = 100,
    UpdateTms = 1000 / UpdateFHz
};
Q_ENUM_NS(UpdateSettings)

} // namespace game::other_settings

namespace game {
Q_NAMESPACE
QML_NAMED_ELEMENT(GameState)

enum class GameState {
    Running,
    GameOver
};
Q_ENUM_NS(GameState)

} // namespace game
