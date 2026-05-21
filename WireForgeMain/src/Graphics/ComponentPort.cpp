#include "ComponentPort.h"

ComponentPort::ComponentPort(const QString& name)
        : m_name(name)
{
}

void ComponentPort::setName(const QString& name)
{
    m_name = name;
}

void ComponentPort::setDirection(PortDirection dir)
{
    m_direction = dir;
}

void ComponentPort::setLocalPos(const QPointF& pos)
{
    m_localPos = pos;
}

void ComponentPort::setConnected(bool connected)
{
    m_connected = connected;
}

void ComponentPort::setSignalName(const QString& signal)
{
    m_signalName = signal;
}

void ComponentPort::setSignalType(const QString& type)
{
    m_signalType = type;
}

void ComponentPort::setCurrentRating(double amp)
{
    m_currentRating = amp;
}

QJsonObject ComponentPort::toJson() const
{
    QJsonObject obj;
    obj["name"] = m_name;
    obj["direction"] = static_cast<int>(m_direction);
    obj["x"] = m_localPos.x();
    obj["y"] = m_localPos.y();
    obj["signalName"] = m_signalName;
    obj["signalType"] = m_signalType;
    obj["currentRating"] = m_currentRating;
    return obj;
}

bool ComponentPort::fromJson(const QJsonObject& obj)
{
    if (obj.contains("name")) m_name = obj["name"].toString();
    if (obj.contains("direction")) m_direction = static_cast<PortDirection>(obj["direction"].toInt());
    if (obj.contains("x") && obj.contains("y"))
        m_localPos = QPointF(obj["x"].toDouble(), obj["y"].toDouble());
    if (obj.contains("signalName")) m_signalName = obj["signalName"].toString();
    if (obj.contains("signalType")) m_signalType = obj["signalType"].toString();
    if (obj.contains("currentRating")) m_currentRating = obj["currentRating"].toDouble();
    return true;
}