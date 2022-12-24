#include <iostream>
#include <string>
#include <map>
#include "tradebookingservice.hpp"
#include "soa.hpp"
#include "products.hpp"
#include "algoexecutionservice.hpp"
#include "guiservice.hpp"
#include "historicaldataservice.hpp"
#include "inquiryservice.hpp"
#include "marketdataservice.hpp"
#include "positionservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "streamingservice.hpp"
#include "algostreamingservice.hpp"
#include "executionservice.hpp"
#include <random>

using namespace std;

int main()
{
	//Create Data
	double bid = 99;
	double offer = 101;
	vector<string> bondidvec{ "91282ACV2", "91282ZXT4", "91282FGH8", "91282SDF1", "91282JKL3", "91282XCV5", "91282BNM7" };
	
	default_random_engine generator;
	uniform_int_distribution<int> dist1(99, 101);
	uniform_int_distribution<int> dist2(11, 80);
	uniform_int_distribution<int> dist3(1, 9);
	uniform_int_distribution<int> dist4(1, 9);
	uniform_int_distribution<int> dist6(0, 9);


	//ofstream iq_file;
	//iq_file.open("inquiry.txt");
	//for (int id = 0; id < bondidvec.size(); id++)
	//{
	//	for (int j = 0; j < 5; j++)
	//	{
	//		int first = dist1(generator);
	//		int second = dist2(generator);
	//		int quantity = dist3(generator) * 100000;

	//		iq_file << "I" + IDcreator(generator, dist6) << "," << bondidvec[id] << ",BUY," << quantity  << "," << first << "-0" << second << ",RECEIVED\n";
	//		iq_file << "I" + IDcreator(generator, dist6) << "," << bondidvec[id] << ",SELL," << quantity << "," << first << "-0" << second << ",RECEIVED\n";
	//	}
	//}
	//iq_file.close();


	//ofstream md_file;
	//md_file.open("marketdata.txt");
	//for (int id = 0; id < bondidvec.size(); id++)
	//{
	//	for (int j = 0; j < 500000; j++)
	//	{
	//		int first = dist1(generator);
	//		int second = dist2(generator);
	//		int quantity = dist3(generator) * 100000;
	//		int var = dist4(generator);
	//		md_file << bondidvec[id] << "," << first << "-0" << second << "," << quantity << "," << "BID\n";
	//		md_file << bondidvec[id] << "," << first << "-0" << second + var << "," << quantity << "," << "OFFER\n";
	//	}
	//}
	//md_file.close();


	//ofstream trade_file;
	//trade_file.open("trade.txt");

	//for (int id = 0; id < bondidvec.size(); id++)
	//{
	//	for (int j = 0; j < 5; j++)
	//	{
	//		int first = dist1(generator);
	//		int second = dist2(generator);
	//		int quantity = dist3(generator) * 100000;
	//		int var = dist4(generator);
	//		string book;
	//		if (j % 3 == 0)
	//			book = "TRSY1";
	//		else if (j % 3 == 1)
	//			book = "TRSY2";
	//		else
	//			book = "TRSY3";
	//		trade_file << bondidvec[id] << ",T" + IDcreator(generator, dist6) << "," << first << "-0" << second << "," << book << "," << quantity << ",BUY\n";
	//		trade_file << bondidvec[id] << ",T" + IDcreator(generator, dist6) << "," << first << "-0" << second + var << "," << book << "," << quantity - 50000 << ",SELL\n";
	//	}
	//}
	//trade_file.close();


	//ofstream price_file;
	//price_file.open("prices.txt");
	//for (int id = 0; id < bondidvec.size(); id++)
	//{
	//	for (int j = 0; j < 1000000; j++)
	//	{
	//		int first = dist1(generator);
	//		int second = dist2(generator);
	//		int var = dist4(generator);
	//		price_file << bondidvec[id] << "," << first << "-0" << second << "," << first << "-0" << second + var << endl;
	//	}
	//}
	//price_file.close();



	
	PricingService<Bond> bondpricingService;
	TradeBookingService<Bond> bondtradeBookingService;
	PositionService<Bond> bondpositionService;
	RiskService<Bond> bondriskService;
	MarketDataService<Bond> bondmarketDataService;
	AlgoExecutionService<Bond> bondalgoExecutionService;
	AlgoStreamingService<Bond> bondalgoStreamingService;
	GUIService<Bond> bondguiService;
	ExecutionService<Bond> bondexecutionService;
	StreamingService<Bond> bondstreamingService;
	InquiryService<Bond> bondinquiryService;


	BondPositionHistoricalDataService<Position<Bond>> bondhistoricalPositionService;
	BondRiskHistoricalDataService<PV01<Bond>>bondhistoricalRiskService;
	BondExecutionHistoricalDataService<ExecutionOrder<Bond>> bondhistoricalExecutionService;
	BondStreamingHistoricalDataService<PriceStream<Bond>> bondhistoricalStreamingService;
	BondInquiryHistoricalDataService<Inquiry<Bond>> bondhistoricalInquiryService;

	//1
	bondmarketDataService.AddListener(bondalgoExecutionService.GetListener());
	bondalgoExecutionService.AddListener(bondexecutionService.GetListener());
	bondexecutionService.AddListener(bondtradeBookingService.GetListener());
	//2
	bondtradeBookingService.AddListener(bondpositionService.GetListener());
	bondpositionService.AddListener(bondriskService.GetListener());
	//3
	bondpricingService.AddListener(bondalgoStreamingService.GetListener());
	bondalgoStreamingService.AddListener(bondstreamingService.GetListener());
	//6
	bondpositionService.AddListener(bondhistoricalPositionService.GetListener());
	bondstreamingService.AddListener(bondhistoricalStreamingService.GetListener());
	bondexecutionService.AddListener(bondhistoricalExecutionService.GetListener());
	bondpositionService.AddListener(bondhistoricalPositionService.GetListener());
	bondriskService.AddListener(bondhistoricalRiskService.GetListener());
	bondinquiryService.AddListener(bondhistoricalInquiryService.GetListener());
	//7
	bondpricingService.AddListener(bondguiService.GetListener());

	

	ifstream price_outsource(PriceFilePath);
	bondpricingService.GetConnector()->Subscribe(price_outsource);

	ifstream trade_outsource(TradeFilePath);
	bondtradeBookingService.GetConnector()->Subscribe(trade_outsource);

	ifstream md_outsource(MarketDataFilePath);
	bondmarketDataService.GetConnector()->Subscribe(md_outsource);

	ifstream iq_outsource(InquiryFilePath);
	bondinquiryService.GetConnector()->Subscribe(iq_outsource);


	return 0;
}
