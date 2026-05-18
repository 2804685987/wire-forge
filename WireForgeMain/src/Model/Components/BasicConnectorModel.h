#pragma once

#include "ConnectorModel.h"

/**
 * @brief BasicConnectorModel 提供可用的默认连接器实现
 */
class BasicConnectorModel : public ConnectorModel
{
    Q_OBJECT
public:
    static QString typeName() { return QStringLiteral("Basic"); }

    explicit BasicConnectorModel(QString name = QString(), Direction dir = Input, QObject *parent = nullptr)
        : ConnectorModel(std::move(name), dir, parent)
    {
    }

    [[nodiscard]] QString connectorType() const override { return typeName(); }

    [[nodiscard]] QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;
};

