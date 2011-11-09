#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TObjArray.h"
#include "AliPHOSDigit.h"
#include "AliRunLoader.h"
#include "TClonesArray.h"
#include "AliPHOSLoader.h"
#include "AliCDBManager.h"
#include "AliGeomManager.h"
#include "AliCDBEntry.h"
#include "AliPHOSCalibData.h"
#include "AliPHOSEmcCalibData.h"
#include "AliPHOSEmcBadChannelsMap.h"
#include <AliPHOSRecoParam.h>
#include <AliPHOSGeoUtils.h>
#include <AliPHOSGeometry.h>
#include <AliPHOSRecoParam.h>
#include <AliCaloAltroMapping.h>
#include "AliHeader.h"
#include "AliCentralTrigger.h"

#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <TChain.h>
#include <AliLog.h>
#include <TRandom.h>
#include <AliDetectorEventHeader.h>
#include <TMath.h>
#include <TH2D.h>

int getSimulatedDigits(TString digitdir);
int mergeDigits(TString digitdir, Int_t offset = 0);
int getFromFile(TString filename);
int retreiveCalibrationData(Int_t runNb);
int createBadChannelMap();
int printDigits();
int addBadChannels(Float_t extra);


TTree *simTree = 0;
TTree *realTree = 0;
AliPHOSEmcBadChannelsMap *phosBcm = 0;
AliPHOSEmcCalibData *phosCalibData = 0;
AliPHOSGeoUtils *geoUtils = 0;
AliPHOSGeometry *geom = 0;
AliPHOSRecoParam *recoParam = 0;

std::vector<Int_t> badChannels;

int digitEmbedder(Bool_t fromFile = true, Int_t simEvOffset = 0, TString filename = "mydigits.root")
{
    AliLog::SetGlobalLogLevel(AliLog::kInfo);
    if (!fromFile)
    {
        retreiveCalibrationData(137366);
        getSimulatedDigits(".");
    }
    else
    {
        retreiveCalibrationData(137366);
        //addBadChannels(0.0);
        getFromFile(filename);
        mergeDigits(".", simEvOffset);
    }

    return 0;
}

int getSimulatedDigits(TString digitdir)
{

    AliRunLoader *rl = AliRunLoader::Open(digitdir+TString("/galice.root"));

    AliPHOSLoader *prl = (AliPHOSLoader*)rl->GetDetectorLoader("PHOS");

    prl->LoadSDigits();
    prl->LoadDigits();

    Int_t nSimEvents = rl->GetNumberOfEvents();
    TFile *mydigitsFile = new TFile("mydigits.root", "RECREATE");
    TTree *mydigitTree = new TTree("mydigitTree", "mydigitTree");
    TClonesArray * mydigits = new TClonesArray(AliPHOSDigit::Class());
    mydigitTree->Branch("Digits", &mydigits);

    for (Int_t ev = 0; ev < nSimEvents; ev++)
    {
        rl->GetEvent(ev);

        Int_t nPhosDigits = prl->SDigits()->GetEntries();


        Int_t nDigsFound = 0;

        for (Int_t iDig = 0; iDig < nPhosDigits; iDig++)
        {
            const AliPHOSDigit *digit = prl->SDigit(iDig);

            Int_t id = digit->GetId();
            if (id > 3*geom->GetNPhi()*geom->GetNZ()) continue;

            for (Int_t n = 0; n < nDigsFound; n++)
            {
                AliPHOSDigit *tmpDig = (AliPHOSDigit*)mydigits->At(n);
                if (id == tmpDig->GetId())
                {
                    *tmpDig += *digit;
                    digit = 0;
                    break;
                }
            }
            if (digit)
            {
                AliPHOSDigit *newDig = new((*mydigits)[nDigsFound]) AliPHOSDigit(-1, id, (float)0.0, (float)0.0);
                *newDig += *digit;

                nDigsFound++;
            }
        }
        for(int i = 0; i <nDigsFound; i++)
        {

            AliPHOSDigit *tmpDig = (AliPHOSDigit*)mydigits->At(i);
            Int_t relId[4];
            geom->AbsToRelNumbering(tmpDig->GetId(), relId);

            // Some necessary ugly hacks needed at the moment, need to get these numbers from OCDB
            tmpDig->SetEnergy((float)((int)(tmpDig->GetEnergy()/phosCalibData->GetADCchannelEmc(relId[0], relId[3], relId[2]))));
            //std::cout << "Sample time step: " << phosCalibData->GetSampleTimeStep() << " " << phosCalibData->GetTimeShiftEmc(relId[0], relId[3], relId[2]) << geom << std::endl;
            tmpDig->SetTime(1.5e-8);
            tmpDig->SetTimeR(1.5e-8);
        }
        mydigitTree->Fill();
        mydigits->Clear("C");
    }
    mydigitsFile->Write();
    simTree = mydigitTree;
    //mydigit
    //mydigitsFile->Close();
    return 0;
}

int mergeDigits(TString digitdir, Int_t /*simEvOffset*/)
{

    AliRunLoader *rl = AliRunLoader::Open(digitdir+TString("/galice.root"));

    AliPHOSLoader *prl = (AliPHOSLoader*)rl->GetDetectorLoader("PHOS");

    prl->LoadDigits("UPDATE");
    
    //prl->LoadDigits();

    Int_t nEvents = rl->GetNumberOfEvents();

    TClonesArray *mydigits = 0;
    simTree->SetBranchAddress("Digits", &mydigits);

    Int_t nDigits = 0;
    Int_t nEmbedDigits = 0;
    Int_t nOverlappingDigits = 0;
    Int_t nNewDigits = 0;
    
    Int_t nPhosDigits = prl->Digits()->GetEntries();

    Int_t nMyEvents = simTree->GetEntries();
    
    std::cout << "Number of real events: " << nEvents << std::endl;
    std::cout << "Number of sim events: " << nMyEvents << std::endl;
    nEvents = TMath::Min(nEvents, nMyEvents);
    std::cout << "Looping over: " << nEvents << std::endl;
	
    for (Int_t ev = 0; ev < nEvents; ev++)
    {
        rl->GetEvent(ev);
        
        simTree->GetEntry(ev);
        Int_t nMyDigits = mydigits->GetEntries();

        //Int_t nDigsFound = 0;
        nEmbedDigits += nMyDigits;
        TClonesArray *phosDigits = prl->Digits();
	nPhosDigits = prl->Digits()->GetEntries();
	
        for (Int_t iDig = 0; iDig < nPhosDigits; iDig++)
        {
            //const AliPHOSDigit *digit = prl->Digit(iDig);
            AliPHOSDigit *digit = (AliPHOSDigit*)phosDigits->At(iDig);
            nDigits++;
            for (Int_t n = 0; n < nMyDigits; n++)
            {
                AliPHOSDigit *myDigit = (AliPHOSDigit*)mydigits->At(n);
                if (digit->GetId() == myDigit->GetId())
                {
                    nOverlappingDigits++;
                    break;
                }
            }
        }
        if(nOverlappingDigits == nMyDigits)
        {
            std::cout << "Digits alredy embedded!" << std::endl;
            continue;
        }
        for (Int_t iDig = 0; iDig < nMyDigits; iDig++)
        {
            AliPHOSDigit *myDigit = (AliPHOSDigit*)mydigits->At(iDig);
            if (myDigit)
            {
                for (Int_t n = 0; n < nPhosDigits; n++)
                {
                    //const AliPHOSDigit *digit = prl->Digit(n);

                    AliPHOSDigit *digit = (AliPHOSDigit*)phosDigits->At(n);
                    if (digit->GetId() == myDigit->GetId())
                    {
                        digit->SetALTROSamplesHG(0, 0);
                        digit->SetALTROSamplesLG(0, 0);

                        *digit += *myDigit;
                        myDigit = 0;
                        break;
                    }
                }
                if (myDigit)
                {
                    TClonesArray *digArray = prl->Digits();
                    AliPHOSDigit *newDig =  new((*digArray)[nPhosDigits+nNewDigits]) AliPHOSDigit(*myDigit);

                    newDig->SetALTROSamplesHG(0, 0);
                    newDig->SetALTROSamplesLG(0, 0);
                    nNewDigits++;
                }
            }
        }
        phosDigits->Compress();
        Int_t ndigits = phosDigits->GetEntries() ;
	phosDigits->Sort();
        // Remove digits that are flagged bad in BCM. Then remove digits that are below threshold
        for (Int_t i = 0 ; i < ndigits ; i++)
        {
            AliPHOSDigit *digit = static_cast<AliPHOSDigit*>( phosDigits->At(i) ) ;
	    //std::cout << digit->GetId() << std::endl;
            if(digit->GetId())
            {
                vector<Int_t>::iterator it;
                it = std::find (badChannels.begin(), badChannels.end(), digit->GetId() );
                if(*it)
                {
                    digit->SetEnergy(0.0);
                }
            }
            if(digit->GetEnergy() <= recoParam->GetGlobalAltroThreshold())
	    {
	      phosDigits->RemoveAt(i);
	    }
        }
        //Set indexes in list of digits and make true digitization of the energy
        phosDigits->Compress();
        phosDigits->Sort();
	ndigits = phosDigits->GetEntries();
        for (Int_t i = 0 ; i < ndigits ; i++)
        {
            AliPHOSDigit *digit = static_cast<AliPHOSDigit*>( phosDigits->At(i) ) ;
            digit->SetIndexInList(i) ;
        }
        // -- create Digits branch
        Int_t bufferSize = 32000 ;

        TObjArray *branchList = prl->TreeD()->GetListOfBranches();

        branchList->RemoveAt(0);

        TBranch * digitsBranch = prl->TreeD()->Branch("PHOS","TClonesArray",&phosDigits,bufferSize);

        digitsBranch->Fill() ;
        prl->WriteDigits("OVERWRITE");
    }
    prl->WriteDigits("OVERWRITE");
    std::cout << "# Digits: " << nDigits << std::endl;
    std::cout << "# Embedded digits: " << nEmbedDigits << std::endl;
    std::cout << "# Overlapping digits: " << nOverlappingDigits << std::endl;
    std::cout << "# New digits: " << nNewDigits << std::endl;
    
    return 0;
}

int getFromFile(TString filename)
{

    //TFile *myfile = TFile::Open("mydigits.root", "READ");
    TFile *myfile = TFile::Open(filename, "READ");
    
    simTree = (TTree*)(myfile->Get("mydigitTree"));
    std::cout << "Found tree: " << simTree << std::endl;
    
    return 0;
}

Int_t retreiveCalibrationData(Int_t fRunNumber)
{
    AliCDBManager * man = AliCDBManager::Instance() ;
    man->SetDefaultStorage("raw://") ;
    //man->SetDefaultStorage("local://$ALICE_ROOT/OCDB") ;
    man->SetRun(fRunNumber) ;
    AliCDBEntry * bmEntry =  man->Get("PHOS/Calib/EmcBadChannels/") ;

    // Get the BCM for PHOS
    phosBcm = (AliPHOSEmcBadChannelsMap*)bmEntry->GetObject();
    if (!phosBcm)
    {
        std::cerr << "ERROR: Could not get the bad channel map for PHOS" << std::endl;
    }
    else
    {
        Int_t *tmpList = new Int_t[phosBcm->GetNumOfBadChannels()];
        phosBcm->BadChannelIds(tmpList);
        badChannels.resize(0);
        for(Int_t n = 0; n < phosBcm->GetNumOfBadChannels(); n++)
        {
            badChannels.push_back(tmpList[n]);
        }
    }

    // Get the gains for PHOS
    AliCDBPath path("PHOS","Calib","EmcGainPedestals");
    if (path.GetPath())
    {
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path);
        if (pEntry)
        {
            phosCalibData = (AliPHOSEmcCalibData*)pEntry->GetObject();

            if (!phosCalibData)
            {
                std::cerr << "ERROR: Could not get calibration data for PHOS" << std::endl;
                return -1;
            }
        }
        else
        {
            std::cerr << "ERROR:  Could not get CDB entry for PHOS calib data" << std::endl;
            return -1;
        }
    }
    AliCDBPath geompath("GRP","Geometry","Data");
    TGeoManager *geoManager = 0;
    if(path.GetPath())
    {
        //      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(geompath/*,GetRunNo()*/);

        if (pEntry)
        {
            if(geoUtils)
            {
                delete geoUtils;
                geoUtils = 0;
            }
            if(!geoManager) geoManager = (TGeoManager*) pEntry->GetObject();

            if(geoManager)
            {
                geoUtils = new AliPHOSGeoUtils("PHOS", "noCPV");
                geom = new AliPHOSGeometry("PHOS", "noCPV");
            }
            else
            {
                std::cerr << "can not get gGeoManager from OCDB" << std::endl;
            }
        }
        else
        {
            std::cerr << "can not fetch object " <<  path.GetPath().Data() << " from OCDB" << std::endl;
        }
    }
    AliCDBPath recoPath("PHOS", "Calib", "RecoParam");

    if(recoPath.GetPath())
    {
//      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(recoPath/*,GetRunNo()*/);
        if (pEntry)
        {

            TObjArray *paramArray = dynamic_cast<TObjArray*>(pEntry->GetObject());
            if(paramArray)
            {
                recoParam = dynamic_cast<AliPHOSRecoParam*>((paramArray)->At(0));
            }
            if(!recoParam)
            {
                std::cerr << "can not fetch object reconstruction parameters from " <<  recoPath.GetPath().Data() << std::endl;;
                return -1;
            }
        }
        else
        {
            std::cerr << "can not fetch object " << recoPath.GetPath().Data() <<  " from OCDB" << std::endl;
            return -1;
        }
    }
        createBadChannelMap();

    return 0;
}

int addBadChannels(Float_t extra)
{

    Int_t nNewBadChannels = phosBcm->GetNumOfBadChannels()*extra;
    //Int_t maxEmcId;
    Int_t nEMC = 3*geom->GetNPhi()*geom->GetNZ();
    for(Int_t n = 0; n < nNewBadChannels; n++)
    {
        //std::cout << (Int_t)(gRandom->Rndm()*nEMC+1) << std::endl;
        badChannels.push_back( (Int_t)(gRandom->Rndm()*nEMC+1));
    }

    return 0;
}

Int_t printDigits()
{

    AliRunLoader *rl = AliRunLoader::Open("galice.root");

    AliPHOSLoader *prl = (AliPHOSLoader*)rl->GetDetectorLoader("PHOS");

    prl->LoadDigits("READ");
    //prl->LoadDigits();

    Int_t nDigits = 0;
    Int_t nSimEvents = rl->GetNumberOfEvents();
    for (Int_t ev = 0; ev < nSimEvents; ev++)
    {
        rl->GetEvent(ev);

        Int_t nPhosDigits = prl->Digits()->GetEntries();

        //Int_t nDigsFound = 0;
        std::cout << "Number of digits found: " << nPhosDigits << std::endl;
        TClonesArray *phosDigits = prl->Digits();

        for (Int_t iDig = 0; iDig < nPhosDigits; iDig++)
        {
            //const AliPHOSDigit *digit = prl->Digit(iDig);
            AliPHOSDigit *digit = (AliPHOSDigit*)phosDigits->At(iDig);
            nDigits++;
	    //if(digit->GetTime() > 1.4e-08 && digit->GetTime() < 1.6e-08)
            std::cout <<"#: " << iDig << " ID: " << digit->GetId() << " " << "Energy: " << digit->GetEnergy() << " Time: " << digit->GetTime() << " N_prim: " << digit->GetNprimary() <<  " " << digit->GetTimeR() << std::endl;
        }
    }
  return 0;
}

int createBadChannelMap()
{
  std::cout << "Number of bad channels from OCDB: " << badChannels.size() << std::endl;
    
    TFile *badFile = TFile::Open("BadMap_LHC11a_pp2760.root", "READ");
    
    TH2D *badMap = (TH2D*)badFile->Get("PHOS_BadMap_mod3");
    
    
    for(Int_t x = 0; x < badMap->GetNbinsX(); x++)
    {
      for(Int_t z = 0; z < badMap->GetNbinsY(); z++)
      {
	if(x<16) 
	{
  	  Int_t relId[4];
	  relId[0] = 3;
	  relId[1] = 0;
	  relId[2] = x+1;
	  relId[3] = z+1;
	  Int_t absId = 0;
	  geoUtils->RelToAbsNumbering(relId, absId);
	  badChannels.push_back(absId);
	}
	//if(badMap->GetBinContent(x,z) == 1)
	if(0)
	{
	  Int_t relId[4];
	  relId[0] = 3;
	  relId[1] = 0;
	  relId[2] = x;
	  relId[3] = z;
	  Int_t absId = 0;
	  geoUtils->RelToAbsNumbering(relId, absId);
	  badChannels.push_back(absId);
	}
      }
    }
    badMap = (TH2D*)badFile->Get("PHOS_BadMap_mod2");
    
    
    for(Int_t x = 0; x < badMap->GetNbinsX(); x++)
    {
      for(Int_t z = 0; z < badMap->GetNbinsY(); z++)
      {
	if(x < 16 && z >=14)
	{
	  Int_t relId[4];
	  relId[0] = 3;
	  relId[1] = 0;
	  relId[2] = x+1;
	  relId[3] = z+1;
	  Int_t absId = 0;
	  geoUtils->RelToAbsNumbering(relId, absId);
	  badChannels.push_back(absId);
	  
	}
	if(x >=32 && x < 48)
	{
	  Int_t relId[4];
	  relId[0] = 3;
	  relId[1] = 0;
	  relId[2] = x+1;
	  relId[3] = z+1;
	  Int_t absId = 0;
	  geoUtils->RelToAbsNumbering(relId, absId);
	  badChannels.push_back(absId);
	}
	//if(badMap->GetBinContent(x,z) == 1)
	if(0)
	{
	  Int_t relId[4];
	  relId[0] = 2;
	  relId[1] = 0;
	  relId[2] = x;
	  relId[3] = z;
	  Int_t absId = 0;
	  geoUtils->RelToAbsNumbering(relId, absId);
	  badChannels.push_back(absId);
	}
      }
    }
    badMap = (TH2D*)badFile->Get("PHOS_BadMap_mod1");
    
    
    for(Int_t x = 0; x < badMap->GetNbinsX(); x++)
    {
      for(Int_t z = 0; z < badMap->GetNbinsY(); z++)
      {
	//if(badMap->GetBinContent(x,z) == 1)
	if(0)
	{
	  Int_t relId[4];
	  relId[0] = 1;
	  relId[1] = 0;
	  relId[2] = x;
	  relId[3] = z;
	  Int_t absId = 0;
	  geoUtils->RelToAbsNumbering(relId, absId);
	  badChannels.push_back(absId);
	}
      }
    }
    std::cout << "New number of bad channels: " << badChannels.size() << std::endl;
    
    
    return 0;
}
