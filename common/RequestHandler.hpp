#ifndef __REQUEST_HANDLER__
#define __REQUEST_HANDLER__

#include <iostream>
#include <memory>
#include <boost/property_tree/ptree.hpp>

class RequestHandler
{
public:
   static std::shared_ptr<RequestHandler> createRequestHandler(std::shared_ptr <std::stringstream> ssRequest); // Factory

   virtual ~RequestHandler() {};  //without this destructor of derived handler is not called
   virtual std::string process() = 0;

   inline void AddTraceInfoInJsonResult() { TraceInfoInJsonResult = true; }
protected:
   RequestHandler() {};	// Need to go through the factory
   std::shared_ptr <std::stringstream> ssRequest;
   bool TraceInfoInJsonResult = false;

   void post_process(boost::property_tree::ptree &response);
};

#endif //__REQUEST_HANDLER__
