#pragma once
#include <QString>
#include <QPointF>

class HarnessComponent;

/**
 * @brief 子组件插槽（用于 Connector 上挂载尾夹、防水塞、卡扣等）
 */
struct SubComponentSlot
{
    QString slotId;                    // 插槽唯一标识，如 "RearClip"、"WaterproofSeal"、"Grommet"
    QString slotName;                  // 显示名称，如 "尾夹"、"防水塞"
    QString compatibleTypes;           // 支持的子组件类型（逗号分隔），例如 "Clip,Seal"

    HarnessComponent* attachedComponent = nullptr;  // 当前挂载的子组件
    QPointF relativePos;               // 相对于 Connector 的本地位置
    bool required = false;             // 是否必须配套

    bool isCompatible(HarnessComponent* component) const;
    bool isOccupied() const { return attachedComponent != nullptr; }
};