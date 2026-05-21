#pragma once
#include <QObject>
#include <QDateTime>

class BluePrintScene;
class HarnessComponent;

class ProjectSerializer : public QObject
{
Q_OBJECT

public:
    explicit ProjectSerializer(QObject* parent = nullptr);

    bool saveProject(const QString& filePath, BluePrintScene* scene);
    bool loadProject(const QString& filePath, BluePrintScene* scene);
};