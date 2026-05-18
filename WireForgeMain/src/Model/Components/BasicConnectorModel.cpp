#include "BasicConnectorModel.h"


namespace {
    QString directionToString(ConnectorModel::Direction dir)
    {
        return dir == ConnectorModel::Output ? QStringLiteral("Output") : QStringLiteral("Input");
    }

    ConnectorModel::Direction directionFromJson(const QJsonValue &value)
    {
        if (value.isString()) {
            const QString s = value.toString();
            if (s.compare(QStringLiteral("Output"), Qt::CaseInsensitive) == 0) {
                return ConnectorModel::Output;
            }
            if (s.compare(QStringLiteral("Input"), Qt::CaseInsensitive) == 0) {
                return ConnectorModel::Input;
            }
        }
        if (value.isDouble()) {
            return static_cast<ConnectorModel::Direction>(value.toInt());
        }
        return ConnectorModel::Input;
    }
}

QJsonObject BasicConnectorModel::toJson() const
{
    QJsonObject obj;
    obj.insert("id", m_id.toString());
    obj.insert("name", m_name);
    obj.insert("direction", directionToString(m_direction));
    obj.insert("type", connectorType());
    return obj;
}

void BasicConnectorModel::fromJson(const QJsonObject &obj)
{
    if (obj.contains("id")) {
        m_id = QUuid(obj.value("id").toString());
    }
    if (obj.contains("name")) {
        m_name = obj.value("name").toString();
    }
    if (obj.contains("direction")) {
        m_direction = directionFromJson(obj.value("direction"));
    }
}


