#pragma once
#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;


template<typename T>
class Position
{
private:
	T m_product;
	map<string, long> m_positions;

public:

	Position() = default;
	Position(const T& product) : m_product(product) {}
	const T& GetProduct() const { return m_product; }
	long GetPosition(string& book) { return m_positions[book]; }
	map<string, long> GetPositions()  { return m_positions; }
	void AddPosition(string& book, long position)  { m_positions[book] += position; }
	long GetAggregatePosition()
	{
		long output = 0;
		for (const auto& posi : m_positions)
		{
			output += posi.second;
		}
		return output;
	}

	vector<string> Record() const
	{
		vector<string> VecPosit;
		for (const auto& position : m_positions)
		{
			VecPosit.push_back(position.first);
			VecPosit.push_back(to_string(position.second));
		}

		vector<string> output;
		output.push_back(m_product.GetProductId());
		for (auto it = VecPosit.begin(); it != VecPosit.end(); it++)
		{
			output.push_back(*it);
		}
		return output;

	}
};




template<typename T>
class BondPositionToTradeBookListener;


template<typename T>
class PositionService : public Service<string, Position<T>>
{

private:

	map<string, Position<T>> m_positions;
	vector<ServiceListener<Position<T>>*> m_listeners;
	BondPositionToTradeBookListener<T>* m_listener;

public:

	PositionService()
	{
		m_positions = map<string, Position<T>>();
		m_listeners = vector<ServiceListener<Position<T>>*>();
		m_listener = new BondPositionToTradeBookListener<T>(this);
	}

	~PositionService() {}
	Position<T>& GetData(string key) { return m_positions[key]; }
	void OnMessage(Position<T>& data) { m_positions[data.GetProduct().GetProductId()] = data;}
	void AddListener(ServiceListener<Position<T>>* listener) { m_listeners.push_back(listener); }
	const vector<ServiceListener<Position<T>>*>& GetListeners() const { return m_listeners; }
	BondPositionToTradeBookListener<T>* GetListener() { return m_listener; }

	virtual void AddTrade(const Trade<T>& trade)
	{
		Position<T> oldPosition = m_positions[trade.GetProduct().GetProductId()];
		Position<T> newPosition (trade.GetProduct());

		Side side = trade.GetSide();
		string book = trade.GetBook();
		int multiplier;
		if (side == BUY)
			multiplier = 1;
		else
			multiplier = -1;
		long quantity = trade.GetQuantity() * multiplier;
		newPosition.AddPosition(book, quantity);

		for (auto& posi : oldPosition.GetPositions())
		{
			book = posi.first;   quantity = posi.second;
			newPosition.AddPosition(book, quantity);
		}
		m_positions[trade.GetProduct().GetProductId()] = newPosition;
		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(newPosition);
		}
	}

};



template<typename T>
class BondPositionToTradeBookListener : public ServiceListener<Trade<T>>
{
private:
	PositionService<T>* m_service;

public:

	BondPositionToTradeBookListener(PositionService<T>* service) : m_service(service) {}
	~BondPositionToTradeBookListener() {}

	void ProcessAdd(Trade<T>& trade) { m_service->AddTrade(trade); }
	void ProcessRemove(Trade<T>& trade) {}
	void ProcessUpdate(Trade<T>& trade) {}
};






