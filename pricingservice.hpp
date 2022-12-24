#pragma once
#include <string>
#include "soa.hpp"


template<typename T>
class Price
{
private:

	T product;
	double mid;
	double bidOfferSpread;

public:

	// ctor for a price
	Price() = default;
	Price(const T& _product, double _mid, double _bidOfferSpread) : product(_product), mid(_mid), bidOfferSpread(_bidOfferSpread){}

	// Get the product
	const T& GetProduct() const { return product; }

	// Get the mid price
	double GetMid() const { return mid; }

	double GetBidOfferSpread() const { return bidOfferSpread; }

	vector<string> Record() const
	{
		return { product.GetProductId(), to_string(mid), to_string(bidOfferSpread) };
	}
};



template<typename T>
class PricingConnector;



template<typename T>
class PricingService : public Service<string, Price<T>>
{
private:

	map<string, Price<T>> m_prices;
	vector<ServiceListener<Price<T>>*> m_listeners;
	PricingConnector<T>* m_connector;

public:

	PricingService()
	{
		m_prices = map<string, Price<T>>();
		m_listeners = vector<ServiceListener<Price<T>>*>();
		m_connector = new PricingConnector<T>(this);
	}

	~PricingService() {}

	Price<T>& GetData(string key) { return m_prices[key]; }

	void OnMessage(Price<T>& price)
	{
		m_prices[price.GetProduct().GetProductId()] = price;
		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(price);
		}
	}

	void AddListener(ServiceListener<Price<T>>* listener) { m_listeners.push_back(listener); }

	const vector<ServiceListener<Price<T>>*>& GetListeners() const { return m_listeners; }

	PricingConnector<T>* GetConnector() { return m_connector; }
};





template<typename T>
class PricingConnector : public Connector<Price<T>>
{

private:

	PricingService<T>* m_service;

public:

	PricingConnector(PricingService<T>* _service) : m_service(_service) {}
	~PricingConnector() {}
	void Publish(Price<T>& _data) {};
	void Subscribe(ifstream& _data);
};


template<typename T>
void PricingConnector<T>::Subscribe(ifstream& file)  //e.g 91282ACV2,99-000,99-002
{
	string path, line;
	double bid, offer;
	while (!file.eof())
	{
		vector<string> token;
		file >> line;
		token = splitLines(line, ',');
		bid = BondPriceConverter(token[1]);
		offer = BondPriceConverter(token[2]);
		Price<T> price(GetBond(token[0]), (bid + offer) / 2.0, offer - bid);
		m_service->OnMessage(price);
	}
	file.close();
}

