#include "LayerModel.h"

LayerModel::LayerModel(const QString &name, QObject *parent)
    : QObject(parent), m_name(name)
{
}


