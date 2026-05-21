// src/Core/ComponentAutoDiscovery.cpp
#include "ComponentAutoDiscovery.h"
#include "../Graphics/ConnectorItem.h"
#include "../Graphics/WireItem.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QStringList>

ComponentAutoDiscovery::ComponentAutoDiscovery(QObject* parent)
        : QObject(parent)
{
}

QList<HarnessComponent*> ComponentAutoDiscovery::discoverComponents(const QString& directory)
{
    QList<HarnessComponent*> components;
    QDir dir(directory);

    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << directory;
        return components;
    }

    // 获取所有 .json 文件
    QStringList jsonFiles = dir.entryList(QStringList() << "*.json", QDir::Files);

    for (const QString& fileName : jsonFiles) {
        QString filePath = dir.absoluteFilePath(fileName);
        HarnessComponent* comp = loadComponentFromJson(filePath);
        if (comp) {
            components.append(comp);
        }
    }

    qDebug() << "Discovered" << components.size() << "components from" << directory;
    return components;
}

HarnessComponent* ComponentAutoDiscovery::loadComponentFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << filePath;
        return nullptr;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        qWarning() << "Invalid JSON format in:" << filePath;
        return nullptr;
    }

    QJsonObject obj = doc.object();
    QString typeStr = obj["type"].toString().toLower().trimmed();

    HarnessComponent* component = nullptr;

    if (typeStr.contains("connector")) {
        component = new ConnectorItem();
    }
    else if (typeStr.contains("wire")) {
        component = new WireItem();
    }
    else {
        qWarning() << "Unknown component type:" << typeStr << "in file:" << filePath;
        return nullptr;
    }

    if (component) {
        component->fromJson(obj);
        qDebug() << "Successfully loaded:" << component->componentType()
                 << "from" << QFileInfo(filePath).fileName();
    }

    return component;
}