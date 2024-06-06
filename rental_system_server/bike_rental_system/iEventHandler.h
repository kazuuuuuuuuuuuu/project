#ifndef _IEVENTHANDLER_H_
#define _IEVENTHANDLER_H_

#include "iEvent.h"
#include "event_type.h"

// virtual base class
class iEventHandler
{
private:
	std::string name_;

public:
	iEventHandler(const std::string& name) :
		name_(name)
	{
	}
	virtual ~iEventHandler() {}
	
	const std::string get_name() const { return name_; }

	// pure virtual function
	// const iEvent* ev -> cannot modify the content pointed to
	virtual iEvent* handle(const iEvent* ev) = 0;
};


#endif
#pragma once
