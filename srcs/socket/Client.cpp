#include "../../includes/socket/Client.hpp"

// constructor and destructor

Client::Client(bool mType, int mFd, int mPort, vector<Server>* mServer, KQueue& mKq)
			: Socket(mType, mFd, mPort, mServer, mKq), mReadStatus(WAITING), mRequestStatus(EMPTY),mResponseCode(0), mPid(0) { }

Client::~Client()
{
	if (mPid) {
		mKq.deleteProcessPid(mPid);
		kill(mPid, 2);
		waitpid(mPid, NULL, 0);
	}
	size_t end = mRequests.size();
	if (end && mRequests.front()->getReadPipe()) {
		close(mRequests.front()->getReadPipe());
	}
	if (end && mRequests.front()->getWritePipe()) {
		close(mRequests.front()->getWritePipe());
	}
	for (size_t i = 0; i < end; ++i) {           
		delete mRequests.front();
		mRequests.pop();
	}
}

// member functions

// public

// handler

void	Client::handleClientRead(struct kevent* event)
{
	if (mRequests.size() && static_cast<int>(event->ident) == mRequests.front()->getReadPipe())
		readPipe(event); //read pipe value;
	else if (getFd() == static_cast<int>(event->ident))
		readSocket(event);
}

void	Client::handleClientWrite(struct kevent* event)
{
	if (mRequests.size() && mRequests.front()->getType() != BAD && static_cast<int>(event->ident) == mRequests.front()->getWritePipe())
		mRequests.front()->getBody().writeBody(event->ident);
	else if (getFd() == static_cast<int>(event->ident))
		writeSocket(event);
}

void	Client::handleProcess(struct kevent* event)
{
	int	exit_status = 0;

	waitpid(event->ident, &exit_status, 0);
	mPid = 0;
	if (exit_status != EXIT_SUCCESS) {
		throw 500;
	}
}

// read

void	Client::readSocket(struct kevent* event)
{
	size_t	buffer_size = 1000000;
	char	buffer[buffer_size];

	memset(buffer, 0, buffer_size);
	ssize_t s = recv(getFd(), buffer, buffer_size, 0);
	if (s < 1) {
		if (s == 0 && event->data == 0) {
			throw 0;
		} else
			throw 500;
	}
	mInputBuffer.append(buffer, s);
	while (addBuffer()) {
		if (mHeader.getHeadBuffer().size())
			addRequests(createRequest(mHeader));
	}
}

int	Client::addBuffer()
{
	if (mReadStatus <= READING_HEADER)	{
		mReadStatus = READING_HEADER;
		return mHeader.addHead(mInputBuffer);
	} else if (mReadStatus == READING_BODY && mRequests.back()->getBody().addBody(mInputBuffer)) {
		if (mRequests.back()->getBody().getMaxBodySize() < mRequests.back()->getBody().getSize()) {
			mReadStatus = ERROR;
			throw 413;
		}
		if (mRequests.back()->getType() == BAD)
			mReadStatus = ERROR;
		else
			mReadStatus = WAITING;

		if (mRequests.size() == 1 && mRequests.front()->getType() == PUT) {
			operateRequest(mRequests.front());
		}
		return 1;
	}
	return 0;
}

void	Client::readPipe(struct kevent* event)
{
	char	buff[1000000];
	string	readvalue;
	int		readlen;

	readlen = read(event->ident, buff, 1000000);
	if (readlen < 0) {
		throw 500;
	}

	if (readlen == 0) {
		close (event->ident);//read pipe close;
		mRequests.front()->getPipeValue() = SpiderMenUtil::replaceCRLF(mRequests.front()->getPipeValue());
		if (mRequests.front()->getPipeValue().find("Content-Type:") == string::npos) {
			size_t pos = mRequests.front()->getPipeValue().find("\r\n\r\n");
			if (pos == string::npos)
				mRequests.front()->getPipeValue() = "\r\n" + mRequests.front()->getPipeValue();
			mRequests.front()->getPipeValue() = CONTENT_PLAIN + mRequests.front()->getPipeValue();
		}
		mRequestStatus = SENDING;
	}
	mRequests.front()->getPipeValue().append(buff, readlen);
}

//requests func

void	Client::addRequests(ARequest* request)
{
	mRequests.push(request);
	if (request->getType() == POST || request->getType() == PUT)
		mReadStatus = READING_BODY;
	else
		mReadStatus = WAITING;
	if (mRequests.size() == 1 && request->getType() != PUT) {
		operateRequest(mRequests.front());
	}
}

ARequest*	Client::createRequest(Head& head)
{
	vector<string>		element_headline;
	map<string,string>	header_key_val;

	string header = head.getHeadBuffer();
	try {
		vector<string>		header_line;

		//TEST CODE
		// cout << "New Request ==========\n" << head.getHeadBuffer() << endl; 

		header_line = SpiderMenUtil::splitString(header, "\r\n");
		header_key_val = createHttpKeyVal(header_line);
		element_headline = SpiderMenUtil::splitString(header_line[0]);
		head.clear();
		if (count(header_line[0].begin(), header_line[0].end(), ' ') != 2 || element_headline.size() != 3) {
			throw 400;
		}
		if (element_headline[2] != "HTTP/1.1") {
			throw 505;
		}
		if (element_headline[0] == "GET")
			return new RGet(element_headline[1], header_key_val, mServer);
		else if (element_headline[0] == "POST")
			return new RPost(element_headline[1], header_key_val, mServer);
		else if (element_headline[0] == "PUT")
			return new RPut(element_headline[1], header_key_val, mServer);
		else if (element_headline[0] == "DELETE")
			return new RDelete(element_headline[1], header_key_val, mServer);
		else {
			throw 405;
		}
	} catch (int error) {
		if (element_headline[0] == "POST")
			mReadStatus = READING_BODY;
		else
			mReadStatus = ERROR;
		if (element_headline[0] == "HEAD")
			return new RBad(error, header_key_val, mServer, HEAD);
		return new RBad(error, header_key_val, mServer);
	} catch(const std::exception& e) {
		return new RBad(400, header_key_val, mServer);
	}
}

map<string,string>	Client::createHttpKeyVal(const vector<string>& header_line)
{
	map<string, string>	rtn;
	size_t				pos;

	for (size_t i = 1, end = header_line.size(); i < end; ++i)
	{
		pos = header_line[i].find(": ");
		if (pos == string::npos)
			throw 400;
		if (rtn[header_line[i].substr(0, pos)] == "")
			rtn[header_line[i].substr(0, pos)] = header_line[i].substr(pos + 2, header_line[i].size() - pos + 2);
		else
			throw 400;
	}
	return rtn;
}

//operate func

void			Client::operateRequest(ARequest* request)
{
	pid_t	pid = request->operate();

	if (pid) {
		mPid = pid;
		mKq.addProcessPid(pid, reinterpret_cast<void*>(this));
		mKq.addPipeFd(mRequests.front()->getWritePipe(), mRequests.front()->getReadPipe(), reinterpret_cast<void*>(this));
		mRequestStatus = PROCESSING;
	} else {
		mRequestStatus = SENDING;
	}
}

//write func

void			Client::writeSocket(struct kevent* event)
{
	if (mResponseMSG.size() == 0 && mRequests.size())
	{
		if (mRequests.front()->getCode() < 400)
			mResponseMSG = mRequests.front()->createResponse();
		else {
			mResponseMSG = mRequests.front()->createResponse();
			mRequests.front()->setType(BAD);
		}
	}
	if (mRequests.size() && sendResponseMSG(event))
	{
		if (mRequests.front()->getType() == BAD) {
			throw 0;
		}
		delete mRequests.front();
		mRequests.pop();
		if (!mRequests.empty()) {
			operateRequest(mRequests.front());
		} else
			mRequestStatus = EMPTY;
	}
}

int			Client::sendResponseMSG(struct kevent* event)
{
	// DONT DELETE >>>>> TEST_CODE: response msg
	// if (getRequests().front()->getSendLen() == 0) {
	// 	if (getResponseMSG().size() > 1000)
	// 		cout << getFd() << " " << event->ident << " Response+++++++++\n" << getResponseMSG().substr(0, 500) << "++++++++++++++"<< endl;
	// 	else
	// 		cout << getFd() << " " << event->ident << " Response+++++++++\n" << getResponseMSG() << "++++++++++++++" << endl;
	// }

	size_t	sendinglen = getResponseMSG().size() - getRequests().front()->getSendLen();
	if (static_cast<size_t>(event->data) <= sendinglen)
		sendinglen = event->data;
	sendinglen = send(getFd(), getResponseMSG().c_str() + getRequests().front()->getSendLen(), sendinglen, 0);

	if (sendinglen == static_cast<size_t>(-1) || sendinglen == 0) {
		throw 0;
	}

	getRequests().front()->addSendLen(sendinglen);

	if (getRequests().front()->getSendLen() == getResponseMSG().size())
	{
		mResponseMSG.clear();
		return 1;
	}
	return 0;
}

void			Client::clearClient()
{
	mReadStatus = WAITING;
	mResponseCode = 0;
	mInputBuffer.clear();
	getInputBuffer().clear();
	mResponseMSG.clear();
}

// getters and setters

int					Client::getReadStatus() const { return mReadStatus; }
queue<ARequest*>&	Client::getRequests() { return mRequests; }
string&				Client::getResponseMSG() { return mResponseMSG; }
string&				Client::getHeadBuffer() { return mInputBuffer; }
string&				Client::getInputBuffer() { return mInputBuffer; }
int					Client::getResponseCode() const { return mResponseCode; }
pid_t				Client::getPid() const { return mPid; }
int					Client::getRequestStatus() const { return mRequestStatus; }

void				Client::setReadStatus(int mStatus) { this->mReadStatus = mStatus; }
void				Client::setResponseCode(int code) { mResponseCode = code; }
void				Client::setPid(pid_t mPid) { this->mPid = mPid; }
void				Client::setRequestStatus(int mRequestStatus) {this->mRequestStatus = mRequestStatus; }
