// src/Graphics/WireItem.h
#pragma once
#include "HarnessComponent.h"
#include "ComponentPort.h"

class WireItem : public HarnessComponent
{
Q_OBJECT
    Q_PROPERTY(QString wireGauge READ wireGauge WRITE setWireGauge)

public:
    explicit WireItem(QGraphicsItem* parent = nullptr);
    ~WireItem() override;

    QString componentType() const override { return "Wire"; }   // ← 修改这里
    // ==================== 连接关系 ====================
    ComponentPort* startPort() const { return m_startPort; }
    ComponentPort* endPort() const { return m_endPort; }

    void setStartPort(ComponentPort* port);
    void setEndPort(ComponentPort* port);

    // 手动控制路径点（支持多段线 / 贝塞尔）
    QList<QPointF> controlPoints() const { return m_controlPoints; }
    void setControlPoints(const QList<QPointF>& points);
    void addControlPoint(const QPointF& point);

    // ==================== 电气属性 ====================
    QString wireGauge() const { return m_wireGauge; }   // 如 "AWG18", "0.5mm²"
    void setWireGauge(const QString& gauge);

    QString color() const { return m_color; }
    void setColor(const QString& color);

    double length() const { return m_length; }         // 自动计算或手动设置

    QString signalName() const { return m_signalName; }
    void setSignalName(const QString& name);

    // ==================== 图形 ====================
    QRectF boundingRect() const override;
    QPainterPath shape() const override;                // 用于精确碰撞检测
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // ==================== 序列化 ====================
    QJsonObject toJson() const override;
    bool fromJson(const QJsonObject& obj) override;

    // ==================== 业务 ====================
    bool validate() const override;
    void updateLength();                                // 重新计算线长

signals:
    void lengthChanged(double newLength);
    void wireConnected();   // 连接完成信号

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    ComponentPort* m_startPort = nullptr;
    ComponentPort* m_endPort = nullptr;

    QList<QPointF> m_controlPoints;     // 支持手动路由的控制点

    QString m_wireGauge = "AWG20";
    QString m_color = "#FF0000";        // 红色默认
    QString m_signalName;
    double m_length = 0.0;

    // 外观设置
    qreal m_lineWidth = 3.0;
    bool m_hasShield = false;
};