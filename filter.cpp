// filter.cpp
// written by S.Kato

#include <iostream>
#include <string>
#include <algorithm>
#include <getopt.h>
#include <boost/lexical_cast.hpp>

#include "filter.hpp"
#include "fasta.hpp"

Read getComplementRead(Read read) {
	char comp[read.length()];
	for (int i = 0; i < read.length(); i++) {
		char c = read.at(i);
		switch(c) {
			case 'a':
			case 'A':
				c = 't';
				break;
			case 'c':
			case 'C':
				c = 'g';
				break;
			case 'g':
			case 'G':
				c = 'c';
				break;
			case 't':
			case 'T':
				c = 'a';
				break;
		}
		comp[read.length()-i-1] = c;
	}
	Read comp_read(comp);
	return comp_read;
}

Filter::Filter(int mer_length, int low_level, int low_interval, int top_level) {
	setMerLength(mer_length);
	_low_level = low_level;
	_low_interval = low_interval;
	_top_level = top_level;
}

Filter::Filter(int mer_length) {
	setMerLength(mer_length);
	_low_level = 1;
	_low_interval = 5;
	_top_level = 10;
}

void Filter::setMerLength(int length) {
	_mer_length = length;
}

void Filter::addMer(Read read, int score) {
	if (read.empty()) return;
	std::transform(read.begin(), read.end(), read.begin(), ::tolower);
	_mer_map[read] = score;
	setMerLength(read.length());
}

bool Filter::check(int scores[]) {
	int low(0), average(0), count(0);
	int i(0), score;
	while ((score = scores[i]) != -1) {
		i++;
		int score(scores[i]);
		if (score == -1)
			break;
		average += score;
		if (score <= _low_level) {
			low++;
		} else {
			count++;
			if (low > _low_interval)
				low = 0;
		}
	}
	if (count == 0) {
		return false;
	}
	average /= count;
	std::cout << low << "," << average << std::endl;
	return low < _low_interval || average < _top_level;
}

mer_iterator Filter::begin() {
	return _mer_map.begin();
}

mer_iterator Filter::end() {
	return _mer_map.end();
}

void Filter::getScoreList(Read read, int scores[]) {
	int index = 0;
	if (read.length() == 0 )
		return;
	for (int i = 0; i <= read.length()-_mer_length; i++) {
		Read sub = read.substr(i, _mer_length);
		if (sub.find("n") != std::string::npos)
			continue;
		int score = _getScore(sub);
		scores[index] = score;
		index++;
	}
	scores[index] = -1;
}

int Filter::_getScore(Read read) {
	std::transform(read.begin(), read.end(), read.begin(), ::tolower);
	int score = _mer_map[read];
	if (score == 0 )
		score = _mer_map[getComplementRead(read)];
	return score;
}

int main(int argc, char** argv) {
	std::string command(argv[0]);
	std::string usage(
			"usage: " + command + " filename mer_file" +
			" [-m low_level] [-f low_frequence] [-t top_level]"
			);
	if (argc < 3) {
		std::cout << usage << std::endl;
		return 1;
	}
	std::string filename(argv[1]);
	std::string mers_file(argv[2]);

	int top_level, low_level, low_interval;
	int result;
	while ((result = getopt(argc, argv, "f:m:t:")) != -1) {
		try {
			switch(result) {
				case 'm':
					low_level = boost::lexical_cast<int>(optarg);
					break;
				case 'f':
					low_interval = boost::lexical_cast<int>(optarg);
					break;
				case 't':
					top_level = boost::lexical_cast<int>(optarg);
					break;
				case ':':
				case '?':
					std::cout << usage << std::endl;
					return 1;
			}
		} catch(boost::bad_lexical_cast) {
			std::cout << usage << std::endl;
			return 1;
		}
	}

	Fasta *mers = new Fasta(mers_file);
	Filter *filter = new Filter(0, low_level, low_interval, top_level);
	while (!mers->eof()) {
		FastaItem item = mers->getItem();
		int score = 0;
		try {
			std::string str = item.getInfo();
			score = boost::lexical_cast<int>(str);
		} catch(boost::bad_lexical_cast) {
			continue;
		}
		filter->addMer(item.getRead(), score);
	}

	std::cout << "finish map" << std::endl;

	Fasta *fasta = new Fasta(filename);
	while (! fasta->eof()) {
		Read read = fasta->getItem().getRead();
		if (read.empty())
			continue;
		int length = read.length();
		int scores[length];
		for (int j = 0; j < length; j++) {
			scores[j] = 0;
		}
		filter->getScoreList(read, scores);
		if (filter->check(scores)) {
			std::string info = fasta->getItem().getInfo();
			std::cout << ">" << info << std::endl;
			std::cout << read << std::endl;
		}
	}

	delete fasta;
	delete mers;
	delete filter;
}