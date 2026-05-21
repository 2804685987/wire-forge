#pragma once
#include <QGraphicsView>
#include <QWheelEvent>

class BluePrintScene;

class BluePrintView : public QGraphicsView
{
Q_OBJECT

public:
    explicit BluePrintView(QWidget* parent = nullptr);
    ~BluePrintView() override;

    BluePrintScene* blueprintScene() const;

    // ==================== 视图控制 ====================
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void fitToView();

    void setSimplifiedMode(bool enabled);   // 全局精简显示模式

    // ==================== 拖拽放置 ====================
    void startComponentDrag(const QString& componentType);  // 从库拖入

    // ==================== Undo/Redo ====================
    void setUndoStack(QUndoStack* stack);

signals:
    void zoomChanged(qreal scale);
    void mousePositionChanged(const QPointF& scenePos);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void updateSimplifiedMode();   // 根据缩放级别自动切换精简模式

private:
    qreal m_currentScale = 1.0;
    bool m_simplifiedMode = false;
    int m_simplifiedThreshold = 0.6;   // 缩放小于0.6时自动精简

    QPoint m_lastPanPoint;
    bool m_isPanning = false;
};