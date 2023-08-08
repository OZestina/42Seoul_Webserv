#include "../../includes/request/RDelete.hpp"

// constructor and destructor

RDelete::RDelete(string mRoot, map<string, string> header_key_val, vector<Server>* servers)
			: ARequest(mRoot, DELETE, header_key_val, servers)
{
	mMethod = "DELETE";
	if (find(mLocation.getLimitExcept().begin(), mLocation.getLimitExcept().end(), "DELETE") == \
			mLocation.getLimitExcept().end())
		throw 405;
	
	if (mBasics.content_length || mBasics.transfer_encoding.size())
		throw 400;

	if (mIsFile == false)
		throw 400;
}

RDelete::~RDelete() { }

// member functions

// public

pid_t			RDelete::operate()
{
    if (remove(getRoot().c_str()) != 0)
		throw 403;
	return 0;
}

const string	RDelete::createResponse()
{
	string	mMSG;
	char 	timeStamp[TIME_SIZE];

	//1st line: STATUS
	mMSG.append(STATUS_200);

	//HEADER============================================
	Time::stamp(timeStamp);
	mMSG.append(timeStamp);
	mMSG.append(SPIDER_SERVER);
	mMSG.append(CONTENT_PLAIN);

	mMSG.append("Content-Length: 15\r\n");
	if (this->getBasics().connection == KEEP_ALIVE)
		mMSG.append("Connection: Keep-Alive\r\n");

	mMSG.append("\r\n");		//end of head

	//BODY============================================가
	mMSG.append("DELETE SUCCESS!\r\n");

	return mMSG;
}
