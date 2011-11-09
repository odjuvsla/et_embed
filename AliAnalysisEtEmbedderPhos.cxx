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

ClassImp(AliAnalysisEtEmbedderPhos);

AliAnalysisEtEmbedderPhos::AliAnalysisEtEmbedderPhos() : AliAnalysisEtEmbedder()
,fPhosBcm(0)
,fPhosCalibData(0)
,fGeoUtils(0)
,fGeom(0)
,fRecoParam(0)
{

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
    if(runNumber != 0)
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
	
        for(Int_t n = 0; n < fPhosBcm->GetNumOfBadChannels(); n++)
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
    if(path.GetPath())
    {
        //      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(geompath/*,GetRunNo()*/);

        if (pEntry)
        {
            if(fGeoUtils)
            {
                delete fGeoUtils;
                fGeoUtils = 0;
            }
            if(!geoManager) geoManager = (TGeoManager*) pEntry->GetObject();

            if(geoManager)
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

    if(recoPath.GetPath())
    {
//      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(recoPath/*,GetRunNo()*/);
        if (pEntry)
        {

            TObjArray *paramArray = dynamic_cast<TObjArray*>(pEntry->GetObject());
            if(paramArray)
            {
                fRecoParam = dynamic_cast<AliPHOSRecoParam*>((paramArray)->At(0));
            }
            if(!fRecoParam)
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

Int_t AliAnalysisEtEmbedderPhos::GetSimulatedDigits(TString /*digitDir*/, TString /*outfile*/)
{
  return 0;
}
