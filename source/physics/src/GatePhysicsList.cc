/*----------------------
  GATE version name: gate_v6

  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See GATE/LICENSE.txt for further details
  ----------------------*/



#ifndef GATEPHYSICSLIST_CC
#define GATEPHYSICSLIST_CC

#include "G4ParticleDefinition.hh"
#include "G4ParticleWithCuts.hh"
#include "G4ProcessManager.hh"
#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4IonConstructor.hh"
#include "G4ShortLivedConstructor.hh"
#include "G4UserLimits.hh"
#include "G4Region.hh"
#include "G4RegionStore.hh"
#include "G4RegionStore.hh"
#include "G4UserLimits.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4ParticleTable.hh"

#include "GatePhysicsList.hh"
#include "GateUserLimits.hh"
#include "GateConfiguration.h"
#include "GatePhysicsListMessenger.hh"

#ifdef GATE_USE_OPTICAL
#include "G4OpticalPhoton.hh"
#endif
//-----------------------------------------------------------------------------------------
GatePhysicsList::GatePhysicsList(): G4VUserPhysicsList()
{

  // default cut value  (1.0mm)
  defaultCutValue = 1.0*mm;
  // default cut value  (1.0mm)
  ParticleCutType worldCuts;
  worldCuts.gammaCut = -1;
  worldCuts.electronCut = -1;
  worldCuts.positronCut = -1;
  worldCuts.protonCut = -1;
  mapOfRegionCuts["DefaultRegionForTheWorld"] = worldCuts;
  //SetVerboseLevel(2);

  mLoadState=0;

  mDEDXBinning=-1;
  mLambdaBinning=-1;
  mEmin=-1;
  mEmax=-1;
  mSplineFlag=true;

  userlimits=0;

  G4double limit=250*eV; // limit for diplay production cuts table
  G4ProductionCutsTable::GetProductionCutsTable()->SetEnergyRange(limit, 100.*GeV);

  pMessenger = new GatePhysicsListMessenger(this);
  pMessenger->BuildCommands("/gate/physics");

  opt = new G4EmProcessOptions();


}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
GatePhysicsList::~GatePhysicsList()
{

  delete pMessenger;

  for(VolumeUserLimitsMapType::iterator i = mapOfVolumeUserLimits.begin(); i!=mapOfVolumeUserLimits.end(); i++)
    {
      delete (*i).second;
      mapOfVolumeUserLimits.erase(i);
    }

  delete userlimits;

  for(std::list<G4ProductionCuts*>::iterator i = theListOfCuts.begin(); i!=theListOfCuts.end(); i++)
    {
      delete (*i);
      i = theListOfCuts.erase(i);
    }


  mapOfRegionCuts.clear();
  theListOfPBName.clear();
  mListOfStepLimiter.clear();
  mListOfG4UserSpecialCut.clear();

  GateVProcess::Delete();


  delete opt;


  // delete the transportation process (should be done in ~G4VUserPhysicsList())
  bool isTransportationDelete = false;
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4ParticleTable::G4PTblDicIterator * theParticleIterator = theParticleTable->GetIterator();
  theParticleIterator->reset();
  while( (*theParticleIterator)() ){//&& !isTransportationDelete){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessVector * vect = particle->GetProcessManager()->GetProcessList();
    for(int i = 0; i<vect->size();i++)
      {
        if((*vect)[i]->GetProcessName()=="Transportation" )//&& !isTransportationDelete)
          {
            if(!isTransportationDelete) delete (*vect)[i];
            isTransportationDelete = true;
            (*vect)[i]=0;
          }
        /*else {
          if( (*vect)[i] ){
          if((*vect)[i]->GetProcessName()=="Decay" ){
          G4cout<<"test  "<<particle->GetParticleName()<<"   "<<(*vect)[i]<<G4endl;
          delete (*vect)[i];
	  }
          }
          }*/
        //else if((*vect)[i]) delete (*vect)[i];
        //(*vect)[i]=0;

      }
  }
  // Transportation process deleted

  /*theParticleIterator->reset();
    while( (*theParticleIterator)()){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessVector * vect = particle->GetProcessManager()->GetProcessList();
    G4cout<<"Particle= "<< particle->GetParticleName() <<G4endl;

    for(int i = 0; i<vect->size();i++)
    {
    if((*vect)[i]) G4cout<<"Process= "<<(*vect)[i]->GetProcessName()<<G4endl;
    }
    }*/


}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void GatePhysicsList::ConstructProcess()
{
  GateMessage("Physic",2,"GatePhysicsList::ConstructProcess " << G4endl);
  GateMessage("Physic",3,"mLoadState = " << mLoadState << G4endl);
  GateMessage("Physic",3,"mListOfStepLimiter.size = " << mListOfStepLimiter.size() << G4endl);

  if(mLoadState==0)
    {
      AddTransportation();

      for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++)
	{
	  theListOfPBName.push_back((*GetTheListOfProcesss())[i]->GetG4ProcessName());
	  for(unsigned int j=0; j<GetTheListOfProcesss()->size(); j++)
	    if(i != j && (*GetTheListOfProcesss())[i]->GetG4ProcessName()==(*GetTheListOfProcesss())[j]->GetG4ProcessName() )
	      GateWarning("Some processes have the same name: "
			  <<(*GetTheListOfProcesss())[i]->GetG4ProcessName() );
	}
    }
  else if(mLoadState==1)
    {
      for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++)
        (*GetTheListOfProcesss())[i]->ConstructProcess();

      //opt->SetVerbose(2);
      if(mDEDXBinning>0)   opt->SetDEDXBinning(mDEDXBinning);
      if(mLambdaBinning>0) opt->SetLambdaBinning(mLambdaBinning);
      if(mEmin>0)          opt->SetMinEnergy(mEmin);
      if(mEmax>0)          opt->SetMaxEnergy(mEmax);
      opt->SetSplineFlag(mSplineFlag);
      //opt->SetStepFunction(0.3,0.077);

    }
  else GateMessage("Physic",1,"GatePhysicsList::Construct() -- Warning: processes already defined!" << G4endl);

  //SetCuts();

  if(mLoadState>0) DefineCuts();

  if(mLoadState==1 && mListOfStepLimiter.size()!=0){
    G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
    G4ParticleTable::G4PTblDicIterator * theParticleIterator = theParticleTable->GetIterator();
    theParticleIterator->reset();
    while( (*theParticleIterator)() ){
      G4ParticleDefinition* particle = theParticleIterator->value();
      G4ProcessManager* pmanager = particle->GetProcessManager();
      G4String particleName = particle->GetParticleName();
      for(unsigned int i=0; i<mListOfStepLimiter.size(); i++) {
	if(mListOfStepLimiter[i]==particleName) {
          GateMessage("Cuts", 3, "Activate G4StepLimiter for " << particleName << G4endl);
          pmanager->AddProcess(new G4StepLimiter, -1,-1,3);
        }
      }
    }
  }


  if(mLoadState==1 && mListOfG4UserSpecialCut.size()!=0){
    G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
    G4ParticleTable::G4PTblDicIterator * theParticleIterator = theParticleTable->GetIterator();
    theParticleIterator->reset();
    while( (*theParticleIterator)() ){
      G4ParticleDefinition* particle = theParticleIterator->value();
      G4ProcessManager* pmanager = particle->GetProcessManager();
      G4String particleName = particle->GetParticleName();
      for(unsigned int i=0; i<mListOfG4UserSpecialCut.size(); i++) {
	if(mListOfG4UserSpecialCut[i]==particleName) {
          GateMessage("Cuts", 3, "Activate G4UserSpecialCuts for " << particleName << G4endl);
          pmanager-> AddProcess(new G4UserSpecialCuts,   -1,-1,4);
        }
      }
    }
  }


  mLoadState++;

}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
void GatePhysicsList::ConstructParticle()
{

  // Construct all bosons
  G4BosonConstructor boson;
  boson.ConstructParticle();

  // Construct all leptons
  G4LeptonConstructor lepton;
  lepton.ConstructParticle();

  //  Construct all mesons
  G4MesonConstructor meson;
  meson.ConstructParticle();

  //  Construct all barions
  G4BaryonConstructor barion;
  barion.ConstructParticle();

  //  Construct light ions
  G4IonConstructor ion;
  ion.ConstructParticle();

  //  Construct  resonaces and quarks
  G4ShortLivedConstructor slive;
  slive.ConstructParticle();

  //#ifdef GATE_USE_OPTICAL
  //G4OpticalPhoton::OpticalPhotonDefinition();
  //#endif
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void GatePhysicsList::Print(G4String type, G4String particlename)
{

  if(type=="Initialized") std::cout<<"\n\nList of initialized processes:\n"<<std::endl;
  else if(type=="Enabled") std::cout<<"\n\nList of Enabled processes:\n"<<std::endl;
  else if(type=="Available") std::cout<<"\n\nList of Available processes:\n"<<std::endl;

  if(type=="Enabled")
    {
      if(particlename != "All") std::cout<<"   ***  "<<particlename<<"  ***"<<std::endl;
      for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++)
	(*GetTheListOfProcesss())[i]->PrintEnabledParticles(particlename);

      std::cout<<"\n"<<std::endl;
    }

  if(type=="Initialized")
    {
      Print(particlename);
      std::cout<<"\n"<<std::endl;
    }

  if(type=="Available")
    {
      std::vector<G4String> DefaultParticles;
      std::vector<G4String> DataSets;
      std::vector<G4String> Models;

      for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++)
	{
	  DefaultParticles = (*GetTheListOfProcesss())[i]->GetTheListOfDefaultParticles();
	  DataSets = (*GetTheListOfProcesss())[i]->GetTheListOfDataSets();
	  Models = (*GetTheListOfProcesss())[i]->GetTheListOfModels();
	  if((*GetTheListOfProcesss())[i]->GetProcessInfo()!="")
	    std::cout<<"  * "<<(*GetTheListOfProcesss())[i]->GetG4ProcessName()<<" ("<<(*GetTheListOfProcesss())[i]->GetProcessInfo()<<")"<<std::endl;
	  else std::cout<<"  * "<<(*GetTheListOfProcesss())[i]->GetG4ProcessName()<<std::endl;

	  if(DefaultParticles.size() > 1) std::cout<<"     - Default particles: "<<std::endl;
	  else if(DefaultParticles.size() == 1) std::cout<<"     - Default particle: "<<std::endl;
	  for(unsigned int i1=0; i1<DefaultParticles.size(); i1++)
	    {
	      std::cout<<"        + "<<DefaultParticles[i1]<<std::endl;
	    }

	  if(Models.size() > 1) std::cout<<"     - Models: "<<std::endl;
	  else if(Models.size() == 1) std::cout<<"     - Model: "<<std::endl;
	  for(unsigned int i1=0; i1<Models.size(); i1++)
	    {
	      std::cout<<"        + "<<Models[i1]<<std::endl;
	    }

	  if(DataSets.size() > 1) std::cout<<"     - DataSets: "<<std::endl;
	  if(DataSets.size() == 1) std::cout<<"     - DataSet: "<<std::endl;
	  for(unsigned int i1=0; i1<DataSets.size(); i1++)
	    {
	      std::cout<<"        + "<<DataSets[i1]<<std::endl;
	    }
	  std::cout<<std::endl;
	}
      std::cout<<"\n"<<std::endl;
    }

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void GatePhysicsList::Print(G4String name)
{
  G4ParticleDefinition* particle=0;
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4ProcessManager* manager = 0;
  G4ProcessVector * processvector = 0;
  G4ParticleTable::G4PTblDicIterator * theParticleIterator;

  int iDisp = 0;

  if(name=="All")
    {
      theParticleIterator = theParticleTable->GetIterator();
      theParticleIterator -> reset();
      while( (*theParticleIterator)() ) {
	particle = theParticleIterator->value();
	manager  = particle->GetProcessManager();
	processvector = manager->GetProcessList();
	if(manager->GetProcessListLength()==0) continue;
	if(manager->GetProcessListLength()==1 && (*processvector)[0]->GetProcessName()== "Transportation") continue;
	// Transportation process is ignored for display;
	std::cout<<"  * "<<particle->GetParticleName()<<std::endl;
	iDisp++;
	for(int j=0;j<manager->GetProcessListLength();j++)
	  {
	    if( (*processvector)[j]->GetProcessName() !=  "Transportation" )
	      std::cout<<"    - "<<(*processvector)[j]->GetProcessName()<<std::endl;
	  }
      }
    }
  else
    {
      G4ParticleDefinition* particle = theParticleTable->FindParticle(name);
      if(!particle)
	{
	  GateWarning("Unknown particle: "<<name );
	  return;
	}
      manager  = particle->GetProcessManager();
      processvector = manager->GetProcessList();
      if(manager->GetProcessListLength()==0) return;
      std::cout<<"Particle: "<<particle->GetParticleName()<<std::endl;
      for(int j=0;j<manager->GetProcessListLength();j++)
	if( (*processvector)[j]->GetProcessName() !=  "Transportation" )
	  std::cout<<"   - "<<(*processvector)[j]->GetProcessName()<<std::endl;
    }

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
std::vector<GateVProcess*> GatePhysicsList::FindProcess(G4String name)
{
  std::vector<GateVProcess*> thelist;

  for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++) {
    if((*GetTheListOfProcesss())[i]->GetG4ProcessName()==name) thelist.push_back((*GetTheListOfProcesss())[i]);
  }
  return thelist;
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void GatePhysicsList::AddProcesses(G4String processname, G4String particle)
{
  if ( processname == "UHadronElastic")
    G4Exception( "GatePhysicsList::AddProcesses","AddProcesses", FatalException,"####### WARNING: 'HadronElastic' process name replace 'UHadronElastic' process name since Geant4 9.5");

  std::vector<GateVProcess *>  process = FindProcess(processname);
  if(process.size()>0)
    for(unsigned int i=0; i<process.size(); i++) process[i]->CreateEnabledParticle(particle);
  else
    {
      GateWarning("Unknown process: "<<processname );
      return;
    }

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void GatePhysicsList::RemoveProcesses(G4String processname, G4String particle)
{

  std::vector<GateVProcess *>  process = FindProcess(processname);
  if(process.size()>0)
    for(unsigned int i=0; i<process.size(); i++) process[i]->RemoveElementOfParticleList(particle);
  else
    {
      GateWarning("Unknown process: "<<processname );;
      return;
    }

}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
/*
  This function is called just before the physics initialization.
  It checks if the fictitious process is activated and if so it forced
  all gamma processes to a standard configuration.
*/
void GatePhysicsList::PurgeIfFictitious()
{
  bool isFictitious = false;
  // We first check if fictitious process is activated
  for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++) {
    if ( (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "Fictitious" &&
         (*GetTheListOfProcesss())[i]->GetTheListOfEnabledParticles().size() != 0)
      {
        isFictitious=true;
        break;
      }
  }
  // If fictitious is activated, we alert the user that gamma processes are forced to:
  // --> Photoelectric: standard
  // --> Compton: standard
  // --> Rayleigh: inactive
  // --> GammaConvertion: inactive
  if (isFictitious) {
    G4cout << "Fictitious interactions are activated, so gamma processes are forced to:" << G4endl
           << "  --> PhotoElectric:   standard" << G4endl
           << "  --> Compton:         standard" << G4endl
           << "  --> Rayleigh:        inactive" << G4endl
           << "  --> GammaConversion: inactive" << G4endl;
    for(unsigned int i=0; i<(*GetTheListOfProcesss()).size(); i++) {
      if ( (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "LowEnergyRayleighScattering" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "PhotoElectric" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "LowEnergyPhotoElectric" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "GammaConversion" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "PenelopeGammaConversion" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "LowEnergyGammaConversion" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "PenelopeCompton" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "Compton" ||
           (*GetTheListOfProcesss())[i]->GetG4ProcessName() == "LowEnergyCompton"
           )
        {
          if ((*GetTheListOfProcesss())[i]->GetTheListOfEnabledParticles().size() != 0)
            {
              RemoveProcesses((*GetTheListOfProcesss())[i]->GetG4ProcessName(),"gamma");
            }
        }
    }
  }
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------
void GatePhysicsList::Write(G4String file)
{
  G4ParticleDefinition* particle=0;
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4ProcessManager* manager = 0;
  G4ProcessVector * processvector = 0;
  G4ParticleTable::G4PTblDicIterator * theParticleIterator;

  int iDisp = 0;

  std::ofstream os;
  os.open(file.data());
  if(mLoadState<2)  os<<"<!> *** Warning *** <!>  Processes not yet initialized!\n\n";

  os<<"List of particles with their associated processes\n\n";
  if(mLoadState<2)  os<<"<!> *** Warning *** <!>  Processes not yet initialized!\n\n";

  theParticleIterator = theParticleTable->GetIterator();
  theParticleIterator -> reset();
  while( (*theParticleIterator)() ) {
    particle = theParticleIterator->value();
    manager  = particle->GetProcessManager();
    processvector = manager->GetProcessList();
    if(manager->GetProcessListLength()==0) continue;
    if(manager->GetProcessListLength()==1 && (*processvector)[0]->GetProcessName()== "Transportation") continue;
    os    <<"  * "<<particle->GetParticleName().data()<<"\n";
    iDisp++;
    for(int j=0;j<manager->GetProcessListLength();j++)
      {
	if( (*processvector)[j]->GetProcessName() !=  "Transportation" )
	  os<<"    - "<<(*processvector)[j]->GetProcessName().data()<<"\n";
      }
  }
  os<<"\n\n-----------------------------------------------------------------------------\n\n";

  os<<"List of processes:\n\n";

  os.close();

  for(unsigned int i=0; i<GetTheListOfProcesss()->size(); i++)
    (*GetTheListOfProcesss())[i]->PrintEnabledParticlesToFile(file);
  os.open(file.data(), std::ios_base::app);
  os << std::endl;
  os.close();
}
//-----------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Cuts
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void GatePhysicsList::SetCuts()
{
  /* if (verboseLevel >0){
     std::cout << "GatePhysicsList::SetCuts: default cut length : "
     << G4BestUnit(defaultCutValue,"Length") << std::endl;
     }  */

  // These values are used as the default production thresholds
  // for the world volume.
  // SetCutsWithDefault();

  // This is needed to enable user cuts
  opt->SetApplyCuts(true);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::DefineCuts()
{
  // GateMessage("Cuts",4,"===================================" << G4endl);
  // GateMessage("Cuts",4,"GatePhysicsList::SetCuts() -- begin" << G4endl);

  //-----------------------------------------------------------------------------
  // Set defaults production cut for the world
  ParticleCutType worldCuts =  mapOfRegionCuts["DefaultRegionForTheWorld"];

  if(worldCuts.gammaCut == -1) worldCuts.gammaCut = defaultCutValue;
  if(worldCuts.electronCut == -1) worldCuts.electronCut = defaultCutValue;
  if(worldCuts.positronCut == -1) worldCuts.positronCut = defaultCutValue;
  if(worldCuts.protonCut == -1) worldCuts.protonCut = defaultCutValue;

  GateMessage("Cuts",3, "Set default production cuts (world) : "
              << worldCuts.gammaCut << " "
              << worldCuts.electronCut << " "
              << worldCuts.positronCut << " "
              << worldCuts.protonCut   << " mm" << G4endl);

  SetCutValue(worldCuts.gammaCut, "gamma","DefaultRegionForTheWorld");
  SetCutValue(worldCuts.electronCut, "e-","DefaultRegionForTheWorld");
  SetCutValue(worldCuts.positronCut, "e+","DefaultRegionForTheWorld");
  SetCutValue(worldCuts.protonCut, "proton","DefaultRegionForTheWorld");

  //-----------------------------------------------------------------------------
  // Set default production cut to other regions
  G4RegionStore * RegionStore = G4RegionStore::GetInstance();
  G4RegionStore::const_iterator pi = RegionStore->begin();
  G4RegionStore::const_iterator pe = RegionStore->end();
  while (pi != pe) {
    G4String regionName = (*pi)->GetName();

    if (regionName != "DefaultRegionForTheWorld" && regionName !="world") {
      RegionCutMapType::iterator current = mapOfRegionCuts.find(regionName);
      if (current == mapOfRegionCuts.end()) {
	// GateMessage("Cuts",5, " Cut not set for region " << regionName << " put -1" << G4endl);
	mapOfRegionCuts[regionName].gammaCut = -1;
	mapOfRegionCuts[regionName].electronCut = -1;
	mapOfRegionCuts[regionName].positronCut = -1;
	mapOfRegionCuts[regionName].protonCut = -1;
      }
    }
    ++pi;
  }

  //-----------------------------------------------------------------------------
  // Loop over regions with a defined cuts
  RegionCutMapType::iterator it = mapOfRegionCuts.begin();
  while (it != mapOfRegionCuts.end()) {
    // do not apply cut for the world region
    // GateMessage("Cuts", 5, "Region (*it).first : " << (*it).first<< G4endl);
    if (((*it).first != "DefaultRegionForTheWorld") && ((*it).first != "world")) {
      G4Region* region = RegionStore->GetRegion((*it).first);
      if (!region) {
	GateError( "The region '" << (*it).first << "' does not exist !");
      }
      // set default values
      ParticleCutType regionCuts =  (*it).second;

      G4Region* parentRegion =  region;

      while(regionCuts.gammaCut == -1)
        {
          G4bool unique;
          parentRegion =  parentRegion->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            ParticleCutType parentRegionCuts = mapOfRegionCuts[parentRegion->GetName()];
            regionCuts.gammaCut = parentRegionCuts.gammaCut;
          }
          else
            regionCuts.gammaCut = worldCuts.gammaCut;
        }

      parentRegion = region;
      while(regionCuts.electronCut == -1)
        {
          G4bool unique;
          parentRegion =  parentRegion->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            ParticleCutType parentRegionCuts = mapOfRegionCuts[parentRegion->GetName()];
            regionCuts.electronCut = parentRegionCuts.electronCut;
          }
          else
            regionCuts.electronCut = worldCuts.electronCut;
        }

      parentRegion = region;
      while(regionCuts.positronCut == -1)
        {
          G4bool unique;
          parentRegion =  parentRegion->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            ParticleCutType parentRegionCuts = mapOfRegionCuts[parentRegion->GetName()];
            regionCuts.positronCut = parentRegionCuts.positronCut;
          }
          else
            regionCuts.positronCut = worldCuts.positronCut;
        }
      parentRegion = region;
      while(regionCuts.protonCut == -1)
        {
          G4bool unique;
          parentRegion =  parentRegion->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            ParticleCutType parentRegionCuts = mapOfRegionCuts[parentRegion->GetName()];
            regionCuts.protonCut = parentRegionCuts.protonCut;
          }
          else
            regionCuts.protonCut = worldCuts.protonCut;
        }
      parentRegion = region;


      GateMessage("Cuts",3, "Set production cuts (g, e-, e+, p) for the region '"
                  << (*it).first << "' : "
                  << G4BestUnit(regionCuts.gammaCut, "Length") << " "
                  << G4BestUnit(regionCuts.electronCut, "Length") << " "
                  << G4BestUnit(regionCuts.positronCut, "Length") << " "
                  << G4BestUnit(regionCuts.protonCut, "Length")   << G4endl);

      // apply the cut
      /* G4ProductionCuts* cuts = region->GetProductionCuts();
         if (!cuts) cuts = new G4ProductionCuts;
         cuts->SetProductionCut(regionCuts.gammaCut, "gamma");
         cuts->SetProductionCut(regionCuts.electronCut, "e-");
         cuts->SetProductionCut(regionCuts.positronCut, "e+");
         region->SetProductionCuts(cuts);*/

      if(region->GetProductionCuts()) theListOfCuts.push_back(region->GetProductionCuts());
      else theListOfCuts.push_back(new G4ProductionCuts);
      theListOfCuts.back()->SetProductionCut(regionCuts.gammaCut, "gamma");
      theListOfCuts.back()->SetProductionCut(regionCuts.electronCut, "e-");
      theListOfCuts.back()->SetProductionCut(regionCuts.positronCut, "e+");
      theListOfCuts.back()->SetProductionCut(regionCuts.protonCut, "proton");
      region->SetProductionCuts(theListOfCuts.back());

      //G4ProductionCutsTable::GetProductionCutsTable()->UpdateCoupleTable(region->GetWorldPhysical());
      //modif Claire 31mars2011
    }
    ++it;
  } // end loop regions


  //-----------------------------------------------------------------------------
  // now set user limits
  //-----------------------------------------------------------------------------

  //G4LogicalVolumeStore * logicalVolumeStore = G4LogicalVolumeStore::GetInstance();

  GateUserLimits *  worldUserLimit = new GateUserLimits();
  if(mapOfVolumeUserLimits["DefaultRegionForTheWorld"] != 0)
    {
      if(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMaxStepSize()       != -1.)
        worldUserLimit->SetMaxStepSize(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMaxStepSize());
      if(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMaxTrackLength()    != -1.)
        worldUserLimit->SetMaxTrackLength(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMaxTrackLength());
      if(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMaxToF()            != -1.)
        worldUserLimit->SetMaxToF(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMaxToF());
      if(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMinKineticEnergy()  != -1.)
        worldUserLimit->SetMinKineticEnergy(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMinKineticEnergy());
      if(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMinRemainingRange() != -1.)
        worldUserLimit->SetMinRemainingRange(mapOfVolumeUserLimits["DefaultRegionForTheWorld"]->GetMinRemainingRange());
    }

  //-----------------------------------------------------------------------------
  // Set default production cut to other regions
  pi = RegionStore->begin();
  pe = RegionStore->end();
  while (pi != pe) {
    G4String regionName = (*pi)->GetName();

    if (regionName != "DefaultRegionForTheWorld" && regionName !="world") {
      VolumeUserLimitsMapType::iterator current = mapOfVolumeUserLimits.find(regionName);
      if (current == mapOfVolumeUserLimits.end()) {
	// GateMessage("Cuts",5, " UserCuts not set for region " << regionName << " put -1" << G4endl);
        mapOfVolumeUserLimits[regionName]= new GateUserLimits();
      }
    }
    ++pi;
  }

  VolumeUserLimitsMapType::iterator it2 = mapOfVolumeUserLimits.begin();
  while (it2 != mapOfVolumeUserLimits.end()) {
    // do not apply cut for the world region
    // GateMessage("Cuts", 5, "Region (*it2).first : " << (*it2).first<< G4endl);
    if (((*it2).first != "DefaultRegionForTheWorld") && ((*it2).first != "world")) {
      G4Region* region = RegionStore->GetRegion((*it2).first);
      if (!region) {
	GateError( "The region '" << (*it2).first << "' does not exist !");
      }
      // set default values
      GateUserLimits *  regionUserLimit =  (*it2).second;
      G4Region* parentRegion =  region;
      G4Region* regionTmp =  region;
      while((regionUserLimit->GetMaxStepSize() == -1) &&
            (regionTmp->GetName() != "DefaultRegionForTheWorld"))
        {
          G4bool unique;
          parentRegion =  regionTmp->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            GateUserLimits * parentRegionUserLimits = mapOfVolumeUserLimits[parentRegion->GetName()];
            regionUserLimit->SetMaxStepSize(  parentRegionUserLimits->GetMaxStepSize()) ;
            // GateMessage("Cuts", 5, "Region " << (*it2).first << " maxStepSize " << parentRegionUserLimits->GetMaxStepSize() << G4endl);
          }
          else regionUserLimit->SetMaxStepSize( worldUserLimit->GetMaxStepSize());
          regionTmp = parentRegion;
        }

      regionTmp =  region;
      while(regionUserLimit->GetMaxTrackLength() == -1 && regionTmp->GetName() != "DefaultRegionForTheWorld")
        {
          G4bool unique;
          parentRegion =  regionTmp->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            GateUserLimits * parentRegionUserLimits = mapOfVolumeUserLimits[parentRegion->GetName()];
            regionUserLimit->SetMaxTrackLength(  parentRegionUserLimits->GetMaxTrackLength()) ;
            // GateMessage("Cuts", 5, "Region " << (*it2).first << " maxTrackLength " << parentRegionUserLimits->GetMaxTrackLength() << G4endl);
          }
          else regionUserLimit->SetMaxTrackLength( worldUserLimit->GetMaxTrackLength());
          regionTmp = parentRegion;
        }


      regionTmp =  region;
      while(regionUserLimit->GetMaxToF() == -1 && regionTmp->GetName() != "DefaultRegionForTheWorld")
        {
          G4bool unique;
          parentRegion =  regionTmp->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            GateUserLimits * parentRegionUserLimits = mapOfVolumeUserLimits[parentRegion->GetName()];
            regionUserLimit->SetMaxToF(  parentRegionUserLimits->GetMaxToF()) ;
          }
          else regionUserLimit->SetMaxToF( worldUserLimit->GetMaxToF());
          regionTmp = parentRegion;
        }

      regionTmp =  region;
      while(regionUserLimit->GetMinKineticEnergy() == -1 && regionTmp->GetName() != "DefaultRegionForTheWorld")
        {
          G4bool unique;
          parentRegion =  regionTmp->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            GateUserLimits * parentRegionUserLimits = mapOfVolumeUserLimits[parentRegion->GetName()];
            regionUserLimit->SetMinKineticEnergy(  parentRegionUserLimits->GetMinKineticEnergy()) ;
          }
          else regionUserLimit->SetMinKineticEnergy( worldUserLimit->GetMinKineticEnergy());
          regionTmp = parentRegion;
        }

      regionTmp =  region;
      while(regionUserLimit->GetMinRemainingRange() == -1 && regionTmp->GetName() != "DefaultRegionForTheWorld")
        {
          G4bool unique;
          parentRegion =  regionTmp->GetParentRegion(unique);
          if(parentRegion->GetName() != "DefaultRegionForTheWorld"){
            GateUserLimits * parentRegionUserLimits = mapOfVolumeUserLimits[parentRegion->GetName()];
            regionUserLimit->SetMinRemainingRange(  parentRegionUserLimits->GetMinRemainingRange()) ;
          }
          else regionUserLimit->SetMinRemainingRange( worldUserLimit->GetMinRemainingRange());
          regionTmp = parentRegion;
        }


      // Set the G4 limits
      G4bool IsULimitDefined = false;
      userlimits = new G4UserLimits(0.1);


      G4String regionName = region->GetName();
      if(regionUserLimit->GetMaxStepSize()       != -1.){
        userlimits->SetMaxAllowedStep(regionUserLimit->GetMaxStepSize());
        GateMessage("Cuts", 3, "Region " << regionName
                    << " maxStepSize " << regionUserLimit->GetMaxStepSize() << G4endl);
        IsULimitDefined = true;
      }
      if(regionUserLimit->GetMaxTrackLength()    != -1.){
        userlimits->SetUserMaxTrackLength(regionUserLimit->GetMaxTrackLength());
        GateMessage("Cuts", 3, "Region " << regionName
                    << " maxTrackLength " << regionUserLimit->GetMaxTrackLength() << G4endl);
        IsULimitDefined = true;
      }
      if(regionUserLimit->GetMaxToF()            != -1.){
        userlimits->SetUserMaxTime(regionUserLimit->GetMaxToF());
        GateMessage("Cuts", 3, "Region " << regionName
                    << " MaxToF " << regionUserLimit->GetMaxToF() << G4endl);
        IsULimitDefined = true;
      }
      if(regionUserLimit->GetMinKineticEnergy()  != -1.){
        userlimits->SetUserMinEkine(regionUserLimit->GetMinKineticEnergy());
        GateMessage("Cuts", 3, "Region " << regionName
                    << " MinEkine " << regionUserLimit->GetMinKineticEnergy() << G4endl);
        IsULimitDefined = true;
      }
      if(regionUserLimit->GetMinRemainingRange() != -1.){
        userlimits->SetUserMinRange(regionUserLimit->GetMinRemainingRange());
        GateMessage("Cuts", 3, "Region " << regionName
                    << " MinRange " << regionUserLimit->GetMinRemainingRange() << G4endl);
        IsULimitDefined = true;
      }
      if(IsULimitDefined) region->SetUserLimits(userlimits);
      else {
        GateMessage("Cuts", 3, "Region " << regionName << " : no UserLimit" << G4endl);
      }
    }
    ++it2;
  }

  // DS
  delete worldUserLimit;

  GateMessageDec("Cuts",4,"GatePhysicsList::SetCuts() -- end" << G4endl);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::SetCutInRegion(G4String particleName, G4String regionName, G4double cutValue)
{

  /*
    GateMessage("Cuts",3,"SetCutInRegion '" << regionName
    << "' for particle '" << particleName
    << "' : " << cutValue << G4endl);
  */

  if(regionName=="world") regionName="DefaultRegionForTheWorld";

  RegionCutMapType::iterator it = mapOfRegionCuts.find(regionName);
  if (it == mapOfRegionCuts.end()) {
    // first time this region is concerned
    mapOfRegionCuts[regionName].gammaCut = -1;
    mapOfRegionCuts[regionName].electronCut = -1;
    mapOfRegionCuts[regionName].positronCut = -1;
    mapOfRegionCuts[regionName].protonCut = -1;
  }
  if (particleName == "gamma") mapOfRegionCuts[regionName].gammaCut = cutValue;
  if (particleName == "e-") mapOfRegionCuts[regionName].electronCut = cutValue;
  if (particleName == "e+") mapOfRegionCuts[regionName].positronCut = cutValue;
  if (particleName == "proton") mapOfRegionCuts[regionName].protonCut = cutValue;

  /*
    GateMessage("Cuts", 3, " Current Cut is g=" <<
    mapOfRegionCuts[regionName].gammaCut << " e-=" <<
    mapOfRegionCuts[regionName].electronCut << " e+=" <<
    mapOfRegionCuts[regionName].positronCut << G4endl);
  */
  // DS verbose is in DefineCuts

}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::SetSpecialCutInRegion(G4String cutType, G4String regionName, G4double cutValue)
{
  if(regionName=="world") regionName="DefaultRegionForTheWorld";

  VolumeUserLimitsMapType::iterator it = mapOfVolumeUserLimits.find(regionName);
  if (it == mapOfVolumeUserLimits.end()) {
    // first time this region is concerned
    mapOfVolumeUserLimits[regionName]= new GateUserLimits();
  }

  if (cutType == "MaxStepSize") mapOfVolumeUserLimits[regionName]->SetMaxStepSize( cutValue);
  if (cutType == "MaxTrackLength") mapOfVolumeUserLimits[regionName]->SetMaxTrackLength( cutValue);
  if (cutType == "MaxToF") mapOfVolumeUserLimits[regionName]->SetMaxToF( cutValue);
  if (cutType == "MinKineticEnergy") mapOfVolumeUserLimits[regionName]->SetMinKineticEnergy( cutValue);
  if (cutType == "MinRemainingRange") mapOfVolumeUserLimits[regionName]->SetMinRemainingRange( cutValue);

}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::GetCuts()
{
  DumpCutValuesTable();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::SetOptDEDXBinning(G4int val)
{
  //G4EmProcessOptions * opt = new G4EmProcessOptions();
  mDEDXBinning=val;
  opt->SetDEDXBinning(mDEDXBinning);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void GatePhysicsList::SetOptLambdaBinning(G4int val)
{
  mLambdaBinning=val;
  opt->SetLambdaBinning(mLambdaBinning);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::SetOptEMin(G4double val)
{
  mEmin=val;
  opt->SetMinEnergy(mEmin);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::SetOptEMax(G4double val)
{
  mEmax=val;
  opt->SetMaxEnergy(mEmax);
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void GatePhysicsList::SetOptSplineFlag(G4bool val)
{
  mSplineFlag=val;
  opt->SetSplineFlag(mSplineFlag);
}

//#endif
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
GatePhysicsList *GatePhysicsList::singleton = 0;
//-----------------------------------------------------------------------------


#endif /* end #define GATEPHYSICSLIST_CC */
