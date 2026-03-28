#include <QTimer>
#include <QPushButton>

#include <QRandomGenerator>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "mainwindow.h"

namespace {
namespace game::spawn_settings {
// Огриничение положения и размера вертикальной компоненты области спавна
inline constexpr auto spawnY0 = 0;
inline constexpr auto spawnY1 = 100;
// Огриничение положения и размера горизонтальной компоненты области спавна
inline constexpr auto spawnX0 = 0;
// spawnX1 формируется runtime
// Диапазон времени спавна
inline constexpr auto minTms = 100;
inline constexpr auto maxTms = 1000;
// Ограничение количества спавна
// 0<= - без ограничений
inline constexpr auto spawnLimit = 0;
}
namespace game::block_settings {
// Размер активного блока
inline constexpr auto sz = 40;
// Разброс базовой скорости блока, пикселей/тик
inline constexpr auto velocityMin = 0.4f;
inline constexpr auto velocityMax = 0.8f;
// Кратность ускорения блока
inline constexpr auto boost = 2.0f;
inline constexpr auto animationDurationMs = 0;
}
namespace game::other_settings {
inline constexpr auto updateFHz = 100; // Программный тик
inline constexpr auto updateTms = 1000 / updateFHz;
}

/**
 * @brief Утилитарная функция анимированного удаления виджета
 * @param[in, out]  widget      Виджет
 * @param[in]       durationMs  Длительность анимации. По умолчанию без анимации.
 */
void animateFadeOutAndDelete(QWidget* widget, uint16_t durationMs = 0)
{
    if (!widget)
        return;

    auto fadeOutAndDelete = [widget]()
    {
        widget->hide();
        widget->deleteLater();
    };

    if (durationMs == 0) {
        fadeOutAndDelete();
        return;
    }

    // effect применяет свойство "opacity" к block
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        effect->setOpacity(1.0);
        widget->setGraphicsEffect(effect);
    }

    // animation изменяет свойство "opacity" (effect) за заданную длительность
    auto* animation = new QPropertyAnimation(effect, "opacity", widget);
    animation->setDuration(durationMs);
    animation->setStartValue(effect->opacity());
    animation->setEndValue(0.0);

    QObject::connect(animation, &QPropertyAnimation::finished, widget, fadeOutAndDelete);

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

namespace game {

using GameBlock = QPushButton;

enum class GameState {
    Running,
    GameOver
} gameState = GameState::Running;

}

namespace game::conditions {
/**
 * @brief Функция проверки валидности блока
 */
bool
isAliveBlock(const GameBlock* const block)
{
    return block && block->isEnabled();
}

/**
 * @brief Функция проверки условия проигрыша
 *
 * Условие проигрыша - выход блока за границы родительского виджета.
 */
bool
isLoseCondition(const GameBlock* const block)
{
    return block->y() + block->height() >= block->parentWidget()->height();
}

/**
 * @brief Функция проверки условия выхода из игры блока
 */
bool
isFallBlock(const GameBlock* const block)
{
    return block->y() >= block->parentWidget()->height();
}

/**
 * @brief Функция проверки, расположен ли курсор мыыши в области блока
 */
bool
isCursorHoveringBlock(const GameBlock* const block)
{
    const QPoint cursorInBlock = block->mapFromGlobal(QCursor::pos());
    return block->rect().contains(cursorInBlock);
}

}

namespace game {

/**
 * @brief Функция-фабрика блоков
 * @param[in]       pos     Начальная позиция блока
 * @param[in, out]  parent  Родительский виджет блока
 * @return GameBlock*
 */
GameBlock*
createBlock(const QPoint& pos, QWidget* const parent)
{
    using namespace block_settings;

    auto* block = new GameBlock("*", parent);

    // Размер и позиция
    block->setFixedSize(sz, sz);
    block->move(pos);

    // Свойства
    {
        constexpr auto velocityRange = velocityMax - velocityMin;
        const float velocityValue =
            velocityMin + velocityRange * QRandomGenerator::global()->generateDouble();

        block->setProperty("velocity_basic", velocityValue);
        block->setProperty("velocity_boost", velocityValue * boost);
        block->setProperty("y_real", static_cast<float>(pos.y()));
    }

    QAbstractButton::connect(block, &GameBlock::clicked, block, [block]()
    {
        // Гарантирует продолжение только для первого нажатия
        if (!conditions::isAliveBlock(block))
            return;
        block->setEnabled(false);

        // Удаление блока + анимация
        animateFadeOutAndDelete(block, animationDurationMs);
    });

    return block;
}

QPoint
makeSpawnPos(const QWidget* const scene)
{
    using namespace spawn_settings;
    using namespace block_settings;

    /// @todo Возможен спавн вне диапазона окна, если
    /// scene->width() <= sz или spawnY1 <= sz

    const int spawnX1 {scene->width()};
    const int xpos {QRandomGenerator::global()->bounded(spawnX0, spawnX1 - sz)};
    const int ypos {QRandomGenerator::global()->bounded(spawnY0, spawnY1 - sz)};

    return QPoint(xpos, ypos);
}

void
moveBlock(GameBlock* block, bool boost)
{
    // if (!isAliveBlock(block))
    //     return;

    const auto vBasic = block->property("velocity_basic").toFloat();
    const auto vBoost = block->property("velocity_boost").toFloat();

    /// @todo Можно обновлять по времени, а не по тактам:
    /// (boost ? vBoost : vBasic) * dtMs
    auto yReal = block->property("y_real").toFloat();
    yReal += boost ? vBoost : vBasic;

    block->setProperty("y_real", yReal);
    block->move(block->x(), qRound(yReal));
}

void
updateBlock(GameBlock* block)
{
    moveBlock(block, conditions::isCursorHoveringBlock(block));
}


} // namespace game
} // namespace


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(300, 600);
/// Your code here...

    using namespace game;

    // 1. Инициализация центрального виджета (сцены)
    auto* central = new QWidget(this);
    central->setObjectName("gameCentral");
    setCentralWidget(central);

    // 2. Настройка цикла спавна
    {
        using namespace spawn_settings;

        // Инициализация таймера
        auto *spawnTimer = new QTimer(this);

        // Запуск спавна по таймеру
        connect(spawnTimer, &QTimer::timeout, central, [central, spawnTimer]()
        {
            const auto blockNumber = central->findChildren<GameBlock*>().size();

            if (spawnLimit <= 0 || blockNumber < spawnLimit) {
                auto* block = createBlock(makeSpawnPos(central), central);
                block->show();
            }

            // Новый интервал спавна
            spawnTimer->start(QRandomGenerator::global()->bounded(minTms, maxTms));
        });

        spawnTimer->setSingleShot(true); // единоразовый отсчёт
        spawnTimer->start(QRandomGenerator::global()->bounded(minTms, maxTms));
    }

    // 3. Настройка цикла процесса
    {
        using namespace other_settings;

        auto* updateTimer = new QTimer(this);

        connect(updateTimer, &QTimer::timeout, this, [central, this]()
        {
            const auto blocks = central->findChildren<GameBlock*>();

            for (auto* block : blocks) {
                if (!conditions::isAliveBlock(block))
                    continue;

                // Удаление упавших блоков
                if (conditions::isFallBlock(block)) {
                    animateFadeOutAndDelete(block, 0);
                    continue;
                }

                // Проверка условия проигрыша
                if (gameState == GameState::Running && conditions::isLoseCondition(block)) {
                    gameState = GameState::GameOver;
                    central->setStyleSheet("#gameCentral { background-color: darkred; }");
                    setWindowTitle(tr("You LOSE!"));
                }

                // Обновление игрового блока
                updateBlock(block);
            }
        });

        // Запуск игрового цикла
        updateTimer->start(updateTms);
    }
}

MainWindow::~MainWindow()
{
}
