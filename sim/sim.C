void sim(Int_t nev=4) {

  AliSimulation simulator;

  simulator.SetMakeDigits(":");
  simulator.SetMakeSDigits("");

  simulator.SetMakeDigits("PHOS");// EMCAL");
  simulator.SetMakeSDigits("PHOS"); // EMCAL");
  //  simulator.SetWriteRawData("PHOS EMCAL");
  simulator.SetMakeDigitsFromHits(":");
  simulator.SetRunQA(kFALSE);
  simulator.SetRunHLT("");

  simulator.SetDefaultStorage("local://$ALICE_ROOT/OCDB");
  simulator.SetSpecificStorage("GRP/GRP/Data",
			       Form("local://%s",gSystem->pwd()));

  TStopwatch timer;
  timer.Start();
  simulator.Run(nev);
  timer.Stop();
  timer.Print();
}
