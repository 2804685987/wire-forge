#pragma once

#include <QObject>
#include <QString>
#include <QPointF>
#include <QUuid>
#include "Model/ProjectModel.h"

class ProjectModel;

/**
 * @brief ProjectController 负责协调 ProjectModel 与界面（View）之间的交互
 *
 * 提供项目的创建/打开/保存等外部 API；在后续实现中将承载命令分发、Undo/Redo
 * 管理以及与 ProjectSerializer 的文件操作交互。
 */
class ProjectController : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造 ProjectController
     * @param parent QObject 父对象
     */
    explicit ProjectController(QObject *parent = nullptr);

    /**
     * @brief 新建项目并初始化空的 ProjectModel
     * @param path 可选的项目初始路径（可为空）
     * @return 成功返回 true，否则 false
     */
    Q_SLOT bool createProject(const QString &path);

    /**
     * @brief 打开并加载指定路径的项目文件
     * @param path 项目文件路径
     * @return 成功返回 true，否则 false
     */
    Q_SLOT bool openProject(const QString &path);

    /**
     * @brief 保存当前项目到指定路径；若 path 为空则保存到上次打开/保存的路径
     * @param path 文件路径（可选）
     * @return 成功返回 true，否则 false
     */
    Q_SLOT bool saveProject(const QString &path = QString());

    /**
     * @brief 添加节点（支持 Undo/Redo）
     */
    Q_SLOT QUuid addNode(const QPointF &pos);

    /**
     * @brief 添加连线（支持 Undo/Redo）
     */
    Q_SLOT QUuid addWire(const QUuid &from, const QUuid &to);

    /**
     * @brief 撤销
     */
    Q_SLOT void undo();

    /**
     * @brief 重做
     */
    Q_SLOT void redo();

    /** @brief 是否可撤销 */
    bool canUndo() const;

    /** @brief 是否可重做 */
    bool canRedo() const;

    /** @brief 设置连线验证规则 */
    void setWireValidationRules(const ProjectModel::WireValidationRules &rules);

    /** @brief 获取连线验证规则 */
    ProjectModel::WireValidationRules wireValidationRules() const;

    /** @brief 返回当前管理的 ProjectModel（若尚未创建则返回 nullptr） */
    ProjectModel* project() const { return m_project; }

Q_SIGNALS:
    /** @brief 当项目被打开或新建完成后发出 */
    void projectOpened();
    /** @brief 当项目成功保存时发出 */
    void projectSaved();

private:
    ProjectModel *m_project{nullptr};
    class QUndoStack *m_undoStack{nullptr};
};


