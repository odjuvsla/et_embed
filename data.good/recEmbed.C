void recEmbed()
{
  AliReconstruction reco;

  //  reco.SetWriteESDfriend();
  //  reco.SetWriteAlignmentData();
  AliCDBManager * man = AliCDBManager::Instance() ;
  //   man->SetDefaultStorage("local://$ALICE_ROOT/OCDB") ;
   man->SetDefaultStorage("raw://") ;
      //  man->SetRun(118339) ;
  man->SetRun(137366) ;

  //  reco.SetDefaultStorage("local://$ALICE_ROOT/OCDB");
  //  reco.SetSpecificStorage("GRP/GRP/Data",
  //  			  Form("local://%s",gSystem->pwd()));
  reco.SetRunPlaneEff(kFALSE);

  //  reco.SetFractionFriends(1.);


   reco.SetRunLocalReconstruction(":");
  //  reco.SetRunReconstruction("PHOS EMCAL");
   reco.SetRunLocalReconstruction("PHOS");

  reco.SetRunHLTTracking(kFALSE);
  reco.SetRunVertexFinder(kFALSE);
  reco.SetRunVertexFinderTracks(kFALSE);
  reco.SetRunMultFinder(kFALSE);
  reco.SetRunTracking(":");
  reco.SetRunQA(":");
  reco.SetFillESD(":");
  reco.SetFillESD("PHOS"); 
    
  TStopwatch timer;
  timer.Start();
  reco.Run();
  timer.Stop();
  timer.Print();
}
