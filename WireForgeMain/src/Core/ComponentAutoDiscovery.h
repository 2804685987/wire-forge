#pragma once
#include <QObject>
#include "../Graphics/HarnessComponent.h"

class ComponentAutoDiscovery : public QObject
{
Q_OBJECT

public:
    explicit ComponentAutoDiscovery(QObject* parent = nullptr);

    QList<HarnessComponent*> discoverComponents(const QString& directory);
    HarnessComponent* loadComponentFromJson(const QString& filePath);
};