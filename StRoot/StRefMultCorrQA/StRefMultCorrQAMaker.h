#ifndef ST_REFMULTCORR_QA_MAKER_H
#define ST_REFMULTCORR_QA_MAKER_H

/**
 * STAR
 */
#include "StMaker.h"

/**
 * STL
 */
#include <string>
#include <vector>
#include <map>
using namespace std;


/**
 * RooBarb
 */
#include "HistoBook.h"
#include "XmlConfig.h"
#include "Logger.h"
#include "ConfigRange.h"
 #include "ConfigPoint.h"
using namespace jdb;


// forward classes
class StMuDstMaker;
class StMuDst;
class StMuEvent;

class StRefMultCorrQAMaker : public StMaker {

protected:
	static int aRunList[];
	vector<int> runList;

	ConfigRange * cutTofMatches;
	ConfigRange * cutEta;
	ConfigRange * cutVertexZ;
	ConfigRange * cutVertexR;
	ConfigPoint * vertexROffset;
	XmlConfig *cfg;
	HistoBook *book;
	Logger logger;

	StMuDst * mMuDst;

	vector<int> triggersToSelect;
	map<int, string> triggerNames;

	vector<string> triggersFired;

public:
	StRefMultCorrQAMaker( string configFile, string outName="rmcQA.histo.root");
	virtual	~StRefMultCorrQAMaker(); 

	/*
	Called Once at the beginning of the job
	@return StOk if no error
	 */
	Int_t Init(); 
	/*
	Called once for each event
	@return StOk if no error
	 */
	Int_t Make(); 
	/*
	Called Once at the end of the job
	@return StOk if no error
	 */
	Int_t Finish(); 

	Int_t processMuDst();

	Int_t analyzePrimaryTracks( Int_t ri, vector<string> tNames );

	Int_t nTofMatchedTracks();

protected:

	int runIndex( UInt_t runId ){
		std::vector<int>::iterator it = std::find( runList.begin(), runList.end(), runId );
		if (it == runList.end()){
			return -1;
		}
		
		return (int)(it - runList.begin());
	}


public:
	ClassDef(StRefMultCorrQAMaker,1);
};




#endif