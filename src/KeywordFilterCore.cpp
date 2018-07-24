#include <cctype>
#include <algorithm>
#include "KeywordFilterCore.h"

static inline KFChar fast_towlower(KFChar ch)
{
	return ch >= 0x41 && ch <= 0x5a? ch + 0x20: ch;
}

#define towlower_exec fast_towlower

static bool keyword_equal_pred(const KFString& a, const KFString& b)
{
	int ret = a.size() - b.size();
	if(ret == 0) {
		for(size_t i = 1, length = a.size(); ret == 0 && i < length; ++i) {
			ret = (int)a[i] - (int)b[i];
		}
	}
	return ret == 0;
}

static bool keyword_sort_pred(const KFString& a, const KFString& b)
{
	int ret = 0;
	for(size_t i = 1, length = min(a.size(), b.size()); ret == 0 && i < length; ++i) {
		ret = (int)a[i] - (int)b[i];
	}
	return ret < 0? true: (ret == 0? a.size() > b.size(): false);
}

KeywordFilterCore::KeywordFilterCore(const KFStringArray& keywords, KFMode mode)
{
	filter_mode = mode;
	for(auto i = keywords.begin(); i != keywords.end(); ++i) {
		const KFString& keyword = *i;
		KFChar first_char = towlower_exec(keyword[0]);
		auto it = keyword_map.find(first_char);
		if(it == keyword_map.end()) {
			keyword_map.insert(make_pair(first_char, KFStringArray()));
			it = keyword_map.find(first_char);
		}
		it->second.push_back(keyword);
		KFString& chars = it->second.back();
		transform(chars.begin(), chars.end(), chars.begin(), towlower_exec);
	}

	for(auto it = keyword_map.begin(); it != keyword_map.end(); ++it) {
		sort(it->second.begin(), it->second.end(), keyword_sort_pred);
		it->second.erase(unique(it->second.begin(), it->second.end(), keyword_equal_pred), it->second.end());

		KFString* last_ptr = NULL;
		for(auto ic = it->second.begin(); ic != it->second.end(); ++ic) {
			KFChar skip = 1;
			KFString& curr = *ic;
			if(last_ptr) {
				KFString& last = *last_ptr;
				while(skip < min(last.size(), curr.size())) {
					if(last[skip] != curr[skip])
						break;
					++skip;
				}
			}
			curr[0] = skip;
			last_ptr = &curr;
		}
	}
}

KeywordFilterCore::~KeywordFilterCore()
{
}

static inline bool is_wordstop(KFChar ch) {
	return iswspace(ch) || iswpunct(ch) || ch > 0x7f;
}

static size_t skip_some_space(const KFString& chars, size_t& pos, size_t length) {
	size_t old = pos;
	while(pos < length && iswspace(chars[pos]))
		++pos;
	return pos - old;
}

static size_t skip_next_word(const KFString& chars, size_t& pos, size_t length) {
	size_t old = pos;
	while(pos < length && !is_wordstop(chars[pos]))
		++pos;
	skip_some_space(chars, pos, length);
	return pos - old;
}

bool KeywordFilterCore::exists(const KFString& text)
{
	if(text.size() < 1 || keyword_map.size() < 1)
		return false;

	KFString chars(text.size());
	transform(text.begin(), text.end(), chars.begin(), towlower_exec);

	bool has = false;
	size_t pos = 0, length = chars.size();
	skip_some_space(chars, pos, length);
	while(!has && pos < length) {
		KFChar first_char = chars[pos];
		auto it = keyword_map.find(first_char);
		if(it == keyword_map.end()) {
			++pos;
			if(filter_mode == KFModeWord && !is_wordstop(first_char)) {
				skip_next_word(chars, pos, length);
			}
			continue;
		}

		size_t last_skip = 1;
		for(auto ic = it->second.rbegin(); ic != it->second.rend(); ++ic) {
			const KFString& curr = *ic;
			const size_t curr_size = curr.size();

			if(last_skip > 1)
				continue;
			if(pos + curr_size > length) {
				last_skip = 1;
				continue;
			}

			bool mark = true;
			last_skip = curr[0];
			for(size_t i = 1; i < curr_size; ++i) {
				if(chars[pos + i] != curr[i]) {
					mark = false;
					break;
				}
			}
			if(mark) {
				if(filter_mode == KFModeDeep)
					has = true;
				else if(pos + curr_size >= length || is_wordstop(chars[pos + curr_size]))
					has = true;
				break;
			}
		}

		++pos;
		if(filter_mode == KFModeWord && !is_wordstop(chars[pos - 1]))
			skip_next_word(chars, pos, length);
	}
	return has;
}

bool KeywordFilterCore::process(const KFString& chars, void (*onskip)(size_t, size_t, void*), void (*onmark)(size_t, size_t, void*), void *context)
{
	bool has = false;
	size_t skip_pos = 0, skip_count = 0;
	size_t pos = 0, length = chars.size();
	while(pos < length) {
		KFChar first_char = chars[pos];
		auto it = keyword_map.find(first_char);
		if(it == keyword_map.end()) {
			if(!skip_count)
				skip_pos = pos;
			++pos;
			++skip_count;
			if(filter_mode == KFModeWord && !is_wordstop(first_char)) {
				skip_count += skip_next_word(chars, pos, length);
			}
			continue;
		}

		bool marked = false;
		size_t last_skip = 1;
		for(auto ic = it->second.begin(); ic != it->second.end(); ++ic) {
			const KFString& curr = *ic;
			const size_t curr_size = curr.size();

			size_t skip = curr[0];
			if(last_skip > 1 && skip > last_skip)
				continue;
			if(pos + curr_size > length) {
				last_skip = 1;
				continue;
			}

			bool mark = true;
			for(size_t i = min(last_skip, skip); i < curr_size; ++i) {
				last_skip = i;
				if(chars[pos + i] != curr[i]) {
					mark = false;
					break;
				}
			}
			if(mark) {
				if(filter_mode == KFModeDeep)
					marked = true;
				else if(pos + curr_size >= length || is_wordstop(chars[pos + curr_size]))
					marked = true;
				if(marked) {
					last_skip = 1;
					if(skip_count) {
						onskip(skip_pos, skip_count, context);
						skip_pos = skip_count = 0;
					}
					onmark(pos, curr_size, context);
					pos += curr_size;
				}
				break;
			}
		}


		if(marked)
			has = true;
		else {
			if(!skip_count)
				skip_pos = pos;
			++pos;
			++skip_count;
			if(filter_mode == KFModeWord && !is_wordstop(chars[pos - 1]))
				skip_count += skip_next_word(chars, pos, length);
		}
	}
	if(skip_count) {
		onskip(skip_pos, skip_count, context);
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

bool KeywordFilterCore::filter(KFString& output, const KFString& text, const KFChar cover, int border)
{
	if(text.size() < 1 || keyword_map.size() < 1)
		return false;

	KFString chars(text.size());
	transform(text.begin(), text.end(), chars.begin(), towlower_exec);

	output.resize(text.size());
	struct filter_params params = {
		output, text, cover, size_t(border < 3? 0: border)
	};
	return process(chars, filter_skip, filter_mark, &params);
}

struct render_params {
	KFString& output;
	const KFString& text;
	const KFString& prefix;
	const KFString& stuffix;
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
	if(params->stuffix.size() > 0)
		params->output.insert(params->output.end(), params->stuffix.begin(), params->stuffix.end());
}

bool KeywordFilterCore::render(KFString& output, const KFString& text, const KFString& prefix, const KFString& stuffix)
{
	if(text.size() < 1 || keyword_map.size() < 1)
		return false;

	KFString chars(text.size());
	transform(text.begin(), text.end(), chars.begin(), towlower_exec);

	output.resize(0);
	struct render_params params = {
		output, text, prefix, stuffix
	};
	return process(chars, render_skip, render_mark, &params);
}


struct parser_params {
	KFPositionArray& output;
	const KFString& text;
};

static void parser_skip(size_t pos, size_t count, void *context)
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
	if(text.size() < 1 || keyword_map.size() < 1)
		return false;

	KFString chars(text.size());
	transform(text.begin(), text.end(), chars.begin(), towlower_exec);

	struct parser_params params = {
		output, text
	};
	return process(chars, parser_skip, parser_mark, &params);
}
