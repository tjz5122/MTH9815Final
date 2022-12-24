#pragma once


#include <string>
#include "soa.hpp"
#include "pricingservice.hpp"




class PriceStreamOrder
{
private:
	double m_price;
	long m_visibleQuantity;
	long m_hiddenQuantity;
	PricingSide m_side;

public:
	// ctor for an order
	PriceStreamOrder() = default;
	PriceStreamOrder(double price, long visibleQuantity, long hiddenQuantity, PricingSide side)
	{
		m_price = price;
		m_visibleQuantity = visibleQuantity;
		m_hiddenQuantity = hiddenQuantity;
		m_side = side;
	}

	double GetPrice() const { return m_price; }

	long GetVisibleQuantity() const { return m_visibleQuantity; }

	long GetHiddenQuantity() const { return m_hiddenQuantity; }

	PricingSide GetSide() const { return m_side; }

	vector<string> Record() const
	{
		return { to_string(m_price), to_string(m_visibleQuantity), to_string(m_hiddenQuantity), ChangeEnum(m_side)};
	}

};


template<typename T>
class PriceStream
{
private:
	T m_product;
	PriceStreamOrder m_bidOrder;
	PriceStreamOrder m_offerOrder;

public:

	PriceStream() = default;
	PriceStream(const T& product, const PriceStreamOrder& bidOrder, const PriceStreamOrder& offerOrder)
	{
		m_product = product;
		m_bidOrder = bidOrder;
		m_offerOrder = offerOrder;
	}

	const T& GetProduct() const { return m_product; }

	const PriceStreamOrder& GetBidOrder() const { return m_bidOrder; }

	const PriceStreamOrder& GetOfferOrder() const { return m_offerOrder; }

	vector<string> Record() const
	{
		vector<string> bids = m_bidOrder.Record();
		vector<string> offers = m_offerOrder.Record();

		vector<string> output;
		output.push_back(m_product.GetProductId());
		for (const auto& b : bids)
		{
			output.push_back(b);
		}
		for (const auto& o : offers)
		{
			output.push_back(o);
		}
		return output;
	}
};





template<typename T>
class AlgoStream
{
private:
	PriceStream<T>* m_priceStream;
	function<tuple<int, int>(int)> divideQuantity;

public:
	AlgoStream() = default;
	AlgoStream(const T& _product, const PriceStreamOrder& _bidOrder, const PriceStreamOrder& _offerOrder)
	{
		m_priceStream = new PriceStream<T>(_product, _bidOrder, _offerOrder);
		divideQuantity = [](int quantity) {return make_pair(quantity / 4, quantity * 3 / 4); };
	}

	PriceStream<T>* GetPriceStream() const { return m_priceStream; }

	void AlgoStreamUpdate(const Price<T>& price)
	{
		if (m_priceStream->GetProduct().GetProductId() == price.GetProduct().GetProductId())
		{
			double visibleQuantity, hiddenQuantity;
			double quantity = 100000;
			double mid = price.GetMid();
			double spread = price.GetBidOfferSpread();
			tie(visibleQuantity, hiddenQuantity) = divideQuantity(quantity);
			PriceStreamOrder bid(mid - spread * 0.5, visibleQuantity, hiddenQuantity, BID);
			PriceStreamOrder offer(mid + spread * 0.5, visibleQuantity, hiddenQuantity, OFFER);
			m_priceStream = new PriceStream<T>(price.GetProduct(), bid, offer);
		}
	}
};


template<typename T>
class BondAlgoStreamToPricingListener;




template<typename T>
class AlgoStreamingService : public Service<string, AlgoStream<T>>
{

private:

	map<string, AlgoStream<T>> m_algoStreams;
	vector<ServiceListener<AlgoStream<T>>*> m_listeners;
	ServiceListener<Price<T>>* m_listener;

public:

	AlgoStreamingService()
	{
		m_algoStreams = map<string, AlgoStream<T>>();
		m_listeners = vector<ServiceListener<AlgoStream<T>>*>();
		m_listener = new BondAlgoStreamToPricingListener<T>(this);
	}

	~AlgoStreamingService() {}
	AlgoStream<T>& GetData(string key) { return m_algoStreams[key]; }
	void OnMessage(AlgoStream<T>& data) { m_algoStreams[data.GetPriceStream()->GetProduct().GetProductId()] = data; }
	void AddListener(ServiceListener<AlgoStream<T>>* listener) { m_listeners.push_back(listener); }
	const vector<ServiceListener<AlgoStream<T>>*>& GetListeners() const { return m_listeners; }
	ServiceListener<Price<T>>* GetListener() { return m_listener; }
	void AddPriceToStream(Price<T>& _price);
};


template<typename T>
void AlgoStreamingService<T>::AddPriceToStream(Price<T>& price)
{
	PriceStreamOrder bid(100, 1000000, 1000000, BID);
	PriceStreamOrder offer(100, 1000000, 1000000, OFFER);
	AlgoStream<T> algoStream(price.GetProduct(), bid, offer);
	algoStream.AlgoStreamUpdate(price);
	m_algoStreams[price.GetProduct().GetProductId()] = algoStream;
	for (auto& lisen : m_listeners)
	{
		lisen->ProcessAdd(algoStream);
	}
}




template<typename T>
class BondAlgoStreamToPricingListener : public ServiceListener<Price<T>>
{
private:
	AlgoStreamingService<T>* m_service;
public:

	BondAlgoStreamToPricingListener(AlgoStreamingService<T>* _service) { m_service = _service; }
	~BondAlgoStreamToPricingListener() {}

	void ProcessAdd(Price<T>& _data) 
	{
		m_service->AddPriceToStream(_data);
	}
	void ProcessRemove(Price<T>& _data) {}
	void ProcessUpdate(Price<T>& _data) {}
};




