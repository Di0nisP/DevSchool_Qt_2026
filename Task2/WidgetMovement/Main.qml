import QtQuick

Window {
    id: mainWindow

    width: 640
    height: 480
    visible: true
    title: qsTr("WidgetMovement")

    Component.onCompleted: {
        console.log(qsTr("Game is ready. Enjoy!"))
    }

    // Контейнер сцены
    Item {
        id: gameCentral
        anchors.fill: parent // Принимает размер parent

        property var blocks: [] // Список игровых блоков
        property int gameState: Constants.GameState.Running

        // Загрузчик фона
        Loader {
            id: backgroundLoader
            anchors.fill: parent
            z: -1

            sourceComponent: gameCentral.gameState === Constants.GameState.GameOver
                ? loseBackgroundComponent
                : skyBackgroundComponent
        }

        // Настройка фона "как небо"
        Component {
            id: skyBackgroundComponent

            Rectangle {
                anchors.fill: parent
                z: -1 // гарантированно фон

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#0b1d3a" }    // тёмно-синий верх
                    GradientStop { position: 0.3; color: "#1f4f8a" }    // холодный синий
                    GradientStop { position: 0.7; color: "#6faee8" }    // светлое небо
                    GradientStop { position: 1.0; color: "#dff3ff" }    // почти белый низ
                }
            }
        }

        // Настройка фона "проигрыш"
        Component {
            id: loseBackgroundComponent

            Rectangle {
                anchors.fill: parent
                z: -1

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#2b0000" }    // почти чёрный с красным оттенком
                    GradientStop { position: 0.3; color: "#5a0000" }    // тёмный бордовый
                    GradientStop { position: 0.7; color: "#a80000" }    // насыщенный красный
                    GradientStop { position: 1.0; color: "#ff2a2a" }    // ярко-красный низ
                }
            }
        }

        // Настройка для создания блока
        Component {
            id: blockComponent

            // Используем Item, чтобы не было квадрата вокруг текста
            Item {
                id: block

                width: Constants.block.size
                height: Constants.block.size

                Text {
                    anchors.centerIn: parent
                    text: "❄"
                    color: "white"
                    font.pixelSize: parent.width
                    font.bold: true
                    style: Text.Outline
                    styleColor: "#cfe9ff"
                    opacity: 0.95
                }

                readonly property real velocity_basic: Utils.randomRange(
                    Constants.block.velocityMin,
                    Constants.block.velocityMax
                )
                readonly property real velocity_boost: velocity_basic * Constants.block.boost

                function isLose() {
                    return block.y + block.height >= gameCentral.height
                }

                function isFall() {
                    return block.y >= gameCentral.height || block.x >= gameCentral.width
                }

                function updateY() {
                    const speed = block.focus
                                ? block.velocity_boost
                                : block.velocity_basic
                    block.y += speed
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onEntered: block.focus = true
                    onExited: block.focus = false

                    onClicked: {
                        const idx = gameCentral.blocks.indexOf(block)
                        if (idx !== -1)
                            gameCentral.blocks.splice(idx, 1)

                        block.destroy()
                    }
                }
            }
        }

        Timer {
            id: spawnTimer
            interval: Constants.spawn.minTms // небольшая задержка
            repeat: false
            running: true

            function restartWithRandomInterval() {
                spawnTimer.interval = Utils.randomRange(
                    Constants.spawn.minTms,
                    Constants.spawn.maxTms
                )
                spawnTimer.start()
            }

            function trySpawn() {
                function spawnPos() {
                    const x = Utils.randomRange(
                        Constants.spawn.x0,
                        gameCentral.width  - Constants.block.size
                    )
                    const y = Utils.randomRange(
                        Constants.spawn.y0,
                        Constants.spawn.y1 - Constants.block.size
                    )

                    return { x: x, y: y }
                }

                const blockNumber = gameCentral.blocks.length
                const spawnLimit = Constants.spawn.limit

                if (spawnLimit <= 0 || blockNumber < spawnLimit) {
                    const block = blockComponent.createObject(gameCentral, spawnPos())

                    if (block !== null) {
                        gameCentral.blocks.push(block)
                    }
                }
            }

            onTriggered: {
                spawnTimer.trySpawn()
                spawnTimer.restartWithRandomInterval()
            }
        }

        // В отличие от Timer не зависает при изменении размеров Window
        FrameAnimation  {
            id: updateLoop
            running: true

            onTriggered: {
                for (let i = gameCentral.blocks.length - 1; i >= 0; --i) {
                    const block = gameCentral.blocks[i]

                    if (block === null) {
                        gameCentral.blocks.splice(i, 1)
                        continue
                    }

                    if (gameCentral.gameState === Constants.GameState.Running && block.isLose()) {
                        gameCentral.gameState = Constants.GameState.GameOver // активирует Loader
                        mainWindow.title = "You LOSE!"
                    }

                    if (block.isFall()) {
                        gameCentral.blocks.splice(i, 1)
                        block.destroy()
                        continue
                    }

                    block.updateY()
                }
            }
        }
    }
}
