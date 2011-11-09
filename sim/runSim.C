void runSim(Int_t nEvs = 200)
{
  gInterpreter->ProcessLine(".L sim.C");
  sim(nEvs);
}
