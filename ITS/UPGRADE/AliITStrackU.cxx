/**************************************************************************
 * Copyright(c) 1998-2003, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/


////////////////////////////////////////////////////////
//  Stand alone track class UPGRADE                   //
//  Authors: A.Mastroserio                            //
//           C.Terrevoli                              //
//           annalisa.mastroserio@cern.ch             //      
//           cristina.terrevoli@ba.infn.it            //                                                    
////////////////////////////////////////////////////////

#include "AliITStrackU.h"
#include "AliITSsegmentationUpgrade.cxx"

ClassImp(AliITStrackU)

//_____________________________________
AliITStrackU:: AliITStrackU() : AliITStrackMI(),
fNU(0),
fNLayers(0)
{
// Default constructor  
  SetNumberOfClusters(0);
  SetNumberOfClustersU(0);
  ResetIndexU();
  for(Int_t nlay=0;nlay<fNLayers;nlay++){ 
    SetNumberOfMarked(nlay,0);
  }
  ResetMarked();
}
//_____________________________________
AliITStrackU:: AliITStrackU(Int_t nlay) : AliITStrackMI(),
fNU(0),
fNLayers(nlay)
{
// Constructor
  SetNumberOfClusters(0);
  SetNumberOfClustersU(0);
  ResetIndexU();
  for(Int_t nl=0;nl<fNLayers;nl++){
    SetNumberOfMarked(nl,0);
  }
  ResetMarked();
}

//___________________________________________________
AliITStrackU::AliITStrackU(const AliITStrackMI& t) : 
AliITStrackMI(t),
fNU(0),
fNLayers(0)
{
//
// Copy a V2 track into a U track
// -> to be checked

  SetNumberOfClustersU(0);
  ResetIndexU();
  for(Int_t nlay=0;nlay<fNLayers;nlay++){ 
    SetNumberOfMarked(nlay,0);
  }
  ResetMarked();

}
//___________________________________________________
AliITStrackU::AliITStrackU(const AliITStrackU& t) : 
AliITStrackMI(t),
fNU(t.fNU),
fNLayers(t.fNLayers)
{
//
// Copy constructor
// to be checked

  ResetIndexU();
  ResetMarked();
  Int_t number = t.GetNumberOfClustersU();
  SetNumberOfClustersU(number);
  for(Int_t nlay=0;nlay<fNLayers;nlay++){
    SetNumberOfMarked(nlay,t.GetNumberOfMarked(nlay));
  }
  for(Int_t i=0;i<number;i++){
    fSain[i]=t.fSain[i];
  }
  for(Int_t nlay=0;nlay<fNLayers;nlay++){
    for(Int_t i=0;i<t.GetNumberOfMarked(nlay);i++){
      fCluMark[nlay][i]=t.fCluMark[nlay][i];
    }
  }
}
//____________________________________________________
AliITStrackU::AliITStrackU(Double_t alpha, Double_t radius, Double_t Ycoor, Double_t Zcoor, Double_t phi, Double_t tanlambda, Double_t curv, Int_t lab, Int_t nlay ):
fNU(0), fNLayers(nlay)
{
  // standard constructor. Used for ITSUpgrade standalone tracking
  // get the azimuthal angle of the detector containing the innermost
  // cluster of this track (data member fAlpha)

  if (alpha<0) alpha+=TMath::TwoPi();
  else if (alpha>=TMath::TwoPi()) alpha-=TMath::TwoPi();
  Init(alpha,radius,Ycoor,Zcoor,phi,tanlambda,curv,lab);
}
//____________________________________________________
  void AliITStrackU::Init(Double_t alpha, Double_t radius, Double_t Ycoor, Double_t Zcoor, Double_t phi, Double_t tanlambda, Double_t curv, Int_t lab ){
    // initialize parameters

  fdEdx = 0;

  Double_t conv=GetBz()*kB2C;
  Double_t sC[] = {0.000009, // 0.000009
                   0.,
                   0.000003, //0.000030
                   0.,
	           0.,
	           0.000001, //0.000001
	           0.,
	           0.,
	           0.,
	           0.000002, //0.000002
	           0.,
	           0.,
	           0.,
	           0.,
		   0.000001/(conv*conv)}; //0.0000001

  Double_t sP[] = {Ycoor,
		   Zcoor,
                   TMath::Sin(phi-alpha),
		   tanlambda,
		   curv/conv};


  // dealing with the case B=0 (taken from AliTPCtrack.cxx)
  Double_t mostProbablePt=AliExternalTrackParam::GetMostProbablePt();
  Double_t p0=TMath::Sign(1/mostProbablePt,sP[4]);
  Double_t w0=sC[14]/(sC[14] + p0*p0), w1=p0*p0/(sC[14] + p0*p0);
  sP[4] = w0*p0 + w1*sP[4];
  sC[14]*=w1;
                                                                              
  Set(radius,alpha,sP,sC);

  for(Int_t i=0; i<fNLayers; i++) fIndex[i] = 0;  // to be set explicitely

  for(Int_t i=0; i<4; i++) fdEdxSample[i] = 0; 

  SetNumberOfClusters(0);
  SetNumberOfClustersU(0);
  for(Int_t nlay=0;nlay<fNLayers;nlay++) SetNumberOfMarked(nlay,0);
  ResetIndexU();
  ResetMarked();
  SetChi2(0);
  SetMass(0.139);    // pion mass
  SetLabel(lab); 
  
}

//____________________________________________________________
void AliITStrackU::AddClusterU(Int_t layer, Int_t clnumb) {
  // add one clusters to the list (maximum number=kMaxNumberOfClusters)
  Int_t presnum = GetNumberOfClustersU();
  if(presnum>=kMaxNumberOfClusters){
    Warning("AddClusterU","Maximum number of clusters already reached. Nothing is done\n");
    return;
  }

  fSain[presnum] = (layer<<28)+clnumb;  
  presnum++;
  SetNumberOfClustersU(presnum);
}

//____________________________________________________________
void AliITStrackU::AddClusterMark(Int_t layer, Int_t clnumb) {
  // add one clusters to the list (maximum number=kMaxNumberOfClusters)
  Int_t presnum = GetNumberOfMarked(layer);
    //printf("presnum=%d\n",presnum);
  if(presnum>=kMaxNumberOfClustersL){
    Warning("AddClusterMark","Maximum number of clusters already reached. Nothing is done\n");
    return;
  }

  fCluMark[layer][presnum] = clnumb;  
  presnum++;
  SetNumberOfMarked(layer,presnum);
}

//____________________________________________________________
void AliITStrackU::AddClusterV2(Int_t layer,Int_t clnumb) {
  // add one clusters to the list (maximum number=6)
  Int_t presnum = GetNumberOfClusters();
  if(presnum>=fNLayers){
    Warning("AddClusterV2","Maximum number of clusters already reached. Nothing is done\n");
    return;
   }    

  fIndex[presnum] = (layer<<28)+clnumb;  
  presnum++;
  SetNumberOfClusters(presnum);
}

//_____________________________________________________________
void AliITStrackU::ResetMarked(){

  //Reset array of marked clusters
  for(Int_t nlay=0;nlay<fNLayers;nlay++){
    for(Int_t k=0; k<kMaxNumberOfClustersL; k++) fCluMark[nlay][k]=0;
  }
}

