//  **************************************************************************
//  * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
//  *                                                                        *
//  * Author: The ALICE Off-line Project.                                    *
//  * Contributors are mentioned in the code where appropriate.              *
//  *                                                                        *
//  * Permission to use, copy, modify and distribute this software and its   *
//  * documentation strictly for non-commercial purposes is hereby granted   *
//  * without fee, provided that the above copyright notice appears in all   *
//  * copies and that both the copyright notice and this permission notice   *
//  * appear in the supporting documentation. The authors make no claims     *
//  * about the suitability of this software for any purpose. It is          *
//  * provided "as is" without express or implied warranty.                  *
//  **************************************************************************
#include "AliHMPIDParam.h"  //class header
#include "AliHMPIDDigit.h"  //ctor
#include <TCanvas.h>       //TestXXX() 
#include <TLatex.h>        //TestTrans()  
#include <TView.h>         //TestTrans()
#include <TPolyMarker3D.h> //TestTrans()
#include <TRotation.h>
#include <AliRunLoader.h>  //Stack()
#include <AliStack.h>      //Stack()
#include <TParticle.h>     //Stack()    

ClassImp(AliHMPIDParam)

AliHMPIDParam* AliHMPIDParam::fgInstance=0x0;        //singleton pointer               
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AliHMPIDParam::AliHMPIDParam():TNamed("RichParam","default version") 
{
// Here all the intitializition is taken place when AliHMPIDParam::Instance() is invoked for the first time.
// In particulare, matrices to be used for LORS<->MARS trasnformations are initialized from TGeo structure.    
// Note that TGeoManager should be already initialized from geometry.root file  
  fX=0.5*AliHMPIDDigit::SizeAllX();
  fY=0.5*AliHMPIDDigit::SizeAllY();
  for(Int_t i=0;i<7;i++) 
    if(gGeoManager)
      fM[i]=(TGeoHMatrix*)gGeoManager->GetVolume("ALIC")->GetNode(Form("HMPID_%i",i))->GetMatrix();
    else{
      fM[i]=new TGeoHMatrix;
      IdealPosition(i,fM[i]);
    } 
  fgInstance=this; 
}//ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDParam::Print(Option_t* opt) const
{
// print some usefull (hopefully) info on some internal guts of HMPID parametrisation 
  
  for(Int_t i=0;i<7;i++) fM[i]->Print(opt);
}//Print()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDParam::IdealPosition(Int_t iCh, TGeoHMatrix *pMatrix)
{
// Construct ideal position matrix for a given chamber
// Arguments: iCh- chamber ID; pMatrix- pointer to precreated unity matrix where to store the results
//   Returns: none
  const Double_t kAngHor=19.5; //  horizontal angle between chambers  19.5 grad
  const Double_t kAngVer=20;   //  vertical angle between chambers    20   grad     
  const Double_t kAngCom=30;   //  common HMPID rotation with respect to x axis  30   grad     
  const Double_t trans[3]={490,0,0}; //center of the chamber is on window-gap surface
  pMatrix->RotateY(90);           //rotate around y since initial position is in XY plane -> now in YZ plane
  pMatrix->SetTranslation(trans); //now plane in YZ is shifted along x 
  switch(iCh){
    case 0:                pMatrix->RotateY(kAngHor);  pMatrix->RotateZ(-kAngVer);  break; //right and down 
    case 1:                                            pMatrix->RotateZ(-kAngVer);  break; //down              
    case 2:                pMatrix->RotateY(kAngHor);                               break; //right 
    case 3:                                                                         break; //no rotation
    case 4:                pMatrix->RotateY(-kAngHor);                              break; //left   
    case 5:                                            pMatrix->RotateZ(kAngVer);   break; //up
    case 6:                pMatrix->RotateY(-kAngHor); pMatrix->RotateZ(kAngVer);   break; //left and up 
  }
  pMatrix->RotateZ(kAngCom);     //apply common rotation  in XY plane    
   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t AliHMPIDParam::Stack(Int_t evt,Int_t tid)
{
// Prints some useful info from stack
// Arguments: evt - event number. if not -1 print info only for that event
//            tid - track id. if not -1 then print it and all it's mothers if any   
//   Returns: mother tid of the given tid if any
  AliRunLoader *pAL=AliRunLoader::Open(); 
  if(pAL->LoadHeader()) return -1;
  if(pAL->LoadKinematics()) return -1;
  
  Int_t mtid=-1;
  Int_t iNevt=pAL->GetNumberOfEvents();
  
  for(Int_t iEvt=0;iEvt<iNevt;iEvt++){//events loop
    if(evt!=-1 && evt!=iEvt) continue; //in case one needs to print the requested event, ignore all others
    pAL->GetEvent(iEvt);    
    AliStack *pStack=pAL->Stack();  
    if(tid==-1){                        //print all tids for this event
      for(Int_t i=0;i<pStack->GetNtrack();i++) pStack->Particle(i)->Print();
      Printf("totally %i tracks including %i primaries for event %i out of %i event(s)",pStack->GetNtrack(),pStack->GetNprimary(),iEvt,iNevt);
    }else{                              //print only this tid and it;s mothers
      if(tid<0 || tid>pStack->GetNtrack()) {Printf("Wrong tid, valid tid range for event %i is 0-%i",iEvt,pStack->GetNtrack());break;}
      TParticle *pTrack=pStack->Particle(tid); mtid=pTrack->GetFirstMother();
      TString str=pTrack->GetName();
      while((tid=pTrack->GetFirstMother()) >= 0){
        pTrack=pStack->Particle(tid);
        str+=" from ";str+=pTrack->GetName();
      } 
    }//if(tid==-1)      
  }//events loop
  pAL->UnloadHeader();  pAL->UnloadKinematics();
  return mtid;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Int_t AliHMPIDParam::StackCount(Int_t pid,Int_t evt)
{
// Counts total number of particles of given sort (including secondary) for a given event
  AliRunLoader *pAL=AliRunLoader::Open(); 
  pAL->GetEvent(evt);    
  if(pAL->LoadHeader()) return 0;
  if(pAL->LoadKinematics()) return 0;
  AliStack *pStack=pAL->Stack();
  
  Int_t iCnt=0;
  for(Int_t i=0;i<pStack->GetNtrack();i++) if(pStack->Particle(i)->GetPdgCode()==pid) iCnt++;
  
  pAL->UnloadHeader();  pAL->UnloadKinematics();
  return iCnt;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
