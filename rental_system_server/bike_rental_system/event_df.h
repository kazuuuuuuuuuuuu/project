#ifndef _EVENTS_DF_H_
#define _EVENTS_DF_H_

#include "iEvent.h"
#include <string>
#include "protobuf/bike.pb.h"

class MobileCodeReqEv : public iEvent
{
private:
	rental_system::mobile_request msg_;

public:
	MobileCodeReqEv(const std::string& mobile, void* args = nullptr) :
		iEvent(EVENT_GET_MOBILE_CODE_REQ, args)
	{
		msg_.set_mobile(mobile);
	}

	const std::string& get_mobile() const { return msg_.mobile(); }

	virtual i32 ByteSize() { return msg_.ByteSize(); } // bytes for SerializeToArray
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); }
	virtual std::ostream& dump(std::ostream& out) const;
};

class MobileCodeRspEv : public iEvent
{
private:
	rental_system::mobile_response msg_;

public:
	MobileCodeRspEv(i32 code, i32 icode, void* args = nullptr) :
		iEvent(EVENT_GET_MOBILE_CODE_RSP, args)
	{
		msg_.set_code(code);
		msg_.set_icode(icode); // verification  code
		msg_.set_data(getErrorMsg(code));
	}

	const i32 get_code() const { return msg_.code(); }
	const i32 get_icode() const { return msg_.icode(); }
	const std::string& get_data() const { return msg_.data(); }

	virtual i32 ByteSize() { return msg_.ByteSize(); }
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); }
	virtual std::ostream& dump(std::ostream& out) const;
};

class ExitRspEv : public iEvent
{
public:
	ExitRspEv(void* args = nullptr) :
		iEvent(EVENT_EXIT_RSP, args)
	{
	}

	virtual i32 ByteSize() { return 0; }
	virtual bool SerializeToArray(char* buf, int len) { return true; }
	virtual std::ostream& dump(std::ostream& out) const;
};

class LoginReqEv : public iEvent
{
private:
	rental_system::login_request msg_;

public:
	LoginReqEv(const std::string& mobile, i32 icode, void* args = nullptr) :
		iEvent(EVENT_LOGIN_REQ, args)
	{
		msg_.set_mobile(mobile);
		msg_.set_icode(icode);
	}

	const std::string& get_mobile() const { return msg_.mobile(); }
	const i32 get_icode() const { return msg_.icode(); }

	virtual i32 ByteSize() { return msg_.ByteSize(); }
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); }
	virtual std::ostream& dump(std::ostream& out) const;
};

class LoginResEv : public iEvent
{
private:
	rental_system::login_response msg_;

public:
	LoginResEv(i32 code, void* args = nullptr) :
		iEvent(EVENT_LOGIN_RSP, args)
	{
		msg_.set_code(code);
		msg_.set_desc(getErrorMsg(code));
	}

	const i32 get_code() const { return msg_.code(); }
	const std::string& get_desc() const { return msg_.desc(); }

	virtual i32 ByteSize() { return msg_.ByteSize(); }
	virtual bool SerializeToArray(char* buf, int len) { return msg_.SerializeToArray(buf, len); }
	virtual std::ostream& dump(std::ostream& out) const;
};

#endif // !_EVENTS_DF_H_


