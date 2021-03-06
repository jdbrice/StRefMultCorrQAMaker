

#include "StRefMultCorrQAMaker.h"

#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StMuDSTMaker/COMMON/StMuEvent.h"
#include "StMuDSTMaker/COMMON/StMuBTofHit.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuPrimaryVertex.h"
#include "StMuDSTMaker/COMMON/StMuTriggerIdCollection.h"
#include "StEvent/StBTofHeader.h"

ClassImp(StRefMultCorrQAMaker);

StRefMultCorrQAMaker::StRefMultCorrQAMaker( string configFile, string outName ) : 
											StMaker("StRefMultCorrQAMaker") {
	int nRuns = 843;
	runList.assign( aRunList, aRunList + nRuns );

	int nBad = 101;
	badRuns.assign( aBadRuns, aBadRuns + nBad );



	cfg = new XmlConfig( configFile );

	Logger::setGlobalLogLevel( Logger::llError );
	logger.setLogLevel( Logger::llError );

	book = new HistoBook( outName, cfg );

}

StRefMultCorrQAMaker::~StRefMultCorrQAMaker() {

	delete cfg;
	// delete also saves
	delete book;

	typedef map<string, ConfigRange*>::iterator scrp_it_type;
	for(scrp_it_type iterator = periods.begin(); iterator != periods.end(); iterator++) {

		if ( iterator->second ){
			logger.debug(__FUNCTION__) << "Releasing " << iterator->first << endl;
			free( periods[ iterator->first ] );
		}

	}

}


Int_t StRefMultCorrQAMaker::Init(){
	

	logger.info( __FUNCTION__ ) << endl;

	if ( cfg->exists( "histograms") ){
		book->cd();
		book->makeAll( "histograms" );
		//book->makeAll( "histograms.PeriodDependent" );
	}
	else
		logger.error( __FUNCTION__ ) << "Could not make histograms " << endl;


	/**
	 * Get Triggers to select
	 */
	triggersToSelect = cfg->getIntVector( "Triggers" );
	logger.info( __FUNCTION__ ) << "Found " << triggersToSelect.size() << " triggers" << endl;
	for ( int i = 0; i < triggersToSelect.size(); i++ ){
		logger.info(__FUNCTION__) << "Triggering on " << triggersToSelect[ i ] << endl;
	}

	compareTriggers = cfg->getBool( "Triggers:compare", false  );
	if ( compareTriggers ){
		vector<string> nToNames = cfg->childrenOf( "TriggerMap" );
		for ( int i = 0; i < nToNames.size(); i++ ){
			int tId = cfg->getInt( nToNames[ i ]+":id" );
			string name = cfg->getString( nToNames[ i ]+":name" );
			triggerNames[ tId ] = name;

			logger.info( __FUNCTION__ ) << tId << " --> " << name << endl;

			vector<string> histos = cfg->childrenOf( "histograms" );
			for ( int j = 0; j < histos.size(); j++ ){

				string hName = cfg->tagName( histos[ j ] );
				if ( cfg->exists( histos[ j ] + ":name" ) )
					hName = cfg->getString(histos[ j ] + ":name" );

				string newHName = (name + "_" + hName);
				if ( !book->exists( newHName ) ){
					logger.info( __FUNCTION__ )<< "Cloning " << hName << " to " << (name + "_" + hName) << endl;
					book->clone( hName, newHName );
				}			

				book->removeFromDir( hName );
			}//loop over histograms
		}//loop on trigger names
	}

	/**
	 * Get Periods
	 */
	vector<string> nPNames = cfg->childrenOf( "Periods" );
	cout << "N Periods" << nPNames.size() << endl;
	for ( int i = 0; i < nPNames.size(); i++ ){
		string name = cfg->getString( nPNames[ i ]+":name" );

		ConfigRange * crg = new ConfigRange( cfg, nPNames[ i ] );
		//cout << " Period " << name << " " << crg->toString() << endl;

		periods[ name ] = crg;
		vector<string> histos = cfg->childrenOf( "histograms.PeriodDependent" );
		for ( int j = 0; j < histos.size(); j++ ){

			string hName = cfg->tagName( histos[ j ] );
			if ( cfg->exists( histos[ j ] + ":name" ) )
				hName = cfg->getString(histos[ j ] + ":name" );

			string newHName = (name + "_" + hName);
			if ( !book->exists( newHName ) ){
				logger.info( __FUNCTION__ )<< "Cloning " << hName << " to " << (name + "_" + hName) << endl;
				book->clone( hName, newHName );
			}		

			book->removeFromDir( hName );
		}//loop over histograms
	}//loop over periods


	/**
	 * Cuts
	 */
	cutVertexZ = new ConfigRange( cfg, "eventCuts.vertexZ", -200, 200 );
	cutVertexR = new ConfigRange( cfg, "eventCuts.vertexR", -200, 200 );
	vertexROffset = new ConfigPoint( cfg, "eventCuts.vertexROffset", -200, 200 );
	cutTofMatches = new ConfigRange( cfg, "eventCuts.tofMult", 0, 100000 );

	cutEta = new ConfigRange( cfg, "trackCuts.eta", -10, 10 );

	logger.info() << "Event Cuts : " << endl;
	logger.info() << "\tcutVertexZ : " << cutVertexZ->toString() << endl;
	logger.info() << "\tcutVertexR : " << cutVertexR->toString() << endl;
	logger.info() << "\tcutVertexROffset : " << vertexROffset->toString() << endl;
	logger.info() << "\tcutTofMult : " << cutTofMatches->toString() << endl;

	logger.info() << "TrackCuts : " << endl;
	logger.info() << "\tcutEta : " << cutEta->toString() << endl;

	

	return kStOK;
} 

Int_t StRefMultCorrQAMaker::Make(){

	

	Int_t iret;
	StMuDstMaker *muDstMaker = (StMuDstMaker*) GetMaker("MuDst");
	if(muDstMaker) {
		mMuDst = muDstMaker->muDst();
		
		if(mMuDst){
			LOG_DEBUG << "Running on MuDst ..." << endm;
			iret = processMuDst();
		} else {
			LOG_ERROR << "No muDST is available ... "<< endm;
			return kStErr;
		}
	} else {
		LOG_ERROR << "No MuDstMaker is available ... "<< endm;
	}

	return kStOK;
}

Int_t StRefMultCorrQAMaker::Finish(){


	return kStOK;
}


Int_t StRefMultCorrQAMaker::processMuDst(){

	StMuEvent *muEvent = mMuDst->event();
	if ( !muEvent )
		return kStOK;

	UInt_t runId = muEvent->runId();
	int ri = runIndex( runId );	

	book->get( "cuts" )->Fill( "All", 1 );

	if ( isBad( runId ) ){
		return kStOK;
	}	

	book->get( "cuts" )->Fill( "Good", 1 );


	triggersFired.clear();
	if ( !compareTriggers  )
	triggersFired.push_back( "" ); // catch all triggers
	// look at the triggers
	bool foundTrigger = false;
	for ( int i = 0; i < triggersToSelect.size(); i++ ){

		if ( muEvent->triggerIdCollection().nominal().isTrigger( triggersToSelect[ i ] ) ){

			if ( compareTriggers )
				triggersFired.push_back(  triggerNames[ triggersToSelect[ i ] ] + "_"  );

			foundTrigger = true;
		}
	}

	if ( !foundTrigger )
		return kStOK;

	book->get( "cuts" )->Fill( "BBC_MB", 1 );
	
	UInt_t refMult = muEvent->refMult();
	Float_t eventBBC = muEvent->runInfo().bbcCoincidenceRate();
	Float_t eventZDC = muEvent->runInfo().zdcCoincidenceRate();

	Int_t nTofMatch = 0;
	Float_t rank = -1000;

	StThreeVectorD pVtx(-999., -999., -999.);  
	if(mMuDst->primaryVertex()) {
		pVtx = mMuDst->primaryVertex()->position();
		nTofMatch = mMuDst->primaryVertex()->nBTOFMatch();
		rank = mMuDst->primaryVertex()->ranking();
		//cout << "RANK : " << rank << endl;
	}


	double vX = pVtx.x() - vertexROffset->x;
	double vY = pVtx.y() - vertexROffset->y;
	double vZ = pVtx.z();

	double vR = TMath::Sqrt( pVtx.x()*pVtx.x() + pVtx.y()*pVtx.y() );
	double svR = TMath::Sqrt( vX*vX + vY*vY );

	Int_t tofMult 	= mMuDst->numberOfBTofHit();
	Int_t nGlobal 	= mMuDst->globalTracks()->GetEntries();
	Int_t nPrimary 	= mMuDst->primaryTracks()->GetEntries();

	double vpdVZ = -999;
	if ( mMuDst->btofHeader() ){
		vpdVZ = mMuDst->btofHeader()->vpdVz();	
	}
	

	/**
	 * Pre Cut histos to fill
	for ( int i = 0; i < triggersFired.size(); i++ ){
		string tn = triggersFired[ i ];

		book->fill( tn+"preVtxZ", ri, pVtx.z() );
		book->fill( tn+"preShiftedVtxR", ri, svR );
		book->fill( tn+"preNTofMatch", ri, nTofMatch );
	} 
	*/


	/**
	 * EVENT CUTS
	 */

	if ( svR < cutVertexR->min || svR > cutVertexR->max )
		return kStOK;

	book->get( "cuts" )->Fill( "vR", 1 );

	if ( vZ < cutVertexZ->min || vZ > cutVertexZ->max )
		return kStOK;

	book->get( "cuts" )->Fill( "vZ", 1 );

	if ( nTofMatch < cutTofMatches->min )
		return kStOK;

	book->get( "cuts" )->Fill( "nTofMatch", 1 );

	for ( int i = 0; i < triggersFired.size(); i++ ){
		string tn = triggersFired[ i ];
		book->fill( tn+"vtxRankPreVpdCut", 	ri, rank );
	}

	//if ( TMath::Abs( vpdVZ - vZ ) > 3 )
	//	return kStOK;

	// analyze the tracks in the event 
	analyzePrimaryTracks( ri, triggersFired );

	// we should have an event with at least 2 tof-matched tracks now
	
	
	/**
	 * Fill QA for each Trigger
	 */
	for ( int i = 0; i < triggersFired.size(); i++ ){
		string tn = triggersFired[ i ];

		book->fill( tn+"nEvents", 		ri );
		book->fill( tn+"bbc", 			ri, eventBBC );
		book->fill( tn+"zdc", 			ri, eventZDC );
			
	
		book->fill( tn+"nGlobal", 		ri, nGlobal );
		book->fill( tn+"nPrimary", 		ri, nPrimary );
		book->fill( tn+"nTofMatch", 	ri, nTofMatch );

		book->fill( tn+"vtxRank", 		ri, rank );
		book->fill( tn+"vtxR", 			ri, vR );
		book->fill( tn+"vtxX", 			ri, pVtx.x() );
		book->fill( tn+"vtxY", 			ri, pVtx.y() );
		book->fill( tn+"vtxZ", 			ri, pVtx.z() );
		book->fill( tn+"vpdVtxZ", 		ri, vpdVZ );
		book->fill( tn+"diffVpdTpc", 	ri, vpdVZ - vZ );
		book->fill( tn+"shiftedVtxR", 	ri, svR );

		book->fill( tn+"refMult", 		ri, refMult );
		book->fill( tn+"tofMult", 		ri, tofMult );
	}


	/**
	 * Loop over periods and report
	 */
	typedef map<string, ConfigRange*>::iterator scrp_it_type;
	for(scrp_it_type iterator = periods.begin(); iterator != periods.end(); iterator++) {
		if ( iterator->second ){
			ConfigRange * crg = iterator->second;
			if ( runId >= crg->min && runId < crg->max ){
				
				string prefix = iterator->first + "_";
				book->fill( prefix+"refMultZ", pVtx.z(), refMult );
				book->fill( prefix+"refMultBBC", eventBBC, refMult );
				book->fill( prefix+"refMultZDC", eventZDC, refMult );
				book->fill( prefix+"refMultTOF", tofMult, refMult );
				

			}
		}
	}


	return kStOK;
}


Int_t StRefMultCorrQAMaker::analyzePrimaryTracks( Int_t ri, vector<string> tNames, bool afterCuts ){

	Int_t nPrimary 	= mMuDst->primaryTracks()->GetEntries();
	
	for (int iNode = 0; iNode < nPrimary; iNode++ ){
		StMuTrack*	tPrimary 	= (StMuTrack*)mMuDst->primaryTracks(iNode);
		StMuTrack*	tGlobal 	= (StMuTrack*)tPrimary->globalTrack();

		if ( !tPrimary ) continue;
		//if ( tPrimary->vertexIndex() != 0 ) continue;

		StThreeVectorF momentum = tPrimary->momentum();

		double eta = momentum.pseudoRapidity();
		if ( eta < cutEta->min || eta > cutEta->max )
			continue;

		for ( int i = 0; i < tNames.size(); i++ ){
			string tn = tNames[ i ];

			if ( afterCuts ){
				book->fill( tn+"ptPrimary", ri, momentum.perp() );
				book->fill( tn+"pPrimary", ri, momentum.magnitude() );
				book->fill( tn+"etaPrimary", ri, momentum.pseudoRapidity() );
				book->fill( tn+"phiPrimary", ri, momentum.phi() );

				book->fill( tn+"etaVsPhi", momentum.phi(), momentum.pseudoRapidity() );

				book->fill( tn+"pxPrimary", ri, momentum.x() );
				book->fill( tn+"pyPrimary", ri, momentum.y() );
				book->fill( tn+"pzPrimary", ri, momentum.z() );

			} else {
				book->fill( tn+"prePzPrimary", ri, momentum.z() );
			}


		}
	}


}

Int_t StRefMultCorrQAMaker::nTofMatchedTracks(){

	Int_t nPrimary 	= mMuDst->primaryTracks()->GetEntries();
	Int_t nTofMatched = 0;
	for (int iNode = 0; iNode < nPrimary; iNode++ ){
		StMuTrack*	tPrimary 	= (StMuTrack*)mMuDst->primaryTracks(iNode);
		StMuTrack*	tGlobal 	= (StMuTrack*)tPrimary->globalTrack();

		if ( !tPrimary ) continue;
		if ( tPrimary->vertexIndex() != 0 ) continue;

		/**
		 * Get Tof info
		 */
		StMuBTofPidTraits bTofPidTraits	= tPrimary->btofPidTraits();

		if ( bTofPidTraits.matchFlag() > 0 ) 
			nTofMatched++;
	}

	return nTofMatched;

}

int StRefMultCorrQAMaker::aRunList[] = {
1000,
15046073, 
15046089, 
15046094, 
15046096, 
15046102, 
15046103, 
15046104, 
15046105, 
15046106, 
15046107, 
15046108, 
15046109, 
15046110, 
15046111, 
15047004, 
15047015, 
15047016, 
15047019, 
15047021, 
15047023, 
15047024, 
15047026, 
15047027, 
15047028, 
15047029, 
15047030, 
15047039, 
15047040, 
15047041, 
15047044, 
15047047, 
15047050, 
15047052, 
15047053, 
15047056, 
15047057, 
15047061, 
15047062, 
15047063, 
15047064, 
15047065, 
15047068, 
15047069, 
15047070, 
15047071, 
15047072, 
15047074, 
15047075, 
15047082, 
15047085, 
15047086, 
15047087, 
15047093, 
15047096, 
15047097, 
15047098, 
15047100, 
15047102, 
15047104, 
15047106, 
15048003, 
15048004, 
15048012, 
15048013, 
15048014, 
15048016, 
15048017, 
15048018, 
15048019, 
15048020, 
15048021, 
15048023, 
15048024, 
15048025, 
15048026, 
15048028, 
15048029, 
15048030, 
15048031, 
15048033, 
15048034, 
15048074, 
15048075, 
15048076, 
15048077, 
15048078, 
15048079, 
15048080, 
15048081, 
15048082, 
15048083, 
15048084, 
15048085, 
15048086, 
15048087, 
15048088, 
15048089, 
15048091, 
15048092, 
15048093, 
15048094, 
15048095, 
15048096, 
15048097, 
15048098, 
15049002, 
15049003, 
15049009, 
15049013, 
15049014, 
15049015, 
15049016, 
15049017, 
15049018, 
15049019, 
15049020, 
15049021, 
15049022, 
15049023, 
15049025, 
15049026, 
15049027, 
15049028, 
15049030, 
15049031, 
15049032, 
15049033, 
15049037, 
15049038, 
15049039, 
15049040, 
15049041, 
15049074, 
15049077, 
15049083, 
15049084, 
15049085, 
15049086, 
15049087, 
15049088, 
15049089, 
15049090, 
15049091, 
15049092, 
15049093, 
15049094, 
15049096, 
15049097, 
15049098, 
15049099, 
15050001, 
15050002, 
15050003, 
15050004, 
15050005, 
15050006, 
15050010, 
15050011, 
15050012, 
15050013, 
15050014, 
15050015, 
15050016, 
15051131, 
15051132, 
15051133, 
15051134, 
15051137, 
15051141, 
15051144, 
15051146, 
15051147, 
15051148, 
15051149, 
15051156, 
15051157, 
15051159, 
15051160, 
15052001, 
15052004, 
15052005, 
15052006, 
15052007, 
15052008, 
15052009, 
15052010, 
15052011, 
15052014, 
15052015, 
15052016, 
15052017, 
15052018, 
15052019, 
15052020, 
15052021, 
15052022, 
15052023, 
15052024, 
15052025, 
15052026, 
15052040, 
15052041, 
15052042, 
15052043, 
15052060, 
15052061, 
15052062, 
15052063, 
15052064, 
15052065, 
15052066, 
15052067, 
15052068, 
15052069, 
15052070, 
15052073, 
15052074, 
15052075, 
15053001, 
15053002, 
15053003, 
15053004, 
15053005, 
15053006, 
15053007, 
15053008, 
15053009, 
15053011, 
15053012, 
15053014, 
15053015, 
15053016, 
15053017, 
15053019, 
15053020, 
15053021, 
15053022, 
15053023, 
15053024, 
15053025, 
15053026, 
15053027, 
15053028, 
15053029, 
15053034, 
15053035, 
15053047, 
15053048, 
15053049, 
15053050, 
15053052, 
15053053, 
15053054, 
15053055, 
15053056, 
15053057, 
15053058, 
15053059, 
15053060, 
15053062, 
15053064, 
15053065, 
15053067, 
15054001, 
15054002, 
15054003, 
15054004, 
15054005, 
15054006, 
15054007, 
15054008, 
15054009, 
15054010, 
15054011, 
15054012, 
15054013, 
15054014, 
15054015, 
15054016, 
15054017, 
15054018, 
15054019, 
15054020, 
15054021, 
15054023, 
15054024, 
15054025, 
15054026, 
15054028, 
15054029, 
15054030, 
15054031, 
15054037, 
15054042, 
15054043, 
15054044, 
15054046, 
15054047, 
15054048, 
15054049, 
15054050, 
15054051, 
15054052, 
15054053, 
15054054, 
15055001, 
15055002, 
15055003, 
15055004, 
15055005, 
15055006, 
15055007, 
15055008, 
15055009, 
15055011, 
15055012, 
15055013, 
15055014, 
15055015, 
15055016, 
15055017, 
15055018, 
15055019, 
15055020, 
15055021, 
15055131, 
15055133, 
15055134, 
15055135, 
15055136, 
15055137, 
15055138, 
15055139, 
15055140, 
15055141, 
15056001, 
15056002, 
15056003, 
15056004, 
15056005, 
15056006, 
15056007, 
15056008, 
15056009, 
15056013, 
15056014, 
15056015, 
15056016, 
15056017, 
15056018, 
15056019, 
15056020, 
15056021, 
15056022, 
15056023, 
15056024, 
15056025, 
15056026, 
15056027, 
15056028, 
15056036, 
15056037, 
15056038, 
15056039, 
15056113, 
15056114, 
15056116, 
15056117, 
15056119, 
15056123, 
15056124, 
15056125, 
15057001, 
15057002, 
15057003, 
15057004, 
15057005, 
15057006, 
15057007, 
15057008, 
15057010, 
15057011, 
15057012, 
15057013, 
15057014, 
15057015, 
15057016, 
15057017, 
15057018, 
15057019, 
15057020, 
15057021, 
15057022, 
15057023, 
15057029, 
15057030, 
15057031, 
15057048, 
15057049, 
15057050, 
15057051, 
15057052, 
15057053, 
15057054, 
15057055, 
15057056, 
15057057, 
15057058, 
15057059, 
15057060, 
15057061, 
15057062, 
15057063, 
15058001, 
15058002, 
15058003, 
15058004, 
15058005, 
15058006, 
15058007, 
15058008, 
15058009, 
15058010, 
15058011, 
15058012, 
15058013, 
15058014, 
15058015, 
15058016, 
15058017, 
15058018, 
15058019, 
15058020, 
15058021, 
15058022, 
15058024, 
15058025, 
15058032, 
15058033, 
15058043, 
15058044, 
15058045, 
15058048, 
15058049, 
15058050, 
15058051, 
15058052, 
15058053, 
15058054, 
15058055, 
15059001, 
15059002, 
15059003, 
15059005, 
15059006, 
15059007, 
15059009, 
15059010, 
15059011, 
15059013, 
15059014, 
15059015, 
15059016, 
15059017, 
15059018, 
15059019, 
15059020, 
15059021, 
15059024, 
15059025, 
15059026, 
15059027, 
15059028, 
15059029, 
15059033, 
15059037, 
15059038, 
15059040, 
15059041, 
15059042, 
15059055, 
15059057, 
15059058, 
15059059, 
15059060, 
15059061, 
15059063, 
15059064, 
15059068, 
15059074, 
15059076, 
15059081, 
15059082, 
15059086, 
15059087, 
15059090, 
15060001, 
15060006, 
15060007, 
15060009, 
15060011, 
15060012, 
15060014, 
15060015, 
15060016, 
15060017, 
15060018, 
15060019, 
15060020, 
15060021, 
15060022, 
15060023, 
15060024, 
15060027, 
15060028, 
15060029, 
15060031, 
15060032, 
15060033, 
15060035, 
15060036, 
15060037, 
15060044, 
15060045, 
15060046, 
15060047, 
15060048, 
15060049, 
15060050, 
15060051, 
15060052, 
15060053, 
15060061, 
15060062, 
15060067, 
15060068, 
15060069, 
15060070, 
15060071, 
15061001, 
15061002, 
15061003, 
15061004, 
15061006, 
15061007, 
15061008, 
15061009, 
15061010, 
15061011, 
15061012, 
15061014, 
15061015, 
15061016, 
15061018, 
15061019, 
15061021, 
15061023, 
15061024, 
15061025, 
15061026, 
15061027, 
15061028, 
15061034, 
15061036, 
15061037, 
15061038, 
15061039, 
15061041, 
15061047, 
15061048, 
15061051, 
15061053, 
15061054, 
15061055, 
15061056, 
15061059, 
15061060, 
15061061, 
15061062, 
15061063, 
15061064, 
15062002, 
15062003, 
15062004, 
15062005, 
15062006, 
15062007, 
15062008, 
15062009, 
15062010, 
15062011, 
15062012, 
15062013, 
15062015, 
15062017, 
15062018, 
15062020, 
15062022, 
15062023, 
15062024, 
15062025, 
15062026, 
15062031, 
15062032, 
15062033, 
15062034, 
15062035, 
15062036, 
15062037, 
15062038, 
15062040, 
15062041, 
15062042, 
15062066, 
15062069, 
15062070, 
15062071, 
15062075, 
15062076, 
15062077, 
15063001, 
15063002, 
15063003, 
15063006, 
15063008, 
15063010, 
15063011, 
15063012, 
15063013, 
15063014, 
15063016, 
15063017, 
15063018, 
15063020, 
15063021, 
15063029, 
15063032, 
15063036, 
15063037, 
15063040, 
15063041, 
15063042, 
15063043, 
15063045, 
15063046, 
15063048, 
15063049, 
15063053, 
15063056, 
15063059, 
15063060, 
15063061, 
15063062, 
15063063, 
15063065, 
15063066, 
15063067, 
15064001, 
15064002, 
15064003, 
15064005, 
15064006, 
15064007, 
15064008, 
15064009, 
15064010, 
15064011, 
15065011, 
15065012, 
15065013, 
15065014, 
15065017, 
15065054, 
15065056, 
15065059, 
15065060, 
15065061, 
15066005, 
15066006, 
15066008, 
15066010, 
15066012, 
15066013, 
15066014, 
15066016, 
15066017, 
15066018, 
15066019, 
15066020, 
15066021, 
15066022, 
15066023, 
15066024, 
15066025, 
15066026, 
15066064, 
15066065, 
15066070, 
15066071, 
15066072, 
15066074, 
15066076, 
15066077, 
15066082, 
15066083, 
15066085, 
15066086, 
15066088, 
15067001, 
15067002, 
15067003, 
15067005, 
15067006, 
15067008, 
15067009, 
15067011, 
15067012, 
15067013, 
15067014, 
15067015, 
15067016, 
15067018, 
15067019, 
15067020, 
15067022, 
15067023, 
15067024, 
15067025, 
15067026, 
15067027, 
15067032, 
15067033, 
15067034, 
15067035, 
15067036, 
15067037, 
15067038, 
15067040, 
15067041, 
15067042, 
15067043, 
15067044, 
15067046, 
15067048, 
15068001, 
15068002, 
15068003, 
15068004, 
15068005, 
15068006, 
15068007, 
15068008, 
15068009, 
15068010, 
15068012, 
15068013, 
15068014, 
15068016, 
15068018, 
15068022, 
15068023, 
15068024, 
15068025, 
15068026, 
15068027, 
15068028, 
15068029, 
15068030, 
15068031, 
15068032, 
15068033, 
15068034, 
15068035, 
15068036, 
15068037, 
15068038, 
15068039, 
15068040, 
15068041, 
15068042, 
15068043, 
15068044, 
15068045, 
15068046, 
15068047, 
15068048, 
15068049, 
15069001, 
15069002, 
15069003, 
15069004, 
15069005, 
15069006, 
15069007, 
15069008, 
15069009, 
15069010, 
15069011, 
15069012, 
15069013, 
15069014, 
15069015, 
15069016, 
15069017, 
15069018, 
15069019, 
15069020, 
15069021, 
15069022, 
15069023, 
15069030, 
15069031, 
15069034, 
15069035, 
15069036, 
15069038, 
15069039, 
15069040, 
15069042, 
15069043, 
15069044, 
15069045, 
15069046, 
15069047, 
15069048, 
15069049, 
15069050, 
15069051, 
15070001, 
15070002, 
15070008, 
15070009, 
15070010, 
15070013, 
15070014, 
15070015, 
15070016, 
15070017, 
15070018, 
15070019, 
15070020, 
15070021
};




int StRefMultCorrQAMaker::aBadRuns[] = { 
15046073, 
15046089, 
15046094, 
15046096, 
15046102, 
15046103, 
15046104, 
15046105, 
15046106, 
15046107, 
15046108, 
15046109, 
15046110, 
15046111, 
15047004, 
15047015, 
15047016, 
15047019, 
15047021, 
15047023, 
15047024, 
15047026, 
15047027, 
15047028, 
15047029, 
15047030, 
15047039, 
15047040, 
15047041, 
15047044, 
15047047, 
15047050, 
15047052, 
15047053, 
15047056, 
15047057, 
15047061, 
15047062, 
15047063, 
15047064, 
15047065, 
15047068, 
15047069, 
15047070, 
15047071, 
15047072, 
15047074, 
15047075, 
15047082, 
15047085, 
15047086, 
15047087, 
15047093, 
15048017, 
15049027, 
15049077, 
15051131, 
15051132, 
15051133, 
15051141, 
15051144, 
15051148, 
15052067, 
15052068, 
15052069, 
15052070, 
15053027, 
15053028, 
15053029, 
15053034, 
15053035, 
15053052, 
15053054, 
15053055, 
15054053, 
15054054, 
15055018, 
15055137, 
15056117, 
15057055, 
15057059, 
15058006, 
15058011, 
15058021, 
15059057, 
15059058, 
15061001, 
15061009, 
15062006, 
15062069, 
15065012, 
15065014, 
15066070, 
15068013, 
15068014, 
15068016, 
15068018, 
15069036, 
15070008, 
15070009, 
15070010
};




