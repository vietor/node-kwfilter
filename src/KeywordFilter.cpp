#include <node.h>
#include <v8.h>
#include <nan.h>
#include "KeywordFilterCore.h"

#ifdef _MSC_VER
#pragma warning(disable:4506)
#pragma warning(disable:4530)
#endif

using namespace v8;
using namespace node;

#define ThrowTypeErrorAndReturnUndefined(message)		\
	do {							\
		Nan::ThrowTypeError(message);			\
		return info.GetReturnValue().SetUndefined();	\
	} while(0)


class KeywordFilter : public ObjectWrap
{
public:
	static void Init(Handle<Object> exports);

private:
	KeywordFilter(Handle<Array> keywords, KFMode mode);
	~KeywordFilter();

	KeywordFilterCore* core;
	static Nan::Persistent<Function> constructor;

	static NAN_METHOD(NodeNew);
	static NAN_METHOD(NodeExists);
	static NAN_METHOD(NodeFilter);
	static NAN_METHOD(NodeRender);
	static NAN_METHOD(NodeParser);
};

Nan::Persistent<Function> KeywordFilter::constructor;

KeywordFilter::KeywordFilter(Handle<Array> keywords, KFMode mode)
{
	KFStringArray keyword_array;
	for(int i = 0; i< (int)keywords->Length(); ++i) {
		Local<String> keyword = keywords->Get(i)->ToString();
		if(keyword->Length() < 1)
			continue;

		KFString chars(keyword->Length());
		keyword->Write(&chars[0], 0, keyword->Length());
		keyword_array.push_back(chars);
	}
	this->core = new KeywordFilterCore(keyword_array, mode);
}

KeywordFilter::~KeywordFilter()
{
	delete this->core;
}

NAN_METHOD(KeywordFilter::NodeNew) {
	if(info.Length() < 1 || info.Length() > 2) {
		ThrowTypeErrorAndReturnUndefined("Wrong number of arguments");
	}
	if(!info[0]->IsArray()) {
		ThrowTypeErrorAndReturnUndefined("Wrong arguments, first string array");
	}
	if(info.Length() > 1) {
		int tmp = info[1]->ToInteger()->Value();
		if(tmp < 0 || tmp >= KFModeValueMax) {
			ThrowTypeErrorAndReturnUndefined("Wrong arguments, second must [0-1]");
		}
	}


	if (info.IsConstructCall()) {
		Local<Array> keywords = Local<Array>::Cast(info[0]);
		for(int i = 0; i< (int)keywords->Length(); ++i) {
			if(!keywords->Get(i)->IsString()) {
				ThrowTypeErrorAndReturnUndefined("Wrong arguments, first string array");
			}
		}

		KFMode mode = KFModeDeep;
		if(info.Length() > 1)
			mode = (KFMode)info[1]->ToInteger()->Value();

		KeywordFilter* obj = new KeywordFilter(keywords, mode);
		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}
	else {
		const int argc = 2;
		Local<Value> argv[argc] = { info[0], info[1] };
		Local<Function> cons = Nan::New(constructor);
		info.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}
}

NAN_METHOD(KeywordFilter::NodeExists) {
	if(info.Length() != 1) {
		ThrowTypeErrorAndReturnUndefined("Wrong number of arguments");
	}
	if(!info[0]->IsString()) {
		ThrowTypeErrorAndReturnUndefined("Wrong arguments, (string)");
	}

	Local<String> text = info[0]->ToString();
	KeywordFilter* obj = ObjectWrap::Unwrap<KeywordFilter>(info.This());

	bool retVal = false;
	if(text->Length() > 0) {
		KFString chars(text->Length());
		text->Write(&chars[0], 0, text->Length());
		retVal = obj->core->exists(chars);
	}

	info.GetReturnValue().Set(retVal);
}

NAN_METHOD(KeywordFilter::NodeFilter) {
	if(info.Length() < 1 || info.Length() > 3) {
		ThrowTypeErrorAndReturnUndefined("Wrong number of arguments");
	}
	if(!info[0]->IsString() || !info[1]->IsString() || (info.Length() > 2 && !info[2]->IsNumber())) {
		ThrowTypeErrorAndReturnUndefined("Wrong arguments, (string, char, [int])");
	}

	Local<String> cover = info[1]->ToString();
	if(cover->Length() < 1) {
		ThrowTypeErrorAndReturnUndefined("Wrong arguments, second is char");
	}
	int border = 0;
	if(info.Length() > 2) {
		border = info[2]->ToInteger()->Value();
		if(border < 3) {
			ThrowTypeErrorAndReturnUndefined("Wrong arguments, third must >= 3");
		}
	}

	Local<String> text = info[0]->ToString();
	KeywordFilter* obj = ObjectWrap::Unwrap<KeywordFilter>(info.This());

	Local<String> retVal = text;
	if(text->Length() > 0) {
		KFChar cover_char;
		KFString output,
			chars(text->Length());
		text->Write(&chars[0], 0, text->Length());
		cover->Write(&cover_char, 0, 1);
		if(obj->core->filter(output, chars, cover_char, border))
			retVal = Nan::New(&output[0], (int)output.size()).ToLocalChecked();
	}

	info.GetReturnValue().Set(retVal);
}

NAN_METHOD(KeywordFilter::NodeRender) {
	if(info.Length() != 3) {
		ThrowTypeErrorAndReturnUndefined("Wrong number of arguments");
	}
	if(!info[0]->IsString() || !info[1]->IsString() || !info[2]->IsString()) {
		ThrowTypeErrorAndReturnUndefined("Wrong arguments, (string, prefix, stuffix)");
	}

	Local<String> text = info[0]->ToString();
	Local<String> prefix = info[1]->ToString();
	Local<String> stuffix = info[2]->ToString();
	KeywordFilter* obj = ObjectWrap::Unwrap<KeywordFilter>(info.This());


	Local<String> retVal = text;
	if(text->Length() > 0) {
		KFString output,
			chars(text->Length()),
			prefix_chars(prefix->Length()),
			stuffix_chars(stuffix->Length());
		text->Write(&chars[0], 0, text->Length());
		prefix->Write(&prefix_chars[0], 0, prefix->Length());
		stuffix->Write(&stuffix_chars[0], 0, stuffix->Length());
		if(obj->core->render(output, chars, prefix_chars, stuffix_chars))
			retVal =  Nan::New(&output[0], (int)output.size()).ToLocalChecked();
	}

	info.GetReturnValue().Set(retVal);
}

NAN_METHOD(KeywordFilter::NodeParser) {
	if(info.Length() != 1) {
		ThrowTypeErrorAndReturnUndefined("Wrong number of arguments");
	}
	if(!info[0]->IsString()) {
		ThrowTypeErrorAndReturnUndefined("Wrong arguments, (string)");
	}

	Local<String> text = info[0]->ToString();
	KeywordFilter* obj = ObjectWrap::Unwrap<KeywordFilter>(info.This());


	Local<Array> retVal = Nan::New<Array>();
	if(text->Length() > 0) {
		KFPositionArray output;
		KFString chars(text->Length());
		text->Write(&chars[0], 0, text->Length());
		if(obj->core->parser(output, chars)) {
			size_t i = 0;
			for(auto it = output.begin(); it != output.end(); ++it) {
				Local<Object> obj = Nan::New<Object>();
				Nan::Set(obj, Nan::New<String>("pos").ToLocalChecked(), Nan::New<Integer>((uint32_t)it->pos));
				Nan::Set(obj, Nan::New<String>("count").ToLocalChecked(), Nan::New<Integer>((uint32_t)it->count));
				Nan::Set(retVal, i++, obj);
			}
		}
	}

	info.GetReturnValue().Set(retVal);
}

void KeywordFilter::Init(Handle<Object> exports) {
	Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(NodeNew);
	tpl->SetClassName(Nan::New("KeywordFilter").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "exists", NodeExists);
	Nan::SetPrototypeMethod(tpl, "filter", NodeFilter);
	Nan::SetPrototypeMethod(tpl, "render", NodeRender);
	Nan::SetPrototypeMethod(tpl, "parser", NodeParser);

	constructor.Reset(tpl->GetFunction());
	exports->Set(Nan::New("KeywordFilter").ToLocalChecked(), tpl->GetFunction());
}

NODE_MODULE(kwfilter, KeywordFilter::Init)
