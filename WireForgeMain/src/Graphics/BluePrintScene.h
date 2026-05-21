#pragma once
#include <QGraphicsScene>
#include <QUndoStack>
#include "ComponentPort.h"

class HarnessComponent;
class WireItem;

class BluePrintScene : public QGraphicsScene
{
Q_OBJECT

public:
    explicit BluePrintScene(QObject* parent = nullptr);
    ~BluePrintScene() override;

    // ==================== 网格与吸附 ====================
    void setGridVisible(bool visible);
    void setGridSize(int size);
    void setSnapEnabled(bool enabled);

    QPointF snapToGrid(const QPointF& pos) const;

    // ==================== 组件管理 ====================
    HarnessComponent* addComponent(HarnessComponent* component);
    WireItem* connectPorts(ComponentPort* startPort, ComponentPort* endPort);

    QList<HarnessComponent*> selectedComponents() const;
    QList<WireItem*> selectedWires() const;

    // ==================== 序列化 ====================
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& obj);

    // ==================== 业务 ====================
    void validateAll();                    // 执行DRC检查
    void autoRoute(WireItem* wire);        // 后续可扩展自动布线

signals:
    void componentAdded(HarnessComponent* component);
    void componentRemoved(HarnessComponent* component);
    void connectionCreated(WireItem* wire);
    void sceneValidated(const QString& message, bool hasError);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    bool m_gridVisible = true;
    int m_gridSize = 20;
    bool m_snapEnabled = true;

    QUndoStack* m_undoStack = nullptr;     // 建议在View层持有，但Scene可访问

    // 临时拖拽状态
    QPointF m_lastMousePos;
};