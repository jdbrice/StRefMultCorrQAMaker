#include "iostream.h"


// Forward the class declarations for StChain and St_db_Maker
class     StChain;
class     St_db_Maker;

void loadLibs();

void makeQA(    const Char_t *fileList = "only1.lis",
				const Char_t *outName = "rmcQA.histo.root" )
{

	StChain  *chain = 0;
	St_db_Maker *dbMk = 0;

	Int_t iEvt = 0, istat = 0;

	Int_t nEvents = 1000000;
	Int_t nfiles = 10000;

	//
	// First load some shared libraries we need
	//
	loadLibs();

	// create the chain
	chain  = new StChain("StChain");


	// create the StMuDstMaker
	StMuDstMaker *muDstMaker = new StMuDstMaker(  0, 0, "", fileList, "MuDst.root", nfiles );

	StRefMultCorrQAMaker * rmcQA = new StRefMultCorrQAMaker( "cMakeQA.xml", outName );
	//StRmcQAMaker * rmcQA = new StRmcQAMaker( muDstMaker, outName );
	
	// Initialize chain
	Int_t iInit = chain->Init();
	
	// ensure that the chain initializes
	if ( iInit ) 
		chain->Fatal(iInit,"on init");

	//
	// Event loop
	//
	int istat = 0, i = 1;


	while ( i <= nEvents && istat != 2 ){
	 
		// clear the chain state
		chain->Clear();

		// make the chain and get that status
		istat = chain->Make(i);

		if (istat == 2) {
			// last event
		} else if (istat == 3) {
			// error
		}
		 
		 i++;
	}


	// Chain Finish
	if (nEvents > 1) {
		chain->Finish();
	}

	delete chain;
}






void loadLibs(){


	if (gClassTable->GetID("TTable") < 0) {
		gSystem->Load("libStar");
		gSystem->Load("libPhysics");
	}  
	gROOT->LoadMacro("$STAR/StRoot/StMuDSTMaker/COMMON/macros/loadSharedLibraries.C");
	loadSharedLibraries();

	gSystem->Load("StRefMultCorrQA");

}