#pragma once

#include <string>
#include "soa.hpp"
#include "marketdataservice.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

string ChangeEnum(OrderType input)
{
	string output;
	switch (input)
	{
	case FOK:
		output = "FOK"; break;
	case IOC:
		output = "IOC"; break;
	case MARKET:
		output = "MARKET"; break;
	case LIMIT:
		output = "LIMIT"; break;
	case STOP:
		output = "STOP"; break;
	default:
		throw invalid_argument("invalid OrderType"); break;
	}
	return output;
}

enum Market { BROKERTEC, ESPEED, CME };


string ChangeEnum(Market input)
{
	string output;
	switch (input)
	{
	case BROKERTEC:
		output = "BROKERTEC"; break;
	case ESPEED:
		output = "ESPEED"; break;
	case CME:
		output = "CME"; break;
	default:
		throw invalid_argument("invalid Market"); break;
	}
	return output;
}



template<typename T>
class ExecutionOrder
{
private:
	T m_product;
	PricingSide m_side;
	string m_orderId;
	OrderType m_orderType;
	double m_price;
	long m_visibleQuantity;
	long m_hiddenQuantity;
	string m_parentOrderId;
	bool m_isChildOrder;


public:

	ExecutionOrder() = default;
	ExecutionOrder(const T& product, PricingSide side, string orderId, OrderType orderType, double price, long visibleQuantity, long hiddenQuantity, string parentOrderId, bool isChildOrder)
	{
		m_product = product;
		m_side = side;
		m_orderId = orderId;
		m_orderType = orderType;
		m_price = price;
		m_visibleQuantity = visibleQuantity;
		m_hiddenQuantity = hiddenQuantity;
		m_parentOrderId = parentOrderId;
		m_isChildOrder = isChildOrder;
	}

	const T& GetProduct() const { return m_product; }

	PricingSide GetPricingSide() const { return m_side; }

	const string& GetOrderId() const { return m_orderId; }

	OrderType GetOrderType() const { return m_orderType; }

	double GetPrice() const { return m_price; }

	long GetVisibleQuantity() const { return m_visibleQuantity; }

	long GetHiddenQuantity() const { return m_hiddenQuantity; }

	const string& GetParentOrderId() const { return m_parentOrderId; }

	bool IsChildOrder() const { return m_isChildOrder; }

	vector<string> Record() const
	{
		string isChildOrder = (m_isChildOrder ? "true" : "false");
		return { m_product.GetProductId() , ChangeEnum(m_side), m_orderId, ChangeEnum(m_orderType), to_string(m_price), to_string(m_visibleQuantity), to_string(m_hiddenQuantity), m_parentOrderId, isChildOrder };
	}
};





template<typename T>
class AlgoExecution
{
private:
	ExecutionOrder<T>* m_executionOrder;
	function<tuple<int, int>(int)> divideQuantity;

public:
	AlgoExecution() = default;
	AlgoExecution(const T& product, PricingSide side, string orderId, OrderType orderType, double price, long visibleQuantity, long hiddenQuantity, string parentOrderId, bool isChildOrder)
	{
		m_executionOrder = new ExecutionOrder<T>(product, side, orderId, orderType, price, visibleQuantity, hiddenQuantity, parentOrderId, isChildOrder);
		divideQuantity = [](int quantity) {return make_pair(quantity / 4, quantity * 3 / 4);};
	}

	ExecutionOrder<T>* GetExecutionOrder() const { return m_executionOrder; }

	void AlgoExecutionUpdate(const OrderBook<T>& orderBook)
	{
		if (m_executionOrder->GetProduct().GetProductId() == orderBook.GetProduct().GetProductId())
		{
			BidOffer insideMarket = orderBook.GetInsideMarket();
			double bidPrice = insideMarket.GetBidOrder().GetPrice();
			double bidQuant = insideMarket.GetBidOrder().GetQuantity();
			double offerPrice = insideMarket.GetOfferOrder().GetPrice();
			double offerQuant = insideMarket.GetOfferOrder().GetQuantity();
			double price, quantity, visibleQuantity, hiddenQuantity;
			PricingSide side;
			default_random_engine generator;
			discrete_distribution<int> distribution{ 1, 1, 2, 2};
			int rvs = distribution(generator);
			if (rvs == 1)
			{
				side = BID;
				price = bidPrice;
				tie(visibleQuantity, hiddenQuantity) = divideQuantity(bidQuant);
			}
			else
			{
				side = OFFER;
				price = offerPrice;
				tie(visibleQuantity, hiddenQuantity) = divideQuantity(offerQuant);
			}
			cout << 1 << endl;
			string orderId = "O" + m_executionOrder->GetProduct().GetProductId();
			string parentOrderId = "P" + m_executionOrder->GetProduct().GetProductId();
			OrderType ordertype = MARKET;
			m_executionOrder = new ExecutionOrder<T>(orderBook.GetProduct(), side, orderId, ordertype, price, visibleQuantity, hiddenQuantity, parentOrderId, false);
		}
	}
	
};





template<typename T>
class BondAlgoExeToMarketDataListener;



template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{

private:

	map<string, AlgoExecution<T>> m_algoExecutions;
	vector<ServiceListener<AlgoExecution<T>>*> m_listeners;
	BondAlgoExeToMarketDataListener<T>* m_listener;
	long count;

public:

	AlgoExecutionService()
	{
		m_algoExecutions = map<string, AlgoExecution<T>>();
		m_listeners = vector<ServiceListener<AlgoExecution<T>>*>();
		m_listener = new BondAlgoExeToMarketDataListener<T>(this);
		count = 0;
	}

	~AlgoExecutionService() {};

	AlgoExecution<T>& GetData(string key) { return m_algoExecutions[key]; }

	void OnMessage(AlgoExecution<T>& algoexe) { m_algoExecutions[algoexe.GetExecutionOrder()->GetProduct().GetProductId()] = algoexe; }

	void AddListener(ServiceListener<AlgoExecution<T>>* listener) { m_listeners.push_back(listener); }

	const vector<ServiceListener<AlgoExecution<T>>*>& GetListeners() const { return m_listeners; }

	BondAlgoExeToMarketDataListener<T>* GetListener() { return m_listener; }

	void ExecuteToOrderBook(OrderBook<T>& orderBook);
};




template<typename T>
void AlgoExecutionService<T>::ExecuteToOrderBook(OrderBook<T>& orderBook)
{
	BidOffer insideMarket = orderBook.GetInsideMarket();
	if (insideMarket.GetOfferOrder().GetPrice() - insideMarket.GetBidOrder().GetPrice() <= 1.0 / 128) //threshold hold for spread
	{
		AlgoExecution<T> algoExe(orderBook.GetProduct(), BID, "OrderID", LIMIT, 100, 1000000, 0, "ParentOrderTicker", false);
		algoExe.AlgoExecutionUpdate(orderBook);
		m_algoExecutions[orderBook.GetProduct().GetProductId()] = algoExe;

		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(algoExe);
		}
	}
}




template<typename T>
class BondAlgoExeToMarketDataListener : public ServiceListener<OrderBook<T>>
{

private:

	AlgoExecutionService<T>* m_service;

public:
	BondAlgoExeToMarketDataListener(AlgoExecutionService<T>* service) : m_service(service) {}
	~BondAlgoExeToMarketDataListener() {}

	void ProcessAdd(OrderBook<T>& data) { m_service->ExecuteToOrderBook(data); }

	void ProcessRemove(OrderBook<T>& data) {}

	void ProcessUpdate(OrderBook<T>& data) {}
};



