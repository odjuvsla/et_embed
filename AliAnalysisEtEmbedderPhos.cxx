#include "AliAnalysisEtEmbedderPhos.h"
#include "AliCDBManager.h"
#include "AliCDBPath.h"
#include "AliCDBEntry.h"
#include "AliPHOSEmcBadChannelsMap.h"
#include "AliPHOSEmcCalibData.h"
#include "AliPHOSGeoUtils.h"
#include "AliPHOSGeometry.h"
#include "AliPHOSRecoParam.h"
#include "AliLog.h"
#include "TGeoManager.h"
#include <iostream>
#include "AliRunLoader.h"
#include "AliPHOSLoader.h"
#include "AliPHOSDigit.h"
#include "TFile.h"
#include "TH2D.h"
#include "TBranch.h"

#include <vector>
#include <algorithm>

ClassImp(AliAnalysisEtEmbedderPhos);

AliAnalysisEtEmbedderPhos::AliAnalysisEtEmbedderPhos() : AliAnalysisEtEmbedder()
        ,fPhosBcm(0)
        ,fPhosCalibData(0)
        ,fGeoUtils(0)
        ,fGeom(0)
        ,fRecoParam(0)
{
    fTreeName = "PHOSSimDigitTree";
    fBranchName = "PHOSDigits";
}

AliAnalysisEtEmbedderPhos::~AliAnalysisEtEmbedderPhos()
{

}

Int_t AliAnalysisEtEmbedderPhos::AddBadChannelsToBcm(Float_t /*a*/)
{
    return 0;
}

Int_t AliAnalysisEtEmbedderPhos::AddDigits(AliDigit* /*lhs*/, AliDigit* /*rhs*/)
{
    return 0;
}

Int_t AliAnalysisEtEmbedderPhos::GetOCDBData(Int_t runNumber)
{
    AliCDBManager * man = AliCDBManager::Instance() ;
    if (runNumber != 0)
    {
        man->SetDefaultStorage("raw://") ;
        //man->SetDefaultStorage("local://$ALICE_ROOT/OCDB") ;

    }
    else
    {
        man->SetDefaultStorage("local://$ALICE_ROOT/OCDB") ;
    }
    man->SetRun(runNumber) ;
    AliCDBEntry * bmEntry =  man->Get("PHOS/Calib/EmcBadChannels/") ;

    // Get the BCM for PHOS
    fPhosBcm = (AliPHOSEmcBadChannelsMap*)bmEntry->GetObject();
    if (!fPhosBcm)
    {
        AliError("Could not get the bad channel map for PHOS");
        //std::cerr << "ERROR: Could not get the bad channel map for PHOS" << std::endl;
    }
    else
    {
        Int_t *tmpList = new Int_t[fPhosBcm->GetNumOfBadChannels()];
        fPhosBcm->BadChannelIds(tmpList);
        fBadChannels.resize(0);

        for (Int_t n = 0; n < fPhosBcm->GetNumOfBadChannels(); n++)
        {
            fBadChannels.push_back(tmpList[n]);
        }
    }

    // Get the gains for PHOS
    AliCDBPath path("PHOS","Calib","EmcGainPedestals");
    if (path.GetPath())
    {
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path);
        if (pEntry)
        {
            fPhosCalibData = (AliPHOSEmcCalibData*)pEntry->GetObject();

            if (!fPhosCalibData)
            {
                AliError("Could not get calibration data for PHOS");
                //std::cerr << "ERROR: Could not get calibration data for PHOS" << std::endl;
                return -1;
            }

        }
        else
        {
            AliError("Could not get CDB entry for PHOS calib data");
            //std::cerr << "ERROR:  Could not get CDB entry for PHOS calib data" << std::endl;
            return -1;
        }
    }
    AliCDBPath geompath("GRP","Geometry","Data");
    TGeoManager *geoManager = 0;
    if (path.GetPath())
    {
        //      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(geompath/*,GetRunNo()*/);

        if (pEntry)
        {
            if (fGeoUtils)
            {
                delete fGeoUtils;
                fGeoUtils = 0;
            }
            if (!geoManager) geoManager = (TGeoManager*) pEntry->GetObject();

            if (geoManager)
            {
                fGeoUtils = new AliPHOSGeoUtils("PHOS", "noCPV");
                fGeom = new AliPHOSGeometry("PHOS", "noCPV");
            }
            else
            {
                AliError("Could not get gGeoManager from OCDB");
                //    std::cerr << "can not get gGeoManager from OCDB" << std::endl;
            }
        }
        else
        {
            AliError("Could not fetch object GRP/Geometry/Data from OCDB");
            //std::cerr << "Could not fetch object " <<  path.GetPath().Data() << " from OCDB" << std::endl;
        }
    }
    AliCDBPath recoPath("PHOS", "Calib", "RecoParam");

    if (recoPath.GetPath())
    {
//      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(recoPath/*,GetRunNo()*/);
        if (pEntry)
        {

            TObjArray *paramArray = dynamic_cast<TObjArray*>(pEntry->GetObject());
            if (paramArray)
            {
                fRecoParam = dynamic_cast<AliPHOSRecoParam*>((paramArray)->At(0));
            }
            if (!fRecoParam)
            {
                AliError("Could not fetch object reconstruction parameters from PHOS/Calib/RecoParam");
                //std::cerr << "can not fetch object reconstruction parameters from " <<  recoPath.GetPath().Data() << std::endl;;
                return -1;
            }
        }
        else
        {
            AliError("Could not fetch object PHOS/Calib/RecoParam from OCDB");
            //std::cerr << "can not fetch object " << recoPath.GetPath().Data() <<  " from OCDB" << std::endl;
            return -1;
        }
    }
    return 0;
}

Int_t AliAnalysisEtEmbedderPhos::GetSimulatedDigits(TString digitdir, TString outfile)
{

    AliRunLoader *rl = AliRunLoader::Open(digitdir+TString("/galice.root"));

    AliPHOSLoader *prl = (AliPHOSLoader*)rl->GetDetectorLoader("PHOS");

    prl->LoadSDigits();
    prl->LoadDigits();

    Int_t nSimEvents = rl->GetNumberOfEvents();
    TFile *mydigitsFile = new TFile(outfile, "RECREATE");
    TTree *mydigitTree = new TTree(fTreeName, fTreeName);
    TClonesArray * mydigits = new TClonesArray(AliPHOSDigit::Class());
    mydigitTree->Branch(fBranchName, &mydigits);


    // Loop over the simulated events
    for (Int_t ev = 0; ev < nSimEvents; ev++)
    {
        rl->GetEvent(ev);

        Int_t nPhosDigits = prl->SDigits()->GetEntries();

        Int_t nDigsFound = 0;

        // Loop over the SDigits
        for (Int_t iDig = 0; iDig < nPhosDigits; iDig++)
        {
            const AliPHOSDigit *digit = prl->SDigit(iDig);

            Int_t id = digit->GetId();
            if (id > 3*fGeom->GetNPhi()*fGeom->GetNZ()) continue; // Make sure we only get digits from the 3 modules that are installed

            // Loop over digits we have already created since several SDigits are usally present within in one tower, and we need to merge them
            for (Int_t n = 0; n < nDigsFound; n++)
            {
                AliPHOSDigit *tmpDig = (AliPHOSDigit*)mydigits->At(n);
                // If we find an SDigit in a tower that already has been found we add it
                if (id == tmpDig->GetId())
                {
                    *tmpDig += *digit;
                    digit = 0;
                    break;
                }
            }
            // If the tower corresponding to the SDigit has not been found already we create a new digit
            if (digit)
            {
                AliPHOSDigit *newDig = new((*mydigits)[nDigsFound]) AliPHOSDigit(-1, id, (float)0.0, (float)0.0);
                *newDig += *digit;
                nDigsFound++;
            }
        }

        // Run over the digits and recalibrate the energy, needed because SDigits are in GeV while digits are in ADC counts. Should probably apply some smearing here
        for (int i = 0; i <nDigsFound; i++)
        {

            AliPHOSDigit *tmpDig = (AliPHOSDigit*)mydigits->At(i);
            Int_t relId[4];
            fGeom->AbsToRelNumbering(tmpDig->GetId(), relId);

            // Some necessary ugly hacks needed at the moment, need to get these numbers from OCDB
            tmpDig->SetEnergy((float)((int)(tmpDig->GetEnergy()/fPhosCalibData->GetADCchannelEmc(relId[0], relId[3], relId[2]))));
            //    std::cout << "Sample time step: " << fPhosCalibData->GetSampleTimeStep() << " " << fPhosCalibData->GetTimeShiftEmc(relId[0], relId[3], relId[2]) << std::endl;
            tmpDig->SetTime(1.5e-8);
            tmpDig->SetTimeR(1.5e-8);
        }
        mydigitTree->Fill();
        mydigits->Clear("C");
    }
    // Write the digits to file
    mydigitsFile->Write();

    mydigitsFile->Close();

    return 0;
}

Int_t AliAnalysisEtEmbedderPhos::Embed(TString dataDir)
{
    // Loading the real data
    AliRunLoader *rl = AliRunLoader::Open(dataDir+TString("/galice.root"));

    AliPHOSLoader *prl = (AliPHOSLoader*)rl->GetDetectorLoader("PHOS");

    prl->LoadDigits("UPDATE");

    // Number of real events
    Int_t nEvents = rl->GetNumberOfEvents();

    // Loading simulated digits
    TClonesArray *simDigits = 0;
    fSimDigitTree->SetBranchAddress(fBranchName, &simDigits);



    // Number of simulated events
    Int_t nSimEvents = fSimDigitTree->GetEntries();

    TString info;
    
    info.Form("Number of real events: %d", nEvents);
    AliInfo(info.Data());
    std::cout << "Number of real events: " << nEvents << std::endl;
    
    info.Form("Number of sim events: %d", nSimEvents);
    AliInfo(info.Data());
    std::cout << "Number of sim events: " << nSimEvents << std::endl;
    
    nEvents = TMath::Min(nEvents, nSimEvents);
    info.Form("Looping over: %d", nEvents);
    AliInfo(info.Data());
    std::cout << "Looping over: " << nEvents << std::endl;

    Int_t nEmbedDigits = 0;
    Int_t nOverlappingDigits = 0;
    Int_t nNewDigits = 0;

    for (Int_t ev = 0; ev < nEvents; ev++)
    {

        rl->GetEvent(ev);

        fSimDigitTree->GetEntry(ev);

        // Number of simulated digits
        Int_t nSimDigits = simDigits->GetEntries();
	if(nSimDigits == 0) continue;

        //Int_t nDigsFound = 0;
        nEmbedDigits += nSimDigits;
        TClonesArray *phosDigits = prl->Digits();

        // Number of real digits
        Int_t nPhosDigits = prl->Digits()->GetEntries();

        Int_t nNonOverlappingDigitsInEvent = 0;
	Int_t nOverlappingDigitsInEvent = 0;
        Bool_t isOverlapping = false;

        // Looping over real digits, counting overlaps
        for (Int_t iDig = 0; iDig < nPhosDigits; iDig++)
        {
	    isOverlapping = false;
            // Checking if there is an overlap
            AliPHOSDigit *digit = (AliPHOSDigit*)phosDigits->At(iDig);
            for (Int_t n = 0; n < nSimDigits; n++)
            {
                AliPHOSDigit *simDigit = (AliPHOSDigit*)simDigits->At(n);
                if (digit->GetId() == simDigit->GetId())
                {
                    nOverlappingDigits++;
		    nOverlappingDigitsInEvent++;
                    isOverlapping = true;
                    break;
                }
            }
            if (!isOverlapping) nNonOverlappingDigitsInEvent++;
        }
        info.Form("Number of overlapping digits in event: %d, Number of non-overlapping digits in event: %d", nOverlappingDigitsInEvent, nNonOverlappingDigitsInEvent);
	AliInfo(info.Data());
	
//        if (nNonOverlappingDigits == 0)
        //{
            //AliWarning("All digits alredy embedded!"); // This is not true!
            //continue;
        //}

        // Looping over the simulated digits
        for (Int_t iDig = 0; iDig < nSimDigits; iDig++)
        {
            AliPHOSDigit *simDigit = (AliPHOSDigit*)simDigits->At(iDig);
            if (simDigit)
            {
                // Looping over the real digits to check if there is an overlap
                for (Int_t n = 0; n < nPhosDigits; n++)
                {
                    AliPHOSDigit *digit = (AliPHOSDigit*)phosDigits->At(n);
                    if (digit->GetId() == simDigit->GetId())
                    {
                        // Have to do it like this for the moment, for our cause the ALTRO samples are not needed anyway
                        digit->SetALTROSamplesHG(0, 0);
                        digit->SetALTROSamplesLG(0, 0);

                        // Overlapping digits, adding the simulated digit to the real one
                        *digit += *simDigit;
                        simDigit = 0;

                        break;
                    }
                }
                if (simDigit)
                {
                    // The simulated digit does not overlap with any real digits, just adding it to the array of digits
                    TClonesArray *digArray = prl->Digits();
                    AliPHOSDigit *newDig =  new((*digArray)[nPhosDigits+nNewDigits]) AliPHOSDigit(*simDigit);
		    std::cout << "New digit!" << std::endl;

                    // Have to do it like this for the moment, for our cause the ALTRO samples are not needed anyway
                    newDig->SetALTROSamplesHG(0, 0);
                    newDig->SetALTROSamplesLG(0, 0);
                    nNewDigits++;
                }
            }
        }
        
        // Compressing the array
        phosDigits->Compress();
        Int_t ndigits = phosDigits->GetEntries() ;
        phosDigits->Sort();
        // Remove digits that are flagged bad in BCM. Then remove digits that are below threshold
        for (Int_t i = 0 ; i < ndigits ; i++)
        {
            AliPHOSDigit *digit = static_cast<AliPHOSDigit*>( phosDigits->At(i) ) ;
            //std::cout << digit->GetId() << std::endl;
            if (digit->GetId())
            {
                vector<Int_t>::iterator it;
                it = std::find (fBadChannels.begin(), fBadChannels.end(), digit->GetId() );
                if (*it)
                {
                    digit->SetEnergy(0.0);
                }
            }
            if (digit->GetEnergy() <= fRecoParam->GetGlobalAltroThreshold())
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
    
    info.Form("# Embedded digits: %d", nEmbedDigits);
    AliInfo(info.Data());
    std::cout << "# Embedded digits: " << nEmbedDigits << std::endl;
    
    info.Form("# Overlapping digits: %d", nOverlappingDigits);
    AliInfo(info.Data());
    std::cout << "# Overlapping digits: " << nOverlappingDigits << std::endl;
    
    info.Form("# New digits: %d", nNewDigits);
    AliInfo(info.Data());
    std::cout << "# New digits: " << nNewDigits << std::endl;

    return 0;
}


Int_t AliAnalysisEtEmbedderPhos::AddBadChannelsToBcmFromFile(TString filename)
{
    std::cout << "Number of bad channels from OCDB: " << fBadChannels.size() << std::endl;
    
    TFile *badFile = TFile::Open(filename, "READ");
    
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
	  fGeoUtils->RelToAbsNumbering(relId, absId);
	  fBadChannels.push_back(absId);
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
	  fGeoUtils->RelToAbsNumbering(relId, absId);
	  fBadChannels.push_back(absId);
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
	  fGeoUtils->RelToAbsNumbering(relId, absId);
	  fBadChannels.push_back(absId);
	  
	}
	if(x >=32 && x < 48)
	{
	  Int_t relId[4];
	  relId[0] = 3;
	  relId[1] = 0;
	  relId[2] = x+1;
	  relId[3] = z+1;
	  Int_t absId = 0;
	  fGeoUtils->RelToAbsNumbering(relId, absId);
	  fBadChannels.push_back(absId);
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
	  fGeoUtils->RelToAbsNumbering(relId, absId);
	  fBadChannels.push_back(absId);
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
	  fGeoUtils->RelToAbsNumbering(relId, absId);
	  fBadChannels.push_back(absId);
	}
      }
    }
    std::cout << "New number of bad channels: " << fBadChannels.size() << std::endl;
    
    
    return 0;
}
