// TradeCSV.cpp : Defines the entry point for the console application.
//

//#include <winbase.h>
//#include <stdlib.h>
//using namespace std;

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <ctime>
#include "NxCoreAPI.h"
#include "NxCoreAPI_class.h"
#include <map>


NxCoreClass nxCoreClass;
int previousHour = 0;
char dir[100];
std::map<std::string, FILE*> symbols;
FILE* global;

void getSymbol(const NxCoreMessage* pNxCoreMsg, char *Symbol) {
	// Is this a valid option?
	if ((pNxCoreMsg->coreHeader.pnxStringSymbol->String[0] == 'o') && (pNxCoreMsg->coreHeader.pnxOptionHdr)) {
		// If pnxsDateAndStrike->String[1] == ' ', then this symbol is in new OSI format.
		if (pNxCoreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[1] == ' ') {
			sprintf(Symbol, "%s%02d%02d%02d%c%08d",
				pNxCoreMsg->coreHeader.pnxStringSymbol->String,
				pNxCoreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Year - 2000,
				pNxCoreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Month,
				pNxCoreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Day,
				(pNxCoreMsg->coreHeader.pnxOptionHdr->PutCall == 0) ? 'C' : 'P',
				pNxCoreMsg->coreHeader.pnxOptionHdr->strikePrice);
		} else {		// Otherwise the symbol is in old OPRA format.
			sprintf(Symbol, "%s%c%c",
				pNxCoreMsg->coreHeader.pnxStringSymbol->String,
				pNxCoreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[0],
				pNxCoreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[1]);
		}
	} else { // Not an option, just copy the symbol
		strcpy(Symbol, pNxCoreMsg->coreHeader.pnxStringSymbol->String);
	}
}

int __stdcall processNxCoreStatus(const NxCoreSystem* pNxCoreSys) {
	switch (pNxCoreSys->Status) {
	case NxCORESTATUS_RUNNING:       // all is well.
		break;
	case NxCORESTATUS_INITIALIZING:  // start up code.
		fprintf(stderr, "NxCoreStatus_Initializing\n");
		break;
	case NxCORESTATUS_COMPLETE:      // tear down code.
		fprintf(stderr, "NxCoreStatus_Complete\n");
		break;
	case NxCORESTATUS_ERROR:
		fprintf(stderr, "NxCoreStatus_Error %s (%d)\n", pNxCoreSys->StatusDisplay, pNxCoreSys->StatusData);
		return NxCALLBACKRETURN_STOP;   // not required to return this
	}
	return NxCALLBACKRETURN_CONTINUE;
}

int __stdcall nxCoreCallback(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMessage) {
	switch (pNxCoreMessage->MessageType) {
		case NxMSG_STATUS:       return processNxCoreStatus(pNxCoreSys);
		case NxMSG_TRADE: {
			const NxCoreTrade& nt = pNxCoreMessage->coreData.Trade;
			const NxCoreHeader& ch = pNxCoreMessage->coreHeader;
			const NxTime&       t = pNxCoreSys->nxTime;

			if ((int)t.Hour != previousHour) {
				time_t now = time(0);
				fprintf(stdout, "%.2d -- %d\n", (int)t.Hour, (int)now);
				previousHour = (int)t.Hour;
			}

			char symbol[23];
			getSymbol(pNxCoreMessage, symbol);

			std::map<std::string, FILE*>::iterator it;
			FILE * outputFile;

			if (symbols.size() == 0) {
				/*char filename[150];
				sprintf(filename, "%s\\%s.txt", dir, symbol);
				outputFile = fopen(filename, "a");*/
				outputFile = global;
			} else {
				it = symbols.find(std::string(symbol));
				if (it == symbols.end()) {
					break; //not found
				}
				else {
					outputFile = it->second;
				}
			}

			fprintf(outputFile,"%s\t%.2d:%.2d:%.2d.%.3d\t%ld\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%I64d\t%.2lf\n",
				symbol,
				(int)t.Hour, (int)t.Minute, (int)t.Second, (int)t.Millisecond,
				nt.Size,
				nxCoreClass.PriceToDouble(nt.Price, nt.PriceType),
				nxCoreClass.PriceToDouble(nt.Open, nt.PriceType),
				nxCoreClass.PriceToDouble(nt.High, nt.PriceType),
				nxCoreClass.PriceToDouble(nt.Low, nt.PriceType),
				nxCoreClass.PriceToDouble(nt.Last, nt.PriceType),
				nt.TotalVolume,
				nxCoreClass.PriceToDouble(nt.NetChange, nt.PriceType));
			
			/*if (symbols.size() == 0) {
				fclose(outputFile);
			}*/
			
			break;
		}
	}
	return NxCALLBACKRETURN_CONTINUE;
}

int main(int argc, char** argv) {

	if (argc < 2) {
		fprintf(stderr, "Input file required as argument.\n");
		return 1;
	}

	if (!nxCoreClass.LoadNxCore("NxCoreAPI.dll")) {
		fprintf(stderr, "Can't find NxCoreAPI.dll\n");
		return -1;
	}

	fprintf(stderr, "Reading input from %s\n", argv[1]);
	if (argc>2) {
		fprintf(stderr, "This will delete the directory output_%s and all of its contents!\n", argv[1]);
		fprintf(stderr, "Press control+c to quit\n");
		system("PAUSE");
		fprintf(stderr, "Deleting contents...");
		wchar_t wtext[100];
		sprintf(dir, "output_%s", argv[1]);
		mbstowcs(wtext, dir, strlen(dir) + 1);//Plus null
		char rmcmd[100];
		sprintf(rmcmd, "del /q /a %s", dir);
		system(rmcmd);
		CreateDirectory(wtext,NULL);
		fprintf(stderr, "...DONE\n");

		char filename[150];
		for (int i = 2; i < argc; i++) {
			sprintf(filename, "%s\\%s.txt", dir, argv[i]);
			symbols[argv[i]] = fopen(filename, "a");
		}
	} else {
		fprintf(stderr, "This will delete the file output_%s.txt!\n", argv[1]);
		fprintf(stderr, "Press control+c to quit\n");
		system("PAUSE");
		sprintf(dir, "output_%s.txt", argv[1]);
		global = fopen(dir, "w");
	}


	nxCoreClass.ProcessTape(argv[1], 0, NxCF_EXCLUDE_CRC_CHECK, 0, nxCoreCallback);
	
	//Close all open files
	fprintf(stderr, "Going to close files...");
	if (symbols.size() > 0) {
		for (std::map<std::string, FILE*>::iterator it = symbols.begin(); it != symbols.end(); ++it)
			fclose(it->second);
	} else {
		fclose(global);
	}
	fprintf(stderr, "...DONE\n");


	return 0;
}