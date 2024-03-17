#include "message.h"

int main()
{
	auto opt = MessageReader::ReadNext(std::cin);
	for (; opt.has_value(); opt = MessageReader::ReadNext(std::cin))
	{
		//implement me
		auto& mh = opt.value();
		std::string str;
		switch(mh.header.msg_type)
		{
			case EventType::ADD:
				str = std::any_cast<OrderAdd>(mh.msg).String();
				break;
			case EventType::UPDATE:
				str = std::any_cast<OrderUpdate>(mh.msg).String();
				break;
			case EventType::DELETE:
				str = std::any_cast<OrderDelete>(mh.msg).String();
				break;
			case EventType::TRADED:
				str = std::any_cast<OrderTraded>(mh.msg).String();
				break;
			default:
				str = "UNKNOWN MESSAGE";
		}
		std::cout << mh.header.seq_num << "/" << static_cast<char>(mh.header.msg_type) << "/" << str << std::endl;
	}

	return 0;
}
