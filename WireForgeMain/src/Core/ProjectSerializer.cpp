// src/Core/ProjectSerializer.cpp
#include "ProjectSerializer.h"
#include "../Graphics/BluePrintScene.h"
#include "../Graphics/HarnessComponent.h"
#include "../Graphics/ConnectorItem.h"
#include "../Graphics/WireItem.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

ProjectSerializer::ProjectSerializer(QObject* parent)
        : QObject(parent)
{
}

bool ProjectSerializer::saveProject(const QString& filePath, BluePrintScene* scene)
{
    if (!scene) {
        qWarning() << "Scene is null";
        return false;
    }

    QJsonObject root;
    QJsonArray componentsArray;

    for (QGraphicsItem* item : scene->items()) {
        // 使用 dynamic_cast 更安全（QGraphicsObject 是多继承）
        if (HarnessComponent* comp = dynamic_cast<HarnessComponent*>(item)) {
            componentsArray.append(comp->toJson());
        }
    }

    root["components"] = componentsArray;
    root["version"] = "1.0";
    root["saveTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(root);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    qDebug() << "Project saved successfully to" << filePath;
    return true;
}

bool ProjectSerializer::loadProject(const QString& filePath, BluePrintScene* scene)
{
    if (!scene) {
        qWarning() << "Scene is null";
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON in:" << filePath;
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray components = root["components"].toArray();

    int loadedCount = 0;
    for (const QJsonValue& val : components) {
        QJsonObject obj = val.toObject();
        QString typeStr = obj["componentType"].toString().toLower();

        HarnessComponent* comp = nullptr;

        if (typeStr.contains("connector")) {
            comp = new ConnectorItem();
        } else if (typeStr.contains("wire")) {
            comp = new WireItem();
        }

        if (comp) {
            if (comp->fromJson(obj)) {
                scene->addComponent(comp);
                loadedCount++;
            } else {
                delete comp;  // 加载失败清理内存
            }
        }
    }

    qDebug() << "Successfully loaded" << loadedCount << "components from" << filePath;
    return true;
}