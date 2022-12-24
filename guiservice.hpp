#pragma once


#include "soa.hpp"
#include "pricingservice.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
using boost::posix_time::millisec;
using boost::posix_time::time_duration;
using boost::posix_time::microsec_clock;
using boost::posix_time::ptime;


template<typename T>
class GUIConnector;
template<typename T>
class BondGUIToPricingListener;


template<typename T>
class GUIService : Service<string, Price<T>>
{

private:
	map<string, Price<T>> m_guis;
	vector<ServiceListener<Price<T>>*> m_listeners;
	ServiceListener<Price<T>>* m_listener;
	GUIConnector<T>* m_connector;

public:

	GUIService()
	{
		m_guis = map<string, Price<T>>();
		m_listeners = vector<ServiceListener<Price<T>>*>();
		m_connector = new GUIConnector<T>(this);
		m_listener = new BondGUIToPricingListener<T>(this);
	}

	~GUIService() {}

	Price<T>& GetData(string key) { return m_guis[key]; }



	void AddListener(ServiceListener<Price<T>>* _listener){ m_listeners.push_back(_listener); }
	const vector<ServiceListener<Price<T>>*>& GetListeners() const { return m_listeners; }
	GUIConnector<T>* GetConnector() { return m_connector; }
	ServiceListener<Price<T>>* GetListener() { return m_listener; }
	void OnMessage(Price<T>& _data)
	{
		m_guis[_data.GetProduct().GetProductId()] = _data;
		m_connector->Publish(_data);
	}
};





template<typename T>
class GUIConnector : public Connector<Price<T>>
{
private:
	GUIService<T>* m_service;

public:
	GUIConnector(GUIService<T>* service) : m_service(service) {}
	~GUIConnector() {}
	void Publish(Price<T>& price);
	void Subscribe(ifstream& data) {}
};



template<typename T>
void GUIConnector<T>::Publish(Price<T>& data)
{
	ptime init_time = microsec_clock::local_time();
	ptime nextime = microsec_clock::local_time();
	while ((nextime - init_time) > millisec(300))
	{
		ptime nextime = microsec_clock::local_time();
	}

	ofstream file;
	file.open(GuiFilePath);
	vector<string> record = data.Record();
	for (auto& r : record)
	{
		file << r << ",";
	}
	file << endl;
}



template<typename T>
class BondGUIToPricingListener : public ServiceListener<Price<T>>
{
private:
	GUIService<T>* m_service;
public:

	BondGUIToPricingListener(GUIService<T>* service) : m_service(service) {}
	~BondGUIToPricingListener() {}

	void ProcessAdd(Price<T>& price) { m_service->OnMessage(price); }

	void ProcessRemove(Price<T>& price) {}

	void ProcessUpdate(Price<T>& price) {}

};


