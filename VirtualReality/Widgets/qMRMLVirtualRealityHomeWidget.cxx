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
#include "qMRMLVirtualRealityHomeWidget.h"

#include "ui_qMRMLVirtualRealityHomeWidget.h"

// VirtualReality MRML includes
#include "vtkMRMLVirtualRealityViewNode.h"

// VTK includes
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityHomeWidgetPrivate: public Ui_qMRMLVirtualRealityHomeWidget
{
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityHomeWidget);

protected:
  qMRMLVirtualRealityHomeWidget* const q_ptr;
public:
  qMRMLVirtualRealityHomeWidgetPrivate(qMRMLVirtualRealityHomeWidget& object);
  virtual ~qMRMLVirtualRealityHomeWidgetPrivate();
  void init();

public:
  /// Virtual reality view MRML node
  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> VirtualRealityViewNode;
};

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidgetPrivate::qMRMLVirtualRealityHomeWidgetPrivate(qMRMLVirtualRealityHomeWidget& object)
  : q_ptr(&object)
{
  this->VirtualRealityViewNode = nullptr;
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidgetPrivate::~qMRMLVirtualRealityHomeWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidgetPrivate::init()
{
  Q_Q(qMRMLVirtualRealityHomeWidget);
  this->setupUi(q);

  // Make connections
  // QObject::connect(this->MRMLNodeComboBox_SourceGeometryNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
  //   q, SLOT(onSourceNodeChanged(vtkMRMLNode*)) );

  // q->setEnabled(this->SegmentationDisplayNode.GetPointer());
}



//-----------------------------------------------------------------------------
// qMRMLVirtualRealityHomeWidget methods

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidget::qMRMLVirtualRealityHomeWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLVirtualRealityHomeWidgetPrivate(*this))
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  d->init();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
qMRMLVirtualRealityHomeWidget::~qMRMLVirtualRealityHomeWidget()
= default;

//-----------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* qMRMLVirtualRealityHomeWidget::virtualRealityViewNode() const
{
  Q_D(const qMRMLVirtualRealityHomeWidget);
  return d->VirtualRealityViewNode;
}

//-----------------------------------------------------------------------------
QString qMRMLVirtualRealityHomeWidget::virtualRealityViewNodeID()const
{
  Q_D(const qMRMLVirtualRealityHomeWidget);
  return (d->VirtualRealityViewNode.GetPointer() ? d->VirtualRealityViewNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::setVirtualRealityViewNode(vtkMRMLVirtualRealityViewNode* node)
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  if (d->VirtualRealityViewNode == node)
  {
    return;
  }

  qvtkReconnect(d->VirtualRealityViewNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  vtkMRMLVirtualRealityViewNode* vrViewNode = vtkMRMLVirtualRealityViewNode::SafeDownCast(node);
  d->VirtualRealityViewNode = vrViewNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  // Sanity check
  // this->setEnabled(d->SegmentationNode.GetPointer());
  // if (!d->SegmentationNode.GetPointer())
  //   {
  //   d->frame_SourceGeometry->setVisible(false);
  //   d->groupBox_VolumeSpacingOptions->setVisible(false);
  //   d->MRMLCoordinatesWidget_Spacing->setEnabled(false);
  //   d->label_Error->setText("No segmentation node specified!");
  //   d->label_Error->setVisible(true);
  //   d->updateGeometryWidgets();
  //   return;
  //   }

  //TODO: Update widgets from VR view node
}
