#ifndef ALIANALYSISETEMBEDDERPHOS_H
#define ALIANALYSISETEMBEDDERPHOS_H

#include "AliAnalysisEtEmbedder.h"
#include "TString.h"


class AliPHOSEmcBadChannelsMap;
class AliPHOSEmcCalibData;
class AliPHOSGeoUtils;
class AliPHOSGeometry;
class AliPHOSRecoParam;

class AliAnalysisEtEmbedderPhos : public AliAnalysisEtEmbedder
{

public:
  
    AliAnalysisEtEmbedderPhos();
    
    virtual ~AliAnalysisEtEmbedderPhos();
    
    virtual Int_t AddBadChannelsToBcm(Float_t a);
    
    virtual Int_t AddDigits(AliDigit* lhs, AliDigit* rhs);
    
    virtual Int_t GetOCDBData(Int_t runNumber);
    
    virtual Int_t GetSimulatedDigits(TString digitDir, TString outfile = "mydigits.root");
    
    virtual Int_t Embed(TString dataDir);
    
    
private:
  
  
    AliPHOSEmcBadChannelsMap *fPhosBcm;
    AliPHOSEmcCalibData *fPhosCalibData;
    AliPHOSGeoUtils *fGeoUtils;
    AliPHOSGeometry *fGeom;
    AliPHOSRecoParam *fRecoParam;

    std::vector<Int_t> fBadChannels;

    ClassDef(AliAnalysisEtEmbedderPhos, 0)
};

#endif // ALIANALYSISETEMBEDDERPHOS_H
