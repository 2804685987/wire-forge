#pragma once

#include <QWidget>

class ProjectController;
class ProjectModel;
class QLabel;
class QCheckBox;
class QPushButton;
class ProjectCanvasWidget;

/**
 * @brief ProjectEditorWidget 是项目管理模块的最小画布视图
 *
 * 提供基础的节点/连线绘制与 Undo/Redo 操作入口。
 */
class ProjectEditorWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造 ProjectEditorWidget
     * @param parent 父 QWidget
     * @param controller 可选的 ProjectController 注入
     */
    explicit ProjectEditorWidget(QWidget *parent = nullptr, ProjectController *controller = nullptr);

    /**
     * @brief 析构函数
     */
    ~ProjectEditorWidget() override = default;

private:
    void setModel(ProjectModel *model);
    void setStatus(const QString &text);
    QPointF nextNodePosition() const;
    void updateUndoRedoButtons();
    void applyWireRules();


private:
    ProjectController *m_controller{nullptr};
    ProjectCanvasWidget *m_canvas{nullptr};
    QLabel *m_statusLabel{nullptr};
    QPushButton *m_undoButton{nullptr};
    QPushButton *m_redoButton{nullptr};
    QCheckBox *m_allowInputInput{nullptr};
    QCheckBox *m_allowOutputOutput{nullptr};
    QCheckBox *m_allowInputToOutput{nullptr};
    QCheckBox *m_allowSameNode{nullptr};
};


