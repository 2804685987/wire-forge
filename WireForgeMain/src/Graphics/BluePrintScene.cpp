// BluePrintScene.cpp（部分核心代码）
#include "BluePrintScene.h"
#include "ConnectorItem.h"
#include "WireItem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

BluePrintScene::BluePrintScene(QObject* parent)
        : QGraphicsScene(parent)
{
    setSceneRect(-10000, -10000, 20000, 20000);   // 超大无限画布
    m_undoStack = new QUndoStack(this);
}

void BluePrintScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    if (!m_gridVisible) return;

    painter->setPen(QPen(QColor(220, 220, 220), 1));
    qreal left = int(rect.left()) - (int(rect.left()) % m_gridSize);
    qreal top = int(rect.top()) - (int(rect.top()) % m_gridSize);

    QVector<QLineF> lines;
    for (qreal x = left; x < rect.right(); x += m_gridSize) {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }
    painter->drawLines(lines);
}

HarnessComponent* BluePrintScene::addComponent(HarnessComponent* component)
{
    if (!component) return nullptr;
    addItem(component);
    emit componentAdded(component);
    return component;
}