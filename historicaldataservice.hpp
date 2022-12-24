#pragma once
#include "soa.hpp"


template<typename T>
class BondPositionHistoricalDataConnector;
template<typename T>
class BondPositionHistoricalDataListener;
template<typename T>
class BondRiskHistoricalDataConnector;
template<typename T>
class BondRiskHistoricalDataListener;
template<typename T>
class BondExecutionHistoricalDataConnector;
template<typename T>
class BondExecutionHistoricalDataListener;
template<typename T>
class BondStreamingHistoricalDataConnector;
template<typename T>
class BondStreamingHistoricalDataListener;
template<typename T>
class BondInquiryHistoricalDataConnector;
template<typename T>
class BondInquiryHistoricalDataListener;



template<typename T>
class BondHistoricalDataService : public Service<string, T>
{
private:
	map<string, T> m_historicalDatas;
	vector<ServiceListener<T>*> m_listeners;
	//HistoricalDataConnector<T>* m_connector;
	ServiceListener<T>* m_listener;
public:

	// Constructor and destructor
	BondHistoricalDataService()
	{
		m_historicalDatas = map<string, T>();
		m_listeners = vector<ServiceListener<T>*>();
		//m_connector = new HistoricalDataConnector<T>(this);
		//m_listener = new HistoricalDataListener<T>(this);
	};

	~BondHistoricalDataService() {};
	T& GetData(string key) { return m_historicalDatas[key]; };
	void OnMessage(T& data) { m_historicalDatas[data.GetProduct().GetProductId()] = data; };
	void AddListener(ServiceListener<T>* listener) { m_listeners.push_back(listener); };
	const vector<ServiceListener<T>*>& GetListeners() const { return m_listeners; };
	//HistoricalDataConnector<T>* GetConnector() { return m_connector; };
	//ServiceListener<T>* GetListener() { return m_listener; };
	//void PersistData(T& data) { m_connector->Publish(data); };
};



/////////////////////////////////////////////////////////// Position ///////////////////////////////////////////////////////////////////////////////

//-------------------- service----------------//
template<typename T>
class BondPositionHistoricalDataService : public BondHistoricalDataService<T>
{
private:
	map<string, T> m_historicalDatas;
	vector<ServiceListener<T>*> m_listeners;
	BondPositionHistoricalDataConnector<T>* m_connector;
	ServiceListener<T>* m_listener;

public:
	BondPositionHistoricalDataService()
	{
		m_historicalDatas = map<string, T>();
		m_listeners = vector<ServiceListener<T>*>();
		m_connector = new BondPositionHistoricalDataConnector<T>(this);
		m_listener = new BondPositionHistoricalDataListener<T>(this);
	};
	~BondPositionHistoricalDataService() {};
	BondPositionHistoricalDataConnector<T>* GetConnector() { return m_connector; };
	ServiceListener<T>* GetListener() { return m_listener; };
	void PersistData(T& data) { m_connector->Publish(data); };
};


//-------------------- Connector----------------//
template<typename T>
class BondPositionHistoricalDataConnector : public Connector<T>
{
private:
	BondPositionHistoricalDataService<T>* service;

public:

	BondPositionHistoricalDataConnector(BondPositionHistoricalDataService<T>* service) : service(service) {}
	~BondPositionHistoricalDataConnector() {}
	void Publish(T& data)
	{
		ofstream output;
		output.open(PositionFilePath);
		vector<string> info = data.Record();
		for (auto& i : info)
		{
			output << i << ",";
		}
		output << endl;
	}
	void Subscribe(ifstream& data) {}
};


//-------------------- Listener----------------//
template<typename T>
class BondPositionHistoricalDataListener : public ServiceListener<T>
{
private:
	BondPositionHistoricalDataService<T>* service;
public:

	BondPositionHistoricalDataListener(BondPositionHistoricalDataService<T>* _service) { service = _service; };
	~BondPositionHistoricalDataListener() {}
	void ProcessAdd(T& _data) { service->PersistData(_data); };
	void ProcessRemove(T& _data) {}
	void ProcessUpdate(T& _data) {}
};




/////////////////////////////////////////////////////////// RISK ///////////////////////////////////////////////////////////////////////////////

//-------------------- service----------------//
template<typename T>
class BondRiskHistoricalDataService : public BondHistoricalDataService<T>
{
private:
	map<string, T> m_historicalDatas;
	vector<ServiceListener<T>*> m_listeners;
	BondRiskHistoricalDataConnector<T>* m_connector;
	ServiceListener<T>* m_listener;

public:
	BondRiskHistoricalDataService()
	{
		m_historicalDatas = map<string, T>();
		m_listeners = vector<ServiceListener<T>*>();
		m_connector = new BondRiskHistoricalDataConnector<T>(this);
		m_listener = new BondRiskHistoricalDataListener<T>(this);
	};
	~BondRiskHistoricalDataService() {};
	BondRiskHistoricalDataConnector<T>* GetConnector() { return m_connector; };
	ServiceListener<T>* GetListener() { return m_listener; };
	void PersistData(T& data) { m_connector->Publish(data); };
};


//-------------------- Connector----------------//
template<typename T>
class BondRiskHistoricalDataConnector : public Connector<T>
{
private:
	BondRiskHistoricalDataService<T>* service;
public:
	BondRiskHistoricalDataConnector(BondRiskHistoricalDataService<T>* service) : service(service) {}
	~BondRiskHistoricalDataConnector() {}
	void Publish(T& data)
	{
		ofstream output;
		output.open(RiskFilePath);
		vector<string> info = data.Record();
		for (auto& i : info)
		{
			output << i << ",";
		}
		output << endl;
	}
	void Subscribe(ifstream& data) {}
};


//-------------------- Listener----------------//
template<typename T>
class BondRiskHistoricalDataListener : public ServiceListener<T>
{
private:
	BondRiskHistoricalDataService<T>* service;
public:
	BondRiskHistoricalDataListener(BondRiskHistoricalDataService<T>* _service) { service = _service; };
	~BondRiskHistoricalDataListener() {}
	void ProcessAdd(T& _data) { service->PersistData(_data); };
	void ProcessRemove(T& _data) {}
	void ProcessUpdate(T& _data) {}
};






/////////////////////////////////////////////////////////// Execution ///////////////////////////////////////////////////////////////////////////////

//-------------------- service----------------//
template<typename T>
class BondExecutionHistoricalDataService : public BondHistoricalDataService<T>
{
private:
	map<string, T> m_historicalDatas;
	vector<ServiceListener<T>*> m_listeners;
	BondExecutionHistoricalDataConnector<T>* m_connector;
	ServiceListener<T>* m_listener;

public:
	BondExecutionHistoricalDataService()
	{
		m_historicalDatas = map<string, T>();
		m_listeners = vector<ServiceListener<T>*>();
		m_connector = new BondExecutionHistoricalDataConnector<T>(this);
		m_listener = new BondExecutionHistoricalDataListener<T>(this);
	};
	~BondExecutionHistoricalDataService() {};
	BondExecutionHistoricalDataConnector<T>* GetConnector() { return m_connector; };
	ServiceListener<T>* GetListener() { return m_listener; };
	void PersistData(T& data) { m_connector->Publish(data); };
};


//-------------------- Connector----------------//
template<typename T>
class BondExecutionHistoricalDataConnector : public Connector<T>
{
private:
	BondExecutionHistoricalDataService<T>* service;
public:
	BondExecutionHistoricalDataConnector(BondExecutionHistoricalDataService<T>* service) : service(service) {}
	~BondExecutionHistoricalDataConnector() {}
	void Publish(T& data)
	{
		ofstream output;
		output.open(ExecutionFilePath);
		vector<string> info = data.Record();
		for (auto& i : info)
		{
			output << i << ",";
		}
		output << endl;
	}
	void Subscribe(ifstream& data) {}
};


//-------------------- Listener----------------//
template<typename T>
class BondExecutionHistoricalDataListener : public ServiceListener<T>
{
private:
	BondExecutionHistoricalDataService<T>* service;
public:
	BondExecutionHistoricalDataListener(BondExecutionHistoricalDataService<T>* _service) { service = _service; };
	~BondExecutionHistoricalDataListener() {}
	void ProcessAdd(T& _data) { service->PersistData(_data); };
	void ProcessRemove(T& _data) {}
	void ProcessUpdate(T& _data) {}
};





/////////////////////////////////////////////////////////// Streaming ///////////////////////////////////////////////////////////////////////////////

//-------------------- service----------------//
template<typename T>
class BondStreamingHistoricalDataService : public BondHistoricalDataService<T>
{
private:
	map<string, T> m_historicalDatas;
	vector<ServiceListener<T>*> m_listeners;
	BondStreamingHistoricalDataConnector<T>* m_connector;
	ServiceListener<T>* m_listener;

public:
	BondStreamingHistoricalDataService()
	{
		m_historicalDatas = map<string, T>();
		m_listeners = vector<ServiceListener<T>*>();
		m_connector = new BondStreamingHistoricalDataConnector<T>(this);
		m_listener = new BondStreamingHistoricalDataListener<T>(this);
	};
	~BondStreamingHistoricalDataService() {};
	BondStreamingHistoricalDataConnector<T>* GetConnector() { return m_connector; };
	ServiceListener<T>* GetListener() { return m_listener; };
	void PersistData(T& data) { m_connector->Publish(data); };
};


//-------------------- Connector----------------//
template<typename T>
class BondStreamingHistoricalDataConnector : public Connector<T>
{
private:
	BondStreamingHistoricalDataService<T>* service;
public:
	BondStreamingHistoricalDataConnector(BondStreamingHistoricalDataService<T>* service) : service(service) {}
	~BondStreamingHistoricalDataConnector() {}
	void Publish(T& data)
	{
		ofstream output;			
		output.open(StreamingFilePath);
		vector<string> info = data.Record();
		for (auto& i : info)
		{
			output << i << ",";
		}
		output << endl;
	}
	void Subscribe(ifstream& data) {}
};


//-------------------- Listener----------------//
template<typename T>
class BondStreamingHistoricalDataListener : public ServiceListener<T>
{
private:
	BondStreamingHistoricalDataService<T>* service;
public:
	BondStreamingHistoricalDataListener(BondStreamingHistoricalDataService<T>* _service) { service = _service; };
	~BondStreamingHistoricalDataListener() {}
	void ProcessAdd(T& _data) { service->PersistData(_data); };
	void ProcessRemove(T& _data) {}
	void ProcessUpdate(T& _data) {}
};



/////////////////////////////////////////////////////////// INQUIRY ///////////////////////////////////////////////////////////////////////////////

//-------------------- service----------------//
template<typename T>
class BondInquiryHistoricalDataService : public BondHistoricalDataService<T>
{
private:
	map<string, T> m_historicalDatas;
	vector<ServiceListener<T>*> m_listeners;
	BondInquiryHistoricalDataConnector<T>* m_connector;
	ServiceListener<T>* m_listener;

public:
	BondInquiryHistoricalDataService()
	{
		m_historicalDatas = map<string, T>();
		m_listeners = vector<ServiceListener<T>*>();
		m_connector = new BondInquiryHistoricalDataConnector<T>(this);
		m_listener = new BondInquiryHistoricalDataListener<T>(this);
	};
	~BondInquiryHistoricalDataService() {};
	BondInquiryHistoricalDataConnector<T>* GetConnector() { return m_connector; };
	ServiceListener<T>* GetListener() { return m_listener; };
	void PersistData(T& data) { m_connector->Publish(data); };
};


//-------------------- Connector----------------//
template<typename T>
class BondInquiryHistoricalDataConnector : public Connector<T>
{
private:
	BondInquiryHistoricalDataService<T>* service;
public:
	BondInquiryHistoricalDataConnector(BondInquiryHistoricalDataService<T>* service) : service(service) {}
	~BondInquiryHistoricalDataConnector() {}
	void Publish(T& data)
	{
		ofstream output;
		output.open(AllInquiryFilePath);
		vector<string> info = data.Record();
		for (auto& i : info)
		{
			output << i << ",";
		}
		output << endl;
	}
	void Subscribe(ifstream& data) {}
};


//-------------------- Listener----------------//
template<typename T>
class BondInquiryHistoricalDataListener : public ServiceListener<T>
{
private:
	BondInquiryHistoricalDataService<T>* service;
public:
	BondInquiryHistoricalDataListener(BondInquiryHistoricalDataService<T>* _service) { service = _service; };
	~BondInquiryHistoricalDataListener() {}
	void ProcessAdd(T& _data) { service->PersistData(_data); };
	void ProcessRemove(T& _data) {}
	void ProcessUpdate(T& _data) {}
};



