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

// Need to be included before qMRMLVirtualRealityView_p

// OpenVR includes
#include <vtkOpenVRCamera.h>
#include <vtkOpenVRModel.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderer.h>

// OpenXR includes
#include "vtkOpenXRRenderWindow.h"
#include "vtkOpenXRRenderWindowInteractor.h"
#include "vtkOpenXRRenderer.h"
#include "vtkOpenXRInteractorStyle.h"

// VirtualReality includes
#include <vtkVirtualRealityViewInteractorStyle.h>
//#include <vtkOpenVRInteractorStyle.h> //TODO: For debugging the original interactor
#include <vtkVirtualRealityViewInteractor.h>
//#include <vtkOpenVRRenderWindowInteractor.h> //TODO: For debugging the original interactor

#include "qMRMLVirtualRealityView_p.h"

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QTimer>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "vtkSlicerCamerasModuleLogic.h"

// VirtualReality includes
#include "vtkMRMLVirtualRealityViewNode.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLVirtualRealityViewDisplayableManagerFactory.h>

#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkCullerCollection.h>
#include <vtkLight.h>
#include <vtkLightCollection.h>
#include <vtkNew.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>
#include <vtkTransform.h>

namespace
{
  //--------------------------------------------------------------------------
  std::string PoseStatusToString(vr::ETrackingResult result)
  {
    switch (result)
    {
      case vr::TrackingResult_Calibrating_InProgress:
        return "CalibratingInProgress";
      case vr::TrackingResult_Calibrating_OutOfRange:
        return "CalibratingOutOfRange";
      case vr::TrackingResult_Running_OK:
        return "RunningOk";
      case vr::TrackingResult_Running_OutOfRange:
        return "RunningOutOfRange";
      case vr::TrackingResult_Uninitialized:
      default:
        return "Uninitialized";
    }
  }
}

//--------------------------------------------------------------------------
// qMRMLVirtualRealityViewPrivate methods

//---------------------------------------------------------------------------
qMRMLVirtualRealityViewPrivate::qMRMLVirtualRealityViewPrivate(qMRMLVirtualRealityView& object)
  : q_ptr(&object)
  , CamerasLogic(nullptr)
{
  this->MRMLVirtualRealityViewNode = nullptr;
}

//---------------------------------------------------------------------------
qMRMLVirtualRealityViewPrivate::~qMRMLVirtualRealityViewPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::init()
{
  QObject::connect(&this->VirtualRealityLoopTimer, SIGNAL(timeout()), this, SLOT(doOpenVirtualReality()));
}

//----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLVirtualRealityView, vtkSlicerCamerasModuleLogic*, setCamerasLogic, CamerasLogic);
CTK_GET_CPP(qMRMLVirtualRealityView, vtkSlicerCamerasModuleLogic*, camerasLogic, CamerasLogic);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkVRRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkVRRenderWindow*, renderWindow, RenderWindow);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLVirtualRealityView, vtkVRRenderWindowInteractor*, interactor, Interactor);

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::createRenderWindow()
{
  Q_Q(qMRMLVirtualRealityView);

  this->LastViewUpdateTime = vtkSmartPointer<vtkTimerLog>::New();
  this->LastViewUpdateTime->StartTimer();
  this->LastViewUpdateTime->StopTimer();
  this->LastViewDirection[0] = 0.0;
  this->LastViewDirection[1] = 0.0;
  this->LastViewDirection[2] = 1.0;
  this->LastViewUp[0] = 0.0;
  this->LastViewUp[1] = 1.0;
  this->LastViewUp[2] = 0.0;
  this->LastViewPosition[0] = 0.0;
  this->LastViewPosition[1] = 0.0;
  this->LastViewPosition[2] = 0.0;

  if (this->MRMLVirtualRealityViewNode && this->MRMLVirtualRealityViewNode->GetUseOpenXR())
  {
    this->RenderWindow = vtkSmartPointer<vtkOpenXRRenderWindow>::New();
    this->Renderer = vtkSmartPointer<vtkOpenXRRenderer>::New();
    this->Interactor = vtkSmartPointer<vtkOpenXRRenderWindowInteractor>::New();
    this->InteractorStyle = vtkSmartPointer<vtkOpenXRInteractorStyle>::New();
  }
  else
  {
    this->RenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();
    this->Renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();
    this->Interactor = vtkSmartPointer<vtkVirtualRealityViewInteractor>::New();
    //this->Interactor = vtkSmartPointer<vtkOpenVRRenderWindowInteractor>::New(); //TODO: For debugging the original interactor
    this->InteractorStyle = vtkSmartPointer<vtkVirtualRealityViewInteractorStyle>::New();
    //this->InteractorStyle = vtkSmartPointer<vtkOpenVRInteractorStyle>::New(); //TODO: For debugging the original interactor
  }

  this->Interactor->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->SetInteractor(this->Interactor);
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->Camera = vtkSmartPointer<vtkOpenVRCamera>::New();
  this->Renderer->SetActiveCamera(this->Camera);

  this->RenderWindow->SetMultiSamples(0);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetInteractor(this->Interactor);
  // Set default 10x magnification (conversion: PhysicalScale = 1000 / Magnification)
  this->RenderWindow->SetPhysicalScale(100.0);
  // Observe VR render window event
  qvtkReconnect(this->RenderWindow, vtkOpenVRRenderWindow::PhysicalToWorldMatrixModified,
                q, SLOT(onPhysicalToWorldMatrixModified()));

  // Observe button press event
  qvtkReconnect(this->Interactor, vtkCommand::Button3DEvent, q, SLOT(onButton3DEvent(vtkObject*,void*,unsigned long,void*)));

  vtkMRMLVirtualRealityViewDisplayableManagerFactory* factory
    = vtkMRMLVirtualRealityViewDisplayableManagerFactory::GetInstance();

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access application logic";
    return;
  }
  factory->SetMRMLApplicationLogic(appLogic);

  QStringList displayableManagers;
  displayableManagers //<< "vtkMRMLCameraDisplayableManager"
  //<< "vtkMRMLViewDisplayableManager"
      << "vtkMRMLModelDisplayableManager"
      << "vtkMRMLThreeDReformatDisplayableManager"
      << "vtkMRMLCrosshairDisplayableManager3D"
      //<< "vtkMRMLOrientationMarkerDisplayableManager" // Not supported in VR
      //<< "vtkMRMLRulerDisplayableManager" // Not supported in VR
      //<< "vtkMRMLAnnotationDisplayableManager" // Not supported in VR
      << "vtkMRMLMarkupsDisplayableManager"
      << "vtkMRMLSegmentationsDisplayableManager3D"
      << "vtkMRMLTransformsDisplayableManager3D"
      << "vtkMRMLLinearTransformsDisplayableManager3D"
      << "vtkMRMLVolumeRenderingDisplayableManager"
      ;
  foreach (const QString& displayableManager, displayableManagers)
  {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
    {
      factory->RegisterDisplayableManager(displayableManager.toLatin1());
    }
  }

  this->DisplayableManagerGroup = vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(
                                    factory->InstantiateDisplayableManagers(q->renderer()));
  this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLVirtualRealityViewNode);

  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
    vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
  if (vrInteractorStyle)
  {
    vrInteractorStyle->SetDisplayableManagers(this->DisplayableManagerGroup);
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }

  qDebug() << Q_FUNC_INFO << ": Number of registered displayable manager:" << this->DisplayableManagerGroup->GetDisplayableManagerCount();

  ///CONFIGURATION OF THE OPENVR ENVIRONEMENT

  this->Renderer->RemoveCuller(this->Renderer->GetCullers()->GetLastItem());
  this->Renderer->SetBackground(0.7, 0.7, 0.7);

  // Create 4 lights for even lighting
  // without this, one side of models may be very dark.
  this->Lights = vtkSmartPointer<vtkLightCollection>::New();
  {
    vtkNew<vtkLight> light;
    light->SetPosition(0.0, 1.0, 0.0);
    light->SetIntensity(1.0);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(0.8, -0.2, 0.0);
    light->SetIntensity(0.8);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(-0.3, -0.2, 0.7);
    light->SetIntensity(0.6);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  {
    vtkNew<vtkLight> light;
    light->SetPosition(-0.3, -0.2, -0.7);
    light->SetIntensity(0.4);
    light->SetLightTypeToSceneLight();
    this->Lights->AddItem(light);
  }
  this->Renderer->SetLightCollection(this->Lights);

  q->updateViewFromReferenceViewCamera();

  this->RenderWindow->Initialize();
  vtkOpenVRRenderWindow* openVrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
  if (openVrRenderWindow && !openVrRenderWindow->GetHMD())
  {
    qWarning() << Q_FUNC_INFO << ": Failed to initialize OpenVR RenderWindow";
    return;
  }
  //TODO: OpenXR
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::destroyRenderWindow()
{
  Q_Q(qMRMLVirtualRealityView);
  this->VirtualRealityLoopTimer.stop();
  // Must break the connection between interactor and render window,
  // otherwise they would circularly refer to each other and would not
  // be deleted.
  this->Interactor->SetRenderWindow(nullptr);
  this->Interactor = nullptr;
  this->InteractorStyle = nullptr;
  this->DisplayableManagerGroup = nullptr;
  this->Renderer = nullptr;
  this->Camera = nullptr;
  this->Lights = nullptr;
  this->RenderWindow = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLVirtualRealityView);
  if (!this->MRMLVirtualRealityViewNode || !this->MRMLVirtualRealityViewNode->GetVisibility())
  {
    if (this->RenderWindow != nullptr)
    {
      this->destroyRenderWindow();
    }
    if (this->MRMLVirtualRealityViewNode)
    {
      this->MRMLVirtualRealityViewNode->ClearError();
    }
    return;
  }

  if (!this->RenderWindow)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    this->createRenderWindow();
    QApplication::restoreOverrideCursor();
    if (!q->isHardwareConnected())
    {
      this->MRMLVirtualRealityViewNode->SetError("Connection failed");
      return;
    }
    this->MRMLVirtualRealityViewNode->ClearError();
  }

  if (this->DisplayableManagerGroup->GetMRMLDisplayableNode() != this->MRMLVirtualRealityViewNode.GetPointer())
  {
    this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLVirtualRealityViewNode);
  }

  // Renderer properties
  this->Renderer->SetGradientBackground(1);
  this->Renderer->SetBackground(this->MRMLVirtualRealityViewNode->GetBackgroundColor());
  this->Renderer->SetBackground2(this->MRMLVirtualRealityViewNode->GetBackgroundColor2());

  this->Renderer->SetTwoSidedLighting(this->MRMLVirtualRealityViewNode->GetTwoSidedLighting());

  bool switchOnAllLights = this->MRMLVirtualRealityViewNode->GetBackLights();
  for (int i = 2; i < this->Lights->GetNumberOfItems(); i++)
  {
    vtkLight* light = vtkLight::SafeDownCast(this->Lights->GetItemAsObject(i));
    light->SetSwitch(switchOnAllLights);
  }

  this->Renderer->SetUseDepthPeeling(this->MRMLVirtualRealityViewNode->GetUseDepthPeeling() != 0);
  this->Renderer->SetUseDepthPeelingForVolumes(this->MRMLVirtualRealityViewNode->GetUseDepthPeeling() != 0);

  // Render window properties
  if (this->RenderWindow)
  {
    // Desired update rate
    this->RenderWindow->SetDesiredUpdateRate(this->desiredUpdateRate());

    vtkOpenVRRenderWindow* openVrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
    if (openVrRenderWindow)
    {
      // Magnification
      double magnification = this->MRMLVirtualRealityViewNode->GetMagnification();
      if (magnification < 0.01)
      {
        magnification = 0.01;
      }
      else if (magnification > 100.0)
      {
        magnification = 100.0;
      }
      vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
        vtkVirtualRealityViewInteractorStyle::SafeDownCast(this->InteractorStyle);
      if (!vrInteractorStyle)
      {
        qCritical() << Q_FUNC_INFO << ": Invalid interactor style type";
        return;
      }
      vrInteractorStyle->SetMagnification(magnification);

      // Dolly physical speed
      double dollyPhysicalSpeedMps = this ->MRMLVirtualRealityViewNode->GetMotionSpeed();

      // 1.6666 m/s is walking speed (= 6 km/h), which is the default. We multiply it with the factor
      this->InteractorStyle->SetDollyPhysicalSpeed(dollyPhysicalSpeedMps);

      if (openVrRenderWindow->GetHMD())
      {
        vtkEventDataDevice deviceIdsToUpdate[] = { vtkEventDataDevice::RightController, vtkEventDataDevice::LeftController, vtkEventDataDevice::Unknown };
        for (int deviceIdIndex = 0; deviceIdsToUpdate[deviceIdIndex] != vtkEventDataDevice::Unknown; deviceIdIndex++)
        {
          vtkOpenVRModel* model = vtkOpenVRModel::SafeDownCast(this->RenderWindow->GetModelForDevice(deviceIdsToUpdate[deviceIdIndex]));
          if (!model)
          {
            continue;
          }
          model->SetVisibility(this->MRMLVirtualRealityViewNode->GetControllerModelsVisible());
        }

        // Update tracking reference visibility
        for (uint32_t deviceIdIndex = 0; deviceIdIndex < vr::k_unMaxTrackedDeviceCount; ++deviceIdIndex)
        {
          if (openVrRenderWindow->GetHMD()->GetTrackedDeviceClass(deviceIdIndex) == vr::TrackedDeviceClass_TrackingReference)
          {
            vtkOpenVRModel* model = vtkOpenVRModel::SafeDownCast(this->RenderWindow->GetModelForDevice(
              openVrRenderWindow->GetDeviceForOpenVRHandle(deviceIdIndex)));
            if (!model)
            {
              continue;
            }
            model->SetVisibility(this->MRMLVirtualRealityViewNode->GetLighthouseModelsVisible());
          }
        }
      }
    }
    else
    {
      //TODO: OpenXR
      qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
    }
  }

  if (this->MRMLVirtualRealityViewNode->GetActive())
  {
    this->VirtualRealityLoopTimer.start(0);
  }
  else
  {
    this->VirtualRealityLoopTimer.stop();
  }
}

//---------------------------------------------------------------------------
double qMRMLVirtualRealityViewPrivate::desiredUpdateRate()
{
  Q_Q(qMRMLVirtualRealityView);
  double rate = this->MRMLVirtualRealityViewNode->GetDesiredUpdateRate();

  // enforce non-zero frame rate to avoid division by zero errors
  const double defaultStaticViewUpdateRate = 0.0001;
  if (rate < defaultStaticViewUpdateRate)
  {
    rate = defaultStaticViewUpdateRate;
  }

  return rate;
}

//---------------------------------------------------------------------------
double qMRMLVirtualRealityViewPrivate::stillUpdateRate()
{
  Q_Q(qMRMLVirtualRealityView);
  return 0.0001;
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::doOpenVirtualReality()
{
  if (!this->Interactor || !this->RenderWindow || !this->Renderer)
  {
    return;
  }

  vtkOpenVRRenderWindow* openVrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
  if (openVrRenderWindow && openVrRenderWindow->GetHMD())
  {
    this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);

    this->LastViewUpdateTime->StopTimer();
    if (this->LastViewUpdateTime->GetElapsedTime() > 0.0)
    {
      bool quickViewMotion = true;

      if (this->MRMLVirtualRealityViewNode->GetMotionSensitivity() > 0.999)
      {
        quickViewMotion = true;
      }
      else if (this->MRMLVirtualRealityViewNode->GetMotionSensitivity() <= 0.001)
      {
        quickViewMotion = false;
      }
      else if (this->LastViewUpdateTime->GetElapsedTime() < 3.0) // don't consider stale measurements
      {
        // limit scale:
        // sensitivity = 0    -> limit = 10.0x
        // sensitivity = 50%  -> limit =  1.0x
        // sensitivity = 100% -> limit =  0.1x
        double limitScale = pow(100, 0.5 - this->MRMLVirtualRealityViewNode->GetMotionSensitivity());

        const double angularSpeedLimitRadiansPerSec = vtkMath::RadiansFromDegrees(5.0 * limitScale);
        double viewDirectionChangeSpeed = vtkMath::AngleBetweenVectors(this->LastViewDirection,
                                          this->Camera->GetViewPlaneNormal()) / this->LastViewUpdateTime->GetElapsedTime();
        double viewUpDirectionChangeSpeed = vtkMath::AngleBetweenVectors(this->LastViewUp,
                                            this->Camera->GetViewUp()) / this->LastViewUpdateTime->GetElapsedTime();

        const double translationSpeedLimitMmPerSec = 100.0 * limitScale;
        // Physical scale = 100 if virtual objects are real-world size; <100 if virtual objects are larger
        double viewTranslationSpeedMmPerSec = this->RenderWindow->GetPhysicalScale() * 0.01 *
                                              sqrt(vtkMath::Distance2BetweenPoints(this->LastViewPosition, this->Camera->GetPosition()))
                                              / this->LastViewUpdateTime->GetElapsedTime();

        if (viewDirectionChangeSpeed < angularSpeedLimitRadiansPerSec
            && viewUpDirectionChangeSpeed < angularSpeedLimitRadiansPerSec
            && viewTranslationSpeedMmPerSec  < translationSpeedLimitMmPerSec)
        {
          quickViewMotion = false;
        }
      }

      double updateRate = quickViewMotion ? this->desiredUpdateRate() : this->stillUpdateRate();
      this->RenderWindow->SetDesiredUpdateRate(updateRate);

      this->Camera->GetViewPlaneNormal(this->LastViewDirection);
      this->Camera->GetViewUp(this->LastViewUp);
      this->Camera->GetPosition(this->LastViewPosition);

      if (this->MRMLVirtualRealityViewNode->GetControllerTransformsUpdate())
      {
        this->MRMLVirtualRealityViewNode->CreateDefaultControllerTransformNodes();
        updateTransformNodeWithControllerPose(vtkEventDataDevice::LeftController);
        updateTransformNodeWithControllerPose(vtkEventDataDevice::RightController);
      }
      if (this->MRMLVirtualRealityViewNode->GetHMDTransformUpdate())
      {
        this->MRMLVirtualRealityViewNode->CreateDefaultHMDTransformNode();
        updateTransformNodeWithHMDPose();
      }
      if (this->MRMLVirtualRealityViewNode->GetTrackerTransformUpdate())
      {
        updateTransformNodesWithTrackerPoses();
      }

      this->LastViewUpdateTime->StartTimer();
    }
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

// --------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodeWithControllerPose(vtkEventDataDevice device)
{
  vtkMRMLLinearTransformNode* node = this->MRMLVirtualRealityViewNode->GetControllerTransformNode(device);

  if (node == nullptr)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to retrieve linear transform node for device: " << (int)device;
    return;
  }

  int disabledModify = node->StartModify();
  auto handle = this->RenderWindow->GetDeviceHandleForDevice(device);
  if (handle == vr::k_unTrackedDeviceIndexInvalid)
  {
    node->SetAttribute("VirtualReality.ControllerActive", "0");
    node->SetAttribute("VirtualReality.ControllerConnected", "0");
    node->EndModify(disabledModify);
    return;
  }

  vr::TrackedDevicePose_t* tdPose;

  vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(this->Interactor);
  if (openVrInteractor)
  {
    openVrInteractor->GetTrackedDevicePose(device, &tdPose);
    if (tdPose == nullptr)
    {
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve pose associated with VR tracker with ID: " << (int)device;
    }
    if (tdPose == nullptr || tdPose->eTrackingResult != vr::TrackingResult_Running_OK)
    {
      node->SetAttribute("VirtualReality.ControllerActive", "0");
    }
    else
    {
      node->SetAttribute("VirtualReality.ControllerActive", "1");
    }

    if (tdPose == nullptr || !tdPose->bDeviceIsConnected)
    {
      node->SetAttribute("VirtualReality.ControllerConnected", "0");
    }
    else
    {
      node->SetAttribute("VirtualReality.ControllerConnected", "1");
    }
    updateTransformNodeWithPose(node, tdPose);

    node->EndModify(disabledModify);
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodeWithHMDPose()
{
  vtkMRMLLinearTransformNode* node = this->MRMLVirtualRealityViewNode->GetHMDTransformNode();

  if (node == nullptr)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to retrieve linear transform node for HMD";
    return;
  }

  int disabledModify = node->StartModify();
  if (this->RenderWindow->GetDeviceHandleForDevice(vtkEventDataDevice::HeadMountedDisplay) == vr::k_unTrackedDeviceIndexInvalid)
  {
    node->SetAttribute("VirtualReality.HMDActive", "0");
    node->EndModify(disabledModify);
    return;
  }

  vr::TrackedDevicePose_t* tdPose;

  vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(this->Interactor);
  if (openVrInteractor)
  {
    openVrInteractor->GetTrackedDevicePose(vtkEventDataDevice::HeadMountedDisplay, &tdPose);

    if (tdPose == nullptr)
    {
      qCritical() << Q_FUNC_INFO << ": Unable to retrieve HMD pose";
    }
    if (tdPose == nullptr || tdPose->eTrackingResult != vr::TrackingResult_Running_OK)
    {
      node->SetAttribute("VirtualReality.HMDActive", "0");
    }
    else
    {
      node->SetAttribute("VirtualReality.HMDActive", "1");
    }

    updateTransformNodeWithPose(node, tdPose);

    node->EndModify(disabledModify);
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodesWithTrackerPoses()
{
  Q_Q(qMRMLVirtualRealityView);
  for (uint32_t i = 0; i < this->RenderWindow->GetNumberOfDeviceHandlesForDevice(vtkEventDataDevice::GenericTracker); ++i)
  {
    vr::TrackedDeviceIndex_t dev = this->RenderWindow->GetDeviceHandleForDevice(vtkEventDataDevice::GenericTracker, i);
    std::stringstream ss;
    ss << dev;

    vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast(this->MRMLVirtualRealityViewNode->GetTrackerTransformNode(ss.str().c_str()));
    if (node == nullptr)
    {
      // Node wasn't found for this device, let's create one
      node = vtkMRMLLinearTransformNode::SafeDownCast(this->MRMLVirtualRealityViewNode->GetScene()->AddNode(vtkMRMLLinearTransformNode::New()));
      if (node == nullptr)
      {
        qCritical() << Q_FUNC_INFO << ": Unable to add transform node to scene. Can't update VR tracker with ID: " << dev;
        continue;
      }
      node->SetAttribute("VirtualReality.VRDeviceID", ss.str().c_str());
      node->SetName("VirtualReality.GenericTracker");
      this->MRMLVirtualRealityViewNode->SetAndObserveTrackerTransformNode(node, ss.str().c_str());
    }

    int disabledModify = node->StartModify();

    // Now, we have our generic tracker node, let's update it!
    vr::TrackedDevicePose_t* tdPose;

    vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(this->Interactor);
    if (openVrInteractor)
    {
      openVrInteractor->GetTrackedDevicePose(vtkEventDataDevice::GenericTracker, dev, &tdPose);

      if (tdPose == nullptr)
      {
        qCritical() << Q_FUNC_INFO << ": Unable to retrieve pose associated with VR tracker with ID: " << dev;
      }
      if (tdPose == nullptr || tdPose->eTrackingResult != vr::TrackingResult_Running_OK)
      {
        node->SetAttribute("VirtualReality.TrackerActive", "0");
      }
      else
      {
        node->SetAttribute("VirtualReality.TrackerActive", "1");
      }
      updateTransformNodeWithPose(node, tdPose);

      node->EndModify(disabledModify);
    }
    else
    {
      //TODO: OpenXR
      qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
    }
  }
}


//----------------------------------------------------------------------------
void qMRMLVirtualRealityViewPrivate::updateTransformNodeWithPose(vtkMRMLTransformNode* node, vr::TrackedDevicePose_t* tdPose)
{
  if (tdPose == nullptr)
  {
    node->SetAttribute("VirtualReality.PoseValid", "False");
    node->SetAttribute("VirtualReality.PoseStatus", "Uninitialized");
    return;
  }
  double pos[3] = { 0. };
  double ppos[3] = { 0. };
  double wxyz[4] = { 1., 0., 0., 0. };
  double wdir[3] = { 1., 0., 0. };

  vtkNew<vtkMatrix4x4> pose;

  vtkOpenVRRenderWindow* openVrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(this->RenderWindow);
  if (openVrRenderWindow)
  {
    openVrRenderWindow->SetMatrixFromOpenVRPose(pose, *tdPose);
    this->Interactor->ConvertPoseToWorldCoordinates(pose, pos, wxyz, ppos, wdir);
    vtkNew<vtkTransform> transform;
    transform->Translate(pos);
    transform->RotateWXYZ(wxyz[0], wxyz[1], wxyz[2], wxyz[3]);
    if (node != nullptr)
    {
      node->SetMatrixTransformToParent(transform->GetMatrix());
    }
    node->SetAttribute("VirtualReality.PoseValid", tdPose->bPoseIsValid ? "True" : "False");
    node->SetAttribute("VirtualReality.PoseStatus", PoseStatusToString(tdPose->eTrackingResult).c_str());
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}


// --------------------------------------------------------------------------
// qMRMLVirtualRealityView methods

// --------------------------------------------------------------------------
qMRMLVirtualRealityView::qMRMLVirtualRealityView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLVirtualRealityViewPrivate(*this))
{
  Q_D(qMRMLVirtualRealityView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLVirtualRealityView::~qMRMLVirtualRealityView()
{
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLVirtualRealityView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toLatin1()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setMRMLVirtualRealityViewNode(vtkMRMLVirtualRealityViewNode* newViewNode)
{
  Q_D(qMRMLVirtualRealityView);
  if (d->MRMLVirtualRealityViewNode == newViewNode)
  {
    return;
  }

  d->qvtkReconnect(
    d->MRMLVirtualRealityViewNode, newViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  d->MRMLVirtualRealityViewNode = newViewNode;

  d->updateWidgetFromMRML();

  // Enable/disable widget
  this->setEnabled(newViewNode != nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLVirtualRealityViewNode* qMRMLVirtualRealityView::mrmlVirtualRealityViewNode()const
{
  Q_D(const qMRMLVirtualRealityView);
  return d->MRMLVirtualRealityViewNode;
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::getDisplayableManagers(vtkCollection* displayableManagers)
{
  Q_D(qMRMLVirtualRealityView);

  if (!displayableManagers || !d->DisplayableManagerGroup)
  {
    return;
  }
  int num = d->DisplayableManagerGroup->GetDisplayableManagerCount();
  for (int n = 0; n < num; n++)
  {
    displayableManagers->AddItem(d->DisplayableManagerGroup->GetNthDisplayableManager(n));
  }
}

//------------------------------------------------------------------------------
bool qMRMLVirtualRealityView::isHardwareConnected()const
{
  vtkVRRenderWindow* renWin = this->renderWindow();
  if (!renWin)
  {
    return false;
  }
  vtkOpenVRRenderWindow* openVrRenderWindow = vtkOpenVRRenderWindow::SafeDownCast(renWin);
  if (openVrRenderWindow)
  {
    if (!openVrRenderWindow->GetHMD())
    {
      return false;
    }
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
  
  // connected successfully
  return true;
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGrabObjectsEnabled(bool enable)
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
    vtkVirtualRealityViewInteractorStyle::SafeDownCast(d->InteractorStyle);
  if (vrInteractorStyle)
  {
    if (enable)
    {
      vrInteractorStyle->GrabEnabledOn();
    }
    else
    {
      vrInteractorStyle->GrabEnabledOff();
    }
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//------------------------------------------------------------------------------
bool qMRMLVirtualRealityView::isGrabObjectsEnabled()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
    vtkVirtualRealityViewInteractorStyle::SafeDownCast(d->InteractorStyle);
  if (vrInteractorStyle)
  {
    return vrInteractorStyle->GetGrabEnabled() != 0;
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
    return false;
  }
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setDolly3DEnabled(bool enable)
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
    vtkVirtualRealityViewInteractorStyle::SafeDownCast(d->InteractorStyle);
  if (vrInteractorStyle)
  {
    if (enable)
    {
      vrInteractorStyle->MapInputToAction(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::TrackPad, VTKIS_DOLLY);
    }
    else
    {
      vrInteractorStyle->MapInputToAction(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::TrackPad, VTKIS_NONE);
    }
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//------------------------------------------------------------------------------
bool qMRMLVirtualRealityView::isDolly3DEnabled()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
    vtkVirtualRealityViewInteractorStyle::SafeDownCast(d->InteractorStyle);
  if (vrInteractorStyle)
  {
    return vrInteractorStyle->GetMappedAction(vtkEventDataDevice::RightController, vtkEventDataDeviceInput::TrackPad) == VTKIS_DOLLY;
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
    return false;
  }
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToTrigger()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(d->Interactor);
  if (openVrInteractor)
  {
    openVrInteractor->SetGestureButtonToTrigger();
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToGrip()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(d->Interactor);
  if (openVrInteractor)
  {
    openVrInteractor->SetGestureButtonToGrip();
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToTriggerAndGrip()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(d->Interactor);
  if (openVrInteractor)
  {
    openVrInteractor->SetGestureButtonToTriggerAndGrip();
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::setGestureButtonToNone()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractor* openVrInteractor = vtkVirtualRealityViewInteractor::SafeDownCast(d->Interactor);
  if (openVrInteractor)
  {
    openVrInteractor->SetGestureButtonToNone();
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::onPhysicalToWorldMatrixModified()
{
  Q_D(qMRMLVirtualRealityView);

  vtkVirtualRealityViewInteractorStyle* vrInteractorStyle =
    vtkVirtualRealityViewInteractorStyle::SafeDownCast(d->InteractorStyle);
  if (vrInteractorStyle)
  {
    d->MRMLVirtualRealityViewNode->SetMagnification(vrInteractorStyle->GetMagnification());
  }
  else
  {
    //TODO: OpenXR
    qWarning() << Q_FUNC_INFO << ": OpenXR implementation not yet available";
  }

  emit physicalToWorldMatrixModified();
}

//------------------------------------------------------------------------------
void qMRMLVirtualRealityView::onButton3DEvent(vtkObject* caller, void* call_data, unsigned long vtk_event, void* client_data)
{
  Q_UNUSED(caller);
  Q_UNUSED(vtk_event);
  Q_UNUSED(client_data);
  Q_D(qMRMLVirtualRealityView);

  vtkEventDataDevice3D * ed = reinterpret_cast<vtkEventDataDevice3D*>(call_data);

  if(ed->GetInput() == vtkEventDataDeviceInput::Trigger)
  {
    if(ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit leftControllerTriggerPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit leftControllerTriggerReleased();
      }
    }
    else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit rightControllerTriggerPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit rightControllerTriggerReleased();
      }
    }
  }
  else if (ed->GetInput() == vtkEventDataDeviceInput::Grip)
  {
    if (ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit leftControllerGripPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit leftControllerGripReleased();
      }
    }
    else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit rightControllerGripPressed();
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit rightControllerGripReleased();
      }
    }
  }
  else if (ed->GetInput() == vtkEventDataDeviceInput::TrackPad)
  {
    if (ed->GetDevice() == vtkEventDataDevice::LeftController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit leftControllerTrackpadPressed(ed->GetTrackPadPosition()[0],ed->GetTrackPadPosition()[1]);
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit leftControllerTrackpadReleased(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
    }
    else if (ed->GetDevice() == vtkEventDataDevice::RightController)
    {
      if (ed->GetAction() == vtkEventDataAction::Press)
      {
        emit rightControllerTrackpadPressed(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
      else if (ed->GetAction() == vtkEventDataAction::Release)
      {
        emit rightControllerTrackpadReleased(ed->GetTrackPadPosition()[0], ed->GetTrackPadPosition()[1]);
      }
    }
  }
}

//---------------------------------------------------------------------------
void qMRMLVirtualRealityView::updateViewFromReferenceViewCamera()
{
  Q_D(qMRMLVirtualRealityView);
  if (!d->MRMLVirtualRealityViewNode)
  {
    return;
  }
  vtkMRMLViewNode* refrenceViewNode = d->MRMLVirtualRealityViewNode->GetReferenceViewNode();
  if (!refrenceViewNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: no reference view node is set";
    return;
  }
  if (!d->CamerasLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: cameras module logic is not set";
    return;
  }
  vtkMRMLCameraNode* cameraNode = d->CamerasLogic->GetViewActiveCameraNode(refrenceViewNode);
  if (!cameraNode || !cameraNode->GetCamera())
  {
    qWarning() << Q_FUNC_INFO << " failed: camera node is not found";
    return;
  }
  if (!d->RenderWindow)
  {
    qWarning() << Q_FUNC_INFO << " failed: RenderWindow has not been created";
    return;
  }

  // The following is based on d->RenderWindow->InitializeViewFromCamera(sourceCamera),
  // but that is not usable for us, as it puts the headset in the focal point (so we
  // need to step back to see the full content) and snaps view direction to the closest axis.

  vtkCamera* sourceCamera = cameraNode->GetCamera();

  vtkRenderer* ren = static_cast<vtkRenderer*>(d->RenderWindow->GetRenderers()->GetItemAsObject(0));
  if (!ren)
  {
    qWarning() << Q_FUNC_INFO << ": The renderer must be set prior to calling InitializeViewFromCamera";
    return;
  }
  vtkOpenVRCamera* cam = vtkOpenVRCamera::SafeDownCast(ren->GetActiveCamera());
  if (!cam)
  {
    qWarning() << Q_FUNC_INFO << ": The renderer's active camera must be set prior to calling InitializeViewFromCamera";
    return;
  }

  double newPhysicalScale = 100.0; // Default 10x magnification

  double* sourceViewUp = sourceCamera->GetViewUp();
  cam->SetViewUp(sourceViewUp);
  d->RenderWindow->SetPhysicalViewUp(sourceViewUp);

  double* sourcePosition = sourceCamera->GetPosition();
  double* viewUp = cam->GetViewUp();
  cam->SetFocalPoint(sourcePosition);
  d->RenderWindow->SetPhysicalTranslation(
    viewUp[0] * newPhysicalScale - sourcePosition[0],
    viewUp[1] * newPhysicalScale - sourcePosition[1],
    viewUp[2] * newPhysicalScale - sourcePosition[2]);

  double* sourceDirectionOfProjection = sourceCamera->GetDirectionOfProjection();
  d->RenderWindow->SetPhysicalViewDirection(sourceDirectionOfProjection);
  double* idop = d->RenderWindow->GetPhysicalViewDirection();
  cam->SetPosition(
    -idop[0] * newPhysicalScale + sourcePosition[0],
    -idop[1] * newPhysicalScale + sourcePosition[1],
    -idop[2] * newPhysicalScale + sourcePosition[2]);

  d->RenderWindow->SetPhysicalScale(newPhysicalScale);

  ren->ResetCameraClippingRange();
}
