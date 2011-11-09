
int runTest(bool createFile)
{
    AliAnalysisEtEmbedderPhos *embedder = new AliAnalysisEtEmbedderPhos();

    embedder->GetOCDBData(137366);

    if (createFile)
    {
        embedder->GetSimulatedDigits("sim/", "mydigits.root");
    }
    else
    {
        embedder->LoadDigitsFromFile("mydigits.root");

        embedder->Embed("data/");
    }

    return 0;
}

int testEmbedder(bool createFile = false)
{
    gInterpreter->ProcessLine(".L AliAnalysisEtEmbedder.cxx+g");
    gInterpreter->ProcessLine(".L AliAnalysisEtEmbedderPhos.cxx+g");

    runTest(createFile);


}
