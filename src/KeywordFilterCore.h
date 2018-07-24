#ifndef KEYWORDFILTERCORE_H
#define KEYWORDFILTERCORE_H

#include <vector>
#ifndef __APPLE__
#include <hash_map>
#define better_map hash_map
#else
#include <unordered_map>
#define better_map unordered_map
#endif

using namespace std;

typedef enum {
	KFModeDeep = 0,
	KFModeWord,
	KFModeValueMax
} KFMode;

typedef struct {
	size_t pos;
	size_t count;
} KFPosition;

typedef uint16_t KFChar;
typedef vector<KFChar> KFString;
typedef vector<KFString> KFStringArray;
typedef vector<KFPosition> KFPositionArray;

class KeywordFilterCore
{
public:
	KeywordFilterCore(const KFStringArray& keywords, KFMode mode);
	virtual ~KeywordFilterCore();

	bool exists(const KFString& text);
	bool filter(KFString& output, const KFString& text, const KFChar cover, int border);
	bool render(KFString& output, const KFString& text, const KFString& prefix, const KFString& stuffix);
	bool parser(KFPositionArray& output, const KFString& text);

private:
	KFMode filter_mode;
	better_map<KFChar, KFStringArray> keyword_map;

	bool process(const KFString& chars, void (*onskip)(size_t, size_t, void*), void (*onmark)(size_t, size_t, void*), void *context);
};

#endif
