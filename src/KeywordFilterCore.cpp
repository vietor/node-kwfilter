#include <stack>
#include <cwctype>
#include <algorithm>
#include "KeywordFilterCore.h"

static inline KFChar fast_towlower(KFChar ch)
{
	return ch >= 0x41 && ch <= 0x5a? ch + 0x20: ch;
}

#define towlower_exec fast_towlower

KeywordFilterCore::KeywordFilterCore(const KFStringArray& keywords, KFMode mode)
{
	filter_mode = mode;
	keyword_trie.word = false;
	for(auto keyword = keywords.begin(); keyword != keywords.end(); ++keyword) {
		TrieNode *node, *trie = &keyword_trie;
		for(auto key = keyword->begin(); key != keyword->end(); ++key) {
			KFChar k = towlower_exec(*key);
			auto child = trie->children.find(k);
			if(child != trie->children.end())
				trie = child->second;
			else {
				node = new TrieNode;
				node->word = false;
				trie->children.insert(make_pair(k, node));
				trie = node;
			}
		}
		trie->word = true;
	}
}

KeywordFilterCore::~KeywordFilterCore()
{
	TrieNode *trie;
	stack<TrieNode*> nodes;

	for(auto it = keyword_trie.children.begin(); it != keyword_trie.children.end(); ++it){
		nodes.push(it->second);
	}
	keyword_trie.children.clear();

	while(!nodes.empty()) {
		trie = nodes.top();
		if(trie->children.empty()) {
			nodes.pop();
			delete trie;
		}
		else {
			for(auto it = trie->children.begin(); it != trie->children.end(); ++it){
				nodes.push(it->second);
			}
			trie->children.clear();
		}
	}
}

static inline bool is_wordstop(KFChar ch) {
	return iswspace(ch) || iswpunct(ch) || ch > 0x7f;
}

static void skip_some_space(const KFString& chars, size_t& pos, size_t length) {
	while(pos < length && iswspace(chars[pos]))
		++pos;
}

static void skip_next_word(const KFString& chars, size_t& pos, size_t length) {
	while(pos < length && !is_wordstop(chars[pos]))
		++pos;
	skip_some_space(chars, pos, length);
}

bool KeywordFilterCore::exists(const KFString& text)
{
	if(text.size() < 1)
		return false;

	KFString chars(text.size());
	transform(text.begin(), text.end(), chars.begin(), towlower_exec);

	size_t pos = 0, length = chars.size();
	skip_some_space(chars, pos, length);

	bool has = false, skip = true;
	TrieNode* trie = &keyword_trie;
	size_t start_pos = pos;
	while(!has && pos < length) {
		KFChar k = chars[pos];
		auto child = trie->children.find(k);
		if(child != trie->children.end()) {
			trie = child->second;
			if(skip) {
				skip = false;
				start_pos = pos;
			}
			++pos;
			if(trie->word) {
				has = true;
				if(filter_mode == KFModeWord && pos < length && !is_wordstop(chars[pos])) {
					has = false;
				}
			}
		}
		else {
			trie = &keyword_trie;
			if(skip)
				++pos;
			else {
				skip = true;
				pos = start_pos + 1;
			}
			if(filter_mode == KFModeWord && !is_wordstop(k)) {
				skip_next_word(chars, pos, length);
			}
		}
	}
	return has;
}

bool KeywordFilterCore::process(const KFString& text, void (*onskip)(size_t, size_t, void*), void (*onmark)(size_t, size_t, void*), void *context)
{
	if(text.size() < 1)
		return false;

	KFString chars(text.size());
	transform(text.begin(), text.end(), chars.begin(), towlower_exec);

	size_t pos = 0, length = chars.size();
	skip_some_space(chars, pos, length);

	bool has = false, skip = true;
	TrieNode* trie = &keyword_trie;
	size_t start_pos = pos, mark_end = 0;
	while(pos < length) {
		KFChar k = chars[pos];
		auto child = trie->children.find(k);
		if(child != trie->children.end()) {
			trie = child->second;
			if(skip) {
				skip = false;
				if(pos > start_pos) {
					onskip(start_pos, pos - start_pos, context);
				}
				start_pos = pos;
			}
			++pos;
			if(trie->word) {
				has = true;
				if(filter_mode == KFModeWord && pos < length && !is_wordstop(chars[pos])) {
					has = false;
				}
				if(has) {
					mark_end = pos;
				}
			}
		}
		else {
			trie = &keyword_trie;
			if(skip)
				++pos;
			else {
				skip = true;
				if(mark_end < 1)
					pos = start_pos + 1;
				else {
					onmark(start_pos, mark_end - start_pos, context);
					start_pos = pos = mark_end;
					mark_end = 0;
				}
			}
			if(filter_mode == KFModeWord && !is_wordstop(k)) {
				skip_next_word(chars, pos, length);
			}
		}
	}
	if(mark_end > 0) {
		onmark(start_pos, mark_end - start_pos, context);
		start_pos = mark_end;
	}
	if(pos > start_pos) {
		onskip(start_pos, pos - start_pos, context);
	}
	return has;
}

struct filter_params {
	KFString& output;
	const KFString& text;
	const KFChar cover;
	size_t border;
};

static void filter_skip(size_t pos, size_t count, void *context)
{
	struct filter_params* params = (struct filter_params*)context;
	for(size_t i = 0; i < count; ++i) {
		params->output[pos + i] = params->text[pos + i];
	}
}

static void filter_mark(size_t pos, size_t count, void *context)
{
	struct filter_params* params = (struct filter_params*)context;
	if(!params->border || params->border > count) {
		for(size_t i = 0; i < count; ++i) {
			params->output[pos + i] = params->cover;
		}
	}
	else {
		size_t lasti = count - 1;
		params->output[pos] = params->text[pos];
		for(size_t i = 1; i < lasti; ++i) {
			params->output[pos + i] = params->cover;
		}
		params->output[pos + lasti] = params->text[pos + lasti];
	}
}

bool KeywordFilterCore::filter(KFString& output, const KFString& text, KFChar cover, int border)
{
	output.resize(text.size());
	struct filter_params params = {
		output, text, cover, size_t(border < 3? 0: border)
	};
	return process(text, filter_skip, filter_mark, &params);
}

struct render_params {
	KFString& output;
	const KFString& text;
	const KFString& prefix;
	const KFString& suffix;
};

static void render_skip(size_t pos, size_t count, void *context)
{
	struct render_params* params = (struct render_params*)context;
	for(size_t i = 0; i < count; ++i) {
		params->output.push_back(params->text[pos + i]);
	}
}

static void render_mark(size_t pos, size_t count, void *context)
{
	struct render_params* params = (struct render_params*)context;
	if(params->prefix.size() > 0)
		params->output.insert(params->output.end(), params->prefix.begin(), params->prefix.end());
	for(size_t j = 0; j < count; ++j)
		params->output.push_back(params->text[pos + j]);
	if(params->suffix.size() > 0)
		params->output.insert(params->output.end(), params->suffix.begin(), params->suffix.end());
}

bool KeywordFilterCore::render(KFString& output, const KFString& text, const KFString& prefix, const KFString& suffix)
{
	output.resize(0);
	struct render_params params = {
		output, text, prefix, suffix
	};
	return process(text, render_skip, render_mark, &params);
}

struct parser_params {
	KFPositionArray& output;
	const KFString& text;
};

static void parser_skip(size_t, size_t, void*)
{
}

static void parser_mark(size_t pos, size_t count, void *context)
{
	struct parser_params* params = (struct parser_params*)context;
	KFPosition position;
	position.pos = pos;
	position.count = count;
	params->output.push_back(position);
}

bool KeywordFilterCore::parser(KFPositionArray& output, const KFString& text)
{
	struct parser_params params = {
		output, text
	};
	return process(text, parser_skip, parser_mark, &params);
}
