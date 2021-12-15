import os
import unittest
import logging
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin
import numpy as np
import time

#------------------------------------------------------------------------------
#
# PointerSimulator
#
#------------------------------------------------------------------------------
class PointerSimulator(ScriptedLoadableModule):
  
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "PointerSimulator"
    self.parent.categories = ["Virtual Reality"]
    self.parent.dependencies = []
    self.parent.contributors = ["David Garcia Mato (Ebatinca, S.L.)"]
    self.parent.helpText = """ """
    self.parent.acknowledgementText = """ """

#------------------------------------------------------------------------------
#
# PointerSimulatorWidget
#
#------------------------------------------------------------------------------
class PointerSimulatorWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
  
  def __init__(self, parent=None):
    """
    Called when the user opens the module the first time and the widget is initialized.
    """
    ScriptedLoadableModuleWidget.__init__(self, parent)
    VTKObservationMixin.__init__(self)  # needed for parameter node observation
    self.logic = None
    self._parameterNode = None
    self._updatingGUIFromParameterNode = False

    self.clickState = False

  def setup(self):
    """
    Called when the user opens the module the first time and the widget is initialized.
    """
    ScriptedLoadableModuleWidget.setup(self)

    # Load widget from .ui file (created by Qt Designer).
    # Additional widgets can be instantiated manually and added to self.layout.
    uiWidget = slicer.util.loadUI(self.resourcePath('UI/PointerSimulator.ui'))
    self.layout.addWidget(uiWidget)
    self.ui = slicer.util.childWidgetVariables(uiWidget)

    # Set scene in MRML widgets. Make sure that in Qt designer the top-level qMRMLWidget's
    # "mrmlSceneChanged(vtkMRMLScene*)" signal in is connected to each MRML widget's.
    # "setMRMLScene(vtkMRMLScene*)" slot.
    uiWidget.setMRMLScene(slicer.mrmlScene)

    # Create logic class. Logic implements all computations that should be possible to run
    # in batch mode, without a graphical user interface.
    self.logic = PointerSimulatorLogic()

    # Connections

    # These connections ensure that we update parameter node when scene is closed
    self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
    self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndCloseEvent, self.onSceneEndClose)

    # These connections ensure that whenever user changes some settings on the GUI, that is saved in the MRML scene
    # (in the selected parameter node).
    self.ui.clickButton.clicked.connect(self.onClickButtonClicked)

    # Make sure parameter node is initialized (needed for module reload)
    self.initializeParameterNode()

    # Create pointer model
    origin = [0.0, -100.0, 0.0]
    direction = [0.0, 1.0, 0.0]
    length = 200.0
    self.logic.updatePointerModel(origin, direction, length)

    # Create pointer transform
    self.logic.updatePointerTransform(0.0,0.0,0.0)

    # Set pointer transform
    self.ui.translationSliders.setMRMLTransformNode(self.logic.pointerTransform)

  def cleanup(self):
    """
    Called when the application closes and the module widget is destroyed.
    """
    self.removeObservers()

  def enter(self):
    """
    Called each time the user opens this module.
    """
    # Make sure parameter node exists and observed
    self.initializeParameterNode()

  def exit(self):
    """
    Called each time the user opens a different module.
    """
    # Do not react to parameter node changes (GUI wlil be updated when the user enters into the module)
    self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromParameterNode)

  def onSceneStartClose(self, caller, event):
    """
    Called just before the scene is closed.
    """
    # Parameter node will be reset, do not use it anymore
    self.setParameterNode(None)

  def onSceneEndClose(self, caller, event):
    """
    Called just after the scene is closed.
    """
    # If this module is shown while the scene is closed then recreate a new parameter node immediately
    if self.parent.isEntered:
      self.initializeParameterNode()

  def initializeParameterNode(self):
    """
    Ensure parameter node exists and observed.
    """
    # Parameter node stores all user choices in parameter values, node selections, etc.
    # so that when the scene is saved and reloaded, these settings are restored.

    self.setParameterNode(self.logic.getParameterNode())

    # Select default input nodes if nothing is selected yet to save a few clicks for the user
    if not self._parameterNode.GetNodeReference("InputVolume"):
      firstVolumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
      if firstVolumeNode:
        self._parameterNode.SetNodeReferenceID("InputVolume", firstVolumeNode.GetID())

  def setParameterNode(self, inputParameterNode):
    """
    Set and observe parameter node.
    Observation is needed because when the parameter node is changed then the GUI must be updated immediately.
    """

    if inputParameterNode:
      self.logic.setDefaultParameters(inputParameterNode)

    # Unobserve previously selected parameter node and add an observer to the newly selected.
    # Changes of parameter node are observed so that whenever parameters are changed by a script or any other module
    # those are reflected immediately in the GUI.
    if self._parameterNode is not None:
      self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromParameterNode)
    self._parameterNode = inputParameterNode
    if self._parameterNode is not None:
      self.addObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.updateGUIFromParameterNode)

    # Initial GUI update
    self.updateGUIFromParameterNode()

  def updateGUIFromParameterNode(self, caller=None, event=None):
    """
    This method is called whenever parameter node is changed.
    The module GUI is updated to show the current state of the parameter node.
    """

    if self._parameterNode is None or self._updatingGUIFromParameterNode:
      return

    # Make sure GUI changes do not call updateParameterNodeFromGUI (it could cause infinite loop)
    self._updatingGUIFromParameterNode = True

    # All the GUI updates are done
    self._updatingGUIFromParameterNode = False

  def updateParameterNodeFromGUI(self, caller=None, event=None):
    """
    This method is called when the user makes any change in the GUI.
    The changes are saved into the parameter node (so that they are restored when the scene is saved and loaded).
    """

    if self._parameterNode is None or self._updatingGUIFromParameterNode:
      return

    wasModified = self._parameterNode.StartModify()  # Modify all properties in a single batch

    self._parameterNode.EndModify(wasModified)

  def onClickButtonClicked(self):
    print('onClickButtonClicked')

    print('Checked: ', self.ui.clickButton.checked)

    if self.ui.clickButton.checked:  
      self.clickState = True
      self.ui.clickButton.setText('Clicked')
      self.logic.updatePointerState(True)
    else:
      self.clickState = False
      self.ui.clickButton.setText('Click')
      self.logic.updatePointerState(False)
    
#------------------------------------------------------------------------------
#
# PointerSimulatorLogic
#
#------------------------------------------------------------------------------
class PointerSimulatorLogic(ScriptedLoadableModuleLogic):
  
  def __init__(self):
    """
    Called when the logic class is instantiated. Can be used for initializing member variables.
    """
    ScriptedLoadableModuleLogic.__init__(self)

    # Pointer model
    self.pointerModel = None
    self.pointerLineSource = None
    self.pointerRadius = 1.0

    # Pointer transform
    self.pointerTransform = None

  def setDefaultParameters(self, parameterNode):
    """
    Initialize parameter node with default settings.
    """
    if not parameterNode.GetParameter("TranslateRL"):
      parameterNode.SetParameter("TranslateRL", "0.0")
    if not parameterNode.GetParameter("TranslateAP"):
      parameterNode.SetParameter("TranslateAP", "0.0")
    if not parameterNode.GetParameter("TranslateSI"):
      parameterNode.SetParameter("TranslateSI", "0.0")

  def updatePointerModel(self, origin, direction, length):

    # Get/create pointer model
    if not self.pointerModel:
      # Line source
      self.pointerLineSource = vtk.vtkLineSource()
      # Tube filter
      tubeFilter = vtk.vtkTubeFilter()
      tubeFilter.SetInputConnection(self.pointerLineSource.GetOutputPort())
      tubeFilter.SetRadius(self.pointerRadius) #Default is 0.5
      tubeFilter.SetNumberOfSides(50)
      tubeFilter.Update()
      # Sphere source
      self.startSphereSource = vtk.vtkSphereSource()
      self.startSphereSource.SetRadius(self.pointerRadius * 5)
      self.endSphereSource = vtk.vtkSphereSource()
      self.endSphereSource.SetRadius(self.pointerRadius * 2)
      # Append poly data
      appendFilter = vtk.vtkAppendPolyData()
      appendFilter.AddInputConnection(tubeFilter.GetOutputPort())
      appendFilter.AddInputConnection(self.startSphereSource.GetOutputPort())
      appendFilter.AddInputConnection(self.endSphereSource.GetOutputPort())
      # Create a mapper and actor
      pointerMapper = vtk.vtkPolyDataMapper()
      pointerMapper.SetInputConnection(appendFilter.GetOutputPort())
      pointerActor = vtk.vtkActor()
      pointerActor.SetMapper(pointerMapper)
      # Create model node
      self.pointerModel = slicer.vtkMRMLModelNode()
      self.pointerModel.SetName('PointerModel')
      self.pointerModel.SetPolyDataConnection(appendFilter.GetOutputPort())
      slicer.mrmlScene.AddNode(self.pointerModel)
      self.pointerModelDisplay = slicer.vtkMRMLModelDisplayNode()
      self.pointerModelDisplay.SetSliceIntersectionVisibility(True)
      self.pointerModelDisplay.SetColor(1,0,0)
      self.pointerModelDisplay.SetOpacity(1.0)
      slicer.mrmlScene.AddNode(self.pointerModelDisplay)
      self.pointerModel.SetAndObserveDisplayNodeID(self.pointerModelDisplay.GetID())
      # Attribute
      self.pointerModel.SetAttribute('Clicked', str(False))

    # Update pointer model properties
    vtk.vtkMath().Normalize(direction)
    startPoint = np.array(origin)
    endPoint = startPoint + np.array(direction) * length
    self.pointerLineSource.SetPoint1(startPoint)
    self.pointerLineSource.SetPoint2(endPoint)
    self.startSphereSource.SetCenter(startPoint)
    self.endSphereSource.SetCenter(endPoint)      

  def updatePointerTransform(self, translationRL, translationAP, translationSI):

    # Create pointer transform if it does not exist
    if not self.pointerTransform:
      self.pointerTransform=slicer.vtkMRMLLinearTransformNode() 
      self.pointerTransform.SetName("PointerTransform")   
      slicer.mrmlScene.AddNode(self.pointerTransform)
      if self.pointerModel:
        self.pointerModel.SetAndObserveTransformNodeID(self.pointerTransform.GetID())
      else:
        logging.error('Pointer model was not found.')
    
    # Update transformation matrix
    transform = vtk.vtkTransform()
    transform.Translate(translationRL, translationAP, translationSI)

    # Set transform
    self.pointerTransform.SetMatrixTransformToParent(transform.GetMatrix())

  def updatePointerState(self, active):
    activeColor = (0.0,1.0,0.0)
    inactiveColor = (1.0,0.0,0.0)
    if self.pointerModel:
      if active:
        print('Setting active color...')
        self.pointerModel.SetAttribute('Clicked', str(active))
        self.pointerModel.GetDisplayNode().SetColor(activeColor)
        self.pointerModel.GetDisplayNode().SetVisibility(False)
        self.pointerModel.GetDisplayNode().SetVisibility(True)
      else:
        print('Setting inactive color...')
        self.pointerModel.SetAttribute('Clicked', str(active))
        self.pointerModel.GetDisplayNode().SetColor(inactiveColor)
        self.pointerModel.GetDisplayNode().SetVisibility(False)
        self.pointerModel.GetDisplayNode().SetVisibility(True)
    

#------------------------------------------------------------------------------
#
# PointerSimulatorTest
#
#------------------------------------------------------------------------------
class PointerSimulatorTest(ScriptedLoadableModuleTest):
  
  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    pass