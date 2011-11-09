#ifndef ALIANALYSISETEMBEDDER_H
#define ALIANALYSISETEMBEDDER_H

#include "TString.h"
#include <Rtypes.h>
#include <AliDigit.h>

class TTree;

class AliAnalysisEtEmbedder : public TObject
{
public:

    AliAnalysisEtEmbedder();

    virtual ~AliAnalysisEtEmbedder();

    /**
     * Load the generated digits from file containing a tree of digits.
     * Branch names must be PHOSDigits and/or EMCALDigits
     */
    Int_t LoadDigitsFromFile(TString filename = "mydigits.root");
    
    /** Get data from OCDB */
    virtual Int_t GetOCDBData(Int_t runNumber) = 0;

    /**
     * Add percentage of total number of channels to the BCM
     */
    virtual Int_t AddBadChannelsToBcm(Float_t a) = 0;
    
    /** 
     * Creates digits from simulated sdigits output and store in a file 
     * @param digitDir the directory of the simulated digits
     * @param outfile is the output file name
     */
    virtual Int_t GetSimulatedDigits(TString digitDir, TString outfile = "mydigits.root") = 0;
    
    /** 
     * Embed the digits
     * @param dataDir is the directory containing the "real" data
     */
    virtual Int_t Embed(TString dataDir) = 0;
    

protected:

    /** Name of the simulated digit tree */
    TString fTreeName;

    /** Name of the simulated digit branch (i.e. PHOSDigits or EMCALDigits) */
    TString fBranchName;
    
    /** Tree containing the simulated digits */
    TTree *fSimDigitTree;

private:
  

    ClassDef(AliAnalysisEtEmbedder, 0)
};

#endif // ALIANALYSISETEMBEDDER_H

