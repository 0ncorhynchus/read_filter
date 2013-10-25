// filter.hpp
// written by S.Kato

#ifndef __FILTER_hpp
#define __FILTER_hpp

#include <string>
#include <boost/unordered_map.hpp>
#include "fasta.hpp"

typedef boost::unordered_map<Read, int> mer_map;

class Filter {
private:
	mer_map _mer_map;
	int _mer_length;
	int _low_level, _top_level;
	int _low_interval;
	bool _debug;
	int _getScore(Read read) const;

public:
	Filter(int low_level, int low_interval, int top_level);
	bool addMer(const Read read, int score);
	bool check(Read read) const;
	void setDebug(bool);
};

#endif
