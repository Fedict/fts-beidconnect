#pragma once

#ifndef __REQUEST_HANDLER__
#define __REQUEST_HANDLER__

#include <iostream>
#include <memory>
#include <boost/property_tree/ptree.hpp>

class RequestHandler
{
public:
	static std::shared_ptr<RequestHandler> createRequestHandler(const std::shared_ptr <std::stringstream>& ssRequest); // Factory

	virtual ~RequestHandler() {};  //without this destructor of derived handler is not called
	virtual std::string process() = 0;

	inline void AddTraceInfoInJsonResult() { TraceInfoInJsonResult = true; }
protected:
	RequestHandler(const std::shared_ptr<boost::property_tree::ptree>& ptreeRequest) : ptreeRequest(ptreeRequest) {};	// Need to go through the factory
	const std::shared_ptr<const boost::property_tree::ptree> ptreeRequest;
	bool TraceInfoInJsonResult = false;

	void post_process(boost::property_tree::ptree& response);
};

#endif //__REQUEST_HANDLER__
