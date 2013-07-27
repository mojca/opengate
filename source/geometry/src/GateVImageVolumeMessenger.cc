/*----------------------
   GATE version name: gate_v6

   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See GATE/LICENSE.txt for further details
----------------------*/


/*! \file
  \brief Implementation of GateVImageVolumeMessenger
 */
#include "GateVImageVolumeMessenger.hh"
#include "GateVImageVolume.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWithABool.hh"

//---------------------------------------------------------------------------
GateVImageVolumeMessenger::GateVImageVolumeMessenger(GateVImageVolume* volume)
  :
  GateVolumeMessenger(volume),
  pVImageVolume(volume)
{

  GateMessage("Volume",5,"GateVImageVolumeMessenger("<<G4endl);
  G4String dir = GetDirectoryName() + "geometry";
  //  G4cout<<dir<<G4endl;

  G4String n = dir +"/SetImage";
  pImageFileNameCmd = 0;
  pImageFileNameCmd = new G4UIcmdWithAString(n,this);
  pImageFileNameCmd->SetGuidance("Sets the name of the image file");

  // Disable this macro for the moment
  // n = dir +"/SetLabelToMaterialFile";
  //   pLabelToMaterialFileNameCmd = new G4UIcmdWithAString(n,this);
  //   pLabelToMaterialFileNameCmd->SetGuidance("Sets the name of the file containing the label to material correspondence");

  n = dir +"/SetHUToMaterialFile";
  pHUToMaterialFileNameCmd = 0;
  pHUToMaterialFileNameCmd = new G4UIcmdWithAString(n,this);
  pHUToMaterialFileNameCmd->SetGuidance("Sets the name of the file containing the HU intervals to material correspondence");

  n = dir +"/TranslateTheImageAtThisIsoCenter";
  pIsoCenterCmd = 0;
  pIsoCenterCmd = new G4UIcmdWith3VectorAndUnit(n,this);
  pIsoCenterCmd->SetGuidance("translate the image so that its center is at the given isocenter coordinate (given according to the 3D image coordinate, in mm)");

  n = dir +"/setOrigin";
  pSetOriginCmd = 0;
  pSetOriginCmd = new G4UIcmdWith3VectorAndUnit(n,this);
  pSetOriginCmd->SetGuidance("Set the image origin (like in dicom).");

  n = dir +"/buildAndDumpDistanceTransfo";
  pBuildDistanceTransfoCmd = 0;
  pBuildDistanceTransfoCmd = new G4UIcmdWithAString(n,this);
  pBuildDistanceTransfoCmd->SetGuidance("Build and dump the distance transfo into the given filename.");

  n = dir +"/buildAndDumpLabeledImage";
  pBuildLabeledImageCmd = 0;
  pBuildLabeledImageCmd = new G4UIcmdWithAString(n,this);
  pBuildLabeledImageCmd->SetGuidance("Build and dump the image labeled according to the materials list. Give the filename.");

  n = dir +"/enableBoundingBoxOnly";
  pDoNotBuildVoxelsCmd = 0;
  pDoNotBuildVoxelsCmd = new G4UIcmdWithABool(n,this);
  pDoNotBuildVoxelsCmd->SetGuidance("Only build the bounding box (no voxels !), for visualization purpose only.");
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
GateVImageVolumeMessenger::~GateVImageVolumeMessenger()
{
  GateMessage("Volume",5,"~GateVImageVolumeMessenger("<<G4endl);

  delete pImageFileNameCmd;
  //delete pLabelToMaterialFileNameCmd;
  delete pHUToMaterialFileNameCmd;
  delete pBuildDistanceTransfoCmd;
  delete pIsoCenterCmd;
  delete pSetOriginCmd;
  delete pBuildLabeledImageCmd;
  delete pDoNotBuildVoxelsCmd;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void GateVImageVolumeMessenger::SetNewValue(G4UIcommand* command,
					    G4String newValue)
{
  GateMessage("Volume",5,"GateVImageVolumeMessenger::SetNewValue "
	      << command->GetCommandPath()
	      << " newValue=" << newValue << G4endl);

  if (command == pImageFileNameCmd) {
    pVImageVolume->SetImageFilename(newValue);
  }
 // else if (command == pLabelToMaterialFileNameCmd) {
 //   pVImageVolume->SetLabelToMaterialTableFilename(newValue);
 // }
  else if (command == pHUToMaterialFileNameCmd) {
    pVImageVolume->SetHUToMaterialTableFilename(newValue);
  }
  else if (command == pIsoCenterCmd) {
    pVImageVolume->SetIsoCenter(pIsoCenterCmd->GetNew3VectorValue(newValue));
  }
  else if (command == pSetOriginCmd) {
    pVImageVolume->SetOriginByUser(pSetOriginCmd->GetNew3VectorValue(newValue));
  }
  else if (command == pBuildDistanceTransfoCmd) {
    pVImageVolume->SetBuildDistanceTransfoFilename(newValue);
  }
  else if (command == pBuildLabeledImageCmd) {
    pVImageVolume->SetLabeledImageFilename(newValue);
  }
  else if (command == pDoNotBuildVoxelsCmd) {
    pVImageVolume->EnableBoundingBoxOnly(pDoNotBuildVoxelsCmd->GetNewBoolValue(newValue));
  }
  // It is necessary to call GateVolumeMessenger::SetNewValue if the command
  // is not recognized
  else {
    GateVolumeMessenger::SetNewValue(command,newValue);
  }
}
//---------------------------------------------------------------------------
