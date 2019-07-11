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
#include <QIcon>

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityHomeWidgetPrivate : public Ui_qMRMLVirtualRealityHomeWidget
{
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityHomeWidget);

protected:
  qMRMLVirtualRealityHomeWidget* const q_ptr;

public:
  qMRMLVirtualRealityHomeWidgetPrivate(qMRMLVirtualRealityHomeWidget& object);
  virtual ~qMRMLVirtualRealityHomeWidgetPrivate();
  
  void init();

  QMap<QPushButton*, QWidget*> ModuleWidgetsMap;

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

  this->backButton->setVisible(false);

  QObject::connect(this->MotionSensitivitySliderWidget, SIGNAL(valueChanged(double)), q, SLOT(onMotionSensitivityChanged(double)));
  QObject::connect(this->FlySpeedSliderWidget, SIGNAL(valueChanged(double)), q, SLOT(onFlySpeedChanged(double)));
  QObject::connect(this->Magnification001xButton, SIGNAL(clicked()), q, SLOT(onMagnification001xPressed()));
  QObject::connect(this->Magnification01xButton, SIGNAL(clicked()), q, SLOT(onMagnification01xPressed()));
  QObject::connect(this->Magnification1xButton, SIGNAL(clicked()), q, SLOT(onMagnification1xPressed()));
  QObject::connect(this->Magnification10xButton, SIGNAL(clicked()), q, SLOT(onMagnification10xPressed()));
  QObject::connect(this->Magnification100xButton, SIGNAL(clicked()), q, SLOT(onMagnification100xPressed()));
  QObject::connect(this->SyncViewToReferenceViewButton, SIGNAL(clicked()), q, SLOT(updateViewFromReferenceViewCamera()));

  //QObject::connect(this->LockMagnificationCheckBox, SIGNAL(toggled(bool)), q, SLOT(setMagnificationLock(bool)));
  //TODO: Magnification lock of view node not implemented yet 
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
void qMRMLVirtualRealityHomeWidget::addModuleButton(QWidget* moduleWidget, QIcon& icon)
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  if (!moduleWidget)
  {
    qCritical() << Q_FUNC_INFO << "Failed: widget is null";
    return;
  }

  QPushButton* moduleButton = new QPushButton(d->ModulesGroupBox);  
  d->ModulesGroupBoxLayout->addWidget(moduleButton);
  moduleButton->setIcon(icon);
  
  moduleWidget->setParent(d->ModuleWidgetFrame);
  d->ModuleWidgetsMap[moduleButton] = moduleWidget;

  QObject::connect(moduleButton, SIGNAL(clicked()), this, SLOT(onModuleButtonPressed()));
  QObject::connect(d->backButton, SIGNAL(clicked()), this, SLOT(onBackButtonPressed()));
 
  moduleButton->setVisible(true);
  moduleWidget->setVisible(false);
  d->backButton->setVisible(false);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onModuleButtonPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);

  QPushButton* moduleButton = qobject_cast<QPushButton*>(sender());
  
  if (!moduleButton)
  {
    qCritical() << Q_FUNC_INFO << "Failed: moduleButton is null";
    return;
  }

  d->HomeWidgetFrame->setVisible(false);
  d->ModulesGroupBox->setVisible(false);
  d->ModuleWidgetsMap[moduleButton]->setVisible(true);
  d->backButton->setVisible(true);
}

//-----------------------------------------------------------------------------
void  qMRMLVirtualRealityHomeWidget::onBackButtonPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  d->HomeWidgetFrame->setVisible(true);
  d->ModulesGroupBox->setVisible(true); 
  d->backButton->setVisible(false);

  QMap<QPushButton*, QWidget*>::const_iterator iteratorForMap; 
  for (iteratorForMap = d->ModuleWidgetsMap.constBegin(); iteratorForMap != d->ModuleWidgetsMap.constEnd(); ++iteratorForMap) 
  {
    iteratorForMap.value()->setVisible(false); 
  }

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
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  bool wasBlocked = d->MotionSensitivitySliderWidget->blockSignals(true);
  d->MotionSensitivitySliderWidget->setValue(vrViewNode != nullptr ? vrViewNode->GetMotionSensitivity() * 100.0 : 0);
  d->MotionSensitivitySliderWidget->setEnabled(vrViewNode != nullptr);
  d->MotionSensitivitySliderWidget->blockSignals(wasBlocked);

  wasBlocked = d->FlySpeedSliderWidget->blockSignals(true);
  d->FlySpeedSliderWidget->setValue(vrViewNode != nullptr ? vrViewNode->GetMotionSpeed() : 1.6666);
  d->FlySpeedSliderWidget->setEnabled(vrViewNode != nullptr);
  d->FlySpeedSliderWidget->blockSignals(wasBlocked);

  /*
  bool wasBlocked = d->LockMagnificationCheckBox->blockSignals(true);
  d->LockMagnificationCheckBox->setChecked(vrViewNode != nullptr && vrViewNode->);
  d->LockMagnificationCheckBox->setEnabled(vrViewNode != nullptr);
  d->LockMagnificationCheckBox->blockSignals(wasBlocked);
  */
  //TODO: Magnification lock of view node not implemented yet 

  d->Magnification001xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification01xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification1xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification10xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL);
  d->Magnification100xButton->setEnabled(vrViewNode != nullptr && vrViewNode->GetMagnification() != NULL); 
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMotionSensitivityChanged(double percent)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMotionSensitivity(percent * 0.01);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onFlySpeedChanged(double speedMps)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMotionSpeed(speedMps);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification001xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(0.01);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification01xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(0.1);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification1xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(1.0);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification10xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(10.0);
}

//-----------------------------------------------------------------------------
void qMRMLVirtualRealityHomeWidget::onMagnification100xPressed()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrViewNode->SetMagnification(100.0);
}

//-----------------------------------------------------------------------------
/*
void qMRMLVirtualRealityHomeWidget::updateViewFromReferenceViewCamera()
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  qSlicerVirtualRealityModule* vrModule = dynamic_cast<qSlicerVirtualRealityModule*>(this->module());
  if (!vrModule)
  {
    qCritical() << Q_FUNC_INFO << " Failed: vrModule is null";
    return;
  }
  qMRMLVirtualRealityView* vrView = vrModule->viewWidget();
  if (!vrView)
  {
    qCritical() << Q_FUNC_INFO << " Failed: view node is null";
    return;
  }
  vrView->updateViewFromReferenceViewCamera();
}
*/
//TODO: This member function won't work unless qSlicerVirtualRealityModule and qMRMLVirtualRealityView are included

//-----------------------------------------------------------------------------
/*
void qMRMLVirtualRealityHomeWidget::setMagnificationLock(bool active)
{
  Q_D(qMRMLVirtualRealityHomeWidget);
  vtkMRMLVirtualRealityViewNode* vrViewNode = d->VirtualRealityViewNode;

  if (!vrViewNode)
  {
   qCritical() << Q_FUNC_INFO << " Failed: view node is null";
   return;
  }
   vrViewNode->????(active);  //TODO: Implement magnification lock for view node

}
*/
