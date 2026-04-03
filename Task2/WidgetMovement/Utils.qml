pragma Singleton
import QtQml

QtObject {
    function randomRange(a, b) {
        const min = Math.min(a, b)
        const max = Math.max(a, b)
        return min + Math.random() * (max - min)
    }
}
