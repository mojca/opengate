/*----------------------
   GATE version name: gate_v6

   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See GATE/LICENSE.txt for further details
----------------------*/


/*! \file
  \brief Implementation of GateImageRegionalizedSubVolume
 */

#include "GateImageRegionalizedSubVolume.hh"
#include "GateImageRegionalizedSubVolumeMessenger.hh"
#include "GateImageRegionalizedSubVolumeSolid.hh"
//#include "GateMiscFunctions.hh"
#include "GateDetectorConstruction.hh"
#include "GateMessageManager.hh"

//#include "G4LogicalVolumeStore.hh"
#include "G4PVPlacement.hh"

//====================================================================
/// Constructor with :
/// the path to the volume to create (for commands)
/// the name of the volume to create
/// Creates the messenger associated to the volume
GateImageRegionalizedSubVolume::GateImageRegionalizedSubVolume(const G4String& name,
								     G4bool acceptsChildren,
								     G4int depth)
  : GateVVolume(name,acceptsChildren,depth)
{
  GateMessageInc("Volume",5,"GateImageRegionalizedSubVolume() - begin"<<G4endl);

  //  pImage=0;
  mLabel = -1;
  //  mHalfSize = G4ThreeVector(0,0,0);

  // messenger
  pMessenger = new GateImageRegionalizedSubVolumeMessenger(this);

 // EnableSmartVoxelOptimisation(false);

  GateMessageDec("Volume",5,"GateImageRegionalizedSubVolume() - end"<<G4endl);
}
//====================================================================

//====================================================================
/// Destructor
GateImageRegionalizedSubVolume::~GateImageRegionalizedSubVolume()
{
  GateMessageInc("Volume",5,"~GateImageRegionalizedSubVolume - begin"<<G4endl);
  if (pMessenger) delete pMessenger;
  GateMessageDec("Volume",5,"~GateImageRegionalizedSubVolume - end"<<G4endl);
}
//====================================================================

//====================================================================
// Construct
G4LogicalVolume* GateImageRegionalizedSubVolume::ConstructOwnSolidAndLogicalVolume(G4Material* mater,
										      G4bool /*flagUpdateOnly*/)
{
  G4String boxname = GetObjectName() + "_solid";
  pBoxSolid = new GateImageRegionalizedSubVolumeSolid(boxname,this);
  pBoxLog = new G4LogicalVolume(pBoxSolid, mater, GetLogicalVolumeName());
  //LoadDistanceMap();

  // AddPhysVolToOptimizedNavigator(GetPhysicalVolume());
  return pBoxLog;
}
//====================================================================

//====================================================================
/// Constructs the solid
//void GateImageRegionalizedSubVolume::ConstructSolid()
//{
  ////GateMessage("Volume",1,"Begin GateImageRegionalizedSubVolume::ConstructSolid()" << G4endl);
  ////GateMessage("Volume",2,"name <"<<GetObjectName()<<">"<<G4endl);

  //=============================
  //G4String solName = GetObjectName() + "_solid";
  //SetSolid ( new GateImageRegionalizedSubVolumeSolid(solName,Gate) );
  ////GateMessage("Volume",3," * solid : <"<<solName<<">"<<G4endl);
  //=============================

  ////GateMessage("Volume",1,"END OF GateImageRegionalizedSubVolume::Construct()" << G4endl);
//}
//====================================================================

//==========================================================================
/*void GateImageRegionalizedSubVolume::PrintInfo()
{
 // GateVVolume::PrintInfo();
  //  pImage->PrintInfo();
  ////GateMessage("Volume",1,"\t Label = " << GetLabel() << G4endl);
}*/
//==========================================================================
