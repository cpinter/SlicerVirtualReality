/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLVirtualRealityView_p_h
#define __qMRMLVirtualRealityView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// VTK includes
#include <vtkEventData.h>
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLVirtualRealityView.h"

// Qt includes
#include <QTimer>

class vtkInteractorStyle3D;
class vtkLightCollection;
class vtkObject;
class vtkTimerLog;

class vtkMRMLCameraNode;
class vtkMRMLDisplayableManagerGroup;
//class vtkMRMLViewInteractorStyle;
class vtkMRMLTransformNode;
class vtkMRMLVirtualRealityViewNode;
//class vtkVRInteractorStyle;
class vtkVRRenderWindowInteractor;

namespace vr
{
  struct TrackedDevicePose_t;
}

//-----------------------------------------------------------------------------
class qMRMLVirtualRealityViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLVirtualRealityView);
protected:
  qMRMLVirtualRealityView* const q_ptr;
public:
  qMRMLVirtualRealityViewPrivate(qMRMLVirtualRealityView& object);
  ~qMRMLVirtualRealityViewPrivate();

  virtual void init();

  void startVirtualReality();
  void stopVirtualReality();

  double desiredUpdateRate();
  double stillUpdateRate();

public slots:
  void updateWidgetFromMRML();
  void doOpenVirtualReality();

protected:
  void updateTransformNodeWithControllerPose(vtkEventDataDevice device);
  void updateTransformNodeWithHMDPose();
  void updateTransformNodesWithTrackerPoses();
  void updateTransformNodeWithPose(vtkMRMLTransformNode* node, vr::TrackedDevicePose_t* tdPose);
  void createRenderWindow();
  void destroyRenderWindow();

  vtkSlicerCamerasModuleLogic* CamerasLogic;

  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagerGroup;
  vtkWeakPointer<vtkMRMLVirtualRealityViewNode> MRMLVirtualRealityViewNode;
  vtkSmartPointer<vtkLightCollection> Lights;

  vtkSmartPointer<vtkVRRenderer> Renderer;
  vtkSmartPointer<vtkVRRenderWindow> RenderWindow;
  vtkSmartPointer<vtkVRRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkInteractorStyle3D> InteractorStyle;
  vtkSmartPointer<vtkVRCamera> Camera;

  vtkSmartPointer<vtkTimerLog> LastViewUpdateTime;
  double LastViewDirection[3];
  double LastViewUp[3];
  double LastViewPosition[3];

  QTimer VirtualRealityLoopTimer;
};

#endif
