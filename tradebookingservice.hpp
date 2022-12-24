#pragma once
#include <string>
#include <vector>
#include "soa.hpp"
#include "executionservice.hpp"

// Trade sides
enum Side { BUY, SELL };


string ChangeEnum(Side input)
{
	string output;
	switch (input)
	{
	case BUY:
		output = "BUY"; break;
	case SELL:
		output = "SELL"; break;
	default:
		throw invalid_argument("invalid Side"); break;
	}
	return output;
}


template<typename T>
class Trade
{
private:

	T m_product;
	string m_tradeId;
	double m_price;
	string m_book;
	long m_quantity;
	Side m_side;

public:

	// ctor for a trade
	Trade() = default;
	Trade(const T& product, string tradeId, double price, string book, long quantity, Side side) : m_product(product)
	{
		m_tradeId = tradeId;
		m_price = price;
		m_book = book;
		m_quantity = quantity;
		m_side = side;
	}

	// Get the product
	const T& GetProduct() const { return m_product; }

	// Get the trade ID
	const string& GetTradeId() const { return m_tradeId; }

	// Get the mid price
	double GetPrice() const { return m_price; }

	// Get the book
	const string& GetBook() const { return m_book; }

	// Get the quantity
	long GetQuantity() const { return m_quantity; }

	// Get the side
	Side GetSide() const { return m_side; }
};




template<typename T>
class TradeBookingConnector;
template<typename T>
class TradeBookingToExecutionListener;




template<typename T>
class TradeBookingService : public Service<string, Trade<T>>
{

private:

	map<string, Trade<T>> m_trades;
	vector<ServiceListener<Trade<T>>*> m_listeners;
	TradeBookingConnector<T>* m_connector;
	TradeBookingToExecutionListener<T>* m_listener;

public:
	TradeBookingService()
	{
		m_trades = map<string, Trade<T>>();
		m_listeners = vector<ServiceListener<Trade<T>>*>();
		m_connector = new TradeBookingConnector<T>(this);
		m_listener = new TradeBookingToExecutionListener<T>(this);
	}
	~TradeBookingService() {}
	Trade<T>& GetData(string key) { return m_trades[key]; }
	void OnMessage(Trade<T>& trade)
	{
		m_trades[trade.GetTradeId()] = trade;
		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(trade);
		}
	}

	void AddListener(ServiceListener<Trade<T>>* listener){ m_listeners.push_back(listener); }
	const vector<ServiceListener<Trade<T>>*>& GetListeners() const { return m_listeners; }
	TradeBookingConnector<T>* GetConnector() { return m_connector; }
	TradeBookingToExecutionListener<T>* GetListener() { return m_listener; }
	void BookTrade(Trade<T>& trade)
	{
		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(trade);
		}
	}
};




template<typename T>
class TradeBookingConnector : public Connector<Trade<T>>
{

private:
	TradeBookingService<T>* m_service;

public:

	TradeBookingConnector(TradeBookingService<T>* service) : m_service(service) {}
	~TradeBookingConnector() {}
	void Publish(Trade<T>& data) {}
	void Subscribe(ifstream& file)  //e.g 91282ACV2,13579024,99-001,TRSY1,2000000,BUY
	{
		string path, line;
		double price, quantity;
		Side side;

		while (!file.eof())
		{
			vector<string> token;
			file >> line;
			token = splitLines(line, ',');
			price = BondPriceConverter(token[2]);
			quantity = stoi(token[4]);
			side = (token[5] == "BUY" ? BUY : SELL);
			Trade<T> trade(GetBond(token[0]), token[1], price, token[3], quantity, side);
			m_service->OnMessage(trade);
		}
		file.close();
	}
};





template<typename T>
class TradeBookingToExecutionListener : public ServiceListener<ExecutionOrder<T>>
{

private:

	TradeBookingService<T>* m_service;

public:

	TradeBookingToExecutionListener(TradeBookingService<T>* service) : m_service(service) {}
	~TradeBookingToExecutionListener() {}

	void ProcessAdd(ExecutionOrder<T>& file)
	{
		string tradeID = "T" + file.GetOrderId();
		long visibleQ = file.GetVisibleQuantity();
		long hiddenQ = file.GetHiddenQuantity();
		Side side = (file.GetPricingSide() == BID ? SELL : BUY);;
		Trade<T> trade(file.GetProduct(), tradeID, file.GetPrice(), "TRSY1", visibleQ + hiddenQ, side);
		m_service->OnMessage(trade);
		m_service->BookTrade(trade);
	}

	void ProcessRemove(ExecutionOrder<T>& _data) {}
	void ProcessUpdate(ExecutionOrder<T>& _data) {}
};


