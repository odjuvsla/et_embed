#include "AliAnalysisEtEmbedder.h"
#include "TFile.h"
#include "TTree.h"
#include <iostream>

ClassImp(AliAnalysisEtEmbedder);

AliAnalysisEtEmbedder::AliAnalysisEtEmbedder() :TObject()
,fTreeName()
,fBranchName()
,fSimDigitTree(0)
{

}

AliAnalysisEtEmbedder::~AliAnalysisEtEmbedder()
{

}

Int_t AliAnalysisEtEmbedder::LoadDigitsFromFile(TString filename)
{
  TFile *f = TFile::Open(filename, "READ");
  
  fSimDigitTree = (TTree*)(f->Get(fTreeName));
  std::cout << "Sim tree: " << fSimDigitTree << std::endl;
  return 0;
}

