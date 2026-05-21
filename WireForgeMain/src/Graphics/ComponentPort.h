#pragma once
#include <QPointF>
#include <QString>
#include <QJsonObject>

enum class PortDirection
{
    Undefined,
    Left,      // 向左输出（常用于连接器左侧端子）
    Right,     // 向右输出（常用于连接器右侧端子）
    Up,
    Down,
    Bidirectional
};

/**
 * @brief 组件端口（端子）类
 * 用于表示连接器上的每个可连接点，支持方向性
 */
class ComponentPort
{
public:
    explicit ComponentPort(const QString& name = "Port");

    QString name() const { return m_name; }
    void setName(const QString& name);

    PortDirection direction() const { return m_direction; }
    void setDirection(PortDirection dir);

    // 相对于所属组件的本地坐标
    QPointF localPos() const { return m_localPos; }
    void setLocalPos(const QPointF& pos);

    // 连接状态
    bool isConnected() const { return m_connected; }
    void setConnected(bool connected);

    // 信号属性
    QString signalName() const { return m_signalName; }
    void setSignalName(const QString& signal);

    QString signalType() const { return m_signalType; }
    void setSignalType(const QString& type);

    // 电气属性
    double currentRating() const { return m_currentRating; }   // 额定电流 (A)
    void setCurrentRating(double amp);

    // 序列化
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& obj);

private:
    QString m_name;
    PortDirection m_direction = PortDirection::Undefined;
    QPointF m_localPos;           // 相对于组件的本地位置
    bool m_connected = false;

    QString m_signalName;
    QString m_signalType;
    double m_currentRating = 0.0;
};