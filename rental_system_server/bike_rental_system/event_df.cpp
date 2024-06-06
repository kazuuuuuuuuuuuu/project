#include "event_df.h"
#include <iostream>

std::ostream& MobileCodeReqEv::dump(std::ostream& out) const
{
	out << "MobileCodeReqEv dump sn = " << get_sn() << ", ";
	out << "mobile = " << get_mobile() << std::endl;
	return out;
}

std::ostream& MobileCodeRspEv::dump(std::ostream& out) const
{
	out << "MobileCodeRspEv dump sn = " << get_sn() << ", ";
	out << "code = " << get_code() << ", ";
	out << "icode = " << get_icode() << ", ";
	out << "code describe = " << get_data() << std::endl;
	return out;
}

std::ostream& ExitRspEv::dump(std::ostream& out) const
{
	out << "ExitRspEv dump sn = " << get_sn() << std::endl;
}

std::ostream& LoginReqEv::dump(std::ostream& out) const
{
	out << "LoginReqEv dump sn = " << get_sn() << ", ";
	out << "icode = " << get_icode() << ", ";
	out << "mobile = " << get_mobile() << std::endl;
	return out;
}

std::ostream& LoginResEv::dump(std::ostream& out) const
{
	out << "LoginResEv dump sn = " << get_sn() << ", ";
	out << "code = " << get_code() << ", ";
	out << "code describe = " << get_desc() << std::endl;
	return out;
}