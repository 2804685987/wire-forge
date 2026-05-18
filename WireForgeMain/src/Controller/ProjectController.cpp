#include "ProjectController.h"

#include "Model/ProjectModel.h"
#include "Model/Components/BasicConnectorModel.h"
#include "Tools/ProjectSerializer.h"

#include <QDebug>
#include <QUndoCommand>
#include <QUndoStack>

namespace {
class AddNodeCommand : public QUndoCommand
{
public:
    AddNodeCommand(ProjectModel *model, const QPointF &pos)
        : m_model(model), m_pos(pos)
    {
        setText(QStringLiteral("添加节点"));
    }

    void redo() override
    {
        if (!m_model) return;
        if (m_id.isNull()) {
            m_id = m_model->addNode(m_pos);
        } else {
            m_model->addNodeWithId(m_id, m_pos);
        }
    }

    void undo() override
    {
        if (!m_model || m_id.isNull()) return;
        m_model->removeNode(m_id);
    }

    QUuid nodeId() const { return m_id; }

private:
    ProjectModel *m_model{nullptr};
    QUuid m_id;
    QPointF m_pos;
};

class AddWireCommand : public QUndoCommand
{
public:
    AddWireCommand(ProjectModel *model, const QUuid &from, const QUuid &to)
        : m_model(model), m_from(from), m_to(to)
    {
        setText(QStringLiteral("添加连线"));
    }

    void redo() override
    {
        if (!m_model) return;
        if (m_id.isNull()) {
            m_id = m_model->addWire(m_from, m_to);
        } else {
            m_model->addWireWithId(m_id, m_from, m_to);
        }
    }

    void undo() override
    {
        if (!m_model || m_id.isNull()) return;
        m_model->removeWire(m_id);
    }

    QUuid wireId() const { return m_id; }

private:
    ProjectModel *m_model{nullptr};
    QUuid m_id;
    QUuid m_from;
    QUuid m_to;
};
}

/**
 * @brief 构造 ProjectController
 */
ProjectController::ProjectController(QObject *parent)
    : QObject(parent)
{
    m_undoStack = new QUndoStack(this);

}

/**
 * @brief 创建一个新项目（当前为最小实现）
 *
 * 将会释放现有的 ProjectModel（如有），创建一个新的实例并发出 projectOpened 信号。
 */
bool ProjectController::createProject(const QString &path)
{
    // minimal skeleton: create model and emit signal
    delete m_project; m_project = nullptr;
    m_project = new ProjectModel(this);
    if (m_undoStack) m_undoStack->clear();
    static_cast<void>(path);
    qDebug() << "ProjectController::createProject" << path;
    emit projectOpened();
    return true;
}

/**
 * @brief 打开项目文件（占位实现）
 *
 * 未来应通过 ProjectSerializer 反序列化项目内容并填充 ProjectModel。
 */
bool ProjectController::openProject(const QString &path)
{
    // 使用 ProjectSerializer 反序列化到 model
    if (!m_project) m_project = new ProjectModel(this);
    bool ok = ProjectSerializer::load(m_project, path);
    if (!ok) {
        qWarning() << "ProjectController::openProject failed to load" << path;
        return false;
    }
    if (m_undoStack) m_undoStack->clear();
    qDebug() << "ProjectController::openProject" << path;
    emit projectOpened();
    return true;
}

/**
 * @brief 保存项目（占位实现）
 *
 * 未来应调用 ProjectSerializer 将 ProjectModel 序列化为磁盘文件。
 */
bool ProjectController::saveProject(const QString &path)
{
    if (!m_project) {
        qWarning() << "ProjectController::saveProject: no project to save";
        return false;
    }
    bool ok = ProjectSerializer::save(m_project, path);
    if (!ok) {
        qWarning() << "ProjectController::saveProject failed" << path;
        return false;
    }
    qDebug() << "ProjectController::saveProject" << path;
    emit projectSaved();
    return true;
}

QUuid ProjectController::addNode(const QPointF &pos)
{
    if (!m_project || !m_undoStack) return {};
    auto *cmd = new AddNodeCommand(m_project, pos);
    m_undoStack->push(cmd);
    return cmd->nodeId();
}

QUuid ProjectController::addWire(const QUuid &from, const QUuid &to)
{
    if (!m_project || !m_undoStack) return {};
    QString error;
    if (!m_project->canAddWire(from, to, &error)) {
        qWarning() << "ProjectController::addWire rejected" << error;
        return {};
    }
    auto *cmd = new AddWireCommand(m_project, from, to);
    m_undoStack->push(cmd);
    return cmd->wireId();
}

void ProjectController::undo()
{
    if (m_undoStack) m_undoStack->undo();
}

void ProjectController::redo()
{
    if (m_undoStack) m_undoStack->redo();
}

bool ProjectController::canUndo() const
{
    return m_undoStack && m_undoStack->canUndo();
}

bool ProjectController::canRedo() const
{
    return m_undoStack && m_undoStack->canRedo();
}

void ProjectController::setWireValidationRules(const ProjectModel::WireValidationRules &rules)
{
    if (m_project) {
        m_project->setWireValidationRules(rules);
    }
}

ProjectModel::WireValidationRules ProjectController::wireValidationRules() const
{
    return m_project ? m_project->wireValidationRules() : ProjectModel::WireValidationRules{};
}

