/*----------------------
   GATE version name: gate_v6

   Copyright (C): OpenGATE Collaboration

This software is distributed under the terms
of the GNU Lesser General  Public Licence (LGPL)
See GATE/LICENSE.txt for further details
----------------------*/


#include "GateUIcmdWithAStringAndADouble.hh"

//---------------------------------------------------------------------------
GateUIcmdWithAStringAndADouble::GateUIcmdWithAStringAndADouble(const char * theCommandPath,G4UImessenger * theMessenger)
:G4UIcommand(theCommandPath,theMessenger)
{
  G4UIparameter * strParam1 = new G4UIparameter('s');
  SetParameter(strParam1);
  G4UIparameter * strParam2 = new G4UIparameter('d');
  SetParameter(strParam2);

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void GateUIcmdWithAStringAndADouble::SetParameterName
(const char * theName1, const char * theName2,
 G4bool omittable1, G4bool omittable2,  G4bool currentAsDefault)
{
  G4UIparameter * theParam = GetParameter(0);
  theParam->SetParameterName(theName1);
  theParam->SetOmittable(omittable1);
  theParam->SetCurrentAsDefault(currentAsDefault);

  theParam = GetParameter(1);
  theParam->SetParameterName(theName2);
  theParam->SetOmittable(omittable2);
  theParam->SetCurrentAsDefault(currentAsDefault);





}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void GateUIcmdWithAStringAndADouble::SetCandidates(const char * candidateList1, const char * candidateList2)
{
  G4UIparameter * theParam = GetParameter(0);
  G4String canList = candidateList1;
  theParam->SetParameterCandidates(canList);

  theParam = GetParameter(1);
  canList = candidateList2;
  theParam->SetParameterCandidates(canList);




}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void GateUIcmdWithAStringAndADouble::SetDefaultValue(const char * defVal1, const char * defVal2 )
{
  G4UIparameter * theParam = GetParameter(0);
  theParam->SetDefaultValue(defVal1);

  theParam = GetParameter(1);
  theParam->SetDefaultValue(defVal2);


}
//---------------------------------------------------------------------------
