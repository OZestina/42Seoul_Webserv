#pragma once
#ifndef AREQUEST_HPP
#define AREQUEST_HPP

#include <map>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <algorithm>
#include "../config/Server.hpp"
#include "../basic.hpp"
#include "../util/Time.hpp"
#include "../util/SpiderMenUtil.hpp"
#include "../socket/Body.hpp"
// #include "./RBad.hpp"

using namespace std;

typedef struct	s_basic
{
	//must for METHOD
	string	host;
	string	connection;
	string	user_agent;
	
	//must for POST
	size_t	content_length;
	string	content_type;
	string	boundary;
	string	transfer_encoding;
	
	//하기 두개는 Body에서 확인 가능함
	string	content_disposition;
	string	filename;
	string	x_secret;

} t_basic;

class ARequest
{
	public:
		ARequest(string Root, int mType, map<string, string> header_key_val, vector<Server>* servers);
		ARequest(int mType, map<string, string> header_key_val, vector<Server>* servers);
		ARequest(int mType, Server mServer);
		virtual	~ARequest();

		virtual pid_t			operate() = 0;
		virtual	const string	createResponse() = 0;

		string&				getPipeValue();
		int					getReadPipe();
		int					getWritePipe();
		size_t				getSendLen() const;
		int					getCode() const;
		int					getType() const;
		Server				getServer();
		const string&		getRoot() const;
		const t_basic&		getBasics() const;
		Body&				getBody();
		const string&		getCgiPath() const;
		const string&		getCgiBin() const;
		void				addSendLen(size_t len);
		void				setCode(int code);
		void				setType(int type);


	protected:
		bool			mIsFile;
		int				mCode;
		string			mRoot;
		string			mQuery;
		string			mPipeValue;
		string			mMethod;
		int				mType;
		Server			mServer;
		Body			mBody;
		Location		mLocation;
		t_basic			mBasics;
		int				mReadPipe;
		int				mWritePipe;
		string			mCgiPath;
		string			mCgiBin;
		size_t			mSendLen;

		Server	 		findServer(vector<Server>* servers);
		void			findRootLocation(Server& server, string root);
		void			findExtensionLocation(Server& server);
		void			setPipe(int* fd);
		void			setCgiEnv();
		void			cutQuery();
};

#endif // AREQUEST_HPP
