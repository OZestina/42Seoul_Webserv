#include "../../includes/socket/Head.hpp"

// constructor and destructor

Head::Head() {}
Head::~Head() {}

// member functions

// pubilc

void	Head::clear() { mHeadBuffer.clear(); }

int	Head::addHead(InputBuffer& inputBuff)
{
	size_t	pos = inputBuff.find("\r\n\r\n");
	if (pos != string::npos)
	{
		mHeadBuffer.append(inputBuff.getCharPointer(), pos);
		if (pos + 4 != inputBuff.size() - inputBuff.getIndex())
			inputBuff.reset(pos + 4);
		else
			inputBuff.reset();
		return 1;
	}
	return 0;
}

// getters and setters

string& Head::getHeadBuffer() { return mHeadBuffer; }
