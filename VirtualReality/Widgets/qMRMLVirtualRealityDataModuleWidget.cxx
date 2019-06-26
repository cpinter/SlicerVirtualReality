/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

// VirtualReality Widgets includes
#include "qMRMLVirtualRealityDataModuleWidget.h"
#include "ui_qMRMLVirtualRealityDataModuleWidget.h"

// VirtualReality MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityDataModuleWidgetPrivate : public Ui_qMRMLVirtualRealityDataModuleWidget
{
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityDataModuleWidget);

protected:
  qMRMLVirtualRealityDataModuleWidget* const q_ptr;

public:
  qMRMLVirtualRealityDataModuleWidgetPrivate(qMRMLVirtualRealityDataModuleWidget& object);
  virtual ~qMRMLVirtualRealityDataModuleWidgetPrivate();

  void init();

public:
  /// Virtual reality view MRML node
  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> VirtualRealityViewNode;
};

//-----------------------------------------------------------------------------
qMRMLVirtualRealityDataModuleWidgetPrivate::qMRMLVirtualRealityDataModuleWidgetPrivate(qMRMLVirtualRealityDataModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityDataModuleWidgetPrivate::~qMRMLVirtualRealityDataModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityDataModuleWidgetPrivate::init()
{
  Q_Q(qMRMLVirtualRealityDataModuleWidget);
  this->setupUi(q);
}

//-----------------------------------------------------------------------------
// qMRMLVirtualRealityDataModuleWidget methods

//-----------------------------------------------------------------------------
qMRMLVirtualRealityDataModuleWidget::qMRMLVirtualRealityDataModuleWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLVirtualRealityDataModuleWidgetPrivate(*this))
{
  Q_D(qMRMLVirtualRealityDataModuleWidget);
  d->init();

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityDataModuleWidget::~qMRMLVirtualRealityDataModuleWidget()
= default;

//-----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* qMRMLVirtualRealityDataModuleWidget::virtualRealityViewNode() const
{
  Q_D(const qMRMLVirtualRealityDataModuleWidget);
  return d->VirtualRealityViewNode;
}

//-----------------------------------------------------------------------------
QString qMRMLVirtualRealityDataModuleWidget::virtualRealityViewNodeID()const
{
  Q_D(const qMRMLVirtualRealityDataModuleWidget);
  return (d->VirtualRealityViewNode.GetPointer() ? d->VirtualRealityViewNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityDataModuleWidget::updateWidgetFromMRML()
{
  //Q_D(qMRMLVirtualRealityDataModuleWidget);
}

//-----------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeView* qMRMLVirtualRealityDataModuleWidget::treeView()
{
  Q_D(qMRMLVirtualRealityDataModuleWidget);
  return d->SubjectHierarchyTreeView;
}
