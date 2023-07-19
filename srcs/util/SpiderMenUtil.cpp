#include "../../includes/util/SpiderMenUtil.hpp"

vector<string>	SpiderMenUtil::splitString(string str, string del)
{
	size_t	pos;
	size_t	before = 0;
	vector<string>	rtn;

	for(size_t i = 0; i < str.size(); i++)
	{
		pos = del.find(str[i]);
		if (pos == string::npos)
			continue ;
		if (i != before)
			rtn.push_back(str.substr(before, (i - before)));
		before = i + 1;
	}
	if (str.size() != before)
		rtn.push_back(str.substr(before, (str.size() - before)));
	return rtn;
}

vector<string>	SpiderMenUtil::splitString(string str)
{
	size_t	before = 0;
	vector<string>	rtn;

	for(size_t i = 0; i < str.size(); i++)
	{
		if (!isspace(str[i]))
			continue ;
		if (i != before)
			rtn.push_back(str.substr(before, (i - before)));
		before = i + 1;
	}
	if (str.size() != before)
		rtn.push_back(str.substr(before, (str.size() - before)));
	return rtn;
}