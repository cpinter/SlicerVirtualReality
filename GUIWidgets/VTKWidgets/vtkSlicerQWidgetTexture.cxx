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

#include "vtkSlicerQWidgetTexture.h"

// SlicerQt includes
#include "qMRMLUtils.h"
#include "qSlicerCoreApplication.h"

// VTK includes
#include <vtkObjectFactory.h>

// Qt includes
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QImage>
#include <QWidget>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerQWidgetTexture);

//------------------------------------------------------------------------------
vtkSlicerQWidgetTexture::vtkSlicerQWidgetTexture()
{
  // The scene is created once and reused for the lifetime of this texture (instead of being
  // recreated every time a widget is set), so that widgets can be cleanly detached and
  // re-embedded across repeated show/hide cycles (see SetWidget).
  this->Scene = new QGraphicsScene();
  this->Widget = nullptr;

  this->UpdateTextureMethod = [this]() {
    if (!this->Widget)
    {
      return;
    }
    QImage grabImage(this->Widget->grab().toImage());
    qMRMLUtils::qImageToVtkImageData(grabImage, this->TextureImageData.GetPointer());
    this->Modified();
  };

  QObject::connect(this->Scene, &QGraphicsScene::changed, this->UpdateTextureMethod);
}

//------------------------------------------------------------------------------
vtkSlicerQWidgetTexture::~vtkSlicerQWidgetTexture()
{
  this->SetWidget(nullptr);
  delete this->Scene;
  this->Scene = nullptr;
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetTexture::ReleaseGraphicsResources(vtkWindow* win)
{
  this->Superclass::ReleaseGraphicsResources(win);
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetTexture::SetWidget(QWidget* w)
{
  if (this->Widget == w)
  {
    return;
  }

  if (this->Widget)
  {
    QObject::disconnect(this->WidgetObjectNameChangedConnection);

    // Detach the previous widget from its graphics proxy (and delete the proxy). This is
    // required (not just removing it from the scene) because QGraphicsProxyWidget::setWidget()
    // refuses to re-embed a widget that still reports a non-null graphicsProxyWidget(), which
    // would otherwise silently break rendering and event handling for the widget the next time
    // it is shown (e.g. after a hide/show cycle).
    QGraphicsProxyWidget* proxy = this->Widget->graphicsProxyWidget();
    if (proxy)
    {
      this->Scene->removeItem(proxy);
      delete proxy;
    }
  }

  this->Widget = w;

  this->SetupWidget();

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkSlicerQWidgetTexture::SetupWidget()
{
  if (!this->Widget)
  {
    return;
  }

  this->Widget->move(0, 0);
  this->Scene->addWidget(this->Widget);

  this->WidgetObjectNameChangedConnection =
    QObject::connect(this->Widget, &QObject::objectNameChanged, this->UpdateTextureMethod); //TODO: Workaround, see vtkSlicerQWidgetRepresentation::OnTextureModified

  if (this->TextureImageData.GetPointer() == nullptr)
  {
    this->TextureImageData = vtkSmartPointer<vtkImageData>::New();
  }
  if (this->TextureTrivialProducer.GetPointer() == nullptr)
  {
    this->TextureTrivialProducer = vtkSmartPointer<vtkTrivialProducer>::New();
    this->TextureTrivialProducer->SetOutput(this->TextureImageData);
    this->SetInputConnection(this->TextureTrivialProducer->GetOutputPort());
  }

  this->UpdateTextureMethod();
}
