#include <nan.h>
#include "KeywordFilterCore.h"

#define THROW_ERROR(message)				\
	Nan::ThrowTypeError(message);			\
	return info.GetReturnValue().SetUndefined()	\

#if NODE_MAJOR_VERSION < 11
#define V8STRING_WRITE(source, buf, len)	\
	source->Write(buf, 0, len)
#else
#define V8STRING_WRITE(source, buf, len)			\
	source->Write(v8::Isolate::GetCurrent(), buf, 0, len)
#endif

#define KFSTRING_CREATE(name, source)				\
	KFString name(source->Length());			\
	V8STRING_WRITE(source, &name[0], source->Length())


class KeywordFilter : public Nan::ObjectWrap
{
public:
	static NAN_MODULE_INIT(Init);

private:
	KeywordFilter(v8::Local<v8::Array> keywords, KFMode mode);
	~KeywordFilter();

	KeywordFilterCore* core;
	static Nan::Persistent<v8::Function> constructor;

	static NAN_METHOD(NodeNew);
	static NAN_METHOD(NodeExists);
	static NAN_METHOD(NodeFilter);
	static NAN_METHOD(NodeRender);
	static NAN_METHOD(NodeParser);
};

Nan::Persistent<v8::Function> KeywordFilter::constructor;

KeywordFilter::KeywordFilter(v8::Local<v8::Array> keywords, KFMode mode)
{
	KFStringArray keyword_array;
	for(int i = 0; i< (int)keywords->Length(); ++i) {
		v8::Local<v8::String> keyword = v8::Local<v8::String>::Cast(Nan::Get(keywords, i).ToLocalChecked());
		if(keyword->Length() < 1)
			continue;

		KFSTRING_CREATE(chars, keyword);
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
		THROW_ERROR("Wrong number of arguments");
	}
	if(!info[0]->IsArray()) {
		THROW_ERROR("Wrong arguments, first string array");
	}
	if(info.Length() > 1) {
		int tmp = v8::Local<v8::Integer>::Cast(info[1])->Value();
		if(tmp < 0 || tmp >= KFModeValueMax) {
			THROW_ERROR("Wrong arguments, second must [0-1]");
		}
	}


	if (info.IsConstructCall()) {
		v8::Local<v8::Array> keywords = v8::Local<v8::Array>::Cast(info[0]);
		for(int i = 0; i< (int)keywords->Length(); ++i) {
			if(!Nan::Get(keywords, i).ToLocalChecked()->IsString()) {
				THROW_ERROR("Wrong arguments, first string array");
			}
		}

		KFMode mode = KFModeDeep;
		if(info.Length() > 1)
			mode = (KFMode)v8::Local<v8::Integer>::Cast(info[1])->Value();

		KeywordFilter* obj = new KeywordFilter(keywords, mode);
		obj->Wrap(info.This());
		info.GetReturnValue().Set(info.This());
	}
	else {
		const int argc = 2;
		v8::Local<v8::Value> argv[argc] = { info[0], info[1] };
		v8::Local<v8::Function> cons = Nan::New(constructor);
		info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
	}
}

NAN_METHOD(KeywordFilter::NodeExists) {
	if(info.Length() != 1) {
		THROW_ERROR("Wrong number of arguments");
	}
	if(!info[0]->IsString()) {
		THROW_ERROR("Wrong arguments, (string)");
	}

	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[0]);
	KeywordFilter* obj = Nan::ObjectWrap::Unwrap<KeywordFilter>(info.This());

	bool retVal = false;
	if(text->Length() > 0) {
		KFSTRING_CREATE(chars, text);
		retVal = obj->core->exists(chars);
	}

	info.GetReturnValue().Set(retVal);
}

NAN_METHOD(KeywordFilter::NodeFilter) {
	if(info.Length() < 1 || info.Length() > 3) {
		THROW_ERROR("Wrong number of arguments");
	}
	if(!info[0]->IsString() || !info[1]->IsString() || (info.Length() > 2 && !info[2]->IsNumber())) {
		THROW_ERROR("Wrong arguments, (string, char, [int])");
	}

	v8::Local<v8::String> cover = v8::Local<v8::String>::Cast(info[1]);
	if(cover->Length() < 1) {
		THROW_ERROR("Wrong arguments, second is char");
	}
	int border = 0;
	if(info.Length() > 2) {
		border = v8::Local<v8::Integer>::Cast(info[2])->Value();
		if(border < 3) {
			THROW_ERROR("Wrong arguments, third must >= 3");
		}
	}

	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[0]);
	KeywordFilter* obj = Nan::ObjectWrap::Unwrap<KeywordFilter>(info.This());

	v8::Local<v8::String> retVal = text;
	if(text->Length() > 0) {
		KFString output;
		KFChar cover_char;
		KFSTRING_CREATE(chars, text);
		V8STRING_WRITE(cover, &cover_char, 1);
		if(obj->core->filter(output, chars, cover_char, border))
			retVal = Nan::New(&output[0], (int)output.size()).ToLocalChecked();
	}

	info.GetReturnValue().Set(retVal);
}

NAN_METHOD(KeywordFilter::NodeRender) {
	if(info.Length() != 3) {
		THROW_ERROR("Wrong number of arguments");
	}
	if(!info[0]->IsString() || !info[1]->IsString() || !info[2]->IsString()) {
		THROW_ERROR("Wrong arguments, (string, prefix, suffix)");
	}

	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[0]);
	v8::Local<v8::String> prefix = v8::Local<v8::String>::Cast(info[1]);
	v8::Local<v8::String> suffix = v8::Local<v8::String>::Cast(info[2]);
	KeywordFilter* obj = Nan::ObjectWrap::Unwrap<KeywordFilter>(info.This());

	v8::Local<v8::String> retVal = text;
	if(text->Length() > 0) {
		KFString output;
		KFSTRING_CREATE(chars, text);
		KFSTRING_CREATE(prefix_chars, prefix);
		KFSTRING_CREATE(suffix_chars, suffix);
		if(obj->core->render(output, chars, prefix_chars, suffix_chars))
			retVal =  Nan::New(&output[0], (int)output.size()).ToLocalChecked();
	}

	info.GetReturnValue().Set(retVal);
}

NAN_METHOD(KeywordFilter::NodeParser) {
	if(info.Length() != 1) {
		THROW_ERROR("Wrong number of arguments");
	}
	if(!info[0]->IsString()) {
		THROW_ERROR("Wrong arguments, (string)");
	}

	v8::Local<v8::String> text = v8::Local<v8::String>::Cast(info[0]);
	KeywordFilter* obj = Nan::ObjectWrap::Unwrap<KeywordFilter>(info.This());

	v8::Local<v8::Array> retVal = Nan::New<v8::Array>();
	if(text->Length() > 0) {
		KFPositionArray output;
		KFSTRING_CREATE(chars, text);
		if(obj->core->parser(output, chars)) {
			size_t i = 0;
			for(auto it = output.begin(); it != output.end(); ++it) {
				v8::Local<v8::Object> obj = Nan::New<v8::Object>();
				Nan::Set(obj, Nan::New<v8::String>("pos").ToLocalChecked(), Nan::New<v8::Integer>((uint32_t)it->pos));
				Nan::Set(obj, Nan::New<v8::String>("count").ToLocalChecked(), Nan::New<v8::Integer>((uint32_t)it->count));
				Nan::Set(retVal, i++, obj);
			}
		}
	}

	info.GetReturnValue().Set(retVal);
}

NAN_MODULE_INIT(KeywordFilter::Init) {
	v8::Local<v8::Context> context = target->CreationContext();

	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(NodeNew);
	tpl->SetClassName(Nan::New("KeywordFilter").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "exists", NodeExists);
	Nan::SetPrototypeMethod(tpl, "filter", NodeFilter);
	Nan::SetPrototypeMethod(tpl, "render", NodeRender);
	Nan::SetPrototypeMethod(tpl, "parser", NodeParser);

	constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
	target->Set(context, Nan::New("KeywordFilter").ToLocalChecked(), tpl->GetFunction(context).ToLocalChecked());
}

NODE_MODULE(kwfilter, KeywordFilter::Init)
