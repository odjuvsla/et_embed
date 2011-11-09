void runExtractDigits()
{
  gInterpreter->ProcessLine(".L digitEmbedder.C+g");
  digitEmbedder(false, 0);
}
