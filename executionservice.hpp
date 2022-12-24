
#pragma once
#include <string>
#include "soa.hpp"
#include "algoexecutionservice.hpp"


template<typename T>
class BondExecutionToAlgoExeListener;



template<typename T>
class ExecutionService : public Service<string, ExecutionOrder<T>>
{
private:
	map<string, ExecutionOrder<T>> m_executionOrders;
	vector<ServiceListener<ExecutionOrder<T>>*> m_listeners;
	BondExecutionToAlgoExeListener<T>* m_listener;
public:
	ExecutionService()
	{
		m_executionOrders = map<string, ExecutionOrder<T>>();
		m_listeners = vector<ServiceListener<ExecutionOrder<T>>*>();
		m_listener = new BondExecutionToAlgoExeListener<T>(this);
	}

	~ExecutionService() {}

	ExecutionOrder<T>& GetData(string key) { return m_executionOrders[key]; }
	void AddListener(ServiceListener<ExecutionOrder<T>>* listener) { m_listeners.push_back(listener); }
	const vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const { return m_listeners; }
	BondExecutionToAlgoExeListener<T>* GetListener() { return m_listener; }
	void OnMessage(ExecutionOrder<T>& executionOrder)
	{
		m_executionOrders[executionOrder.GetProduct().GetProductId()] = executionOrder;
		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(executionOrder);
		}
	}
};


template<typename T>
class BondExecutionToAlgoExeListener : public ServiceListener<AlgoExecution<T>>
{
private:
	ExecutionService<T>* m_service;

public:
	BondExecutionToAlgoExeListener(ExecutionService<T>* service) : m_service(service) {}
	~BondExecutionToAlgoExeListener() {}

	void ProcessAdd(AlgoExecution<T>& algo_exe)
	{
		ExecutionOrder<T>* exe_order = algo_exe.GetExecutionOrder();
		m_service->OnMessage(*exe_order);
	}

	void ProcessRemove(AlgoExecution<T>& algo_exe) {}
	void ProcessUpdate(AlgoExecution<T>& algo_exe) {}
};

