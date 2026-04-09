pragma Singleton
import QtQml

QtObject {
    readonly property QtObject spawn: QtObject {
        // Координаты поля спавна
        readonly property int y0: 0
        readonly property int y1: 100
        readonly property int x0: 0
        // Разброс времени спавна
        readonly property int minTms: 100
        readonly property int maxTms: 1000
        // Ограничение количества спавна
        readonly property int limit: 0
    }

    readonly property QtObject block: QtObject {
        readonly property int size: 40
        readonly property real velocityMin: 0.4
        readonly property real velocityMax: 0.8
        readonly property real boost: 2.0
    }

    enum GameState {
        Running,
        GameOver
    }
}