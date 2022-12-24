#pragma once
#include "soa.hpp"
#include "algostreamingservice.hpp"


template<typename T>
class StreamingToAlgoStreamingListener;


template<typename T>
class StreamingService : public Service<string, PriceStream<T>>
{

private:
	map<string, PriceStream<T>> m_priceStreams;
	vector<ServiceListener<PriceStream<T>>*> m_listeners;
	ServiceListener<AlgoStream<T>>* m_listener;

public:

	StreamingService()
	{
		m_priceStreams = map<string, PriceStream<T>>();
		m_listeners = vector<ServiceListener<PriceStream<T>>*>();
		m_listener = new StreamingToAlgoStreamingListener<T>(this);
	}

	~StreamingService() {}
	PriceStream<T>& GetData(string key) { return m_priceStreams[key]; }
	void AddListener(ServiceListener<PriceStream<T>>* lisener) { m_listeners.push_back(lisener); }
	const vector<ServiceListener<PriceStream<T>>*>& GetListeners() const { return m_listeners; }
	ServiceListener<AlgoStream<T>>* GetListener() { return m_listener; }
	void OnMessage(PriceStream<T>& stream)
	{
		m_priceStreams[stream.GetProduct().GetProductId()] = stream;

		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(stream);
		}
	}
};



template<typename T>
class StreamingToAlgoStreamingListener : public ServiceListener<AlgoStream<T>>
{

private:

	StreamingService<T>* m_service;

public:

	StreamingToAlgoStreamingListener(StreamingService<T>* service) : m_service(service) {}
	~StreamingToAlgoStreamingListener() {}

	void ProcessAdd(AlgoStream<T>& stream)
	{
		PriceStream<T>* price_stream = stream.GetPriceStream();
		m_service->OnMessage(*price_stream);
	}

	void ProcessRemove(AlgoStream<T>& stream) {}
	void ProcessUpdate(AlgoStream<T>& stream) {}

};


