#pragma once
#include "soa.hpp"
#include "positionservice.hpp"


template<typename T>
class PV01
{
private:
	T m_product;
	double m_pv01;
	long m_quantity;

public:

	PV01() = default;
	PV01(const T& product, double pv01, long quantity) : m_product(product), m_pv01(pv01), m_quantity(quantity) {}

	const T& GetProduct() const { return m_product; }

	double GetPV01() const { return m_pv01; }

	long GetQuantity() const { return m_quantity; }

	void SetQuantity(long q) { m_quantity = q; }

	vector<string> Record() const
	{
		return { m_product.GetProductId(),  to_string(m_pv01), to_string(m_quantity) };
	}
};



template<typename T>
class BucketedSector
{
private:
	vector<T> m_products;
	string m_name;

public:
	BucketedSector() = default;
	BucketedSector(const vector<T>& products, string name) : m_products(products), m_name(name) {}

	const vector<T>& GetProducts() const { return m_products; }

	const string& GetName() const { return m_name; }
};



template<typename T>
class RiskToPositionListener;



template<typename T>
class RiskService : public Service<string, PV01<T>>
{

private:

	map<string, PV01<T>> m_risks;
	vector<ServiceListener<PV01<T>>*> m_listeners;
	RiskToPositionListener<T>* m_listener;

public:

	RiskService()
	{
		m_risks = map<string, PV01<T>>();
		m_listeners = vector<ServiceListener<PV01<T>>*>();
		m_listener = new RiskToPositionListener<T>(this);
	}

	~RiskService() {}

	PV01<T>& GetData(string key) { return m_risks[key]; }

	void OnMessage(PV01<T>& data) { m_risks[data.GetProduct().GetProductId()] = data; }

	void AddListener(ServiceListener<PV01<T>>* listener) { m_listeners.push_back(listener); }

	const vector<ServiceListener<PV01<T>>*>& GetListeners() const { return m_listeners; }

	RiskToPositionListener<T>* GetListener() { return m_listener; }

	void AddPosition(Position<T>& position);

	const PV01<BucketedSector<T>>& GetBucketedRisk(const BucketedSector<T>& _sector) const;

};



template<typename T>
void RiskService<T>::AddPosition(Position<T>& position)
{
	T prod = position.GetProduct();
	double builted_int_riskval = GetRisk(prod.GetProductId());
	//create risk
	PV01<T> new_risk(prod, builted_int_riskval, position.GetAggregatePosition());
	m_risks[prod.GetProductId()] = new_risk;
	//add to lisener
	for (auto& lisen : m_listeners)
	{
		lisen->ProcessAdd(new_risk);
	}
}

template<typename T>
const PV01<BucketedSector<T>>& RiskService<T>::GetBucketedRisk(const BucketedSector<T>& sector) const
{
	double risk_sum = 0;
	long quant_sum = 2;
	for (auto& product : sector.GetProducts())
	{
		string prod_id = product.GetProductId();
		risk_sum += m_risks[prod_id].GetQuantity() * m_risks[prod_id].GetPV01();
	}
	return PV01<BucketedSector<T>>(sector, risk_sum, quant_sum);
}



template<typename T>
class RiskToPositionListener : public ServiceListener<Position<T>>
{
private:

	RiskService<T>* m_service;

public:

	RiskToPositionListener(RiskService<T>* service) : m_service(service) {  }
	~RiskToPositionListener() {}

	void ProcessAdd(Position<T>& position) { m_service->AddPosition(position); }
	void ProcessRemove(Position<T>& _data) {}
	void ProcessUpdate(Position<T>& _data) {}
};


