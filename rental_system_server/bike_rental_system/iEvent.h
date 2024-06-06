#ifndef _IEVENT_H_
#define _IEVENT_H_

#include "global_define.h"
#include "event_type.h"
#include <string>
#include <iostream>

// virual base class
class iEvent
{
private:
	u32 eid_; // event type id
	u32 sn_; // event unique id
	void* args_; // point to the arguments for handling the event

public:
	iEvent(u32 eid, void* args):
		eid_(eid), args_(args)
	{
		sn_ = generateSeqNo();
	}
	virtual ~iEvent() {}
	u32 generateSeqNo()
	{
		// static variable
		static u32 sn = 0;
		return sn++;
	}

	u32 get_eid() const { return eid_; }
	u32 get_sn() const { return sn_; }
	void* get_args() const { return args_; }

	void set_eid(u32 eid) { eid_ = eid; }
	void set_sn(u32 sn) { sn_ = sn; }
	void set_args(void* args) { args_ = args; }

	// pure virtual function
	virtual i32 ByteSize() = 0;
	virtual bool SerializeToArray(char* buf, i32 len) = 0;
	virtual std::ostream& dump(std::ostream& out) const = 0;
};

#endif
