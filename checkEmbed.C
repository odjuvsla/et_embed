#include "AliESDEvent.h"

#include "AliRunLoader.h"
#include "AliPHOSLoader.h"
#include "AliPHOSDigit.h"
#include "AliPHOSHit.h"
#include "AliESDEvent.h"
#include <iostream>
#include "AliStack.h"
#include <TH1F.h>
#include <TH2D.h>
#include <TNtuple.h>
#include <TParticle.h>
#include <TFile.h>
#include "TChain.h"
#include "AliCDBManager.h"
#include "AliPHOSEmcBadChannelsMap.h"
#include "AliCDBEntry.h"
#include <AliPHOSGeoUtils.h>
#include "TGeoManager.h"
#include "AliPHOSEmcCalibData.h"
#include "AliCaloCalibPedestal.h"


// simDir is relative path to the directory where the simulated galice.root resides wrt to the directory of the AliESDs.root file
Int_t checkEmbed(TString simDir = "../sim/") 
{

    AliCDBManager * man = AliCDBManager::Instance() ;
    man->SetDefaultStorage("raw://") ;
    //man->SetDefaultStorage("local://$ALICE_ROOT/OCDB") ;
    man->SetRun(137366) ;

    AliCDBPath path("GRP","Geometry","Data");

    TGeoManager *geoManager = 0;

    AliPHOSGeoUtils *gu = new AliPHOSGeoUtils("noCPV");

    if (path.GetPath())
    {
        //      HLTInfo("configure from entry %s", path.GetPath());
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path);

        if (pEntry)
        {
            if (!geoManager) geoManager = (TGeoManager*) pEntry->GetObject();

            if (geoManager)
            {
                gu = new AliPHOSGeoUtils("PHOS", "noCPV");
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
    AliPHOSEmcBadChannelsMap *phosBcm = 0;
    AliCDBPath path2("PHOS","Calib","EmcBadChannels");
    // Get the BCM for PHOS
    if (path2.GetPath())
    {
        AliCDBEntry *pEntry = AliCDBManager::Instance()->Get(path2);
        if (pEntry)
        {
            phosBcm = (AliPHOSEmcBadChannelsMap*)pEntry->GetObject();

            if (!phosBcm)
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
    std::cout << "Number of bad channels: " << phosBcm->GetNumOfBadChannels() << std::endl;
    TH1D *hEnergyOfLabeled  = new TH1D("hEnergyOfLabeled", "hEnergyOfLabeled", 1000, 0, 10);
    TH1D *hEnergyOfNonLabeled  = new TH1D("hEnergyOfNonLabeled", "hEnergyOfNonLabeled", 1000, 0, 10);
    TH1D *hEnergyOfAll  = new TH1D("hEnergyOfAll", "hEnergyOfAll", 1000, 0, 10);
    TH1D *hPidOfLabeled  = new TH1D("hPidOfLabeled", "hPidOfLabeled", 1000, -0.5, 999.5);
    TH1D *hFractionOfLabeled  = new TH1D("hFractionOfLabeled", "hFractionOfLabeled", 1000, 0.0, 1.01);
    TH1D *hNcellsLabled = new TH1D("hNcellsLabled", "hNcellsLabled", 100, 0, 99);
    TH2D *hNcellsvsFrac = new TH2D("hNcellsvsFrac", "hNcellsvsFrac", 100, 0, 99, 1000, 0.0, 1.01);

    TH1D *hGenerated = new TH1D("hGenerated", "hGenerated", 1000, 0, 10);
    TH1D *hFound = new TH1D("hFound", "hFound", 1000, 0, 10);

    TH2D *hGeneratedvsClusterMult = new TH2D("hGeneratedvsClusterMult", "hGeneratedvsClusterMult", 1000, 0, 10, 200, 0, 200);
    TH2D *hFoundvsClusterMult = new TH2D("hFoundvsClusterMult", "hFoundvsClusterMult", 1000, 0, 10, 200, 0, 200);

    TH1I *hClusterMult = new TH1I("hClusterMult", "hClusterMult", 200, -0.5, 199.5);

    TH1D *hEnergyDiff = new TH1D("hEnergyDiff", "hEnergyDiff", 1000, -1.0, 1.0);

    TH2D *clusterPos = new TH2D("hPosition", "hPosition", 1000, -500, 500, 1000, -500, 500);

    TH1D *hFound2 = new TH1D("hFound2", "hFound2", 1000, 0, 10);
    TH1D *hFound3 = new TH1D("hFound3", "hFound3", 1000, 0, 10);
    TH1D *hFound4 = new TH1D("hFound4", "hFound4", 1000, 0, 10);

    
    TChain * chain = new TChain("esdTree") ;
    chain->Add("data/AliESDs.root");
//    chain->Add(recFile);
    /*
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366024/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366011/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366007/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366028/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366034/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366020/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366008/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366032/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366022/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366003/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366029/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366038/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366006/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366010/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366012/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366014/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366015/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366002/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366019/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366009/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366037/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366018/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366013/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366031/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366030/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366016/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366027/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366035/AliESDs.root");
    chain->Add("/data/alice/analysis/physics_analysis/tools/test/embedded/10000137366023/AliESDs.root");
    */
    /*
        chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366014.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366007.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366006.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366035.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366028.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366032.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366031.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366001.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366010.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366024.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366002.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366040.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366038.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366034.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366030.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366032.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366023.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366018.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366031.200/AliESDs.root");
    //chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366037.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366003.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366011.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366010.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366018.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366023.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366012.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366029.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366007.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366009.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366006.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366020.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366022.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366019.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366009.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366028.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366024.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366024.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366002.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366005.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366001.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366027.1100/AliESDs.root");
    //chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366016.2200/AliESDs.root");

    //chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366020.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366006.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366023.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366016.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366019.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366039.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366008.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366038.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366007.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366038.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366001.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366003.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366022.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366011.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366039.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366031.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366019.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366008.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366011.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366040.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366037.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366041.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366005.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366024.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366037.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366010.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366032.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366028.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366034.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366022.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366018.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366036.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366018.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366016.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366015.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366005.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366008.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366014.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366030.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366022.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366013.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366012.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366013.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366031.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366012.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366014.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366007.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366008.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366010.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366013.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366034.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366003.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366038.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366011.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366028.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366002.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366030.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366006.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366009.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366003.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366036.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366029.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366029.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366015.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366009.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366023.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366019.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366041.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366001.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366032.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366013.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366005.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366041.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366012.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366002.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366020.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366020.200/AliESDs.root");
    //chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366027.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366039.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366037.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366029.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366015.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366030.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366015.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run2/embedded/10000137366014.1100/AliESDs.root");

      chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366014.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366007.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366006.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366035.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366028.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366032.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366031.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366001.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366010.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366024.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366002.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366040.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366038.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366034.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366030.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366032.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366023.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366018.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366031.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366037.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366003.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366011.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366010.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366018.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366023.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366012.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366029.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366007.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366009.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366006.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366020.1200/AliESDs.root");
    //chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366022.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366019.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366009.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366028.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366024.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366024.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366002.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366005.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366001.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366027.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366016.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366020.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366006.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366023.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366016.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366019.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366039.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366008.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366038.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366007.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366038.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366001.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366003.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366022.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366011.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366039.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366031.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366019.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366008.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366011.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366040.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366037.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366041.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366005.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366024.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366037.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366010.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366032.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366028.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366034.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366022.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366018.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366036.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366018.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366016.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366015.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366005.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366008.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366014.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366030.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366022.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366013.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366012.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366013.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366031.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366012.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366014.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366007.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366008.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366010.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366013.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366034.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366003.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366038.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366011.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366028.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366002.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366030.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366006.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366009.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366003.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366036.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366029.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366029.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366015.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366009.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366023.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366019.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366041.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366001.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366032.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366013.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366005.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366041.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366012.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366002.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366020.1100/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366020.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366027.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366039.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366037.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366029.1200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366015.200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366030.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366015.2200/AliESDs.root");
    chain->Add("/data/alice/analysis/embedding/run0/embedded/10000137366014.1100/AliESDs.root");
    */

    AliESDEvent *esdEvent = new AliESDEvent();
    esdEvent->ReadFromTree(chain);

    AliRunLoader *runLoader = 0;
    AliPHOSLoader *prl = 0;

    Int_t fno = -1;
    Int_t nEvents = chain->GetEntries();
    std::cout << "Number of events: " << nEvents << std::endl;
    TString filePath;

    Int_t tmpEvs = 0;
    Int_t nSimEvents = 0;

    Int_t nTotEvents = 0;
    TString trueFile;

    for (Int_t iEv = 0; iEv < nEvents; iEv++)
    {
        chain->GetEvent(iEv);

        if (fno != chain->GetTreeNumber())
        {
            // Let's do some magic to get the correct true data for this file
            fno = chain->GetTreeNumber();
            std::cout << "File number: " << chain->GetTreeNumber() << " path: " << chain->GetFile()->GetPath() << std::endl;
            filePath = chain->GetFile()->GetPath();
            filePath = filePath.Remove(filePath.Index("AliESDs.root"));
            trueFile = filePath + "/" + simDir + "/galice.root";
            if (runLoader)
            {
                runLoader->UnloadAll();
                //runLoader->CleanFolders();

            }
            std::cout << "Loading new true file: " << trueFile <<  std::endl;

            TString eventFolder = "Event__";
            eventFolder += chain->GetTreeNumber();
            runLoader = AliRunLoader::Open(trueFile, eventFolder);
            runLoader->SetEventFolderName(eventFolder);
            prl = (AliPHOSLoader*)runLoader->GetDetectorLoader("PHOS");
            runLoader->LoadKinematics();
            prl->LoadDigits();
            tmpEvs = 0;
            nSimEvents = runLoader->GetNumberOfEvents();
            std::cout << "Number of sim events: " << nSimEvents << std::endl;
            std::cout << "Number of real events: " << chain->GetTree()->GetEntries() << std::endl;

        }
        tmpEvs++;

        // Don't want to analyse events which are not embedded
        if (tmpEvs > nSimEvents) continue;

        //std::cout << "File number: " << chain->GetTreeNumber() << std::endl;
        runLoader->GetEvent(tmpEvs-1);
        if (iEv%100 == 0) std::cout << "Analyzing event: " << iEv << std::endl;

        Float_t primEnergy = 0;
        Float_t primPid = 0;
        Int_t primIdx = -1;

        AliStack *stack = runLoader->Stack();
        if (stack)
        {
            primEnergy = stack->Particle(0)->Energy();
            primPid = stack->Particle(0)->GetPdgCode();
            //std::cout << stack->GetNtrack() << std::endl;

            Float_t maxPrimEnergy = 9999;
            //  if(stack->GetNprimary() < 3) std::cout << "Number of primary less than 3!: " << stack->GetNprimary() << std::endl;
            for (Int_t n = 0; n < stack->GetNprimary(); n++)
            {
                if (TMath::Abs(stack->Particle(stack->GetPrimary(n))->Px()/stack->Particle(stack->GetPrimary(n))->Pt()) < maxPrimEnergy)
                {
                    maxPrimEnergy = stack->Particle(stack->GetPrimary(n))->Energy();

                    primIdx = stack->GetPrimary(n);
                }
                if (n < 3)
                {
                    hGenerated->Fill(stack->Particle(stack->GetPrimary(n))->Energy());
                    hGeneratedvsClusterMult->Fill(stack->Particle(stack->GetPrimary(n))->Energy(), esdEvent->GetNumberOfCaloClusters());
                }
            }
        }
        
        std::cout << "Mult: " << esdEvent->GetNumberOfCaloClusters() << std::endl;
	
	std::cout << "Number of primary particles: " << stack->GetNprimary() << std::endl;

        hClusterMult->Fill(esdEvent->GetNumberOfCaloClusters());
	
        for (Int_t iCl = 0; iCl < esdEvent->GetNumberOfCaloClusters(); iCl++)
        {
            AliESDCaloCluster *cluster = esdEvent->GetCaloCluster(iCl);
            Float_t gPos[3];

            cluster->GetPosition(gPos);

            Int_t relId[4];
            TVector3 glVec(gPos);
            gu->GlobalPos2RelId(glVec, relId);
	    
	    std::cout << "Labels in cluster: " << cluster->GetNLabels() << std::endl;

	    if (cluster->GetNLabels())
            {
                clusterPos->Fill(gPos[0], gPos[2]);
                hEnergyOfLabeled->Fill(cluster->E());

		if (stack->Particle(cluster->GetLabel())->GetMother(0) == -1)
                {
                    hEnergyDiff->Fill((stack->Particle(cluster->GetLabel())->Energy()-cluster->E()));
                    hFound->Fill(stack->Particle(cluster->GetLabel())->Energy());
                    hFoundvsClusterMult->Fill(stack->Particle(cluster->GetLabel())->Energy(), esdEvent->GetNumberOfCaloClusters());
                    if (relId[0] == 3)
                    {
                        hFound2->Fill(stack->Particle(cluster->GetLabel())->Energy());
                    }
                    if (relId[0] == 2)
                    {
                        hFound3->Fill(stack->Particle(cluster->GetLabel())->Energy());
                    }
                    if (relId[0] == 1)
                    {
                        hFound4->Fill(stack->Particle(cluster->GetLabel())->Energy());
                    }

                    hFractionOfLabeled->Fill(cluster->GetCellAmplitudeFraction(0));
                    hNcellsLabled->Fill(cluster->GetNCells());
                    hNcellsvsFrac->Fill(cluster->GetNCells(), cluster->GetCellAmplitudeFraction(0));
                }
                //}
            }
            else
            {

                hEnergyOfNonLabeled->Fill(cluster->E());
            }
            hEnergyOfAll->Fill(cluster->E());
        }
        nTotEvents++;

    }

    std::cout << "Analyzed " << nTotEvents << " events." << std::endl;

    TFile *f = TFile::Open("hists.root", "RECREATE");
    hEnergyOfLabeled->Write();
    hEnergyOfNonLabeled->Write();
    hEnergyOfAll->Write();
    hPidOfLabeled->Write();
    hFractionOfLabeled->Write();
    hNcellsLabled->Write();
    hNcellsvsFrac->Write();
    hEnergyDiff->Write();

    hGenerated->Write();
    hFound->Write();
    hGeneratedvsClusterMult->Write();
    hFoundvsClusterMult->Write();

    hFound2->Write();
    hFound3->Write();
    hFound4->Write();

    hClusterMult->Write();

    clusterPos->Write();
    
    f->Close();
    
    return 0;

}


