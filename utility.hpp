#pragma once


#include <iostream>
#include <string>
#include <chrono>
#include "products.hpp"
#include <random>

using namespace std;
using namespace chrono;


//input filepath
static string PriceFilePath = "InputData/prices.txt"; 
static string TradeFilePath = "InputData/trades.txt";   
static string MarketDataFilePath = "InputData/marketdata.txt";
static string InquiryFilePath = "InputData/inquiry.txt";

   

//output filepath
static string GuiFilePath = "OutputData/gui.txt";
static string PositionFilePath = "OutputData/position.txt";
static string RiskFilePath = "OutputData/risk.txt";
static string ExecutionFilePath = "OutputData/execution.txt";
static string StreamingFilePath = "OutputData/streaming.txt";
static string AllInquiryFilePath = "OutputData/allinquiries.txt";



void splitString(string str, string delim, vector<string>& out)
{
	size_t start;
	size_t end = 0;
	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

vector<string> splitLines(const string& text, const char& deliminator)
{
	vector<string> output;
	istringstream input;
	input.str(text);
	for (string line; getline(input, line, deliminator);)
	{
		output.push_back(line);
	}
	return output;
}



Bond GetBond(string id)
{
	map<string, Bond> bondMap;
	bondMap["91282ACV2"] = Bond("91282ACV2", CUSIP, "US2Y", 0.01233, from_string("2022/12/23"));
	bondMap["91282ZXT4"] = Bond("91282ZXT4", CUSIP, "US3Y", 0.01976, from_string("2023/12/23"));
	bondMap["91282FGH8"] = Bond("91282FGH8", CUSIP, "US5Y", 0.02149, from_string("2025/12/23"));
	bondMap["91282SDF1"] = Bond("91282SDF1", CUSIP, "US7Y", 0.02294, from_string("2027/12/23"));
	bondMap["91282JKL3"] = Bond("91282JKL3", CUSIP, "US10Y", 0.02453, from_string("2030/12/23"));
	bondMap["91282XCV5"] = Bond("91282XCV5", CUSIP, "US30Y", 0.02652, from_string("2052/12/23"));
	bondMap["91282BNM7"] = Bond("91282BNM7", CUSIP, "US50Y", 0.02866, from_string("2104/12/23"));
	return bondMap[id];
}


double GetRisk(string id)
{
	map<string, double> riskMap;
	riskMap["91282ACV2"] = 0.01845533;
	riskMap["91282ZXT4"] = 0.02214152;
	riskMap["91282FGH8"] = 0.04123453;
	riskMap["91282SDF1"] = 0.06274738;
	riskMap["91282JKL3"] = 0.08266364;
	riskMap["91282XCV5"] = 0.13918773;
	riskMap["91282BNM7"] = 0.14486884;
	return riskMap[id];
}



string IDcreator(default_random_engine& generator, uniform_int_distribution<int> distribution)
{
	int length = 15;
	random_device rd;
	string output = "ID";
	int idx = 0;
	for (int i = 0; i < length; i++)
	{
		output += to_string(distribution(generator));
	}
	return output;
}


template<typename T>
ostream& operator << (ostream& os, const vector<T>& vec)
{
	os << "[";
	for (T ele : vec)
	{
		os << ele << ", ";
	}
	os << "]";

	return os;
}

double BondPriceConverter(const string& cusip_format)
{
	vector<string> priceVec;
	splitString(cusip_format, "-", priceVec);
	double first = stoi(priceVec[0]);
	double d12 = stoi(priceVec[1].substr(0, 2));
	double d3 = stoi(priceVec[1].substr(2, 3));
	return first + d12 / 32.0 + d3 / 256.0;
}



