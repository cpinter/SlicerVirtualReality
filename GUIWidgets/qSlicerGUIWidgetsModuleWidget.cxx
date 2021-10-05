/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L., and
  development was supported by "ICEX Espana Exportacion e Inversiones" under
  the program "Inversiones de Empresas Extranjeras en Actividades de I+D
  (Fondo Tecnologico)- Convocatoria 2021"

==============================================================================*/

// GUI Widgets includes
#include "qSlicerGUIWidgetsModuleWidget.h"
#include "ui_qSlicerGUIWidgetsModuleWidget.h"

#include "vtkMRMLGUIWidgetNode.h"
#include "vtkMRMLGUIWidgetDisplayNode.h"

// VirtualReality includes
#include "vtkMRMLVirtualRealityViewNode.h"
#include "vtkSlicerVirtualRealityLogic.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

#include "vtkSlicerApplicationLogic.h"

// qMRMLWidget includes
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Virtual Reality includes
#include "qMRMLVirtualRealityHomeWidget.h"
#include "qMRMLVirtualRealityDataModuleWidget.h"
#include "qMRMLVirtualRealitySegmentEditorWidget.h"
#include "qMRMLVirtualRealityTransformWidget.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include "vtkRenderer.h"

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerGUIWidgetsModuleWidgetPrivate: public Ui_qSlicerGUIWidgetsModuleWidget
{
public:
  qSlicerGUIWidgetsModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerGUIWidgetsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModuleWidgetPrivate::qSlicerGUIWidgetsModuleWidgetPrivate()
{
}


//-----------------------------------------------------------------------------
// qSlicerGUIWidgetsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModuleWidget::qSlicerGUIWidgetsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerGUIWidgetsModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerGUIWidgetsModuleWidget::~qSlicerGUIWidgetsModuleWidget()
{
  this->GUIWidgetsMap.clear();
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::setup()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QObject::connect(d->AddHelloWorldGUIWidgetNodeButton, SIGNAL(clicked()), this, SLOT(onAddHelloWorldNodeClicked()));
  QObject::connect(d->UpdateButtonLabelButton, SIGNAL(clicked()), this, SLOT(onUpdateButtonLabelButtonClicked()));

  QObject::connect(d->AddHomeWidgetButton, SIGNAL(clicked()), this, SLOT(onAddHomeWidgetButtonClicked()));
  QObject::connect(d->AddDataModuleWidgetButton, SIGNAL(clicked()), this, SLOT(onAddDataModuleWidgetButtonClicked()));
  QObject::connect(d->AddSegmentEditorWidgetButton, SIGNAL(clicked()), this, SLOT(onAddSegmentEditorWidgetButtonClicked()));
  QObject::connect(d->AddTransformWidgetButton, SIGNAL(clicked()), this, SLOT(onAddTransformWidgetButtonClicked()));
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::setWidgetToGUIWidgetMarkupsNode(vtkMRMLGUIWidgetNode* node, QWidget* widget)
{
  if (!node)
  {
    return;
  }

  node->SetWidget((void*)widget);

  this->GUIWidgetsMap[node] = widget;
}

//-----------------------------------------------------------------------------
QWidget* qSlicerGUIWidgetsModuleWidget::onAddHelloWorldNodeClicked()
{
  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("TestButtonWidgetNode");

  QPushButton* newButton = new QPushButton("Hello world!");
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, newButton);

  return newButton;
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onUpdateButtonLabelButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  // Get last widget
  QWidget* lastWidget = this->GUIWidgetsMap.last();

  QPushButton* button = qobject_cast<QPushButton*>(lastWidget);
  if (button)
  {
    button->setText(d->NewLabelLineEdit->text());
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Widget is not a push button";
  }
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddHomeWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("HomeWidgetNode");

  qMRMLVirtualRealityHomeWidget* widget = new qMRMLVirtualRealityHomeWidget();
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddDataModuleWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("DataModuleWidgetNode");

  qMRMLVirtualRealityDataModuleWidget* widget = new qMRMLVirtualRealityDataModuleWidget();
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddSegmentEditorWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("SegmentEditorWidgetNode");

  qMRMLVirtualRealitySegmentEditorWidget* widget = new qMRMLVirtualRealitySegmentEditorWidget();
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}

//-----------------------------------------------------------------------------
void qSlicerGUIWidgetsModuleWidget::onAddTransformWidgetButtonClicked()
{
  Q_D(qSlicerGUIWidgetsModuleWidget);

  qSlicerApplication* app = qSlicerApplication::application();
  vtkMRMLGUIWidgetNode* widgetNode = vtkMRMLGUIWidgetNode::SafeDownCast(
    app->mrmlScene()->AddNewNodeByClass("vtkMRMLGUIWidgetNode") );
  widgetNode->SetName("TransformWidgetNode");

  vtkSlicerVirtualRealityLogic* vrLogic =
    vtkSlicerVirtualRealityLogic::SafeDownCast(app->applicationLogic()->GetModuleLogic("VirtualReality"));
  if (!vrLogic)
  {
    qCritical() << Q_FUNC_INFO << " : invalid VR logic";
    return;
  }
 
  qMRMLVirtualRealityTransformWidget* widget = new qMRMLVirtualRealityTransformWidget(vrLogic->GetVirtualRealityViewNode());
  widget->setMRMLScene(app->mrmlScene());
  this->setWidgetToGUIWidgetMarkupsNode(widgetNode, widget);
}
