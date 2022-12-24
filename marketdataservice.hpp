#pragma once
#include <string>
#include <vector>
#include "soa.hpp"

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

string ChangeEnum(PricingSide input)
{
	string output;
	switch (input)
	{
	case BID:
		output = "BID"; break;
	case OFFER:
		output = "OFFER"; break;
	default:
		throw invalid_argument("invalid PricingSide"); break;
	}
	return output;
}

/**
* A market data order with price, quantity, and side.
*/
class Order
{
private:
	double m_price;
	long m_quantity;
	PricingSide m_side;

public:

	Order() = default;
	Order(double price, long quantity, PricingSide side) : m_price(price), m_quantity(quantity), m_side(side) {}
	double GetPrice() const { return m_price; }
	long GetQuantity() const { return m_quantity; }
	PricingSide GetSide() const { return m_side; }
};



class BidOffer
{
private:
	Order m_bidOrder;
	Order m_offerOrder;

public:
	// ctor for bid/offer
	BidOffer() = default;
	BidOffer(const Order& bidOrder, const Order& offerOrder) : m_bidOrder(bidOrder), m_offerOrder(offerOrder) {}

	// Get the bid order
	const Order& GetBidOrder() const { return m_bidOrder; }

	// Get the offer order
	const Order& GetOfferOrder() const {return m_offerOrder; }
};



template<typename T>
class OrderBook
{
private:
	T m_product;
	vector<Order> m_bidStack;
	vector<Order> m_offerStack;

public:
	OrderBook() = default;
	OrderBook(const T& product, const vector<Order>& bidStack, const vector<Order>& offerStack) :m_product(product), m_bidStack(bidStack), m_offerStack(offerStack) {}
	const T& GetProduct() const {return m_product; }
	const vector<Order>& GetBidStack() const { return m_bidStack; }
	const vector<Order>& GetOfferStack() const { return m_offerStack; }
	const BidOffer& GetInsideMarket() const
	{
		Order bestBid, bestOffer;
		double bestBidPrice = DBL_MIN;
		for (auto& curr_bid : m_bidStack)
		{
			double curr_price = curr_bid.GetPrice();
			if (curr_price > bestBidPrice)
			{
				bestBid = curr_bid;
				bestBidPrice = curr_price;
			}
		}
		double bestOfferPrice = DBL_MAX;
		for (auto& curr_offer : m_offerStack)
		{
			double curr_price = curr_offer.GetPrice();
			if (curr_price < bestOfferPrice)
			{
				bestOffer = curr_offer;
				bestOfferPrice = curr_price;
			}
		}
		return BidOffer(bestBid, bestOffer);
	}
};






template<typename T>
class MarketDataConnector;



template<typename T>
class MarketDataService : public Service<string, OrderBook<T>>
{

private:

	map<string, OrderBook<T>> m_orderBooks;
	vector<ServiceListener<OrderBook<T>>*> m_listeners;
	MarketDataConnector<T>* m_connector;

public:

	MarketDataService()
	{	
		m_orderBooks = map<string, OrderBook<T>>();
		m_listeners = vector<ServiceListener<OrderBook<T>>*>();
		m_connector = new MarketDataConnector<T>(this);
	}

	~MarketDataService() {}

	OrderBook<T>& GetData(string key)
	{
		return m_orderBooks[key];
	}

	void OnMessage(OrderBook<T>& data)
	{
		m_orderBooks[data.GetProduct().GetProductId()] = data;

		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(data);
		}
	}

	void AddListener(ServiceListener<OrderBook<T>>* listener)
	{
		m_listeners.push_back(listener);
	}

	const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const
	{
		return m_listeners;
	}

	MarketDataConnector<T>* GetConnector()
	{
		return m_connector;
	}

	const BidOffer& GetBestBidOffer(const string& id)
	{
		return m_orderBooks[id].GetInsideMarket();
	}

	const OrderBook<T>& AggregateDepth(const string& id)
	{
		vector<Order>& oldBids = m_orderBooks[id].GetBidStack();
		vector<Order>& oldOffers = m_orderBooks[id].GetOfferStack();
		vector<Order>& NewBids, NewOffers;
		double price;
		long quantity;

		unordered_map<double, long> aggregatedBids;
		unordered_map<double, long> aggregatedOffers;

		for (auto& b : oldBids)
		{
			price = b.GetPrice();
			quantity = b.GetQuantity();
			auto it = aggregatedBids.find(price);
			if (it == aggregatedBids.end())
				aggregatedBids[price] += quantity;
			else
				aggregatedBids.insert(make_pair(price, quantity));
		}

		for (auto& o : oldOffers)
		{
			price = o.GetPrice();
			quantity = o.GetQuantity();
			auto it = aggregatedOffers.find(price);
			if (it == aggregatedOffers.end())
				aggregatedOffers[price] += quantity;
			else
				aggregatedOffers.insert(make_pair(price, quantity));
		}

		for (auto& b : aggregatedBids)
		{
			Order new_b = Order(b.first, b.second, BID);
			NewBids.push_back(new_b);
		}

		for (auto& o : aggregatedOffers)
		{
			Order new_o = Order(o.first, o.second, OFFER);
			NewOffers.push_back(new_o);
		}

		return OrderBook<T>(m_orderBooks[id].GetProduct(), NewBids, NewOffers);
	}

};




template<typename T>
class MarketDataConnector : public Connector<OrderBook<T>>
{

private:

	MarketDataService<T>* m_service;

public:

	MarketDataConnector(MarketDataService<T>* service) : m_service(service) {}  
	~MarketDataConnector() {}
	void Publish(OrderBook<T>& data) {}
	void Subscribe(ifstream& file)   //e.g 91282ACV2,99-000,10000000,BID
	{
		string path, line;
		double price, quantity;
		vector<Order> bidOrderVec, offerOrderVec;
		PricingSide side;
		int count = 0;

		while (!file.eof())
		{
			vector<string> token;
			file >> line;
			token = splitLines(line, ',');
			price = BondPriceConverter(token[1]);
			quantity = stoi(token[2]);
			side = (token[3] == "BID" ? BID : OFFER);
			Order order(price, quantity, side);

			if (side == BID)
				bidOrderVec.push_back(order);
			else
				offerOrderVec.push_back(order);
			count += 1;

			//every twice of market deepth, create orderbook
			if (count % 10 == 0)
			{
				OrderBook<T> order_book(GetBond(token[0]), bidOrderVec, offerOrderVec);
				m_service->OnMessage(order_book);
				bidOrderVec.clear();
				offerOrderVec.clear();
				count %= 10;
			}
		}
		file.close();
	}
};
