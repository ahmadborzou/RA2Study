/*
Code's main Structure:
First in the int main() the constructor of mainClass will be called. In this mainClass particles will be loaded, all the cuts will be applied and histograms will be filled. To
fill the histograms from inside mainClass the constructor of histClass will be called.

To define a cut first name it inside the cutname which is a map. Then tell what cuts should be applied, when this name is called, inside checkcut()
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
#include <vector>
#include <map>
#include "Delphes-3.0.10/external/ExRootAnalysis/ExRootTreeReader.h"
#include "Delphes-3.0.10/classes/DelphesClasses.h"
using namespace std;


class histClass{
double * a;
TH1D * b_hist;
public:
void fill(double * eveinfarr_, TH1D * hist_){
a = eveinfarr_;
b_hist=hist_;
(*b_hist).Fill(*a);
for(int i=1; i<=3 ; i++){
(*(b_hist+i)).Fill(*(a+i),*a);
}
}
};

//define a function to evaluate delta phi
double delphi(vector<double> a, double tPx, double tPy,double mht){
//-totpx is the ptx comp of MHT
double jetpt = sqrt((a[1]*cos(a[2]))*(a[1]*cos(a[2]))+(a[1]*sin(a[2]))*(a[1]*sin(a[2])));
double MHT_Jet_Dot = (-tPx*(a[1]*cos(a[2]))-tPy*(a[1]*sin(a[2])));
double deltaphi = acos(MHT_Jet_Dot/(mht*jetpt));
return deltaphi;
///end of function deltaphi
}

//this function is exclusively written for BJ processes with emphesis on one B.
bool bg_type(string bg_ ,vector<GenParticle*> pvec){

if(bg_=="allEvents"){return 1;}

if(bg_=="Zvv"){
vector<int> vvvec;
for(int i = 0; i < pvec.size(); ++i){
GenParticle * p = pvec.at(i);
if(fabs(p->PID)==23){//23 is the PID code of Z boson.
vvvec.clear();
for(int j = 0; j < pvec.size(); ++j){
GenParticle * pp = pvec.at(j);
//if (pp->Status != 3 || pp->M1 > pvec.size() || pp->M2 > pvec.size() ){continue;}// Assuming the status 3 electron directly from Z. Delphes has broken mother links, can't track back to mother Z
if (pp->Status == 3 && pp->M1 < pvec.size() && pp->M2 < pvec.size() && fabs(pp->PID) == 12 || fabs(pp->PID) == 14 || fabs(pp->PID) == 16 ){
vvvec.push_back(pp->PID);
}//end of if pp->PID == 12, 14, 16 = nutrinos
}//end of second loop
if(vvvec.size()==2){
return true;}//end of if 
}//end of if PID==23=Z boson
}//end of loop
return false;
}//end of if Zvv


if(bg_=="Wlv"){
vector<int> llvec;
for(int i = 0; i < pvec.size(); ++i){
GenParticle * pa = pvec.at(i);
if(fabs(pa->PID)==24){//+-24 are the PID codes of W bosons.
llvec.clear();
for(int j = 0; j < pvec.size(); ++j){
GenParticle * ppa = pvec.at(j);
//if (ppa->Status != 3 || ppa->M1 > pvec.size() || ppa->M2 > pvec.size() ){continue;}// Assuming the status 3 electron directly from W. Delphes has broken mother links, can't track back to mother W
if (ppa->Status == 3 && ppa->M1 < pvec.size() && ppa->M2 < pvec.size() && fabs(ppa->PID) == 11 || fabs(ppa->PID) == 13 || fabs(ppa->PID) == 15 ){
llvec.push_back(ppa->PID);
}//end of if ppa->PID == 11, 13, 15 = electron , muon, tau
}//end of second loop
if(llvec.size()==1){//llvec.size() ==1 since W decays to one lepton and one nutrino
return true;}//end of if 
}//end of if PID==24=W boson
}//end of loop
return false;
}//end of if Wlv


} //end of function bg_type


bool jetremove(Jet * jet , vector<Muon> muonvec, vector<Electron> electronvec, vector<Photon> photonvec){
////////////////////////////Modifying-Removing some Jets
//Reference Ben
//  --> Matching jet and lepton in the eta-phi plane. In case of matched,
//  compare the energy fraction: 
//  * fraction > 90% (mostly jet from the lepton), remove this jet
//  * fraction < 90%: additional energy from pileup, just correct this jet by
//  removing the lepton energy
for(int i = 0; i < muonvec.size(); ++i){
if(jet->P4().DeltaR(muonvec[i].P4()) < .4) //hint:jet->P4() returns a TLorentzVector
{
if(muonvec[i].P4().E()/jet->P4().E() > .9) return true;
}
}
for(int i = 0; i < electronvec.size(); ++i){
if(jet->P4().DeltaR(electronvec[i].P4()) < .4)
{
if(electronvec[i].P4().E()/jet->P4().E() > .9) return true;
}
}
for(int i = 0; i < photonvec.size(); ++i){
if(jet->P4().DeltaR(photonvec[i].P4()) < .4)
{
if(photonvec[i].P4().E()/jet->P4().E() > .9)
{//cout << " photon: deltar " << jet->P4().DeltaR(photonvec[i].P4()) << ", energy fraction " << photonvec[i].P4().E()/jet->P4().E() <<  endl; 
 return true;
}
}
}

return false; //if false is returned, jet shouldn't be removed

}//end of jetremove()

bool METMHTAsys(string PileUp ,MissingET* met,vector<Jet> jetvec,vector<Muon> muonvec,vector<Electron> electronvec,vector<Photon> photonvec){
double Met=-99;
double METAsys=-99;
double AsysCut = -99;
TVector2 PUCorMet;
if (PileUp == "NoPileUp") AsysCut = 0.2;
if (PileUp == "50PileUp") AsysCut = 0.3;
if (PileUp == "140PileUp") AsysCut = 0.5;
assert(AsysCut != -99.);
TLorentzVector allvecsum;
allvecsum.SetPxPyPzE(0, 0, 0, 0);
PUCorMet.Set(0., 0.);
for(int i=0; i<jetvec.size(); i++) {allvecsum += jetvec.at(i).P4();}
for(int j=0; j<muonvec.size(); j++) {allvecsum += muonvec.at(j).P4();}
for(int k=0; k<electronvec.size(); k++) {allvecsum += electronvec.at(k).P4();}
for(int l=0; l<photonvec.size(); l++) {allvecsum += photonvec.at(l).P4();}


PUCorMet.Set(-allvecsum.Px(),-allvecsum.Py());
Met= PUCorMet.Mod();

METAsys=fabs(Met-(met->MET))/(Met+(met->MET));

return METAsys < AsysCut;
}



///////////////////////////////////////////
//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()//Begining of the main()
////////////////////////////////////////////
class mainClass{
//List of variables
int terminator;
float xs, xserr;
double weight, CrossSection, CrossSectionError, totPx, totPy, HT, MHT, cutHT, cutMHT, pt, coss, sinn;
vector<vector<double> > vecjvec, vecelecvec, vecmuvec;
vector<double> jvec, elecvec, muvec;
vector<GenParticle*> GenParticlevec;
vector<TH1D > vec;
vector<Photon> photonvec; vector< Electron> electronvec; vector<Muon> muonvec;vector<Jet> jetvec; 
char TreeList[200], tempname[200];
string pro, line;
map<int, string> cutname;
fstream file, input, cutflowfile;
map<string, TH1D> cutflowmap;
map<string , vector<TH1D> > cut_histvec_map;  
map<string, map<string , vector<TH1D> > > map_map;
map<string, histClass> histobjmap;
histClass histObj;
//define different cuts here
bool threejet(){if(vecjvec.size() >= 3 && vecjvec[0][1]> 50 )return true; return false;}
bool ht(){if(HT>=500) return true; return false;}
bool mht(){if(MHT>=200)return true; return false;}
bool dphi(){if(delphi(vecjvec[0],totPx,totPy,MHT)>0.5 && delphi(vecjvec[1],totPx,totPy,MHT)>0.3 && delphi(vecjvec[2],totPx,totPy,MHT)>0.3)return true; return false;}
bool nolep(){if(vecelecvec.size()==0 && vecmuvec.size()==0)return true; return false;}
bool fourjet(){if(vecjvec.size() >= 4)return true; return false;}
bool fivejet(){if(vecjvec.size() >= 5)return true; return false;}
bool sixjet(){if(vecjvec.size() >= 6)return true; return false;}
bool highMht(){if(MHT>=1000)return true; return false;}
bool highHt(){if(HT>=2500)return true; return false;}
//Reference: Ben
//there are jets missing in the event, which cause much larger MHT than expected. In order to supress this problem,
//bool Asys(){if(METMHTAsys(Pileup,met,jetvec,muonvec,electronvec,photonvec))return true; return false;}


//function checkcut()
bool checkcut(string ss){
if(ss == cutname[0])return true;
if(ss== cutname[1]){if(threejet())return true;}
if(ss== cutname[2]){if(threejet() && ht())return true;}
if(ss== cutname[3]){if(threejet()&&ht()&&mht())return true;}
if(ss== cutname[4]){if(threejet()&&ht()&&mht()&&dphi())return true;}
if(ss== cutname[5]){if(threejet()&&ht()&&mht()&&dphi()&&nolep())return true;}
if(ss== cutname[6]){if(threejet()&&ht()&&mht()&&dphi()&&nolep()&&fourjet())return true;}
if(ss== cutname[7]){if(threejet()&&ht()&&mht()&&dphi()&&nolep()&&fivejet())return true;}
if(ss== cutname[8]){if(threejet()&&ht()&&mht()&&dphi()&&nolep()&&sixjet())return true;}
if(ss== cutname[9]){if(threejet()&&ht()&&mht()&&dphi()&&nolep()&&sixjet()&&highMht())return true;}
if(ss== cutname[10]){if(threejet()&&ht()&&mht()&&dphi()&&nolep()&&sixjet()&&highHt())return true;}
if(ss== cutname[11]){if(threejet()&&ht()&&mht()&&dphi()&&nolep()&&sixjet()&&highHt()&&highMht())return true;}

return false; 
}

//constructor
public:
mainClass(string Pileup, string Process, string Detector, string Outdir, string inputnumber){
terminator=0; CrossSection=-999.0; CrossSectionError=0.0; totPx=0; totPy=0; HT=0; MHT=0; cutHT=0; cutMHT=0; pt=0; coss=0; sinn=0;
TChain chain("Delphes");

//build a vector of histograms
TH1D  weight_hist = TH1D("weight", "Weight Distribution", 5,0,5);
vec.push_back(weight_hist);
TH1D  RA2HT_hist =  TH1D("HT","HT Distribution",50,0,5000);
vec.push_back(RA2HT_hist);
TH1D  RA2MHT_hist =  TH1D("MHT","MHT Distribution",100,0,5000);
vec.push_back(RA2MHT_hist);
TH1D  RA2NJet_hist = TH1D("NJet","Number of Jets Distribution",10,0,20);
vec.push_back(RA2NJet_hist);


TH1D cutflowhist = TH1D("cutflowhist","Cut Flow", 30,0,30);
for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){
cutflowmap[itt->first]=cutflowhist;
}

//initialize a map between string=cutnames and histvecs. copy one histvec into all of them. The histograms, though, will be filled differently.
cutname[0]="RA2nocut";cutname[1]="RA23Jetcut";cutname[2]="RA2HT500cut" ;cutname[3]="RA2MHT200cut" ;cutname[4]="RA2delphicut" ;cutname[5]="RA2noleptoncut" ;cutname[6]="RA24Jetcut" ;cutname[7]="RA25Jetcut" ;cutname[8]="RA26Jetcut" ;cutname[9]="RA2allbutHT2500cut" ;cutname[10]="RA2allbutMHT1000cut";cutname[11]= "RA2allcut";
for(int i=0; i< cutname.size();i++){
cut_histvec_map[cutname[i]]=vec;
}

//initialize a map between string and maps. copy the map of histvecs into each
map_map["allEvents"]=cut_histvec_map; map_map["Zvv"]=cut_histvec_map; map_map["Wlv"]=cut_histvec_map;

//initialize histobjmap
for(map<string , vector<TH1D> >::iterator it=cut_histvec_map.begin(); it!=cut_histvec_map.end();it++){
histobjmap[it->first]=histObj;

}

///Add the root files to a chain called Delphes
sprintf(TreeList,"./FileList/%s/%s_%s_%s",Detector.c_str(),Process.c_str(),Pileup.c_str(),inputnumber.c_str());
input.open(TreeList,std::fstream::in);
if(!input.is_open()){sprintf(TreeList,"./FileList/%s/%s_%s.list",Detector.c_str(),Process.c_str(),Pileup.c_str());input.open(TreeList,std::fstream::in);}
cout << "file name " << TreeList << endl; 
for(std::string linee; getline(input, linee);)
    {
      if (linee[0] == '#') continue;
      std::cout << "Add File: " << linee << std::endl;
      chain.Add(linee.c_str());
    }
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

// Create object of class ExRootTreeReader
  ExRootTreeReader *treeReader = new ExRootTreeReader(&chain);
// Get pointers to branches used in this analysis
TClonesArray * branchEvent  = treeReader->UseBranch("Event");
TClonesArray * branchJet = treeReader->UseBranch("Jet");
TClonesArray * branchElectron = treeReader->UseBranch("Electron");
TClonesArray * branchMuon = treeReader->UseBranch("Muon");
TClonesArray * branchPhoton = treeReader->UseBranch("Photon");
TClonesArray * branchMet = treeReader->UseBranch("MissingET");
TClonesArray * branchHT = treeReader->UseBranch("ScalarHT");
TClonesArray * branchParticle = treeReader->UseBranch("Particle"); 
//report the total number of events
cout << "the total number of events: " << treeReader->GetEntries() << endl; 

//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events//Loop Over all Events/
//for(int entry = 0; entry < treeReader->GetEntries() ; entry++ ){
for(int entry = 0; entry < 1000; entry++){
treeReader->ReadEntry(entry);

//Set Weight
LHEFEvent* event = (LHEFEvent*) branchEvent->At(0);
weight= event->Weight;

//a counter
if (entry % 5000 == 0){cout << "--------------------" << entry << endl;}

//MET and HT of the event
MissingET* met =(MissingET*) branchHT->At(0); 
//cout << "MET: " << met->MET << endl;
//cout << "MHT: " << MHT << endl;
ScalarHT* ht= (ScalarHT*) branchHT->At(0);
//cout << "HT : " << ht->HT << endl;

GenParticlevec.clear();
///loop over all the particles in the history of an event. load them to a vector
for (int i = 0; i < branchParticle->GetEntries(); ++i)
  {
///define a pointer of class GenParticle and point it to the "entry"th event in the branch particle. Hence, we have access to PID, status and other properties of the particles in the event. 
GenParticle * particle = (GenParticle*)branchParticle->At(i);
GenParticlevec.push_back(particle);
}//end of loop over "particles in history" 

//////////////////loop over photons and load them to a vector
for (int i = 0; i < branchPhoton->GetEntries(); ++i)
  {
    Photon* pho = (Photon*)branchPhoton->At(i);
    if (fabs(pho->Eta) > 5)
      continue;
    photonvec.push_back(*pho);
  }
/////////////////end of loop over photons

////loop over electrons (load them to a vector)
electronvec.clear();
elecvec.clear();
vecelecvec.clear();
for(int elecn=0; elecn <branchElectron->GetEntries();elecn++)
{

Electron* elec = (Electron*) branchElectron->At(elecn);
///for HT we want events with all elecs pt > 10 and |eta|< 2.5
if(elec->PT > 10 && elec->Eta < 2.5 && elec->Eta > (-2.5))
{
//the zeroth component is the tag of the elec/first:pt /second:phi/third:eta
elecvec.push_back((double) elecn);
elecvec.push_back(elec->PT);
elecvec.push_back((double)elec->Phi);
elecvec.push_back((double)elec->Eta);
vecelecvec.push_back(elecvec);
electronvec.push_back(*elec);
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
///for HT we want events with all muons pt > 10 and |eta|< 2.4
if(mu->PT > 10 && mu->Eta < 2.4 && mu->Eta > (-2.4))
{
//the zeroth component is the tag of the mu/first:pt /second:phi/third:eta
muvec.push_back((double) mun);
muvec.push_back(mu->PT);
muvec.push_back((double)mu->Phi);
muvec.push_back((double)mu->Eta);
vecmuvec.push_back(muvec);
muonvec.push_back(*mu);
/// end of if over pt and eta for HT
}
////end of loop over muons
}
///making the values zero for each event
totPx=0;
totPy=0;
HT=0;
///////////////////////////////////////////////////////////////////loop over jets    (load them to a vector)
jetvec.clear();
vecjvec.clear();
jvec.clear();
for(int jetn=0; jetn <branchJet->GetEntries();jetn++){
Jet* jet = (Jet*) branchJet->At(jetn);
sinn = (double) sin(jet->Phi);
coss = (double) cos(jet->Phi);
pt = jet->PT;
jetvec.push_back(*jet);

///for HT we want events with all jets pt > 50 and |eta|< 2.5
//if(pt>50 && jet->Eta < 2.5 && jet->Eta > (-2.5) && jetremove(jet,muonvec,electronvec,photonvec)==false)
if(pt>50 && jet->Eta < 2.5 && jet->Eta > (-2.5))
{
//the zeroth component is the tag of the jet/first:pt /second:phi/third:eta
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
while(terminator!=0){
terminator=0;
for(int iv=0; iv<vecjvec.size()-1;iv++){

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

//build and array that contains the quantities we need a histogram for. Here order is important and must be the same as RA2nocutvec
double eveinfvec[] = {weight, HT, MHT , vecjvec.size()}; //the last one gives the RA2 defined number of jets.


//loop over all the different backgrounds: "allEvents", "Wlv", "Zvv"
for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){//this will be terminated after the cuts

//cout << "bg_type:  " << itt->first << ", bool:  " << bg_type(itt->first , GenParticlevec) << endl;
//determine what type of background should pass
if(bg_type(itt->first , GenParticlevec)==true){//all the cuts are inside this


//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts//Cuts

//loop over cut names and fill the histograms
for(map<string , vector<TH1D> >::iterator ite=cut_histvec_map.begin(); ite!=cut_histvec_map.end();ite++){
if(checkcut(ite->first)==true){histobjmap[ite->first].fill( &eveinfvec[0] ,&itt->second[ite->first][0]);}
}//end of loop over cut names

//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts//EndOfCuts


}//end of bg_type determination
}//end of loop over all the different backgrounds: "allEvents", "Wlv", "Zvv"

}///End of loop over all Events//end of loop over events//end of loop over events//end of loop over events//end of loop over events//end of loop over events//


//fill the cut flow here
//we generate one cut flow hist for each bg type. bins inside each histogram correspond to different cut names
int nnn;
for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){
nnn=0;
for(map<string , vector<TH1D> >::iterator it=itt->second.begin(); it!=itt->second.end();it++){
//cutflowmap[itt->first].GetXaxis()->SetBinLabel(nnn+1,it->first.c_str());
//cutflowmap[itt->first].SetBinContent(nnn,it->second[1].GetEntries());
cutflowmap[itt->first].Fill(it->first.c_str(),it->second[1].GetEntries());
nnn+=1;
}
}




//open a file to write the histograms
sprintf(tempname,"%s/results_%s_%s_%s_%s.root",Outdir.c_str(),Detector.c_str(),Process.c_str(),Pileup.c_str(),inputnumber.c_str());
TFile *resFile = new TFile(tempname, "RECREATE");
TDirectory *cdtoitt;
TDirectory *cdtoit;

for(map<string, map<string , vector<TH1D> > >::iterator itt=map_map.begin(); itt!=map_map.end();itt++){
cdtoitt = resFile->mkdir((itt->first).c_str());
cdtoitt->cd();
cutflowmap[itt->first].Write("CutFlow");
for(map<string , vector<TH1D> >::iterator it=itt->second.begin(); it!=itt->second.end();it++){ 
cdtoit =  cdtoitt->mkdir((it->first).c_str());
cdtoit->cd();
for(int i=0; i<=3; i++){//since we only have 4 type of histograms 
sprintf(tempname,"%s_%s_hist%d",(itt->first).c_str(),(it->first).c_str(),i);
it->second[i].Write(tempname);
}

cdtoitt->cd();
}
}
file.close();
input.close(); 




}//end of the constructor
};//end of class mainClass








int main()
{

mainClass mainObj("NoPileUp","T1qqqq_14TEV","PhaseI", "Results","00");
//mainClass mainObj_BJ("NoPileUp","BJ_14TEV_HT1","PhaseI", "Results","00");


return 0;
}

