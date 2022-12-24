#pragma once
#include "soa.hpp"
#include "tradebookingservice.hpp"

// Various inqyury states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

string ChangeEnum(InquiryState input)
{
	string output;
	switch (input)
	{
	case RECEIVED:
		output = "RECEIVED"; break;
	case QUOTED:
		output = "QUOTED"; break;
	case DONE:
		output = "DONE"; break;
	case REJECTED:
		output = "REJECTED"; break;
	case CUSTOMER_REJECTED:
		output = "CUSTOMER_REJECTED"; break;
	default:
		throw invalid_argument("invalid InquiryState"); break;
	}
	return output;
}


InquiryState ChangeToInquiryState(string input)
{
	InquiryState output;
	if (input == "RECEIVED") 
		output = RECEIVED;
	else if (input == "QUOTED") 
		output = QUOTED;
	else if (input == "DONE") 
		output = DONE;
	else if (input == "REJECTED")
		output = REJECTED;
	else if (input == "CUSTOMER_REJECTED") 
		output = CUSTOMER_REJECTED;
	return output;
}


template<typename T>
class Inquiry
{
private:
	string m_inquiryId;
	T m_product;
	Side m_side;
	long m_quantity;
	double m_price;
	InquiryState m_state;

public:

	Inquiry() = default;
	Inquiry(string inquiryId, const T& product, Side side, long quantity, double price, InquiryState state) : m_product(product)
	{
		m_inquiryId = inquiryId;
		m_side = side;
		m_quantity = quantity;
		m_price = price;
		m_state = state;
	}

	const string& GetInquiryId() const { return m_inquiryId; }
	const T& GetProduct() const { return m_product; }
	Side GetSide() const { return m_side; }
	long GetQuantity() const { return m_quantity; }
	double GetPrice() const { return m_price; }
	double SetPrice(double price) { m_price = price; }
	InquiryState GetState() const { return m_state; }
	void SetState(InquiryState state) { m_state = state; }
	vector<string> Record() const
	{
		return { m_inquiryId, m_product.GetProductId(),ChangeEnum(m_side), to_string(m_quantity), to_string(m_price),ChangeEnum(m_state) };
	}
};



template<typename T>
class InquiryConnector;



template<typename T>
class InquiryService : public Service<string, Inquiry<T>>
{

private:

	map<string, Inquiry<T>> m_inquiries;
	vector<ServiceListener<Inquiry<T>>*> m_listeners;
	InquiryConnector<T>* m_connector;

public:

	// Constructor and destructor
	InquiryService()
	{
		m_inquiries = map<string, Inquiry<T>>();
		m_listeners = vector<ServiceListener<Inquiry<T>>*>();
		m_connector = new InquiryConnector<T>(this);
	}

	~InquiryService() {}
	Inquiry<T>& GetData(string key) { return m_inquiries[key]; }
	void AddListener(ServiceListener<Inquiry<T>>* listener) { m_listeners.push_back(listener); }
	const vector<ServiceListener<Inquiry<T>>*>& GetListeners() const { return m_listeners; }
	InquiryConnector<T>* GetConnector() { return m_connector; }

	void OnMessage(Inquiry<T>& data)
	{
		string inquiryid = data.GetInquiryId();
		switch (data.GetState())
		{
		case RECEIVED:
			m_inquiries[inquiryid] = data;
			m_connector->Publish(data);

			for (auto& lisen : m_listeners)
			{
				lisen->ProcessAdd(data);
			}

			break;
		case QUOTED:
			data.SetState(DONE);
			m_inquiries[inquiryid] = data;
			break;
		}
	}

	void SendQuote(const string& id, double price)
	{
		m_inquiries[id].SetPrice(price);
		for (auto& lisen : m_listeners)
		{
			lisen->ProcessAdd(m_inquiries[id]);
		}
	}

	void RejectInquiry(const string& _inquiryId)
	{
		m_inquiries[_inquiryId].SetState(REJECTED);
	}
};



template<typename T>
class InquiryConnector : public Connector<Inquiry<T>>
{

private:

	InquiryService<T>* m_service;

public:
	InquiryConnector(InquiryService<T>* service) : m_service(service) {}
	~InquiryConnector() {}

	void Publish(Inquiry<T>& inquiry)
	{
		if (inquiry.GetState() == RECEIVED)
		{
			inquiry.SetState(QUOTED);
			m_service->OnMessage(inquiry);
		}
	}

	void Subscribe(ifstream& file)
	{
		string path, line;
		double price, quantity;
		Side side;
		while (!file.eof())
		{
			vector<string> token;
			file >> line;
			token = splitLines(line, ',');
			side = (token[2] == "BUY" ? BUY : SELL);
			quantity = stoi(token[3]);
			price = BondPriceConverter(token[4]);
			InquiryState state = ChangeToInquiryState(token[5]);
			Inquiry<T> inquiry(token[0], GetBond(token[1]), side, quantity, price, state);
			m_service->OnMessage(inquiry);
		}
		file.close();
	}
};

