#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sstream>
#include <queue>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "../kqueue/KQueue.hpp"
#include "../request/ARequest.hpp"
#include "../request/RBad.hpp"
#include "../request/RGet.hpp"
#include "../request/RPost.hpp"
#include "../request/RDelete.hpp"
#include "../request/RPut.hpp"
#include "./Socket.hpp"
#include "./Head.hpp"
#include "./Body.hpp"
#include "../util/Time.hpp"
#include "../util/InputBuffer.hpp"

class Client : public Socket
{
	public:
		Client(bool mType, int mFd, int mPort, vector<Server>* mServer, KQueue& mKq);
		virtual ~Client();

		void				handleClientRead(struct kevent* event);
		void				handleClientWrite(struct kevent* event);
		void				handleProcess(struct kevent* event);

		void				readSocket(struct kevent* event);
		int					addBuffer();
		void				addRequests(ARequest* request);
		ARequest*			createRequest(Head& head);
		map<string,string>	createHttpKeyVal(const vector<string>&	header_line);

		void				operateRequest(ARequest* request);

		void				writeSocket(struct kevent* event);
		int					sendResponseMSG(struct kevent* event);

		void				readPipe(struct kevent* event);

		void				clearClient();

		//getter && setter
		int					getReadStatus() const;
		queue<ARequest*>&	getRequests();
		string&				getResponseMSG();
		string&				getHeadBuffer();
		string&				getInputBuffer();
		int					getResponseCode() const;
		pid_t				getPid() const;
		int					getRequestStatus() const;

		void				setReadStatus(int mStatus);
		void				setResponseCode(int code);
		void				setPid(pid_t mPid);
		void				setRequestStatus(int mRequestStatus);

	private:
		queue<ARequest*>	mRequests;
		int					mReadStatus;
		int					mRequestStatus;
		int					mResponseCode;
		InputBuffer			mInputBuffer;
		string				mResponseMSG;
		pid_t				mPid;
		Head				mHeader;
};

#endif //CLIENT_HPP
