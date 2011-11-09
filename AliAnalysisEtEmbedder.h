#ifndef ALIANALYSISETEMBEDDER_H
#define ALIANALYSISETEMBEDDER_H

#include "TString.h"
#include <Rtypes.h>
#include <AliDigit.h>

class AliAnalysisEtEmbedder : public TObject
{
public:

    AliAnalysisEtEmbedder();

    virtual ~AliAnalysisEtEmbedder();

    void SetSimDigitInputFile(TString /*simFile*/) {}

    /**
     * Directory containing simulated SDigits.
     * TODO: Implement this!
     */
    void SetSimDigitInputDirectory(TString /*simDir*/) {}

    /**
     * Directory containing reconstructed real data
     */
    void SetRealDigitInputDirectory(TString /*realDir*/) {}

    /**
     * Load the generated digits from file containing a tree of digits.
     * Branch names must be PHOSDigits and/or EMCALDigits
     */
    Int_t LoadDigitsFromFile(TString filename);

    /**
     * Add two digits
     * @param lhs (left hand side) will contain the resulting digit
     * @param rhs (right hand side) is the digit to add to lhs
     */
    virtual Int_t AddDigits(AliDigit *lhs, AliDigit *rhs) = 0;
    
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
    

protected:

    /** Name of the simulated digit branch (i.e. PHOSDigits or EMCALDigits) */
    TString fBranchName;

private:

    /** Directory containing the real data to embed into */
    TString fRealDir;

    /** File containing the simulated digits */
    TString fSimFile;

    /** Directory containing the simulated data
     * (this method is not working at the moment, use the
     * file method instead)
     */
    TString fSimDir;

    ClassDef(AliAnalysisEtEmbedder, 0)
};

#endif // ALIANALYSISETEMBEDDER_H

