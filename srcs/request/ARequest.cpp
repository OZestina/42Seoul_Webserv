#include "../../includes/request/ARequest.hpp"

// constructors and destructor

ARequest::ARequest(string root, int mType, map<string, string> header_key_val, vector<Server>* servers)
			: mType(mType)
{
	mBasics.host = header_key_val["Host"];
	mBasics.user_agent = header_key_val["User-Agent"];
	mBasics.content_length = atoi(header_key_val["Content-Length"].c_str());
	mBasics.content_type = header_key_val["Content-Type"];
	mBasics.content_disposition = header_key_val["Content-Disposition"];
	mBasics.transfer_encoding = header_key_val["Transfer-Encoding"];

	//method 필수요소 확인: host, user_agent
	if (mBasics.host.size() == 0 || mBasics.user_agent.size() == 0)
		throw 400;
	//method 필수요소 확인: connection
	if (header_key_val["Connection"] == "keep-alive") {
		mBasics.connection = KEEP_ALIVE;
	} else if (header_key_val["Connection"] == "close") {
		mBasics.connection = CLOSE;
	} else {
		mBasics.connection = KEEP_ALIVE;
	}
	//method 필수요소 확인: content_length 확인 (숫자인지 아닌지 여부)
	// if (mBasics.content_length == 0 && header_key_val["Content-Length"].size()) {
	// 	throw 400;
	// }

	//초기화
	mRoot = root;

	cutQuery();
	mServer = findServer(servers);
	findLocation(mServer);
	mSendLen = 0;

	//GET && dir일 경우, index page가 있으면 root에 붙여준다
	if (mType == GET && mRoot == mLocation.getKey()) {
		if (mLocation.getIndex().size()) {
			mRoot += "/" + mLocation.getIndex();
			mIsFile = true;
		}
	}

	cout << "mLocation: " << mLocation.getKey() << endl;
	cout << "mLocation root: " << mLocation.getRoot() << endl;
	cout << "cur mRoot: " << mRoot << endl;

	//요청한 file/dir을 실제 location/server block root 주소에 따라 변경
	if (mLocation.getRoot().size())//location 블럭에 루트가 있는 경우
		mRoot.replace(mRoot.find(mLocation.getKey()), mLocation.getKey().size(), mLocation.getRoot());
	else
		mRoot = mServer.getRoot() + "/" + mRoot;

	//존재 확인
	if (!mLocation.getRedirect().size() && access(mRoot.c_str(), F_OK) < 0)
	{
		cout << "404 not found? : "<< mRoot.c_str() << endl;
		throw 404;
	}

	//file vs dir
	DIR* dir = opendir(mRoot.c_str());
	if (dir) {
		mIsFile = false;
		closedir(dir);
	} else {
		mIsFile = true;
	}
}

ARequest::ARequest(int mType) : mSendLen(0), mRoot(""), mType(mType) {}
ARequest::~ARequest() { }

// member functions
void	ARequest::cutQuery()
{
	size_t pos = mRoot.rfind("?");
	if (pos == string::npos || mRoot.find("/", pos) != string::npos)
	{
		// cout << "false ? " << endl;
		return;
	}
	mQuery = mRoot.substr(pos + 1);
	mRoot = mRoot.substr(0, pos);
	cout << "mQuery : " << mQuery << "mRoot : " << mRoot << endl;
}

// public

void	ARequest::setCode(int code) { this->mCode = code; }

Server	ARequest::findServer(vector<Server>* servers)
{
	for (int server_idx = 0,server_end = servers->size(); server_idx < server_end; ++server_idx)
	{
		for (int name_idx = 0, name_end = (*servers)[server_idx].getServerName().size(); name_idx < name_end; ++name_idx)
			if (mBasics.host == (*servers)[server_idx].getServerName()[name_idx])
				return (*servers)[server_idx];
	}
	return (*servers)[0];
}

void	ARequest::findRootLocation(Server& server, string root)
{
	size_t		find_len = 0;

	for(int loc_idx = 0, end = server.getLocation().size(); loc_idx < end; ++loc_idx)
	{
		string locationKey = server.getLocation()[loc_idx].getKey();
		if (locationKey.size() > 1 && server.getLocation()[loc_idx].getKey()[server.getLocation()[loc_idx].getKey().size() - 1] == '/')
			locationKey = locationKey.substr(0, locationKey.size() - 1);

		if (find_len == 0 && server.getLocation()[loc_idx].getKey() == "/") {
			
			mLocation = server.getLocation()[loc_idx];
			find_len = server.getLocation()[loc_idx].getKey().size();
			cout << "UPDATED root" << endl;

		} else if (root.substr(0, locationKey.size()) == locationKey && \
			server.getLocation()[loc_idx].getKey().size() - 1 > find_len) {
			
			mLocation = server.getLocation()[loc_idx];
			find_len = server.getLocation()[loc_idx].getKey().size();
			cout << "UPDATED" << endl;
		}
		// if (root.substr(0, server.getLocation()[loc_idx].getKey().size()) == server.getLocation()[loc_idx].getKey() && \
		// 	server.getLocation()[loc_idx].getKey().size() > find_len) {
			
		// 	mLocation = server.getLocation()[loc_idx];
		// 	find_len = server.getLocation()[loc_idx].getKey().size();
		// 	cout << "UPDATED" << endl;
		// }
	}
	if (find_len == 0)
		findRootLocation(server, "/");

	cout << "find location: " << mLocation.getKey();
}

void	ARequest::findExtentionLocation(Server& server)
{
	string extension;

	size_t	pos = mRoot.rfind('.');
	if (pos == string::npos || mRoot.find('/', pos) != string::npos || pos == mRoot.size())
		return;
	extension = mRoot.substr(pos);
	for(size_t loc_idx = 0, end = server.getLocation().size(); loc_idx < end; ++loc_idx)
	{
		if (extension == server.getLocation()[loc_idx].getKey()) {
			mCgiPath = server.getLocation()[loc_idx].getCgiPath();
			mCgiBin = server.getLocation()[loc_idx].getCgiBin();
			return;
		}
	}
}

//원본 코드 ======
// int	ARequest::findExtentionLocation(Server& server)
// {
// 	string extension;

// 	size_t	pos = mRoot.rfind('.');
// 	if (pos == string::npos || mRoot.find('/', pos) != string::npos || pos == mRoot.size())
// 		return 1;
// 	extension = mRoot.substr(pos);
// 	for(int loc_idx = 0, end = server.getLocation().size(); loc_idx < end; ++loc_idx)
// 	{
// 		if (extension == server.getLocation()[loc_idx].getKey()) {
// 			mLocation = server.getLocation()[loc_idx];
// 			return 0;
// 		}
// 	}
// 	return 1;
// }

void	ARequest::checkPipe()
{
	char	buff[1024];
	string	readvalue;
	int		readlen = 1;

	while (readlen > 0)
	{
		readlen = read(mPipe[0], buff, 1024);
		if (readlen > 0)
			readvalue.append(buff, readlen);
	}
	close(mPipe[0]);
	readvalue = SpiderMenUtil::replaceCRLF(readvalue);
	if (readvalue.find("Content-Type:") == string::npos)
	{
		size_t pos = readvalue.find("\r\n\r\n");
		if (pos == string::npos)
			readvalue = "\r\n" + readvalue;
		readvalue = "Content-Type: text/html; charset=UTF-8\r\n" + readvalue;
	}
	// cout << readvalue << endl;
	mPipeValue = readvalue;
}

void	ARequest::findLocation(Server& server)
{
	//Find extension location으로 cgi path 업데이트 해주기 (있는 경우에만)
	findExtentionLocation(server);
	//Find Root Location으로 루트 설정
	findRootLocation(server, mRoot);
	
	//이 아래가 원본
	// if (findExtentionLocation(server))
	// 	findRootLocation(server, mRoot);
}

void	ARequest::setPipe()
{
	if (pipe(mPipe) < 0)
		throw runtime_error("Error: pipe create failed");
}


void	ARequest::setCgiEnv()
{
	setenv("SERVER_NAME", getBasics().host.c_str(), 1);
	setenv("CONTENT_TYPE", getBasics().content_type.c_str(), 1);
	setenv("CONTENT_LENGTH", SpiderMenUtil::itostr(getBasics().content_length).c_str(), 1);
	setenv("DOCUMENT_ROOT", mRoot.c_str(), 1);
	setenv("HTTP_USER_AGENT", getBasics().user_agent.c_str(), 1);
	setenv("SERVER_SOFTWARE", SPIDER_SERVER, 1); // "\r\n 빼야하나?"
	setenv("REQUEST_METHOD", mRequest.c_str(), 1);
	setenv("SERVER_PORT", to_string(mServer.getListen()).c_str(), 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("QUERY_STRING", mQuery.c_str(), 1);
	//setenv("PATH_INFO", mLocation.getCgiPath().c_str(), 1);
	setenv("PATH_INFO", getCgiPath().c_str(), 1);

	// // SERVER_NAME
	// // SERVER_SOFTWARE // CGI 프로그램 이름 및 버전 ex) 아파치 / 2.2.14
	// SERVER_PROTOCOL // ??? HTTP/1.1 ?
	// // SERVER_PORT // ???
	// // REQUEST_METHOD // GET POST ..
	// // PATH_INFO // CGI path /wevsrv/cgi_bin/example.cgi
	// DOCUMENT_ROOT // ???
	// QUERY_STRING // url?key=value&key=value ..
	// REMOTE_HOST	//client host name 없으면 정의 x
	// REMOTE_ADDR	//client ip
	// // CONTENT_TYPE
	// // CONTENT_LENGTH
	// HTTP_REFERER // ????
	// // HTTP_USER_AGENT
}

// void	ARequest::createErrorRequest(int code)
// {
// 	ARequest* toDelete = this;
// 	this = new RBad(code);
// 	delete toDelete;
// }

// getters


string&				ARequest::getPipeValue() { return mPipeValue; }
int					ARequest::getCode() const { return mCode; }
int					ARequest::getType() const { return mType; }
const string&		ARequest::getRoot() const { return mRoot; }
const t_basic&		ARequest::getBasics() const { return mBasics; }
// const t_response&	ARequest::getResponse() const { return mResponse; }
Body&				ARequest::getBody() { return mBody; }
string				ARequest::getCgiPath() const { return mCgiPath; }
string				ARequest::getCgiBin() const { return mCgiBin; }