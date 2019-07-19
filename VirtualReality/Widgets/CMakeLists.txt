project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
find_package(VTK COMPONENTS vtkRenderingOpenVR REQUIRED)

#-----------------------------------------------------------------------------
set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerVirtualRealityModuleMRML_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${qSlicerSubjectHierarchyModuleWidgets_INCLUDE_DIRS}
  ${qSlicerSegmentationsModuleWidgets_INCLUDE_DIRS}
  ${qSlicerSegmentationsEditorEffects_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qMRMLVirtualRealityView.cxx
  qMRMLVirtualRealityView_p.h
  qMRMLVirtualRealityView.h
  qMRMLVirtualRealityHomeWidget.cxx
  qMRMLVirtualRealityHomeWidget.h
  qMRMLVirtualRealityDataModuleWidget.cxx
  qMRMLVirtualRealityDataModuleWidget.h
  qMRMLVirtualRealitySegmentEditorWidget.cxx
  qMRMLVirtualRealitySegmentEditorWidget.h
  )

set(${KIT}_MOC_SRCS
  qMRMLVirtualRealityView.h
  qMRMLVirtualRealityView_p.h
  qMRMLVirtualRealityHomeWidget.h
  qMRMLVirtualRealityDataModuleWidget.h
  qMRMLVirtualRealitySegmentEditorWidget.h 
  )

set(${KIT}_UI_SRCS
  Resources/UI/qMRMLVirtualRealityHomeWidget.ui
  Resources/UI/qMRMLVirtualRealityDataModuleWidget.ui
  Resources/UI/qMRMLVirtualRealitySegmentEditorWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/${KIT}.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerCamerasModuleLogic
  ${VTK_LIBRARIES}
  qSlicerSubjectHierarchyModuleWidgets
  qSlicerSegmentationsModuleWidgets

  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
#add_subdirectory(DesignerPlugins)

# Testing
if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif()

