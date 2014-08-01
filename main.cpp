/*
Code's main Structure:
First, in the int main(), the constructor of mainClass will be called. 
In this mainClass, various reconstructed objects (jets, leptons, photons, etc) are loaded, all the cuts are applied, and histograms are filled.
In order to fill the histograms from inside mainClass, the constructor of histClass is called.
In order to define a cut, first name it inside the cutname which is a map. Then tell what cuts should be applied, when this name is called, inside checkcut()

Desired number of events: sometimes there are many different files in a .list file containing the same interaction. And also, we only need a limited number of events. In that case we don't want to waste our time and read all those .list files. We just need to read as many file as needed to give the desired number of events. In this case there is a varioable "desirednumeve" in the constructor of mainClass. Just change it to whatever value you need. 
Otherwise######## comment it out ############
Otherwise######## comment it out ############
Otherwise######## comment it out ############
Otherwise######## comment it out ############

To change the output histograms one need to first determine how many of them exist in the histClass. Then introduce the histograms and add themto vecTH and then finally give the corresponding information to eveinfvec. Here order is important. I hope I find some time to change this frommanual to auto :)

*/

#include <cassert>
#include "TChain.h"
#include "TH1.h"
#include "TVector2.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include "TSystem.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include <vector>
#include <map>
#include "Delphes/external/ExRootAnalysis/ExRootTreeReader.h"
#include "Delphes/classes/DelphesClasses.h"
///this is  related to weight calculation and should be removed when the new samples arrive
#include "DelWeight.h"

using namespace std;

//
class histClass{
  double * a;
  TH1D * b_hist;
public:
  void fill(double * eveinfarr_, TH1D * hist_){
    a = eveinfarr_;
    b_hist=hist_;
    (*b_hist).Fill(*a);
    for(int i=1; i<=17 ; i++){
      (*(b_hist+i)).Fill(*(a+i),*a);
    }
  }
};

//
//define a function to evaluate delta phi
double delphi(vector<double> a, double tPx, double tPy,double mht){
  //-totpx is the ptx comp of MHT
  double jetpt = sqrt((a[1]*cos(a[2]))*(a[1]*cos(a[2]))+(a[1]*sin(a[2]))*(a[1]*sin(a[2])));
  double MHT_Jet_Dot = (-tPx*(a[1]*cos(a[2]))-tPy*(a[1]*sin(a[2])));
  double deltaphi = acos(MHT_Jet_Dot/(mht*jetpt));
  //KH std::cout << deltaphi << std::endl;
  return deltaphi;
  ///end of function deltaphi
}

//
//this function is exclusively written for BJ processes with emphesis on one B.
bool bg_type(string bg_ ,vector<GenParticle> pvec){

  if(bg_=="allEvents"){return 1;}

  if(bg_=="H"){
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==25)return true;
    }
    return false;
  }

  if(bg_=="photon"){
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==22)return true;
    }
    return false;
  }

  
  if(bg_=="Z"){
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==23)return true;
    }
    return false;
  }

  if(bg_=="Zvv"){
    vector<int> vvvec;
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==23){//23 is the PID code of Z boson.
	vvvec.clear();
	for(int j = 0; j < (int)(pvec.size()); ++j){
	  GenParticle * pp = &pvec.at(j);
	  if (pp->Status == 3 && pp->M1 < (int)(pvec.size()) && pp->M2 < (int)(pvec.size())
              && (fabs(pp->PID) == 12 || fabs(pp->PID) == 14 || fabs(pp->PID) == 16) ){
	    vvvec.push_back(pp->PID);
	  }//end of if pp->PID == 12, 14, 16 = nutrinos
	}//end of second loop
	if((int)vvvec.size()==2){
	  return true;}//end of if 
      }//end of if PID==23=Z boson
    }//end of loop
    return false;
  }//end of if Zvv

  if(bg_=="Zll"){
    vector<int> vvvec;
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==23){//23 is the PID code of Z boson.
	vvvec.clear();
	for(int j = 0; j < (int)(pvec.size()); ++j){
	  GenParticle * pp = &pvec.at(j);
	  if (pp->Status == 3 && pp->M1 < (int)(pvec.size()) && pp->M2 < (int)(pvec.size())
              && (fabs(pp->PID) == 11 || fabs(pp->PID) == 13 || fabs(pp->PID) == 15)){
	    vvvec.push_back(pp->PID);
	  }//end of if pp->PID == 11, 13 15 = leptons
	}//end of second loop
	if((int)vvvec.size()==2){
	  return true;}//end of if 
      }//end of if PID==23=Z boson
    }//end of loop
    return false;
  }//end of if Zll


  if(bg_=="Zjj"){
    vector<int> vvvec;
    vector<int> llvec;
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==23){//23 is the PID code of Z boson.
	vvvec.clear();
	llvec.clear();
	for(int j = 0; j < (int)(pvec.size()); ++j){
	  GenParticle * pp = &pvec.at(j);
	  if (pp->Status == 3 && pp->M1 < (int)(pvec.size()) && pp->M2 < (int)(pvec.size()) 
              && (fabs(pp->PID) == 11 || fabs(pp->PID) == 13 || fabs(pp->PID) == 15)){
	    llvec.push_back(pp->PID);
	  }//end of if pp->PID == 11, 13 15 = leptons
	  if (pp->Status == 3 && pp->M1 < (int)(pvec.size()) && pp->M2 < (int)(pvec.size()) 
              && (fabs(pp->PID) == 12 || fabs(pp->PID) == 14 || fabs(pp->PID) == 16)){
	    vvvec.push_back(pp->PID);
	  }//end of if pp->PID == 12, 14, 16 = neutrino

	}//end of second loop
	if((int)vvvec.size()==2 || (int)llvec.size()==2){
	  return false;}else return true;//end of if 
      }//end of if PID==23=Z boson
    }//end of loop
    return false;
  }//end of if Zjj
  
  if(bg_=="W"){
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * pa = &pvec.at(i);
      if(fabs(pa->PID)==24)return true;
    }
    return false;
  }

  if(bg_=="Wlv"){
    vector<int> llvec;
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * pa = &pvec.at(i);
      if(fabs(pa->PID)==24){//+-24 are the PID codes of W bosons.
	llvec.clear();
	for(int j = 0; j < (int)(pvec.size()); ++j){
	  GenParticle * ppa = &pvec.at(j);
	  if (ppa->Status == 3 && ppa->M1 < (int)(pvec.size()) && ppa->M2 < (int)(pvec.size())
              && (fabs(ppa->PID) == 11 || fabs(ppa->PID) == 13 || fabs(ppa->PID) == 15) ){
	    llvec.push_back(ppa->PID);
	  }//end of if ppa->PID == 11, 13, 15 = electron , muon, tau
	}//end of second loop
	if((int)llvec.size()==1){//llvec.size() ==1 since W decays to one lepton and one nutrino
	  return true;}//end of if 
      }//end of if PID==24=W boson
    }//end of loop
    return false;
  }//end of if Wlv
  if(bg_=="Wjj"){
    vector<int> llvec;
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * pa = &pvec.at(i);
      if(fabs(pa->PID)==24){//+-24 are the PID codes of W bosons.
        llvec.clear();
        for(int j = 0; j < (int)(pvec.size()); ++j){
          GenParticle * ppa = &pvec.at(j);
          if (ppa->Status == 3 && ppa->M1 < (int)(pvec.size()) && ppa->M2 < (int)(pvec.size())
              && (fabs(ppa->PID) == 11 || fabs(ppa->PID) == 13 || fabs(ppa->PID) == 15) ){
            llvec.push_back(ppa->PID);
          }//end of if ppa->PID == 11, 13, 15 = electron , muon, tau
        }//end of second loop
        if((int)llvec.size()==1){//llvec.size() ==1 since W decays to one lepton and one nutrino
          return false;}else return true;//end of if 
      }//end of if PID==24=W boson
    }//end of loop
    return false;
  }//end of if Wjj

  if(bg_=="TTbar"){
    int numofT=0;//this will determine how many T or Tbar exist in the event.
    for(int i = 0; i < (int)(pvec.size()); ++i){
      GenParticle * pa = &pvec.at(i);
      //+-6 are the PID codes of T quark 
      if(fabs(pa->PID)==6) numofT+=1;
    }
    if(numofT==2) return true;
    return false;
  }//end of TTbar

  ///These following functions make sense only if the leptons in the event are coming from TTbar. If there are TTbar and leptons not from TTbar in the events this function still consider the leptons are coming from TTbar. There are many solutions for this. But since currently we are only dealing with TTbar events these are good.  
  if(bg_=="TTSingLep"){
    int GenSize = (int)pvec.size();
    int numofT=0;//this will determine how many T or Tbar exist in the event.
    vector<GenParticle> lepvec;//this will determine how many lepton exist in the event
    for(int i = 0; i < GenSize; ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==6) numofT+=1;
      if(p->M1 < GenSize && p->M2 < GenSize
         && (abs(p->PID) == 11 || abs(p->PID) == 13 || abs(p->PID) == 15)) lepvec.push_back(*p);
    }
    //now modify lepton vector
    if((int)lepvec.size()==2){
      if(lepvec.at(0).P4().DeltaR(lepvec.at(1).P4())<=0.8) lepvec.erase(lepvec.begin()+1);
    }
    if((int)numofT==2 && (int)lepvec.size()==1) return true;
    return false;
  }//end of TTSingLep

  if(bg_=="TTdiLep"){
    int GenSize = pvec.size();
    int numofT=0;//this will determine how many T or Tbar exist in the event.
    vector<GenParticle> lepvec;//this will determine how many lepton exist in the event
    for(int i = 0; i < GenSize; ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==6) numofT+=1;
      if(p->M1 < GenSize && p->M2 < GenSize
         && (abs(p->PID) == 11 || abs(p->PID) == 13 || abs(p->PID) == 15)) lepvec.push_back(*p);
    }
    //now modify lepton vector
    if((int)lepvec.size()==2){
      if(lepvec.at(0).P4().DeltaR(lepvec.at(1).P4())<=0.8) lepvec.erase(lepvec.begin()+1);
    }
    if(numofT==2 && (int)lepvec.size()==2) return true;
    return false;
  }//end of TTdiLep

  if(bg_=="TThadronic"){
    int GenSize = (int)pvec.size();
    int numofT=0;//this will determine how many T or Tbar exist in the event.
    vector<GenParticle> lepvec;//this will determine how many lepton exist in the event
    for(int i = 0; i < GenSize; ++i){
      GenParticle * p = &pvec.at(i);
      if(fabs(p->PID)==6) numofT+=1;
      if(p->M1 < GenSize && p->M2 < GenSize
         && (abs(p->PID) == 11 || abs(p->PID) == 13 || abs(p->PID) == 15)) lepvec.push_back(*p);
    }
    //now modify lepton vector
    if((int)lepvec.size()==2){
      if(lepvec.at(0).P4().DeltaR(lepvec.at(1).P4())<=0.8) lepvec.erase(lepvec.begin()+1);
    }
    if(numofT==2 && (int)lepvec.size()==0) return true;
    return false;
  }//end of TTdiLep


if(bg_=="glgl"){
int Ngo=0;
for(int i = 0; i < (int)(pvec.size()); ++i){
GenParticle * particle = &pvec.at(i);
if(particle->PID==1000021){Ngo++;}
}//end of for
if(Ngo==2){return true;}
return false;
}

} //end of function bg_type
/*    glgl = false;
      sqgl = false;
      c1c1 = false;
      c1n2 = false;
      n2n2 = false;
      t1t1 = false;
      Ngo  = 0;
      Nc1  = 0;
      Nn2  = 0;
      Nt1  = 0; 
      vecTopQuarks.clear();*/
/*          if (particle->PID==1000021) sqgl = true;
          if (abs(particle->PID)==1000001) sqgl = true;
          if (abs(particle->PID)==1000002) sqgl = true;
          if (abs(particle->PID)==1000003) sqgl = true;
          if (abs(particle->PID)==1000004) sqgl = true;
          if (particle->PID==1000021) Ngo++;
          if (abs(particle->PID)==1000023) Nn2++;
          if (abs(particle->PID)==1000024) Nc1++;
          if (abs(particle->PID)==1000006) Nt1++;
          if (abs(particle->PID)==6) vecTopQuarks.push_back(*particle); 
      if (Ngo==2){
        glgl = true;
      } else if (Nn2==2){
        n2n2 = true;
      } else if (Nc1==1&&Nn2==1){
        c1n2 = true;
      } else if (Nc1==2){
        c1c1 = true;
      } else if (Nt1==2){
        t1t1 = true;
      }*/
//
double  METMHTAsys(MissingET* met,vector<Jet> jetvec,vector<Muon> muonvec,vector<Electron> electronvec,vector<Photon> photonvec){
  double Met=-99;
  double METAsys=-99;
  TVector2 PUCorMet, RawMet;
   TLorentzVector allvecsum;
  allvecsum.SetPxPyPzE(0, 0, 0, 0);
  PUCorMet.Set(0., 0.);
  RawMet.Set(0.0, 0.0);
 
  for(int i=0; i<(int)jetvec.size(); i++) {allvecsum += jetvec.at(i).P4();}
  for(int j=0; j<(int)muonvec.size(); j++) {allvecsum += muonvec.at(j).P4();}
  for(int k=0; k<(int)electronvec.size(); k++) {allvecsum += electronvec.at(k).P4();}
  for(int l=0; l<(int)photonvec.size(); l++) {allvecsum += photonvec.at(l).P4();}
 
  PUCorMet.Set(-allvecsum.Px(),-allvecsum.Py());
  Met= PUCorMet.Mod();
  RawMet.SetMagPhi(met->MET, met->Phi);
  
  METAsys=fabs(Met-(RawMet.Mod()))/(Met+(RawMet.Mod()));//this is funny. RawMet.Mod() must return met->MET. We didn't need to build RawMet to obtain its magnitude :):0
  //cout << "......................RawMet.Mod(): " << RawMet.Mod() << endl;
  //cout << "...................... Met: " << Met << endl; 
  //cout << "...................... METAsys: " << METAsys << endl;
  return METAsys;

}

///////////////////////////////////////////
//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()
////////////////////////////////////////////
class mainClass{

  //List of variables
  int terminator, desirednumeve;
  float xs, xserr;
  double weight, CrossSection, CrossSectionError, totPx, totPy, HT, MHT, cutHT, cutMHT, pt, coss, sinn;
  vector<vector<double> > vecjvec, vecelecvec, vecmuvec;
  vector<TLorentzVector> vecJets, vecBtagLoose, vecBtagTight;
  vector<double> jvec, elecvec, muvec;
  vector<Jet*> tauvec;
  vector<GenParticle> GenParticlevec;
  //KH
  vector<TH1D> vecTH;  //KH vec-->vecTH
  vector<Photon> photonvec,phovec; vector<Electron> electronvec; vector<Muon> muonvec;vector<Jet> jetvec; 
  char TreeList[200], tempname[200];
  string pro, line, Pileup_ ;
  map<int, string> cutname;
  map<int, string> eventType;
  fstream file, input, cutflowfile;
  map<string, TH1D> cutflowmap;
  map<string , vector<TH1D> > cut_histvec_map;  
  map<string, map<string , vector<TH1D> > > map_map;
  map<string, histClass> histobjmap;
  histClass histObj;
  MissingET* met ;
  MissingET* metpujetid ;
  MissingET* metpuppi ;
  ScalarHT* sht; 
//  TClonesArray * branchEvent ;
  TClonesArray * branchJet;
  TClonesArray * branchElectron;
  TClonesArray * branchMuon;
  TClonesArray * branchPhoton;
  TClonesArray * branchMet;
  TClonesArray * branchMetPUJetID;
  TClonesArray * branchMetPUPPI;
  TClonesArray * branchHT;
  TClonesArray * branchParticle;
  TLorentzVector tempLorvec;

/*
  bool sqgl;
  bool glgl;
  bool c1c1;
  bool c1n2;
  bool n2n2;
  bool t1t1;
  int Ngo;
  int Nc1;
  int Nn2;
  int Nt1;
  int Ntop;
*/

  //define different cuts here
  bool threejet(){if(vecjvec.size() >= 3 && vecjvec[0][1]> 50 )return true; return false;}
  bool twoloosebtag(){if(vecBtagLoose.size() >= 2)return true; return false;}
  bool twotightbtag(){if(vecBtagTight.size() >= 2)return true; return false;}
  bool ht(){if(HT>=1000) return true; return false;}
  bool mht(){if(MHT>=500)return true; return false;}
  bool dphi(){if(delphi(vecjvec[0],totPx,totPy,MHT)>0.5 && delphi(vecjvec[1],totPx,totPy,MHT)>0.5 && delphi(vecjvec[2],totPx,totPy,MHT)>0.3)return true; return false;}
  bool nolep(){if(vecelecvec.size()==0 && vecmuvec.size()==0)return true; return false;}
  bool nopho(){if(phovec.size()==0)return true; return false;}
  bool fourjet(){if(vecjvec.size() >= 4)return true; return false;}
  bool fivejet(){if(vecjvec.size() >= 5)return true; return false;}
  bool sixjet(){if(vecjvec.size() >= 6)return true; return false;}
  bool highMht(){if(MHT>=1000)return true; return false;}
  bool highHt(){if(HT>=2500)return true; return false;}
  //Reference: Ben
  //there are jets missing in the event, which cause much larger MHT than expected. In order to supress this problem,
  bool Asys(){
double AsysCut = -99;
 if (Pileup_ == "NoPileUp") AsysCut = 0.2;
  if (Pileup_ == "50PileUp") AsysCut = 0.3;
  if (Pileup_ == "140PileUp") AsysCut = 0.5;
  assert(AsysCut != -99.);
if(METMHTAsys(met,jetvec,muonvec,electronvec,photonvec) < AsysCut )return true; return false;}


  //function checkcut()
  bool checkcut(string ss){
    if(ss== cutname[0])return true;
    if(ss== cutname[1]) {if(Asys())return true;}
    if(ss== cutname[2]) {if(Asys()&&threejet())return true;}
    if(ss== cutname[3]) {if(Asys()&&threejet()&&ht())return true;}
    if(ss== cutname[4]) {if(Asys()&&threejet()&&ht()&&mht())return true;}
    if(ss== cutname[5]) {if(Asys()&&threejet()&&ht()&&mht()&&dphi())return true;}
    if(ss== cutname[6]) {if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep())return true;}
    if(ss== cutname[7]) {if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho())return true;}
    if(ss== cutname[8]) {if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fourjet())return true;}
    if(ss== cutname[9]) {if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fivejet())return true;}
    if(ss== cutname[10]) {if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet())return true;}
    if(ss== cutname[11]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet()&&highMht())return true;}
    if(ss== cutname[12]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet()&&highHt())return true;}
    if(ss== cutname[13]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet()&&highHt()&&highMht())return true;}
    if(ss== cutname[14]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&highMht())return true;}
    if(ss== cutname[15]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&twoloosebtag())return true;}
    if(ss== cutname[16]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&twoloosebtag()&&highMht())return true;}    
    if(ss== cutname[17]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fourjet()&&highMht())return true;}
    if(ss== cutname[18]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fourjet()&&twoloosebtag())return true;}
    if(ss== cutname[19]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fourjet()&&highMht()&&twoloosebtag())return true;}
    if(ss== cutname[20]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fivejet()&&highMht())return true;}
    if(ss== cutname[21]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fivejet()&&twoloosebtag())return true;}
    if(ss== cutname[22]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&fivejet()&&highMht()&&twoloosebtag())return true;}
    if(ss== cutname[23]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet()&&highMht())return true;}
    if(ss== cutname[24]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet()&&twoloosebtag())return true;}
    if(ss== cutname[25]){if(Asys()&&threejet()&&ht()&&mht()&&dphi()&&nolep()&&nopho()&&sixjet()&&highMht()&&twoloosebtag())return true;}

return false; 
  }

//constructor
public:
  mainClass(string Pileup, string Process, string Detector, string Outdir, string inputnumber){
    Pileup_ = Pileup;
     CrossSection=-999.0; CrossSectionError=0.0; totPx=0;desirednumeve=-999; totPy=0; HT=0; MHT=0; cutHT=0; cutMHT=0; pt=0; coss=0; sinn=0;
  
    /////Here you should determine howmany events you need. If you need all the events, please comment this out. 
      //desirednumeve = 1000;
  
    TChain chain("Delphes");
    // Create object of class ExRootTreeReader
    ExRootTreeReader *treeReader = new ExRootTreeReader(&chain);

    //build a vector of histograms
    TH1D  weight_hist = TH1D("weight", "Weight Distribution", 5,0,5);
    vecTH.push_back(weight_hist);
    TH1D  RA2HT_hist =  TH1D("HT","HT Distribution",50,0,5000);
    vecTH.push_back(RA2HT_hist);
    TH1D  RA2MHT_hist =  TH1D("MHT","MHT Distribution",100,0,5000);
    vecTH.push_back(RA2MHT_hist);
    TH1D  RA2NJet_hist = TH1D("NJet","Number of Jets Distribution",20,0,20);
    vecTH.push_back(RA2NJet_hist);
    TH1D  RA2NBtagLoose_hist = TH1D("NBtagLoose","Number of BtagLoose Distribution",20,0,20);
    vecTH.push_back(RA2NBtagLoose_hist);
    TH1D  RA2NBtagTight_hist = TH1D("NBtagTight","Number of BtagTight Distribution",20,0,20);
    vecTH.push_back(RA2NBtagTight_hist);
    TH1D  BtagLoose1Pt_hist =  TH1D("BtagLoose1Pt","First Loose btag Pt Distribution",50,0,5000); //first Btag jet
    vecTH.push_back(BtagLoose1Pt_hist);
    TH1D  BtagLoose1Eta_hist = TH1D("BtagLoose1Eta","Eta of the first Loose btag",100,-5,5);
    vecTH.push_back(BtagLoose1Eta_hist);
    TH1D  BtagLoose1Phi_hist = TH1D("BtagLoose1Phi","Phi of the first Loose btag",50,-3.3,3.3);
    vecTH.push_back(BtagLoose1Phi_hist);
    TH1D  BtagLoose2Pt_hist =  TH1D("BtagLoose2Pt","Second Loose btag Pt Distribution",50,0,5000);
    vecTH.push_back(BtagLoose2Pt_hist);
    TH1D  BtagLoose2Eta_hist = TH1D("BtagLoose2Eta","Eta of the second Loose btag",100,-5,5);
    vecTH.push_back(BtagLoose2Eta_hist);
    TH1D  BtagLoose2Phi_hist = TH1D("BtagLoose2Phi","Phi of the second Loose btag",50,-3.3,3.3);
    vecTH.push_back(BtagLoose2Phi_hist);

    TH1D  BtagTight1Pt_hist =  TH1D("BtagTight1Pt","First Tight btag Pt Distribution",50,0,5000); //first Btag jet
    vecTH.push_back(BtagTight1Pt_hist);
    TH1D  BtagTight1Eta_hist = TH1D("BtagTight1Eta","Eta of the first Tight btag",100,-5,5);
    vecTH.push_back(BtagTight1Eta_hist);
    TH1D  BtagTight1Phi_hist = TH1D("BtagTight1Phi","Phi of the first Tight btag",50,-3.3,3.3);
    vecTH.push_back(BtagTight1Phi_hist);
    TH1D  BtagTight2Pt_hist =  TH1D("BtagTight2Pt","Second Tight btag Pt Distribution",50,0,5000);
    vecTH.push_back(BtagTight2Pt_hist);
    TH1D  BtagTight2Eta_hist = TH1D("BtagTight2Eta","Eta of the second Tight btag",100,-5,5);
    vecTH.push_back(BtagTight2Eta_hist);
    TH1D  BtagTight2Phi_hist = TH1D("BtagTight2Phi","Phi of the second Tight btag",50,-3.3,3.3);
    vecTH.push_back(BtagTight2Phi_hist);


    TH1D cutflowhist = TH1D("cutflowhist","Cut Flow", 30,0,30);
    for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){
      cutflowmap[itt->first]=cutflowhist;
    }

    //
    //initialize a map between string=cutnames and histvecs. copy one histvec into all of them. The histograms, though, will be filled differently.
    cutname[0]="RA2nocut";
    cutname[1]="RA2Asys";
    cutname[2]="RA2Inc3Jetcut";
    cutname[3]="RA2HT500cut";
    cutname[4]="RA2MHT200cut";
    cutname[5]="RA2delphicut";
    cutname[6]="RA2noleptoncut";
    cutname[7]="noPhotoncut";
    cutname[8]="RA2Inc4Jetcut";
    cutname[9]="RA2Inc5Jetcut";
    cutname[10]="RA2Inc6Jetcut";
    cutname[11]="RA2allbutHT2500cut";
    cutname[12]="RA2allbutMHT1000cut";
    cutname[13]="RA2allcut";
    cutname[14]="RA2noleptoncutMHT1000";
    cutname[15]="RA2noleptoncutBtag2";
    cutname[16]="RA2noleptoncutBtag2MHT1000";
    cutname[17]="RA2Inc4JetcutMHT1000";
    cutname[18]="RA2Inc4JetcutBtag2";
    cutname[19]="RA2Inc4JetcutBtag2MHT1000";
    cutname[20]="RA2Inc5JetcutMHT1000";
    cutname[21]="RA2Inc5JetcutBtag2";
    cutname[22]="RA2Inc5JetcutBtag2MHT1000";
    cutname[23]="RA2Inc6JetcutMHT1000";
    cutname[24]="RA2Inc6JetcutBtag2";
    cutname[25]="RA2Inc6JetcutBtag2MHT1000";

     for(int i=0; i< cutname.size();i++){
      cut_histvec_map[cutname[i]]=vecTH;
    }


    ///
    //initialize a map between string and maps. copy the map of histvecs into each
if(Process.find("BJ")!=string::npos){//for BJ background only
eventType[0]="allEvents";
eventType[1]="W";
eventType[2]="Wlv";
eventType[3]="Wjj";
eventType[4]="Z";
eventType[5]="Zll";
eventType[6]="Zvv";
eventType[7]="Zjj";
eventType[8]="photon";
eventType[9]="H";

}
else if(Process.find("TT")!=string::npos){//TTbar background only
eventType[0]="allEvents";
eventType[1]="TTbar";
eventType[2]="TTSingLep";
eventType[3]="TTdiLep";
eventType[4]="TThadronic";

}
else{//For all other types 
eventType[0]="allEvents";
eventType[1]="glgl";

}
/*We don't need for example TTbar events when considering BJ. This only slows the code. So the following is commented out.
    eventType[0]="allEvents";
    eventType[1]="W";
    eventType[2]="Wlv";
    eventType[3]="Wjj";
    eventType[4]="Z";
    eventType[5]="Zll";
    eventType[6]="Zvv";
    eventType[7]="Zjj";
    eventType[8]="photon";
    eventType[9]="H";
    eventType[10]="TTbar";
    eventType[11]="TTSingLep";
    eventType[12]="TTdiLep";
    eventType[13]="TThadronic";
*/
    for(int i=0; i< eventType.size();i++){
      map_map[eventType[i]]=cut_histvec_map;
    }
    //KH
    //     map_map["allEvents"]=cut_histvec_map;
    //     map_map["W"]=cut_histvec_map;
    //     map_map["Wlv"]=cut_histvec_map;
    //     map_map["Wjj"]=cut_histvec_map;
    //     map_map["Z"]=cut_histvec_map;
    //     map_map["Zll"]=cut_histvec_map;
    //     map_map["Zvv"]=cut_histvec_map;
    //     map_map["Zjj"]=cut_histvec_map;
    //     map_map["photon"]=cut_histvec_map;
    //     map_map["H"]=cut_histvec_map; 
    
    //
    //initialize histobjmap
    for(map<string , vector<TH1D> >::iterator it=cut_histvec_map.begin(); it!=cut_histvec_map.end();it++){
      histobjmap[it->first]=histObj;
    }

    //
    //Add the root files to a chain called Delphes
    sprintf(TreeList,"./FileList/%s/%s_%s_%s",Detector.c_str(),Process.c_str(),Pileup.c_str(),inputnumber.c_str());
if(Process.find("_HT")!=string::npos || Process.find("StauC")!=string::npos){input.open(TreeList,std::fstream::in);}//use this when running on Background. 
else{ if(!input.is_open()){sprintf(TreeList,"./FileList/%s/%s_%s.list",Detector.c_str(),Process.c_str(),Pileup.c_str());input.open(TreeList,std::fstream::in);}} ///use this line if running on signal or a file with *.list suffix.
    cout << "file name " << TreeList << endl; 
//reset the chain before loading the TTrees    
chain.Reset();
     for(std::string linee; getline(input, linee);)
      {
	if (linee[0] == '#') continue;
	std::cout << "Add File: " << linee << std::endl;
	chain.Add(linee.c_str());

if(desirednumeve != -999 ){if(desirednumeve < treeReader->GetEntries()) break;}
       }
    cout << " treeReader->GetEntries() " << treeReader->GetEntries()  << endl;
    if (chain.GetListOfFiles()->GetEntries() == 0)
      {
	std::cout << "No files attached! Exiting ...."  << std::endl;
	
      }
    //end of adding file

    //Get the cross-section from the file ./FileList/CrossSection.list
    //open the file
    file.open("FileList/CrossSection.list", std::fstream::in);
    if (!file.is_open())
      {
	std::cout << " Error to open the Cross Section file!" << std::endl;
      }

    while (getline(file, line)){
      if (line.empty()) continue;
      if (line[0] == '#') continue;

      stringstream ss; ss << line;
      ss >>  pro >> xs >> xserr;
      if (pro==Process){CrossSection = xs; CrossSectionError = xserr ; break;}
    }
   
    if (CrossSection == -999.)
      {
	std::cerr << "Unable to find the process and its cross section!" << std::endl;
    
      }
    cout<<"\nCrossSection : "<<CrossSection<<" +- "<<CrossSectionError<<endl<<endl;
    //end of acquiring XS

    // Get pointers to branches used in this analysis
//    branchEvent  = treeReader->UseBranch("Event");
    branchJet = treeReader->UseBranch("Jet");
    branchElectron = treeReader->UseBranch("Electron");
    branchMuon = treeReader->UseBranch("Muon");
    branchPhoton = treeReader->UseBranch("Photon");
    branchMet = treeReader->UseBranch("MissingET");
    branchHT = treeReader->UseBranch("ScalarHT");
    branchParticle = treeReader->UseBranch("Particle"); 
    //report the total number of events
    cout << "the total number of events: " << treeReader->GetEntries() << endl; 

    //======================================================================
    //
    //Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events/
    //
    for(int entry = 0; entry < treeReader->GetEntries() ; entry++ ){
          //KH if (entry >=10000) break; // Check only the first 10K events for validation purpose
     if(desirednumeve != -999 ){if(desirednumeve < entry) break;} 
      treeReader->ReadEntry(entry);

      //met and sht
      met =(MissingET*) branchMet->At(0);
      sht= (ScalarHT*) branchHT->At(0);

      //Set Weight
   //   LHEFEvent* event = (LHEFEvent*) branchEvent->At(0);//->At(1) and higher there in nothing. There is no point to make a vector.
   //   weight= event->Weight;
///////////////////
///To get the weight of events
DelWeight dw;
dw.initialize();
////////////////////
 
      //a counter
      if (entry % 5000 == 0){cout << "--------------------" << entry << endl;}

GenParticlevec.clear();
      ///loop over all the particles in the history of an event. load them to a vector
      for (int i = 0; i < branchParticle->GetEntries(); ++i)
        {
          // Define a pointer of class GenParticle and point it to the "entry"th event in the branch particle.
          // Hence, we have access to PID, status and other properties of the particles in the event. 
          GenParticle * particle = (GenParticle*)branchParticle->At(i);
          GenParticlevec.push_back(*particle);
	}//end of loop over "particles in history" 

///this is to calculate the weights
int isample; // 1: TTbar, 2: BJ, 0: All other samples
if(Process.find("TT")!=string::npos){isample=1;}
else if(Process.find("BJ")!=string::npos){isample=2;}
else{isample =0;}
weight = dw.weight(isample, GenParticlevec);
////////////////////////////////////////////////////////////


//////////////////loop over photons and load them to a vector
      photonvec.clear();
      for (int i = 0; i < branchPhoton->GetEntries(); ++i)
	{
	  Photon* pho = (Photon*)branchPhoton->At(i);
	  if (fabs(pho->Eta) > 5) 
	    continue;
	  photonvec.push_back(*pho);
	}

      ///here we load photons to another vector for another study
      phovec.clear();
      for (int i = 0; i < branchPhoton->GetEntries(); ++i)
        {
          Photon* pho = (Photon*)branchPhoton->At(i);
          if (pho->PT < 30)
            continue;
          phovec.push_back(*pho);
        }
      /////////////////end of loop over photons
      
      ////loop over electrons (load them to a vector)
      electronvec.clear();
      elecvec.clear();
      vecelecvec.clear();
      for(int elecn=0; elecn <branchElectron->GetEntries();elecn++)
	{
	  Electron* elec = (Electron*) branchElectron->At(elecn);
	  electronvec.push_back(*elec);
	  
	  ///for HT we want events with all elecs pt > 10 and |eta|< 2.5
//	  if(elec->PT > 10 && elec->Eta < 2.5 && elec->Eta > (-2.5))
        if(elec->PT > 10 && elec->Eta < 2.5 && elec->Eta > (-2.5) && elec->IsolationVar<0.2)
	    {
	      //the zeroth component is the tag of the elec/first:pt /second:phi/third:eta
	      elecvec.clear();
	      elecvec.push_back((double) elecn);
	      elecvec.push_back(elec->PT);
	      elecvec.push_back((double)elec->Phi);
	      elecvec.push_back((double)elec->Eta);
	      vecelecvec.push_back(elecvec);
	      /// end of if over pt and eta for HT
	    } 
	  ////end of loop over electrons
	}

      ////loop over muons    (load them to a vector)
      muonvec.clear();
      muvec.clear();
      vecmuvec.clear();
      for(int mun=0; mun <branchMuon->GetEntries();mun++)
	{
	  Muon* mu = (Muon*) branchMuon->At(mun);
	  muonvec.push_back(*mu);
	  
	  ///for HT we want events with all muons pt > 10 and |eta|< 2.4
//	  if(mu->PT > 10 && mu->Eta < 2.4 && mu->Eta > (-2.4))
          if(mu->PT > 10 && mu->Eta < 2.4 && mu->Eta > (-2.4)&& mu->IsolationVar<0.2 )
	    {
	      //the zeroth component is the tag of the mu/first:pt /second:phi/third:eta
	      muvec.clear();
	      muvec.push_back((double) mun);
	      muvec.push_back(mu->PT);
	      muvec.push_back((double)mu->Phi);
	      muvec.push_back((double)mu->Eta);
	      vecmuvec.push_back(muvec);
	      /// end of if over pt and eta for HT
	    }
	  ////end of loop over muons
	}

      ///loading tau to a vector. Unlike electron and muon there is no class for tau in Delphes. We need to 
      tauvec.clear();
      for(int i = 0; i < branchJet->GetEntries(); ++i){
        Jet* jet = (Jet*) branchJet->At(i);
        if(jet->TauTag==true && jet->PT >20 && fabs(jet->Eta)< 2.3){
          tauvec.push_back(jet);
        }
      }
      //cout << " tauSize " << tauvec.size() << endl;
      //if(tauvec.size()>0){cout << " tau->pt: " << tauvec[0]->PT << endl;}

      /*int genTaunum=0;
        for(int i = 0; i < (int)(GenParticlevec.size()); ++i){
        GenParticle * pa = &GenParticlevec.at(i);
        if(fabs(pa->PID)==15){genTaunum+=1;}
        }
        cout << "genTaunum: " << genTaunum << endl;
      */

      ///making the values zero for each event
      totPx=0;
      totPy=0;
      HT=0;


      //
      // Store the jets and btagging information
      vecJets.clear();
      vecBtagLoose.clear();
      vecBtagTight.clear();

      // 
      TLorentzVector v;
      double jet_pt_threshold  = 50.;
      double jet_eta_threshold = 2.5;
      for(int i=0; i <branchJet->GetEntries(); i++){
        v.SetPxPyPzE(0,0,0,0);
        Jet* jet = (Jet*) branchJet->At(i);
        v.SetPtEtaPhiM(jet->PT,jet->Eta,jet->Phi,jet->Mass);
        vecJets.push_back(v);
      /*  if (sqgl)
          std::cout << "Jet: " << jet->PT << " " << jet->Eta << " " << jet->Phi << " " << jet->Mass << " "
                    << jet->BTag << std::endl;*/
        if(jet->PT>jet_pt_threshold && fabs(jet->Eta)<jet_eta_threshold){
          vecJets.push_back(v);
          if (jet->BTag& (1 << 1)){  // Btag loose
            vecBtagLoose.push_back(v);
          }
          if (jet->BTag& (1 << 0)){  // Btag tight
            vecBtagTight.push_back(v);
          }
        }
      }


      ///////////////////////////////////////////////////////////////////loop over jets    (load them to a vector)
      jetvec.clear();
      vecjvec.clear();
      jvec.clear();
      //cout << " ...............................................branchJet->GetEntries() " << branchJet->GetEntries() << endl;
      for(int jetn=0; jetn <branchJet->GetEntries();jetn++){
	
	tempLorvec.SetPxPyPzE(0,0,0,0);
	
	Jet* jet = (Jet*) branchJet->At(jetn);
	
	////////////////////////////Modifying-Removing some Jets
	//Reference Ben
	//  --> Matching jet and lepton in the eta-phi plane. In case of matched,
	//  compare the energy fraction: 
	//  * fraction > 90% (mostly jet from the lepton), remove this jet
	//  * fraction < 90%: additional energy from pileup, just correct this jet by
	//  removing the lepton energy

        //KH: thought this adjustment to jet 4-momentum was no longer necessary.
        //    it was meant for early version of delphes, where identified leptons
        //    were still included in jets, which lead to double-counting.
      
        /*
	for (int i = 0; i < muonvec.size(); ++i){
	  Muon muon = muonvec.at(i);
	  if (jet->P4().DeltaR(muon.P4()) < 0.4){tempLorvec += muon.P4();}
	}
	
	for (int i = 0; i < electronvec.size(); ++i){
	  Electron elec = electronvec.at(i);
	  if (jet->P4().DeltaR(elec.P4()) < 0.4){tempLorvec += elec.P4();}
	}
	
	for (int i = 0; i < photonvec.size(); ++i){
	  Photon pho = photonvec.at(i);
	  if (jet->P4().DeltaR(pho.P4()) < 0.4){tempLorvec += pho.P4();}
	}
	if((tempLorvec.E() )/( jet->P4().E() ) > 0.9){continue;}
	if((tempLorvec.E() )/( jet->P4().E() ) < 0.9 && (tempLorvec.E() )/( jet->P4().E() ) > 0.0){
	  // Projection of the tempLorvec in the 
	  //jet direction is  
	  // (\vec{jet}.\vec{temp})/(\vec{jet}.\vec{jet})*\vec{jet}
	  //now we use tempLorvec as the projected vector
	  tempLorvec = (jet->P4().Dot(tempLorvec))/(jet->P4().Dot(jet->P4())) * jet->P4();  
	  
	  jet->P4() = jet->P4() - tempLorvec;
	  jet->P4() = jet->P4();
          jet->PT  =  jet->P4().Pt();
	  jet->Eta =  jet->P4().Eta();
	  jet->Phi  = jet->P4().Phi();
	  jet->Mass = jet->P4().M();
	  
	}
        */

	sinn = (double) sin(jet->Phi);
	coss = (double) cos(jet->Phi);
	pt = jet->PT;
	jetvec.push_back(*jet);
	
	///for HT we want events with all jets pt > 50 and |eta|< 2.5
	//if(jetremove(jet,muonvec,electronvec,photonvec)==false && pt>50 && jet->Eta < 2.5 && jet->Eta > (-2.5))
	if(pt>50 && jet->Eta < 2.5 && jet->Eta > (-2.5))
	  {
	    //the zeroth component is the tag of the jet/first:pt /second:phi/third:eta
	    jvec.clear();
	    jvec.push_back((double) jetn);
	    jvec.push_back(pt);
	    jvec.push_back((double)jet->Phi);
	    jvec.push_back((double)jet->Eta);
	    vecjvec.push_back(jvec);
	    ///calculate HT
	    HT+=pt;
	    /// end of if over pt and eta for HT
	  }

	//// for MHT we want events with all jets pt > 30 and |eta|< 5
	if(pt>30 && jet->Eta < 5 && jet->Eta > (-5))
	  {
	    ///calculate total jet-px and jet-py
	    totPx += pt * coss;  
	    totPy += pt * sinn;
	    ///end of if over pt and eta for MHT
	  }
      }///////////////////////////////////////////////////////////////////end of loop over jets

      ///find the three most energetic jets
      terminator=1;
      while(terminator!=0){
	terminator=0;
	for(int iv=0; iv<((int)vecjvec.size()-1);iv++){
	  
	  if(vecjvec[iv][1]<vecjvec[iv+1][1]){
	    swap(vecjvec[iv],vecjvec[iv+1]);
	    terminator+=1;
	  }
	  //end of the for
}
	//end of the while
      }
      ///end of find the three most energetic jets

      ///calculate MHT
      MHT = sqrt( totPx*totPx + totPy*totPy );

      //build an array that contains the quantities we need a histogram for. Here order is important and must be the same as RA2nocutvec
double BtagLoose1pt=0;
double BtagLoose1Eta=-1000;
double BtagLoose1Phi=-1000;
double BtagLoose2pt=0;
double BtagLoose2Eta=-1000;
double BtagLoose2Phi=-1000;
if((int)vecBtagLoose.size()>=1){BtagLoose1pt=vecBtagLoose[0].Pt();BtagLoose1Eta=vecBtagLoose[0].Eta();BtagLoose1Phi=vecBtagLoose[0].Phi();}
if((int)vecBtagLoose.size()>=2){BtagLoose2pt=vecBtagLoose[1].Pt();BtagLoose2Eta=vecBtagLoose[1].Eta();BtagLoose2Phi=vecBtagLoose[1].Phi();}

double BtagTight1pt=0;
double BtagTight1Eta=-1000;
double BtagTight1Phi=-1000;
double BtagTight2pt=0;
double BtagTight2Eta=-1000;
double BtagTight2Phi=-1000;
if((int)vecBtagTight.size()>=1){BtagTight1pt=vecBtagTight[0].Pt();BtagTight1Eta=vecBtagTight[0].Eta();BtagTight1Phi=vecBtagTight[0].Phi();}
if((int)vecBtagTight.size()>=2){BtagTight2pt=vecBtagTight[1].Pt();BtagTight2Eta=vecBtagTight[1].Eta();BtagTight2Phi=vecBtagTight[1].Phi();}

///Important: here order is sensitive. The order must be the same as that of histograms in vecTH.
double eveinfvec[] = {
weight, 
HT, 
MHT, 
vecjvec.size(),
vecBtagLoose.size(), 
vecBtagTight.size(),
BtagLoose1pt,
BtagLoose1Eta,
BtagLoose1Phi,
BtagLoose2pt,
BtagLoose2Eta,
BtagLoose2Phi,
BtagTight1pt,
BtagTight1Eta,
BtagTight1Phi,
BtagTight2pt,
BtagTight2Eta,
BtagTight2Phi
}; //the last one gives the RA2 defined number of jets.

      //
      //loop over all the different event types: "allEvents", "Wlv", "Zvv"
      for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){//this will be terminated after the cuts
	
	//
	//cout << "bg_type:  " << itt->first << ", bool:  " << bg_type(itt->first , GenParticlevec) << endl;
	//determine what type of background should pass
	if(bg_type(itt->first , GenParticlevec)==true){//all the cuts are inside this

	  //
	  //Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts

	  //
	  //loop over cut names and fill the histograms
	  for(map<string , vector<TH1D> >::iterator ite=cut_histvec_map.begin(); ite!=cut_histvec_map.end();ite++){
	    if(checkcut(ite->first)==true){histobjmap[ite->first].fill( &eveinfvec[0] ,&itt->second[ite->first][0]);}
	  }//end of loop over cut names
	  
	  //
	  //EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts

	}//end of bg_type determination
      }//end of loop over all the different event types: "allEvents", "Wlv", "Zvv"
      
    }///End of loop over all Events//end of loop over events//end of loop over events//end of loop over events//end of loop over events//end of loop over events//
    //======================================================================

    //
    //fill the cut flow here
    //we generate one cut flow hist for each bg type. bins inside each histogram correspond to different cut names
    int nnn;
    for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){
      //KH
      //std::cout << itt->first.c_str() << std::endl;
      nnn=0;

      //KH
      for(int i=0; i< (int)cutname.size();i++){
      for(map<string , vector<TH1D> >::iterator it=itt->second.begin(); it!=itt->second.end();it++){
	
	if (cutname[i]==it->first){
	  //KH
	  //std::cout << i << " " << cutname[i] << " " << it->first.c_str() << " " << it->second[1].GetEntries() << std::endl;
	  //
	  cutflowmap[itt->first].Fill(it->first.c_str(),it->second[1].GetEntries());	  

	}
	nnn+=1;
      } // it
      } // cutname[i]
    }   // itt

    //
    //open a file to write the histograms
    sprintf(tempname,"%s/results_%s_%s_%s_%s.root",Outdir.c_str(),Detector.c_str(),Process.c_str(),Pileup.c_str(),inputnumber.c_str());
    TFile *resFile = new TFile(tempname, "RECREATE");
    TDirectory *cdtoitt;
    TDirectory *cdtoit;

    //
    // Loop over different event categories (e.g. "All events, Wlnu, Zll, Zvv, etc")
    for(int iet=0;iet<(int)eventType.size();iet++){
    for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){
    if (eventType[iet]==itt->first){

      //KH
      //std::cout << (itt->first).c_str() << std::endl;
      cdtoitt = resFile->mkdir((itt->first).c_str());
      cdtoitt->cd();
      cutflowmap[itt->first].Write("CutFlow");
      //KH
      //cutflowmap[itt->first].Print("all");
      
      //
      // Loop over different cut flow stages (e.g. RA2nocut, RA2Inc3Jetcut, ...)
      //KH
      for(int i=0; i< (int)cutname.size();i++){
      for(map<string , vector<TH1D> >::iterator it=itt->second.begin(); it!=itt->second.end();it++){ 
      if (cutname[i]==it->first){
	//KH
	//std::cout << (it->first).c_str() << std::endl;
	cdtoit =  cdtoitt->mkdir((it->first).c_str());
	cdtoit->cd();

	int nHist = it->second.size();
	//KH for(int i=0; i<=3; i++){//since we only have 4 type of histograms 
	for(int i=0; i<nHist; i++){//since we only have 4 type of histograms 
	  //KH sprintf(tempname,"%s_%s_hist%d",(itt->first).c_str(),(it->first).c_str(),i);
	  sprintf(tempname,"%s_%s_%s",it->second[i].GetName(),(it->first).c_str(),(itt->first).c_str());
	  it->second[i].Write(tempname);
	}

	cdtoitt->cd();
      }
      } 
      }

    }
    }
    }
    file.close();
    input.close(); 

  }//end of the constructor
};//end of class mainClass

//
int main( int argc, char *argv[] )
{

if (argc != 6)
  {
    std::cout << "Please enter the pileup, process name,  Dir_Eta_PT and Detector to be run on ! " <<  std::endl;
    return EXIT_FAILURE;
  }

  const string Pileu   = argv[1];
  const string proc    = argv[2];
  const string Outd    = argv[4];
  const string Detect  = argv[3];
  const string inputn  = argv[5];

mainClass mainObj(Pileu,proc,Detect,Outd ,inputn);



/////////////////////////////////////////////////////////////////////////////////
//mainClass mainObj("NoPileUp","T1qqqq_14TEV_2200_100","PhaseI", "Results","00");
//mainClass mainObj("NoPileUp","T1qqqq_14TEV","PhaseI", "Results","00");
//mainClass mainObj1_BJ("NoPileUp","BJ_14TEV_HT1","PhaseI", "Results","00");
//mainClass mainObj2_BJ("NoPileUp","BJ_14TEV_HT2","PhaseI", "Results","00");
//mainClass mainObj3_BJ("NoPileUp","BJ_14TEV_HT3","PhaseI", "Results","00");
//mainClass mainObj4_BJ("NoPileUp","BJ_14TEV_HT4","PhaseI", "Results","00");
//mainClass mainObj5_BJ("NoPileUp","BJ_14TEV_HT5","PhaseI", "Results","00");
//mainClass mainObj6_BJ("NoPileUp","BJ_14TEV_HT6","PhaseI", "Results","00");
//mainClass mainObj7_BJ("NoPileUp","BJ_14TEV_HT7","PhaseI", "Results","00");
//mainClass mainObj1_TT("NoPileUp","TT_14TEV_HT1","PhaseI", "Results","00");
//mainClass mainObj2_TT("NoPileUp","TT_14TEV_HT2","PhaseI", "Results","00");
//mainClass mainObj3_TT("NoPileUp","TT_14TEV_HT3","PhaseI", "Results","00");
//mainClass mainObj4_TT("NoPileUp","TT_14TEV_HT4","PhaseI", "Results","00");
//mainClass mainObj5_TT("NoPileUp","TT_14TEV_HT5","PhaseI", "Results","00");

return 0;
}

